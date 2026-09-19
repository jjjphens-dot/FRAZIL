#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <string_view>

namespace frazil::water::preview {
enum class WaterModel { fluid, resonant };
enum class MacroId { size, motion, decay };
struct WaterExperimentState final {
    WaterModel model{WaterModel::fluid};
    double size{.5}, motion{.5}, decay{.5};
    bool operator==(const WaterExperimentState&) const = default;
};
struct FluidResearchTargets final {
    double bubbleMinimumHz{}, bubbleMaximumHz{}, dropletMinimumHz{}, dropletMaximumHz{};
    double bubbleRateHz{}, dropletThreshold{}, dropletRefractorySeconds{};
    double flowIntervalSeconds{}, flowDepthSeconds{};
    double bubbleDecaySeconds{}, dropletDecaySeconds{}, dropletEventsEnabled{};
};
struct ResonantResearchTargets final {
    double rootHz{}, decaySeconds{}, motionDepth{}, motionIntervalSeconds{};
};
struct ResearchWaterTargets final {
    FluidResearchTargets fluid;
    ResonantResearchTargets resonant;
};

// Pure, allocation-free research candidate. No JUCE, Host state, device or DSP object dependency.
// The prescribed curves are listening hypotheses, not a frozen product/perceptual contract.
struct ResearchWaterMacroMapper final {
    static constexpr std::string_view revision{"research-water-mapping-v0.2"};
    // Separate candidate, not adopted by v0.2 mapping. Exporter/engineering comparisons opt in.
    static std::optional<double> continuousDropletActivity(double motion) noexcept {
        if (!std::isfinite(motion))
            return std::nullopt;
        const double m = std::clamp(motion, 0.0, 1.0);
        return std::min(1.0, 4 * m * m);
    }
    static std::optional<ResearchWaterTargets> map(const WaterExperimentState& state) noexcept {
        if (!std::isfinite(state.size) || !std::isfinite(state.motion) ||
            !std::isfinite(state.decay))
            return std::nullopt;
        const double s = std::clamp(state.size, 0.0, 1.0);
        const double m = std::clamp(state.motion, 0.0, 1.0);
        const double d = std::clamp(state.decay, 0.0, 1.0);
        const double frequency = std::exp2(1 - 2 * s);
        const double activity = std::pow(4.0, 2 * m - 1);
        return ResearchWaterTargets{
            {250 * frequency, 2800 * frequency, 600 * frequency, 4500 * frequency, 480 * m * m,
             .015 / activity, .020 / activity, .250 / activity, .0005 + .001 * m,
             .070 * std::pow(3.5, 2 * d - 1), .012 * std::pow(3.0, 2 * d - 1), m > 0 ? 1.0 : 0.0},
            {260 * frequency, .120 * std::pow(4.0, 2 * d - 1), .35 * m,
             .7 * std::pow(2.8, 1 - 2 * m)}};
    }
};
} // namespace frazil::water::preview
