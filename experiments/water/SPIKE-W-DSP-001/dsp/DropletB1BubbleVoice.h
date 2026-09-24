#pragma once
#include "DropletB1AcousticEmission.h"
#include "DropletB1EntrainmentModel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace frazil::water::research {
// One shared phase/envelope trajectory, separate signed source amplitudes. Coefficients
// are created at event start. Only a one-time cap crossing needs transcendental setup.
class DropletB1BubbleVoice final {
  public:
    void start(const DropletB1Event& e, double rate) noexcept {
        event_ = e;
        rate_ = rate;
        age_ = 0;
        sine_ = 0;
        cosine_ = envelope_ = 1;
        decay_ = std::exp(-e.physics.effectiveDamping / rate);
        omega0_ = 2 * std::numbers::pi * e.physics.frequencyHz;
        acceleration_ = omega0_ * e.riseXi * e.physics.dampingPerSecond;
        omegaCap_ =
            2 * std::numbers::pi * std::min(std::sqrt(2.) * e.physics.frequencyHz, .45 * rate);
        capTime_ = acceleration_ > 0 ? (omegaCap_ - omega0_) / acceleration_
                                     : std::numeric_limits<double>::infinity();
        const double angle = omega0_ / rate + .5 * acceleration_ / (rate * rate);
        rotation_ = {std::cos(angle), std::sin(angle)};
        const double increment = acceleration_ / (rate * rate);
        rotationStep_ = {std::cos(increment), std::sin(increment)};
        emissionScale_ =
            DropletB1AcousticEmission::referenceScale(e.physics.equivalentRadiusMeters);
        const double amplitude =
            e.physics.renderAmplitudeScale * e.impact.sourceExcitation * e.gain;
        for (std::size_t ch = 0; ch < 2; ++ch)
            amplitudes_[ch] = amplitude * e.impact.carrier[ch];
        // Conservative relative emission envelope for deterministic least-audible stealing.
        const double d = e.physics.effectiveDamping;
        emissionBound_ = e.emission == 1 ? 1
                                         : emissionScale_ * (d * d + omegaCap_ * omegaCap_ +
                                                             acceleration_ + 2 * d * omegaCap_);
        capped_ = false;
        active_ = true;
    }
    std::array<double, 2> process() noexcept {
        if (!active_)
            return {};
        const double time = double(age_) / rate_;
        const double omega = std::min(omega0_ + acceleration_ * time, omegaCap_);
        const double slope = time < capTime_ ? acceleration_ : 0;
        const double value =
            event_.emission == 1
                ? envelope_ * sine_
                : DropletB1AcousticEmission::relativeVolumeAcceleration(
                      envelope_ * sine_, envelope_ * cosine_, event_.physics.effectiveDamping,
                      omega, slope, emissionScale_);
        const std::array result{amplitudes_[0] * value, amplitudes_[1] * value};
        const double nextTime = double(age_ + 1) / rate_;
        if (!capped_ && nextTime >= capTime_) {
            const double h = std::max(0., capTime_ - time);
            const double angle = (omega0_ + acceleration_ * time) * h + .5 * acceleration_ * h * h +
                                 omegaCap_ * (nextTime - std::max(time, capTime_));
            rotate(cosine_, sine_, {std::cos(angle), std::sin(angle)});
            rotation_ = {std::cos(omegaCap_ / rate_), std::sin(omegaCap_ / rate_)};
            capped_ = true;
        } else {
            rotate(cosine_, sine_, rotation_);
            if (!capped_)
                rotate(rotation_[0], rotation_[1], rotationStep_);
        }
        envelope_ *= decay_;
        ++age_;
        if (envelope_ <= event_.tailFloor ||
            double(age_) >= rate_ * DropletB1Model::kMaximumLifetimeSeconds)
            active_ = false;
        return result;
    }
    bool active() const noexcept {
        return active_;
    }
    double priority() const noexcept {
        return std::max(std::abs(amplitudes_[0]), std::abs(amplitudes_[1])) * envelope_ *
               emissionBound_;
    }
    const DropletB1Event& event() const noexcept {
        return event_;
    }

  private:
    static void rotate(double& real, double& imag, const std::array<double, 2>& rotation) noexcept {
        const double next = real * rotation[0] - imag * rotation[1];
        imag = real * rotation[1] + imag * rotation[0];
        real = next;
    }
    DropletB1Event event_{}; // Captured identity/physics for this voice's whole lifetime.
    std::array<double, 2> amplitudes_{}, rotation_{}, rotationStep_{};
    double rate_{}, decay_{}, emissionScale_{}, emissionBound_{};
    double omega0_{}, acceleration_{}, omegaCap_{}, capTime_{};
    double sine_{}, cosine_{},
        envelope_{};      // Audio-owner phase/envelope recurrence, reset by start.
    std::uint64_t age_{}; // Elapsed voice samples, independent of source/due timestamps.
    bool active_{}, capped_{};
};
} // namespace frazil::water::research
