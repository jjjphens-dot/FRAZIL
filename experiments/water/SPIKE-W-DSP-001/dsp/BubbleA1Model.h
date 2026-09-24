#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <optional>

namespace frazil::water::research {
inline bool a1Range(double x, double lo, double hi) noexcept {
    return std::isfinite(x) && x >= lo && x <= hi;
}
inline bool a1Capacity(std::size_t n) noexcept {
    return n == 64 || n == 128 || n == 256 || n == 512 || n == 1024;
}

enum class BubbleA1RiseModel { physicalDampingP0, effectiveDampingP1 };

struct BubbleA1Config final {
    double radiusMinMm{.2}, radiusMaxMm{10};
    double populationGamma{2}, amplitudeRadiusExponent{1.5}, depthExponent{10};
    double persistenceScale{1}, maxEventRateHz{1000}, motionFactor{1};
    double riseXi{.1}, riseCutoff{.9};
    double tailFloorDb{-80}, stealReleaseMs{1.5}, residualGain{.2};
    std::size_t voiceCapacity{256};
    BubbleA1RiseModel riseModel{BubbleA1RiseModel::effectiveDampingP1};
    // A1-1/2 ablation only: fixed linked amplitude, still source-gated and stereo-linked.
    bool sourceEnergyAmplitude{true};
};

struct BubbleA1Bin final {
    double radiusMeters{}, frequencyHz{}, dampingPerSecond{}, tauSeconds{};
    double probability{}, cdf{}, amplitude{}, poleRadius{};
};

// Non-realtime construction; no spectral clipping of the physical radius-frequency relation.
// Source provenance and approximation limits: experiments/water/EXP-W-BA-001.md.
class BubbleA1Model final {
  public:
    static constexpr std::size_t kBins = 128;
    static constexpr double kPressurePa = 101325, kDensityKgM3 = 998, kKappa = 1.4;
    static double frequency(double radiusMeters) noexcept {
        return std::sqrt(3 * kKappa * kPressurePa / kDensityKgM3) /
               (2 * std::numbers::pi * radiusMeters);
    }
    static double damping(double radiusMeters) noexcept {
        return .13 / radiusMeters + .0072 / std::pow(radiusMeters, 1.5);
    }
    bool prepare(double rate, const BubbleA1Config& c) noexcept {
        bins_ = {};
        if (!a1Range(rate, 44100, 96000) || !a1Range(c.radiusMinMm, .2, 10) ||
            !a1Range(c.radiusMaxMm, 2, 50) || c.radiusMinMm >= c.radiusMaxMm ||
            !a1Range(c.populationGamma, 0, 6) || !a1Range(c.amplitudeRadiusExponent, .75, 2.25) ||
            !a1Range(c.depthExponent, 1, 16) || !a1Range(c.persistenceScale, .25, 4) ||
            !a1Range(c.maxEventRateHz, 0, 10000) || !a1Range(c.motionFactor, 0, 1) ||
            !a1Range(c.riseXi, 0, .2) || !a1Range(c.riseCutoff, .8, 1) ||
            !a1Range(c.tailFloorDb, -100, -60) || !a1Range(c.stealReleaseMs, .5, 4) ||
            !a1Range(c.residualGain, 0, 1) || !a1Capacity(c.voiceCapacity) ||
            (c.riseModel != BubbleA1RiseModel::physicalDampingP0 &&
             c.riseModel != BubbleA1RiseModel::effectiveDampingP1))
            return false;
        double weightSum{}, amplitudeMoment{};
        for (std::size_t i = 0; i < kBins; ++i) {
            auto& b = bins_[i];
            const double mm = i == kBins - 1
                                  ? c.radiusMaxMm
                                  : c.radiusMinMm * std::pow(c.radiusMaxMm / c.radiusMinMm,
                                                             double(i) / (kBins - 1));
            b.radiusMeters = mm * .001;
            b.frequencyHz = frequency(b.radiusMeters);
            b.dampingPerSecond = damping(b.radiusMeters);
            b.tauSeconds = c.persistenceScale / b.dampingPerSecond;
            b.poleRadius = std::exp(-1 / (b.tauSeconds * rate));
            // Ratios avoid large dimensional powers; normalization cancels the reference.
            b.probability =
                b.frequencyHz <= .45 * rate ? std::pow(mm / c.radiusMinMm, -c.populationGamma) : 0;
            b.amplitude = std::pow(mm / c.radiusMinMm, c.amplitudeRadiusExponent);
            weightSum += b.probability;
            amplitudeMoment += b.probability * b.amplitude * b.amplitude;
        }
        if (weightSum <= 0 || amplitudeMoment <= 0)
            return false;
        const double normalization = std::sqrt(amplitudeMoment / weightSum);
        double cumulative{};
        for (auto& b : bins_) {
            b.probability /= weightSum;
            b.cdf = cumulative += b.probability;
            b.amplitude /= normalization;
        }
        bins_.back().cdf = 1;
        return true;
    }
    const auto& bins() const noexcept {
        return bins_;
    }
    // At most eight binary decisions. u must be [0,1); zero-probability bins are skipped.
    std::size_t sample(double u) const noexcept {
        auto it =
            std::upper_bound(bins_.begin(), bins_.end(), u,
                             [](double value, const BubbleA1Bin& b) { return value < b.cdf; });
        return std::min<std::size_t>(it - bins_.begin(), kBins - 1);
    }

  private:
    std::array<BubbleA1Bin, kBins> bins_{};
};

// Explicit offline-only candidate. No dependency from preview/session or Host parameters.
inline std::optional<BubbleA1Config> mapBubbleA1(double size, double motion,
                                                 double decay) noexcept {
    if (!a1Range(size, 0, 1) || !a1Range(motion, 0, 1) || !a1Range(decay, 0, 1))
        return std::nullopt;
    BubbleA1Config c;
    c.radiusMinMm = .2 * std::pow(10., size);
    c.radiusMaxMm = 2 * std::pow(25., size);
    c.motionFactor = motion * motion;
    c.persistenceScale = std::pow(4., 2 * decay - 1);
    return c;
}
} // namespace frazil::water::research
