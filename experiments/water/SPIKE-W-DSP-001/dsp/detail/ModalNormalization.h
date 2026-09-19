#pragma once

#include "DampedResonator.h"

#include <algorithm>
#include <array>

namespace frazil::water::research {
enum class ModalNormalization { c0, c3 };
}

namespace frazil::water::research::detail {

// Sum_{n>=0} q^n sin^2(n theta), written without subtracting nearly equal terms.
inline double sineEnergy(double q, double theta) noexcept {
    const double d = 1 - q, sine = std::sin(theta), square = sine * sine;
    return q * (1 + q) * square / (d * (d * d + 4 * q * square));
}

struct ModalPoleBounds final {
    double energy{}, l1{};
};

inline ModalPoleBounds poleBounds(const ResonatorCoefficients& coefficient) noexcept {
    // Use the actual rounded Cartesian coefficients, not the ideal radius before sin/cos.
    const double radius = std::hypot(coefficient.real, coefficient.imaginary);
    const double theta = std::atan2(coefficient.imaginary, coefficient.real);
    // Cauchy-Schwarz: sum r^n |sin(n theta)| <= sqrt(sum_{n>=1} r^n * sineEnergy(r)).
    return {sineEnergy(radius * radius, theta),
            std::sqrt(radius / (1 - radius) * sineEnergy(radius, theta))};
}

struct ModalNormalizationReadout final {
    std::array<double, 6> excitation{};
    double residualBound{};
    double energyScale{1};
    double safetyScale{1};
};

// Research C3 only. Emax=1 is enforced by the selected bounded conditioner before this bank.
// Budget 4 is a conservative residual peak ceiling (+12.04 dBFS), not an output target or limiter.
// At Reference E=0 dB / Monitor=-18 dB, |x|<=1 implies |monitor| < (1+4)*10^(-18/20) < .63.
// Higher Focus monitoring can still over-range and must retain the existing warning.
inline ModalNormalizationReadout normalizeModal(std::array<ResonatorCoefficients, 6>& coefficients,
                                                const std::array<ResonatorCoefficients, 6>& anchor,
                                                ModalNormalization normalization,
                                                double gain) noexcept {
    constexpr double maxGain = .3;
    constexpr double maxWeight = 1.35 / .65; // Covers every current legal time-varying weight.
    constexpr double residualBudget = 4;
    constexpr double numericalMargin = .99;
    ModalNormalizationReadout result;
    double anchorGain{}, candidateGain{};
    std::array<ModalPoleBounds, 6> bounds;
    for (std::size_t i = 0; i < coefficients.size(); ++i) {
        bounds[i] = poleBounds(coefficients[i]);
        const auto anchored = poleBounds(anchor[i]);
        anchorGain += anchored.l1 / std::sqrt(anchored.energy);
    }
    if (normalization == ModalNormalization::c3) {
        // Common 480 ms anchor holds the energy target across 30/120/480 ms. Neither Motion nor
        // the raw gain control changes this target. Longer legal raw Decay values may need a cap.
        result.energyScale = residualBudget * numericalMargin /
                             (maxGain * maxWeight * anchorGain / coefficients.size());
        for (std::size_t i = 0; i < coefficients.size(); ++i) {
            result.excitation[i] = result.energyScale / std::sqrt(bounds[i].energy);
            candidateGain += result.excitation[i] * bounds[i].l1;
        }
        result.safetyScale =
            std::min(1.0, residualBudget * numericalMargin /
                              (maxGain * maxWeight * candidateGain / coefficients.size()));
        for (std::size_t i = 0; i < coefficients.size(); ++i)
            coefficients[i].excitation = result.excitation[i] * result.safetyScale;
    }
    for (std::size_t i = 0; i < coefficients.size(); ++i) {
        result.excitation[i] = coefficients[i].excitation;
        result.residualBound +=
            gain * maxWeight / coefficients.size() * result.excitation[i] * bounds[i].l1;
    }
    return result;
}
} // namespace frazil::water::research::detail
