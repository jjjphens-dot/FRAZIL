#pragma once
#include "BubbleA1ConfigSpec.h"
#include "physics/BubblePhysics.h"

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
    return kA1VoiceCapacity.accepts(static_cast<double>(n));
}

enum class BubbleA1RiseModel { physicalDampingP0, effectiveDampingP1 };

struct BubbleA1Config final {
    double radiusMinMm{kA1RadiusMinMm.initial};
    double radiusMaxMm{kA1RadiusMaxMm.initial};
    double populationGamma{kA1PopulationGamma.initial};
    double amplitudeRadiusExponent{kA1AmplitudeRadiusExponent.initial};
    double depthExponent{kA1DepthExponent.initial};
    double persistenceScale{kA1PersistenceScale.initial};
    double maxEventRateHz{kA1MaxEventRateHz.initial};
    double motionFactor{kA1MotionFactor.initial};
    double riseXi{kA1RiseXi.initial};
    double riseCutoff{kA1RiseCutoff.initial};
    double tailFloorDb{kA1TailFloorDb.initial};
    double stealReleaseMs{kA1StealReleaseMs.initial};
    double residualGain{kA1ResidualGain.initial};
    std::size_t voiceCapacity{static_cast<std::size_t>(kA1VoiceCapacity.initial)};
    BubbleA1RiseModel riseModel{
        static_cast<BubbleA1RiseModel>(static_cast<int>(kA1RiseModel.initial))};
    // Fixed amplitude is an offline ablation, still source-gated and stereo-linked.
    bool sourceEnergyAmplitude{kA1SourceEnergyAmplitude.initial == 1};
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
    static constexpr double kPressurePa = BubblePhysics::kPressurePa,
                            kDensityKgM3 = BubblePhysics::kDensityKgM3,
                            kKappa = BubblePhysics::kGamma;
    static double frequency(double radiusMeters) noexcept {
        return BubblePhysics::minnaertFrequency(radiusMeters);
    }
    static double damping(double radiusMeters) noexcept {
        return BubblePhysics::damping(radiusMeters);
    }
    bool prepare(double rate, const BubbleA1Config& c) noexcept {
        bins_ = {};
        if (!a1Range(rate, 44100, 96000) || !kA1RadiusMinMm.accepts(c.radiusMinMm) ||
            !kA1RadiusMaxMm.accepts(c.radiusMaxMm) || c.radiusMinMm >= c.radiusMaxMm ||
            !kA1PopulationGamma.accepts(c.populationGamma) ||
            !kA1AmplitudeRadiusExponent.accepts(c.amplitudeRadiusExponent) ||
            !kA1DepthExponent.accepts(c.depthExponent) ||
            !kA1PersistenceScale.accepts(c.persistenceScale) ||
            !kA1MaxEventRateHz.accepts(c.maxEventRateHz) ||
            !kA1MotionFactor.accepts(c.motionFactor) || !kA1RiseXi.accepts(c.riseXi) ||
            !kA1RiseCutoff.accepts(c.riseCutoff) || !kA1TailFloorDb.accepts(c.tailFloorDb) ||
            !kA1StealReleaseMs.accepts(c.stealReleaseMs) ||
            !kA1ResidualGain.accepts(c.residualGain) || !a1Capacity(c.voiceCapacity) ||
            !kA1RiseModel.accepts(static_cast<int>(c.riseModel)))
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
