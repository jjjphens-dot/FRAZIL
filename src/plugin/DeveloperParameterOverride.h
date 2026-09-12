#pragma once

#include "../app/ParameterSnapshot.h"
#include "DeveloperExperimentState.h"

#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>

namespace frazil::plugin {

// Debug/ASAN-only latest-state transport for temporary developer comparisons. The APVTS remains
// untouched, so applying or resetting a comparison cannot serialize the temporary state or emit
// Host parameter/gesture notifications.
class DeveloperParameterOverride final {
  public:
    DeveloperParameterOverride() noexcept {
        for (auto& slot : slots_)
            for (auto& value : slot.values)
                value.store(0.0f, std::memory_order_relaxed);
    }

    void set(const DeveloperHostParameterSnapshot& snapshot) noexcept {
        const auto active = activeSlot_.load(std::memory_order_relaxed);
        const auto targetSlot = active == 0 ? 1 : 0;
        auto& slot = slots_[targetSlot];
        slot.sequence.fetch_add(1, std::memory_order_release);
        for (std::size_t index = 0; index < kDeveloperHostParameterCount; ++index)
            slot.values[index].store(snapshot.rawValues[index], std::memory_order_relaxed);
        slot.sequence.fetch_add(1, std::memory_order_release);
        activeSlot_.store(targetSlot, std::memory_order_release);
    }

    void clear() noexcept {
        activeSlot_.store(-1, std::memory_order_release);
    }

    bool isActive() const noexcept {
        return activeSlot_.load(std::memory_order_acquire) >= 0;
    }

    bool read(DeveloperHostParameterSnapshot& destination) const noexcept {
        const auto active = activeSlot_.load(std::memory_order_acquire);
        if (active < 0 || active > 1)
            return false;

        for (int attempt = 0; attempt < 2; ++attempt) {
            const auto sequenceBefore = slots_[active].sequence.load(std::memory_order_acquire);
            if ((sequenceBefore & 1u) != 0u)
                continue;

            for (std::size_t index = 0; index < kDeveloperHostParameterCount; ++index)
                destination.rawValues[index] =
                    slots_[active].values[index].load(std::memory_order_relaxed);

            const auto sequenceAfter = slots_[active].sequence.load(std::memory_order_acquire);
            if (sequenceBefore == sequenceAfter && (sequenceAfter & 1u) == 0u &&
                activeSlot_.load(std::memory_order_acquire) == active)
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
    static float value(const DeveloperHostParameterSnapshot& snapshot,
                       DeveloperHostParameter parameter) noexcept {
        return snapshot.rawValues[static_cast<std::size_t>(parameter)];
    }

    struct Slot final {
        std::array<std::atomic<float>, kDeveloperHostParameterCount> values{};
        std::atomic<std::uint32_t> sequence{};
    };

    std::array<Slot, 2> slots_{};
    std::atomic<int> activeSlot_{-1};
};

} // namespace frazil::plugin
