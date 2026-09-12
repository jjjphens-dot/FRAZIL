#pragma once

#include "../app/ParameterSnapshot.h"
#include "DeveloperExperimentState.h"

#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <limits>
#include <mutex>

namespace frazil::plugin {

// Debug/ASAN-only latest-state transport for temporary developer comparisons. The non-realtime
// set/clear control operations are serialized because Host state restore may arrive on a different
// thread from the editor. The audio read/apply path remains lock-free. The APVTS remains untouched,
// so applying or resetting a comparison cannot serialize the temporary state or emit Host
// parameter/gesture notifications.
class DeveloperParameterOverride final {
  public:
    static constexpr auto kInactivePublication = std::numeric_limits<std::uint64_t>::max();

    DeveloperParameterOverride() noexcept {
        for (auto& slot : slots_)
            for (auto& value : slot.values)
                value.store(0.0f, std::memory_order_relaxed);
    }

    void set(const DeveloperHostParameterSnapshot& snapshot) noexcept {
        const std::lock_guard controlLock(controlMutex_);
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

    void clear() noexcept {
        const std::lock_guard controlLock(controlMutex_);
        activePublication_.store(kInactivePublication, std::memory_order_seq_cst);
    }

    bool isActive() const noexcept {
        return activePublication_.load(std::memory_order_seq_cst) != kInactivePublication;
    }

    bool read(DeveloperHostParameterSnapshot& destination) const noexcept {
        for (int attempt = 0; attempt < 4; ++attempt) {
            const auto activePublication = activePublication_.load(std::memory_order_seq_cst);
            if (activePublication == kInactivePublication)
                return false;
            const auto active = static_cast<int>(activePublication & 1u);
            const auto sequenceBefore = slots_[active].sequence.load(std::memory_order_seq_cst);
            if ((sequenceBefore & 1u) != 0u)
                continue;

            for (std::size_t index = 0; index < kDeveloperHostParameterCount; ++index)
                destination.rawValues[index] =
                    slots_[active].values[index].load(std::memory_order_seq_cst);

            const auto sequenceAfter = slots_[active].sequence.load(std::memory_order_seq_cst);
            if (sequenceBefore == sequenceAfter && (sequenceAfter & 1u) == 0u &&
                activePublication_.load(std::memory_order_seq_cst) == activePublication)
                return true;
        }

        return false;
    }

    void applyTo(ParameterSnapshot& snapshot) const noexcept {
        DeveloperHostParameterSnapshot values;
        if (!read(values))
            return;

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

  private:
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free,
                  "Developer override publication token must be lock-free");
    static_assert(std::atomic<std::uint32_t>::is_always_lock_free,
                  "Developer override sequence must be lock-free");
    static_assert(std::atomic<float>::is_always_lock_free,
                  "Developer override values must be lock-free");
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
    std::uint64_t nextPublication_{};
};

} // namespace frazil::plugin
