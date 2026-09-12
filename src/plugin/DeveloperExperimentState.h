#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace frazil::plugin {

enum class DeveloperHostParameter : std::uint8_t {
    waterEnabled,
    iceEnabled,
    routingMode,
    parallelBalance,
    waterAmount,
    iceAmount,
    inputGainDb,
    globalMix,
    outputGainDb,
};

constexpr std::size_t kDeveloperHostParameterCount = 9;

struct DeveloperHostParameterSnapshot final {
    // Values use the same denormalised units as ParameterSnapshot and the APVTS raw values.
    // This is developer workflow state, not a second Host parameter registry.
    std::array<float, kDeveloperHostParameterCount> rawValues{};
};

enum class DeveloperWaterModel : std::uint8_t { fluid, resonant };

struct DeveloperWaterExperimentSnapshot final {
    DeveloperWaterModel model{DeveloperWaterModel::fluid};
    float size{0.5f};
    float motion{0.5f};
};

enum class DeveloperComparisonMode : std::uint8_t { processed, dry };

struct DeveloperExperimentSnapshot final {
    DeveloperHostParameterSnapshot host{};
    DeveloperWaterExperimentSnapshot water{};
    DeveloperComparisonMode comparisonMode{DeveloperComparisonMode::processed};
};

class DeveloperExperimentSlots final {
  public:
    static constexpr std::size_t kSlotCount = 2;

    void capture(std::size_t slotIndex, const DeveloperExperimentSnapshot& snapshot) noexcept {
        if (slotIndex >= kSlotCount)
            return;

        snapshots_[slotIndex] = snapshot;
        captured_[slotIndex] = true;
    }

    bool apply(std::size_t slotIndex, DeveloperExperimentSnapshot& destination) const noexcept {
        if (slotIndex >= kSlotCount || !captured_[slotIndex])
            return false;

        destination = snapshots_[slotIndex];
        return true;
    }

    bool isCaptured(std::size_t slotIndex) const noexcept {
        return slotIndex < kSlotCount && captured_[slotIndex];
    }

  private:
    std::array<DeveloperExperimentSnapshot, kSlotCount> snapshots_{};
    std::array<bool, kSlotCount> captured_{};
};

} // namespace frazil::plugin
