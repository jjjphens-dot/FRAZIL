#pragma once
#include "FlowD1Config.h"

#include <algorithm>

namespace frazil::water::research {
// Pure SI reduction. EXP-W-FD-001 owns assumptions and omitted geometry/radiation.
struct FlowD1Model final {
    static constexpr double referenceSoundSpeedMps = 1484.0;
    static bool validRate(double rate) noexcept {
        return std::isfinite(rate) && rate >= 44100 && rate <= 96000;
    }
    static double delaySeconds(double pathMeters) noexcept {
        return pathMeters / referenceSoundSpeedMps;
    }
    static double characteristicRateHz(const FlowD1Config& c) noexcept {
        return c.velocityScaleMps / c.virtualStructureLengthMeters;
    }
    // q'=6p(1-p)<=1.5. Avoid L/U overflow at arbitrarily tiny legal U.
    static double phaseStep(const FlowD1Config& c, double deltaMeters, double rate) noexcept {
        return c.velocityScaleMps / rate /
               std::max(c.virtualStructureLengthMeters, 1.5 * std::abs(deltaMeters));
    }
};
} // namespace frazil::water::research
