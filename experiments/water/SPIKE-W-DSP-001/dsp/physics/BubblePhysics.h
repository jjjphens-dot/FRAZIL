#pragma once
#include <cmath>
#include <numbers>

namespace frazil::water::research {
// PHYSICAL reference approximation, SI units. Not a geometry/propagation solver.
// EXP-W-DB-001 B1-PHY-001/002. A1 is intentionally not migrated in this change.
struct BubblePhysics final {
    static constexpr double kPressurePa = 101325, kDensityKgM3 = 998, kGamma = 1.4;
    static double minnaertFrequency(double radiusMeters) noexcept {
        return std::sqrt(3 * kGamma * kPressurePa / kDensityKgM3) /
               (2 * std::numbers::pi * radiusMeters);
    }
    // van den Doel's fitted total damping, valid above approximately 0.15 mm.
    static double damping(double radiusMeters) noexcept {
        return .13 / radiusMeters + .0072 / std::pow(radiusMeters, 1.5);
    }
};
} // namespace frazil::water::research
