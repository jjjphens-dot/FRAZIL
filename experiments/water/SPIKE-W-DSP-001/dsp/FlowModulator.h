#pragma once

#include "WaterDspConfig.h"
#include "WaterExcitationFeatures.h"

#include <array>
#include <vector>

namespace frazil::water::research {

struct FlowConfig final {
    double baseDelaySeconds{0.004};
    double depthSeconds{0.001};
    double targetIntervalSeconds{0.25};
    double residualGain{0.1};
};

// Flow D: shared smooth random trajectory and linked source activity, separate channel reads.
// Output is gain*(delayed-input); there is no duplicated carrier and no feedback.
class FlowModulator final {
  public:
    // May allocate. A failed prepare disables processing; no stale buffer is processed.
    bool prepare(const ResearchConfig& research, const FlowConfig& config = {}) {
        ready_ = false;
        const double rate = research.sampleRateHz;
        if (!features_.prepare(rate) || !std::isfinite(config.baseDelaySeconds) ||
            !std::isfinite(config.depthSeconds) || config.depthSeconds < 0.0 ||
            config.baseDelaySeconds - config.depthSeconds < 1.0 / rate ||
            config.baseDelaySeconds + config.depthSeconds > 0.02 ||
            !std::isfinite(config.targetIntervalSeconds) || config.targetIntervalSeconds < 0.02 ||
            config.targetIntervalSeconds > 10.0 || !std::isfinite(config.residualGain) ||
            config.residualGain < 0.0 || config.residualGain > 0.15)
            return false;
        baseSamples_ = config.baseDelaySeconds * rate;
        depthSamples_ = config.depthSeconds * rate;
        intervalSamples_ = static_cast<std::size_t>(std::ceil(config.targetIntervalSeconds * rate));
        gain_ = config.residualGain;
        seed_ = research.seedFor(RandomDomain::flow);
        const auto capacity = static_cast<std::size_t>(std::ceil(baseSamples_ + depthSamples_)) + 2;
        for (auto& channel : delay_)
            channel.resize(capacity);
        ready_ = true;
        reset();
        return true;
    }

    void reset() noexcept {
        for (auto& channel : delay_)
            std::fill(channel.begin(), channel.end(), 0.0f);
        features_.reset();
        random_.reseed(seed_);
        write_ = phase_ = 0;
        from_ = 0.0;
        to_ = 2.0 * random_.nextUnipolar() - 1.0;
        lastDelay_ = baseSamples_;
    }

    StereoFrame process(const StereoFrame& input) noexcept {
        if (!ready_)
            return {};
        const double activity = features_.process(input).slow;
        const double phase = static_cast<double>(phase_) / intervalSamples_;
        const double curve = phase * phase * (3.0 - 2.0 * phase);
        const double motion = from_ + (to_ - from_) * curve;
        const auto capacity = delay_[0].size();
        lastDelay_ = std::clamp(baseSamples_ + depthSamples_ * motion * activity, 1.0,
                                static_cast<double>(capacity - 2));
        const auto integer = static_cast<std::size_t>(lastDelay_);
        const double fraction = lastDelay_ - integer;
        const auto recent = (write_ + capacity - integer) % capacity;
        const auto older = (recent + capacity - 1) % capacity;
        StereoFrame output{};
        for (std::size_t channel = 0; channel < 2; ++channel) {
            delay_[channel][write_] = input[channel];
            const double delayed =
                (1.0 - fraction) * delay_[channel][recent] + fraction * delay_[channel][older];
            output[channel] = static_cast<float>(gain_ * (delayed - input[channel]));
        }
        write_ = (write_ + 1) % capacity;
        if (++phase_ == intervalSamples_) {
            phase_ = 0;
            from_ = to_;
            to_ = 2.0 * random_.nextUnipolar() - 1.0;
        }
        return output;
    }

    double lastDelaySamples() const noexcept {
        return lastDelay_;
    }

  private:
    // Allocated only in prepare, cleared in reset; no growth in process.
    std::array<std::vector<float>, 2> delay_;
    WaterExcitationFeatures features_;
    RandomSource random_;
    RandomSource::Seed seed_{RandomSource::kDefaultSeed};
    std::size_t write_{}; // Circular write index, reset to zero.
    std::size_t phase_{}; // Samples within current nonperiodic target segment.
    std::size_t intervalSamples_{1};
    double from_{}, to_{}; // Smoothstep endpoints in [-1,1]; reset reseeds the first target.
    double baseSamples_{}, depthSamples_{}, lastDelay_{};
    double gain_{};
    bool ready_{};
};
} // namespace frazil::water::research
