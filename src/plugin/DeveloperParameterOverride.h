#pragma once

#include "../app/ParameterSnapshot.h"
#include "DeveloperExperimentState.h"

#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <limits>
#include <mutex>
#include <optional>

namespace frazil::plugin {

// Debug/ASAN-only latest-state transport for temporary developer comparisons. The control plane is
// non-realtime and serialized: Editor updates use a conditional publication token, while Host
// restore clear invalidates stale edits. The audio data plane is lock-free and bounded: it reads a
// coherent publication and may use the reader-local last coherent snapshot within the same
// override session when all bounded retries contend. The session epoch prevents that cache from
// crossing a clear/new-activation boundary. The APVTS remains untouched, so applying or resetting
// a comparison cannot serialize the temporary state or emit Host parameter/gesture notifications.
class DeveloperParameterOverride final {
  public:
    using ControlToken = std::uint64_t;

    static constexpr auto kInactivePublication = std::numeric_limits<std::uint64_t>::max();

    struct AudioReadState final {
        // This state belongs to one audio reader (the owning processor instance), not to the
        // shared control transport or to non-realtime readers.
        DeveloperHostParameterSnapshot lastCoherent{};
        std::uint64_t sessionEpoch{};
        bool hasLastCoherent{};
    };

    enum class AudioReadResult : std::uint8_t { fresh, cached, inactive, unavailable };

    DeveloperParameterOverride() noexcept {
        for (auto& slot : slots_)
            for (auto& value : slot.values)
                value.store(0.0f, std::memory_order_relaxed);
    }

    void set(const DeveloperHostParameterSnapshot& snapshot) noexcept {
        const std::lock_guard controlLock(controlMutex_);
        publishLocked(snapshot);
    }

    bool trySetIfCurrent(const DeveloperHostParameterSnapshot& snapshot,
                         ControlToken expectedToken) noexcept {
        const std::lock_guard controlLock(controlMutex_);
        if (activePublication_.load(std::memory_order_seq_cst) != expectedToken)
            return false;

        publishLocked(snapshot);
        return true;
    }

    std::optional<ControlToken> getActiveControlToken() const noexcept {
        const auto activePublication = activePublication_.load(std::memory_order_seq_cst);
        if (activePublication == kInactivePublication)
            return std::nullopt;

        return activePublication;
    }

    void clear() noexcept {
        const std::lock_guard controlLock(controlMutex_);
        // Publish inactive before advancing the session. A reader that observes the new epoch
        // must also observe the inactive marker, while a reader in the old session fails its
        // post-read epoch validation.
        activePublication_.store(kInactivePublication, std::memory_order_seq_cst);
        sessionEpoch_.fetch_add(1, std::memory_order_seq_cst);
    }

    bool isActive() const noexcept {
        return activePublication_.load(std::memory_order_seq_cst) != kInactivePublication;
    }

    AudioReadResult read(DeveloperHostParameterSnapshot& destination,
                         AudioReadState& readerState) const noexcept {
        for (int attempt = 0; attempt < 4; ++attempt) {
            const auto sessionBefore = sessionEpoch_.load(std::memory_order_seq_cst);
            const auto activePublication = activePublication_.load(std::memory_order_seq_cst);
            if (activePublication == kInactivePublication) {
                readerState.hasLastCoherent = false;
                return AudioReadResult::inactive;
            }

            const auto active = static_cast<int>(activePublication & 1u);
            const auto sequenceBefore = slots_[active].sequence.load(std::memory_order_seq_cst);
            if ((sequenceBefore & 1u) != 0u)
                continue;

            for (std::size_t index = 0; index < kDeveloperHostParameterCount; ++index)
                destination.rawValues[index] =
                    slots_[active].values[index].load(std::memory_order_seq_cst);

            const auto sequenceAfter = slots_[active].sequence.load(std::memory_order_seq_cst);
            const auto sessionAfter = sessionEpoch_.load(std::memory_order_seq_cst);
            const auto publicationAfter = activePublication_.load(std::memory_order_seq_cst);
            if (sequenceBefore == sequenceAfter && (sequenceAfter & 1u) == 0u &&
                sessionBefore == sessionAfter && activePublication == publicationAfter) {
                readerState.lastCoherent = destination;
                readerState.sessionEpoch = sessionBefore;
                readerState.hasLastCoherent = true;
                return AudioReadResult::fresh;
            }
        }

        const auto activePublication = activePublication_.load(std::memory_order_seq_cst);
        const auto sessionNow = sessionEpoch_.load(std::memory_order_seq_cst);
        if (activePublication == kInactivePublication) {
            readerState.hasLastCoherent = false;
            return AudioReadResult::inactive;
        }

        if (readerState.hasLastCoherent && readerState.sessionEpoch == sessionNow) {
            destination = readerState.lastCoherent;
            return AudioReadResult::cached;
        }

        return AudioReadResult::unavailable;
    }

    bool read(DeveloperHostParameterSnapshot& destination) const noexcept {
        AudioReadState readerState;
        const auto result = read(destination, readerState);
        return result == AudioReadResult::fresh || result == AudioReadResult::cached;
    }

    void applyTo(ParameterSnapshot& snapshot, AudioReadState& readerState) const noexcept {
        DeveloperHostParameterSnapshot values;
        const auto result = read(values, readerState);
        if (result != AudioReadResult::fresh && result != AudioReadResult::cached)
            return;

        applyValues(snapshot, values);
    }

    void applyTo(ParameterSnapshot& snapshot) const noexcept {
        AudioReadState readerState;
        applyTo(snapshot, readerState);
    }

  private:
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free,
                  "Developer override publication token must be lock-free");
    static_assert(std::atomic<std::uint32_t>::is_always_lock_free,
                  "Developer override sequence must be lock-free");
    static_assert(std::atomic<float>::is_always_lock_free,
                  "Developer override values must be lock-free");

    void publishLocked(const DeveloperHostParameterSnapshot& snapshot) noexcept {
        const auto activePublication = activePublication_.load(std::memory_order_seq_cst);
        const auto active = activePublication == kInactivePublication
                                ? 1
                                : static_cast<int>(activePublication & 1u);
        const auto targetSlot = active == 0 ? 1 : 0;
        auto& slot = slots_[targetSlot];
        // A single total order for the marker and active index prevents accepting an ABA reuse.
        slot.sequence.fetch_add(1, std::memory_order_seq_cst);
        for (std::size_t index = 0; index < kDeveloperHostParameterCount; ++index)
            slot.values[index].store(snapshot.rawValues[index], std::memory_order_seq_cst);
        slot.sequence.fetch_add(1, std::memory_order_seq_cst);
        activePublication_.store((nextPublication_++ << 1u) |
                                     static_cast<std::uint64_t>(targetSlot),
                                 std::memory_order_seq_cst);
    }

    static void applyValues(ParameterSnapshot& snapshot,
                            const DeveloperHostParameterSnapshot& values) noexcept {
        snapshot.waterEnabled = value(values, DeveloperHostParameter::waterEnabled) >= 0.5f;
        snapshot.iceEnabled = value(values, DeveloperHostParameter::iceEnabled) >= 0.5f;

        const auto routing = value(values, DeveloperHostParameter::routingMode);
        if (std::isfinite(routing) && routing >= 0.0f && routing <= 2.0f)
            snapshot.routingModeIndex = static_cast<int>(routing);

        snapshot.parallelBalance = value(values, DeveloperHostParameter::parallelBalance);
        snapshot.waterAmount = value(values, DeveloperHostParameter::waterAmount);
        snapshot.iceAmount = value(values, DeveloperHostParameter::iceAmount);
        snapshot.inputGainDb = value(values, DeveloperHostParameter::inputGainDb);
        snapshot.globalMix = value(values, DeveloperHostParameter::globalMix);
        snapshot.outputGainDb = value(values, DeveloperHostParameter::outputGainDb);
    }

    static float value(const DeveloperHostParameterSnapshot& snapshot,
                       DeveloperHostParameter parameter) noexcept {
        return snapshot.rawValues[static_cast<std::size_t>(parameter)];
    }

    struct Slot final {
        std::array<std::atomic<float>, kDeveloperHostParameterCount> values{};
        std::atomic<std::uint32_t> sequence{};
    };

    std::array<Slot, 2> slots_{};
    mutable std::mutex controlMutex_;
    std::atomic<std::uint64_t> activePublication_{kInactivePublication};
    std::atomic<std::uint64_t> sessionEpoch_{};
    std::uint64_t nextPublication_{};
};

} // namespace frazil::plugin
