#pragma once
#include <array>
#include <cmath>
#include <cstddef>
#include <string_view>

namespace frazil::water::research {
enum class B1Parameter : std::size_t {
    radius,
    persistence,
    admission,
    delay,
    onset,
    hysteresis,
    spacing,
    floor,
    rise,
    gain,
    tail,
    release,
    capacity,
    amplitudePolicy,
    emission,
    count
};
struct B1ParameterSpec final {
    std::string_view name, unit, classification;
    double minimum, maximum, initial;
    std::array<double, 5> choices{};
    std::size_t choiceCount{};
    bool accepts(double value) const noexcept {
        if (!std::isfinite(value) || value < minimum || value > maximum)
            return false;
        if (choiceCount == 0)
            return true;
        for (std::size_t i = 0; i < choiceCount; ++i)
            if (value == choices[i])
                return true;
        return false;
    }
};
// Single numeric authority for defaults, validation, parser and runtime descriptor.
inline constexpr std::array<B1ParameterSpec, static_cast<std::size_t>(B1Parameter::count)>
    kB1Parameters{
        {{"equivalentBubbleRadiusMm", "mm", "PHYSICAL", .2, 7, 2},
         {"persistenceScale", "ratio", "PRODUCT_MAPPING", .25, 4, 1},
         {"entrainmentProbability", "probability", "REDUCED_PHYSICAL_MODEL", 0, 1, 1},
         {"pinchOffDelayMs", "ms", "REDUCED_PHYSICAL_MODEL", 8, 40, 24},
         {"onsetRatioDb", "dB", "ENGINEERING", 3, 12, 6},
         {"rearmHysteresisDb", "dB", "ENGINEERING", 1, 6, 3},
         {"minOnsetSpacingMs", "ms", "ENGINEERING", 8, 80, 20},
         {"sourceFloorDbFS", "dBFS", "ENGINEERING", -80, -40, -60},
         {"riseXi", "ratio", "REDUCED_PHYSICAL_MODEL", 0, .1, 0, {0, .05, .1}, 3},
         {"residualGain", "linear", "ENGINEERING", 0, 1, .15},
         {"tailFloorDb", "relative envelope dB", "ENGINEERING", -100, -60, -80},
         {"stealReleaseMs", "ms", "ENGINEERING", .5, 4, 1.5},
         {"voiceCapacity", "voices", "ENGINEERING", 16, 256, 32, {16, 32, 64, 128, 256}, 5},
         {"amplitudePolicy", "0=raw,1=normalized", "ENGINEERING", 0, 1, 0, {0, 1}, 2},
         {"emission",
          "0=volume-acceleration,1=displacement-ablation",
          "ENGINEERING",
          0,
          1,
          0,
          {0, 1},
          2}}};
struct DropletB1Config final {
    std::array<double, kB1Parameters.size()> values = [] {
        std::array<double, kB1Parameters.size()> result{};
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] = kB1Parameters[i].initial;
        return result;
    }();
    double& operator[](B1Parameter id) noexcept {
        return values[static_cast<std::size_t>(id)];
    }
    double operator[](B1Parameter id) const noexcept {
        return values[static_cast<std::size_t>(id)];
    }
    bool valid() const noexcept {
        for (std::size_t i = 0; i < values.size(); ++i)
            if (!kB1Parameters[i].accepts(values[i]))
                return false;
        return true;
    }
};
} // namespace frazil::water::research
