#pragma once
#include "SharedExcitationConfig.h"
#include "WaterExcitationFeatures.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace frazil::water::research {
struct SharedExcitation final {
    double fastPower{}, slowPower{}, rms{}, activity{};
};

// One linked detector and a 2 ms stereo observation window. Event direction comes from ONE
// observed stereo frame, selected by joint energy, not independently selected channel peaks.
class SharedExcitationAnalyzer final {
  public:
    bool prepare(double rate, const SharedExcitationConfig& c = {}) noexcept {
        ready_ = false;
        reset();
        const auto valid = [](double x, double lo, double hi) {
            return std::isfinite(x) && x >= lo && x <= hi;
        };
        if (!valid(rate, 44100, 96000) || !kSharedFastAttackMs.accepts(c.fastAttackMs) ||
            !kSharedFastReleaseMs.accepts(c.fastReleaseMs) ||
            !kSharedSlowAttackMs.accepts(c.slowAttackMs) ||
            !kSharedSlowReleaseMs.accepts(c.slowReleaseMs) ||
            !kSharedActivityFloorDbFS.accepts(c.activityFloorDbFS) ||
            !kSharedActivityKneeDb.accepts(c.activityKneeDb))
            return false;
        const std::array times{c.fastAttackMs, c.fastReleaseMs, c.slowAttackMs, c.slowReleaseMs};
        for (std::size_t i = 0; i < 4; ++i)
            coefficients_[i] = std::exp(-1000 / (rate * times[i]));
        windowSize_ = static_cast<std::size_t>(std::ceil(.002 * rate));
        floorPower_ = std::pow(10., c.activityFloorDbFS / 10);
        kneePower_ = floorPower_ * std::pow(10., c.activityKneeDb / 10);
        ready_ = true;
        return true;
    }
    void reset() noexcept {
        window_ = {};
        sums_ = {};
        cursor_ = 0;
        state_ = {};
    }
    SharedExcitation process(const StereoFrame& input) noexcept {
        if (!ready_)
            return {};
        for (std::size_t ch = 0; ch < 2; ++ch) {
            // Bounded analysis domain only; neither source nor residual audio is clipped.
            const double x = std::isfinite(input[ch]) ? std::clamp(double(input[ch]), -1., 1.) : 0;
            const double old = window_[cursor_][ch];
            sums_[ch] = std::max(0., sums_[ch] + x * x - old * old);
            window_[cursor_][ch] = x;
        }
        cursor_ = (cursor_ + 1) % windowSize_;
        const double power = (sums_[0] + sums_[1]) / (2 * windowSize_);
        const auto follow = [&](double old, std::size_t offset) {
            const double next =
                power + coefficients_[offset + (power > old ? 0 : 1)] * (old - power);
            return next < 1e-25 ? 0. : next;
        };
        state_.fastPower = follow(state_.fastPower, 0);
        state_.slowPower = follow(state_.slowPower, 2);
        state_.rms = std::sqrt(power);
        const double gate =
            std::clamp((state_.fastPower - floorPower_) / (kneePower_ - floorPower_), 0., 1.);
        // Amplitude activity remains level-sensitive above the knee. Exact empty-window gate
        // prevents the analysis release from inventing new source energy after silence.
        state_.activity = power > floorPower_ ? gate * std::sqrt(state_.slowPower) : 0;
        return state_;
    }
    std::array<double, 2> eventCarrier(bool sourceEnergy) const noexcept {
        std::array<double, 2> peak{}, result{};
        double jointPeak{};
        for (std::size_t i = 0; i < windowSize_; ++i) {
            const auto& frame = window_[i];
            const double energy = frame[0] * frame[0] + frame[1] * frame[1];
            if (energy > jointPeak) { // Equal energies retain the lowest physical window index.
                jointPeak = energy;
                peak = frame;
            }
        }
        const double norm = std::sqrt(jointPeak / 2);
        if (norm <= 1e-12)
            return result;
        const double level = sourceEnergy ? std::sqrt(state_.fastPower) : .25;
        for (std::size_t ch = 0; ch < 2; ++ch)
            result[ch] = peak[ch] / norm * level;
        return result;
    }
    const SharedExcitation& state() const noexcept {
        return state_;
    }

  private:
    std::array<std::array<double, 2>, 192> window_{};
    std::array<double, 2> sums_{};
    std::array<double, 4> coefficients_{};
    std::size_t cursor_{}, windowSize_{96};
    double floorPower_{}, kneePower_{};
    SharedExcitation state_{};
    bool ready_{};
};
} // namespace frazil::water::research
