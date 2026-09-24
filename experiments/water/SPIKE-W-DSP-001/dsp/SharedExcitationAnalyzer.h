#pragma once
#include "BubbleA1Model.h"
#include "WaterExcitationFeatures.h"

namespace frazil::water::research {
struct SharedExcitationConfig final {
    double fastAttackMs{1}, fastReleaseMs{30}, slowAttackMs{30}, slowReleaseMs{200};
    double activityFloorDbFS{-60}, activityKneeDb{6};
};
struct SharedExcitation final {
    double fastPower{}, slowPower{}, rms{}, activity{};
    std::array<double, 2> channelRms{};
};

// One linked detector and a 2 ms stereo observation window. Independent channel measurements
// are not event generators. Event polarity comes from the strongest window sample, never the
// triggering PCM sample; this preserves channel swap, isolation, dual mono and anti-phase.
class SharedExcitationAnalyzer final {
  public:
    bool prepare(double rate, const SharedExcitationConfig& c = {}) noexcept {
        ready_ = false;
        reset();
        if (!a1Range(rate, 44100, 96000) || !a1Range(c.fastAttackMs, .5, 5) ||
            !a1Range(c.fastReleaseMs, 10, 80) || !a1Range(c.slowAttackMs, 10, 80) ||
            !a1Range(c.slowReleaseMs, 80, 500) || !a1Range(c.activityFloorDbFS, -80, -40) ||
            !a1Range(c.activityKneeDb, 3, 12))
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
            state_.channelRms[ch] = std::sqrt(sums_[ch] / windowSize_);
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
        if (state_.rms == 0)
            return result;
        for (std::size_t i = 0; i < windowSize_; ++i)
            for (std::size_t ch = 0; ch < 2; ++ch)
                if (std::abs(window_[i][ch]) > std::abs(peak[ch]))
                    peak[ch] = window_[i][ch];
        for (std::size_t ch = 0; ch < 2; ++ch)
            // The window supplies stereo direction; linked AR energy supplies event level.
            // Separating these prevents low-frequency phase/zero crossings from collapsing
            // an otherwise sustained source's excitation. No independent channel followers.
            result[ch] = std::copysign(state_.channelRms[ch], peak[ch]) / state_.rms *
                         (sourceEnergy ? std::sqrt(state_.fastPower) : .25);
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
