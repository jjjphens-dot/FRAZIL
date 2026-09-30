#pragma once
#include "DropletB1Config.h"

#include <algorithm>
namespace frazil::water::research {
enum class B2Parameter : std::size_t {
    radiusSpread,
    detectorMode,
    slope,
    gamma,
    detune,
    beat,
    count
};
inline constexpr std::array<B1ParameterSpec, 6> kB2ExtraParameters{
    {{"eventRadiusSpreadPct", "%", "REDUCED_PHYSICAL_MODEL", 0, 5, 2.5},
     {"detectorMode", "0=RATIO_ONLY,1=HYBRID_RATIO_SLOPE", "ENGINEERING", 0, 1, 1, {0, 1}, 2},
     {"positiveSlopeThresholdDbPerMs", "dB/ms", "ENGINEERING", .1, 20, 1},
     {"sourceExcitationGamma", "ratio", "PRODUCT_MAPPING", .5, 1, .75},
     {"stereoDetuneCents", "cents/channel", "PRODUCT_MAPPING", 0, 1, .5},
     {"maximumInterchannelBeatHz", "Hz", "ENGINEERING", .1, 2, 2}}};
// B2 owns a separate schema. Unchanged baseline fields reuse the frozen B1 specs.
inline constexpr auto kB2Parameters = [] {
    std::array<B1ParameterSpec, kB1Parameters.size() + kB2ExtraParameters.size()> result{};
    std::copy(kB1Parameters.begin(), kB1Parameters.end(), result.begin());
    std::copy(kB2ExtraParameters.begin(), kB2ExtraParameters.end(),
              result.begin() + kB1Parameters.size());
    return result;
}();
struct DropletB2Config final {
    std::array<double, kB2Parameters.size()> values = [] {
        std::array<double, kB2Parameters.size()> result{};
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] = kB2Parameters[i].initial;
        return result;
    }();
    double operator[](B2Parameter p) const noexcept {
        return values[kB1Parameters.size() + static_cast<std::size_t>(p)];
    }
    double& operator[](B2Parameter p) noexcept {
        return values[kB1Parameters.size() + static_cast<std::size_t>(p)];
    }
    DropletB1Config baseline() const noexcept {
        DropletB1Config c;
        std::copy_n(values.begin(), c.values.size(), c.values.begin());
        return c;
    }
    bool valid() const noexcept {
        for (std::size_t i = 0; i < values.size(); ++i)
            if (!kB2Parameters[i].accepts(values[i]))
                return false;
        return true;
    }
};
} // namespace frazil::water::research
