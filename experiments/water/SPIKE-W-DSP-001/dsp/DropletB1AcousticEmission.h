#pragma once
#include "DropletB1Model.h"

#include <numbers>

namespace frazil::water::research {
// B1-PHY-005: linearized volume acceleration, with explicit ENGINEERING reference
// calibration. The returned value is relative emission, never microphone pressure in Pa.
struct DropletB1AcousticEmission final {
    static double referenceScale(double radiusMeters) noexcept {
        const double ref = DropletB1Model::kReferenceRadiusMeters;
        const double w = 2 * std::numbers::pi * BubblePhysics::minnaertFrequency(ref);
        return radiusMeters * radiusMeters / (ref * ref * w * w);
    }
    static double relativeVolumeAcceleration(double sineState, double cosineState, double damping,
                                             double omega, double omegaPrime,
                                             double scale) noexcept {
        return scale * ((damping * damping - omega * omega) * sineState +
                        (omegaPrime - 2 * damping * omega) * cosineState);
    }
};
} // namespace frazil::water::research
