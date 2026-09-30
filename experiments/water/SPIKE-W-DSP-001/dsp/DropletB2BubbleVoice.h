#pragma once
#include "DropletB1BubbleVoice.h"
#include "DropletB2ImpactDescriptor.h"
#include "DropletB2SpatialRenderer.h"
namespace frazil::water::research {
// Center physics/envelope stays shared. Two phase recurrences render PRODUCT_MAPPING
// channel offsets. Zero detune delegates exactly to the preserved center emission.
class DropletB2BubbleVoice final {
  public:
    void start(const DropletB2Event& event, double rate) noexcept {
        event_ = event;
        rate_ = rate;
        age_ = 0;
        envelope_ = 1;
        active_ = true;
        auto center = event.center;
        center.impact.sourceExcitation = event.mappedExcitation;
        stereo_ = event.detuneCents != 0;
        if (!stereo_) {
            baseline_.start(center, rate);
            return;
        }
        const auto& physical = center.physics;
        const double maximum = std::min(std::sqrt(2.) * physical.frequencyHz, .45 * rate);
        // Conservative event-locked cap: evaluate the maximum of the unchanged rise,
        // so a fixed channel ratio obeys the beat bound at every point, including cap.
        const double boundFrequency = center.riseXi > 0 ? maximum : physical.frequencyHz;
        const double cents = DropletB2SpatialRenderer::boundedCents(
            boundFrequency, event.detuneCents, event.maximumBeatHz);
        const auto frequencies =
            DropletB2SpatialRenderer::frequencies(physical.frequencyHz, cents, event.polarity);
        const double amplitude =
            physical.renderAmplitudeScale * event.mappedExcitation * center.gain;
        decay_ = std::exp(-physical.effectiveDamping / rate);
        emissionScale_ = DropletB1AcousticEmission::referenceScale(physical.equivalentRadiusMeters);
        priorityBound_ = 0;
        for (std::size_t c = 0; c < 2; ++c) {
            const double ratio = frequencies[c] / physical.frequencyHz;
            auto& p = phases_[c];
            p = {};
            p.cosine = 1;
            p.omega0 = 2 * std::numbers::pi * frequencies[c];
            p.acceleration = p.omega0 * center.riseXi * physical.dampingPerSecond;
            p.cap = 2 * std::numbers::pi * maximum * ratio;
            p.capTime = p.acceleration > 0 ? (p.cap - p.omega0) / p.acceleration
                                           : std::numeric_limits<double>::infinity();
            const double angle = p.omega0 / rate + .5 * p.acceleration / (rate * rate);
            p.rotation = {std::cos(angle), std::sin(angle)};
            const double step = p.acceleration / (rate * rate);
            p.step = {std::cos(step), std::sin(step)};
            amplitudes_[c] = amplitude * center.impact.carrier[c];
            const double d = physical.effectiveDamping;
            const double bound =
                center.emission == 1
                    ? 1
                    : emissionScale_ * (d * d + p.cap * p.cap + p.acceleration + 2 * d * p.cap);
            priorityBound_ = std::max(priorityBound_, std::abs(amplitudes_[c]) * bound);
        }
    }
    std::array<double, 2> process() noexcept {
        if (!stereo_)
            return baseline_.process();
        if (!active_)
            return {};
        std::array<double, 2> result{};
        const double t = double(age_) / rate_, next = double(age_ + 1) / rate_;
        for (std::size_t c = 0; c < 2; ++c) {
            auto& p = phases_[c];
            const double omega = std::min(p.omega0 + p.acceleration * t, p.cap);
            const double slope = t < p.capTime ? p.acceleration : 0;
            const double value =
                event_.center.emission == 1
                    ? envelope_ * p.sine
                    : DropletB1AcousticEmission::relativeVolumeAcceleration(
                          envelope_ * p.sine, envelope_ * p.cosine,
                          event_.center.physics.effectiveDamping, omega, slope, emissionScale_);
            result[c] = amplitudes_[c] * value;
            if (!p.capped && next >= p.capTime) {
                const double h = std::max(0., p.capTime - t);
                const double angle = (p.omega0 + p.acceleration * t) * h +
                                     .5 * p.acceleration * h * h +
                                     p.cap * (next - std::max(t, p.capTime));
                rotate(p.cosine, p.sine, {std::cos(angle), std::sin(angle)});
                p.rotation = {std::cos(p.cap / rate_), std::sin(p.cap / rate_)};
                p.capped = true;
            } else {
                rotate(p.cosine, p.sine, p.rotation);
                if (!p.capped)
                    rotate(p.rotation[0], p.rotation[1], p.step);
            }
        }
        envelope_ *= decay_;
        ++age_;
        if (envelope_ <= event_.center.tailFloor ||
            double(age_) >= rate_ * DropletB1Model::kMaximumLifetimeSeconds)
            active_ = false;
        return result;
    }
    bool active() const noexcept {
        return stereo_ ? active_ : baseline_.active();
    }
    double priority() const noexcept {
        return stereo_ ? priorityBound_ * envelope_ : baseline_.priority();
    }
    const DropletB2Event& event() const noexcept {
        return event_;
    }

  private:
    struct Phase {
        double sine{}, cosine{}, omega0{}, acceleration{}, cap{}, capTime{};
        std::array<double, 2> rotation{}, step{};
        bool capped{};
    };
    static void rotate(double& real, double& imag, const std::array<double, 2>& r) noexcept {
        const double next = real * r[0] - imag * r[1];
        imag = real * r[1] + imag * r[0];
        real = next;
    }
    DropletB2Event event_{};
    DropletB1BubbleVoice baseline_;
    std::array<Phase, 2> phases_{}; // Audio-owned event phases; initialized only on start.
    std::array<double, 2> amplitudes_{};
    double rate_{}, envelope_{}, decay_{}, emissionScale_{}, priorityBound_{};
    std::uint64_t age_{};
    bool stereo_{}, active_{};
};
} // namespace frazil::water::research
