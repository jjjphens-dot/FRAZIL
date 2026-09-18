#pragma once

#include "ProtectDetector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace frazil::water::research {

enum class ProtectScore { difference, logRatio };

struct ProtectConfig final {
    ProtectScore score{ProtectScore::logRatio};
    double floor{1.0e-4}, epsilon{1.0e-8};
    double thresholdLow{1.0}, thresholdHigh{9.0}; // dB for D1; amplitude for D0.
    double capDb{9.0}, depthExponent{1.0}, scoreExponent{1.0};
    double attackSeconds{.001}, releaseSeconds{.08};
    double offSeconds{.01}; // Finite dB ramp to exact unity, independent of release tau.
};

// Pure score/depth -> target attenuation. Config is validated by ResidualProtect::prepare.
inline double protectTargetDb(double score, double depth, const ProtectConfig& config) noexcept {
    const double z = std::clamp(
        (score - config.thresholdLow) / (config.thresholdHigh - config.thresholdLow), 0.0, 1.0);
    const double q = z * z * (3.0 - 2.0 * z);
    return -config.capDb * std::pow(depth, config.depthExponent) *
           std::pow(q, config.scoreExponent);
}

// Experiment-only, one processing owner; no Host transport or generator ownership. Depth may
// be retargeted at a sample boundary. All generator/state/RNG advancement stays with the caller.
class ResidualProtect final {
  public:
    bool prepare(double rate, const ProtectConfig& config = {}, double depth = 0.0) noexcept {
        ready_ = false;
        reset();
        const std::array values{config.thresholdLow,   config.thresholdHigh, config.capDb,
                                config.depthExponent,  config.scoreExponent, config.attackSeconds,
                                config.releaseSeconds, config.offSeconds,    depth};
        for (double value : values)
            if (!std::isfinite(value))
                return false;
        if ((config.score != ProtectScore::difference && config.score != ProtectScore::logRatio) ||
            config.thresholdLow < 0.0 || config.thresholdHigh <= config.thresholdLow ||
            config.thresholdHigh > (config.score == ProtectScore::difference ? 1.0 : 100.0) ||
            config.capDb < 0.0 || config.capDb > 12.0 || config.depthExponent < .1 ||
            config.depthExponent > 8.0 || config.scoreExponent < .1 || config.scoreExponent > 8.0 ||
            config.attackSeconds < .00025 || config.attackSeconds > .002 ||
            config.releaseSeconds < .04 || config.releaseSeconds > .2 || config.offSeconds < .001 ||
            config.offSeconds > .1 || depth < 0.0 || depth > 1.0 ||
            !detector_.prepare(rate, config.floor, config.epsilon))
            return false;
        config_ = config;
        depth_ = depth;
        attack_ = std::exp(-1.0 / (rate * config.attackSeconds));
        release_ = std::exp(-1.0 / (rate * config.releaseSeconds));
        offSamples_ = static_cast<std::size_t>(std::ceil(rate * config.offSeconds));
        ready_ = true;
        return true;
    }

    void reset() noexcept {
        detector_.reset();
        gainDb_ = offStartDb_ = 0.0;
        remaining_ = 0;
        detection_ = {};
    }

    bool setDepth(double depth) noexcept {
        if (!ready_ || !std::isfinite(depth) || depth < 0.0 || depth > 1.0)
            return false;
        if (depth == depth_)
            return true; // Repeated OFF cannot restart its finite transition.
        if (depth == 0.0) {
            offStartDb_ = gainDb_;
            remaining_ = offSamples_;
        } else {
            remaining_ = 0; // Retarget from current envelope without a state discontinuity.
        }
        depth_ = depth;
        return true;
    }

    double processSource(const StereoFrame& source) noexcept {
        if (!ready_)
            return 1.0;
        detection_ = detector_.process(source); // Follower continues during OFF.
        if (depth_ == 0.0) {
            if (remaining_ > 0) {
                --remaining_;
                gainDb_ = offStartDb_ * static_cast<double>(remaining_) / offSamples_;
            } else {
                gainDb_ = 0.0;
            }
        } else {
            const double score = config_.score == ProtectScore::difference ? detection_.difference
                                                                           : detection_.logRatioDb;
            const double target = protectTargetDb(score, depth_, config_);
            const double alpha = target < gainDb_ ? attack_ : release_;
            gainDb_ = std::clamp(target + alpha * (gainDb_ - target), -config_.capDb, 0.0);
        }
        return gainDb_ == 0.0 ? 1.0 : std::pow(10.0, gainDb_ / 20.0);
    }

    double reductionDb() const noexcept {
        return -gainDb_;
    }
    ProtectDetection detection() const noexcept {
        return detection_;
    }

    // Preserve exact baseline accumulation when gain is unity. Source is added by caller once.
    static StereoFrame apply(const StereoFrame& residual, double gain) noexcept {
        if (gain == 1.0)
            return residual;
        return {static_cast<float>(gain * residual[0]), static_cast<float>(gain * residual[1])};
    }

  private:
    // All members have one processing owner. reset() clears history, retaining prepared settings.
    // Validated curve/timing settings; replaced by prepare(), retained on reset.
    ProtectConfig config_;
    // Independent source follower; reset clears its envelopes, not generator state.
    ProtectDetector detector_;
    // Latest amplitude/D1-dB observation for offline inspection; reset zeroes it.
    ProtectDetection detection_;
    // Unitless [0,1] target; prepare/setDepth update it, reset retains it.
    double depth_{};
    // Dimensionless one-pole memory coefficients [0,1]; cached by prepare from seconds/rate.
    // reset retains both so the next source sample uses the same attack/release timing.
    double attack_{}, release_{};
    // Smoothed attenuation [-capDb,0] dB; reset restores exact unity (0 dB).
    double gainDb_{};
    // Latched dB attenuation for a finite OFF ramp; reset clears to 0 dB.
    double offStartDb_{};
    // OFF duration in samples, >=1; prepare computes ceil(rate*time), reset retains.
    std::size_t offSamples_{1};
    // OFF samples left [0,offSamples_]; reset/positive retarget cancel the countdown.
    std::size_t remaining_{};
    // Only successful prepare enables processing; reset preserves readiness for reuse.
    bool ready_{};
};
} // namespace frazil::water::research
