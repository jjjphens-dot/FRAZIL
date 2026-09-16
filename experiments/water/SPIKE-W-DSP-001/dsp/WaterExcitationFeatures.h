#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace frazil::water::research {
using StereoFrame = std::array<float, 2>;

struct FeatureConfig final {
    double fastAttackSeconds{0.001};
    double fastReleaseSeconds{0.03};
    double slowAttackSeconds{0.03};
    double slowReleaseSeconds{0.2};
};

struct ExcitationFeatures final {
    double fast{};
    double slow{};
    double transient{};
};

// Linked max(abs(L),abs(R)) control only: audio channels are never summed or crossfed.
// Single processing owner; prepare/reset clear state. Output is a normalized control proxy,
// not RMS, energy, a perceptual score, or a modification of the source audio.
class WaterExcitationFeatures final {
  public:
    bool prepare(double sampleRateHz, const FeatureConfig& config = {}) noexcept {
        ready_ = false;
        reset();
        if (!std::isfinite(sampleRateHz) || sampleRateHz < 44100.0 || sampleRateHz > 96000.0)
            return false;
        const std::array times{config.fastAttackSeconds, config.fastReleaseSeconds,
                               config.slowAttackSeconds, config.slowReleaseSeconds};
        for (std::size_t i = 0; i < times.size(); ++i) {
            if (!std::isfinite(times[i]) || times[i] < 0.0001 || times[i] > 2.0)
                return false;
            coefficients_[i] = std::exp(-1.0 / (times[i] * sampleRateHz));
        }
        ready_ = true;
        return true;
    }

    void reset() noexcept {
        state_ = {};
    }

    ExcitationFeatures process(const StereoFrame& input) noexcept {
        if (!ready_)
            return {};
        const double magnitude = std::min(1.0, std::max(std::abs(static_cast<double>(input[0])),
                                                        std::abs(static_cast<double>(input[1]))));
        const auto follow = [&](double previous, std::size_t index) {
            const double coefficient = coefficients_[index + (magnitude > previous ? 0 : 1)];
            const double next = magnitude + coefficient * (previous - magnitude);
            return next < 1.0e-15 ? 0.0 : next;
        };
        state_.fast = follow(state_.fast, 0);
        state_.slow = follow(state_.slow, 2);
        state_.transient = std::max(0.0, state_.fast - state_.slow);
        return state_;
    }

  private:
    // Per-sample attack/release multipliers, computed only by non-realtime prepare.
    std::array<double, 4> coefficients_{};
    // Dimensionless [0,1] envelope state; owned by the processing caller, reset to zero.
    ExcitationFeatures state_{};
    bool ready_{};
};
} // namespace frazil::water::research
