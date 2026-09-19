#pragma once

#include "WaterDspConfig.h"
#include "WaterExcitationFeatures.h"
#include "detail/EventVoicePool.h"

namespace frazil::water::research {

struct BubbleConfig final {
    double minimumFrequencyHz{250.0};
    double maximumFrequencyHz{2800.0};
    double decaySeconds{0.07};
    double maximumEventRateHz{120.0};
    double excitationThreshold{0.0001};
    double residualGain{0.2};
    std::size_t voices{16};
};

// Input-excited stochastic bubble approximation, not autonomous Foley. Frequency bounds are
// engineering controls; lower frequency represents larger bubble scale directionally only.
class BubbleEnsemble final {
  public:
    bool prepare(const ResearchConfig& research, const BubbleConfig& config = {}) noexcept {
        ready_ = false;
        config_ = config;
        seed_ = research.seedFor(RandomDomain::bubble);
        reset();
        const double rate = research.sampleRateHz;
        if (!features_.prepare(rate) || !std::isfinite(config.minimumFrequencyHz) ||
            !std::isfinite(config.maximumFrequencyHz) || config.minimumFrequencyHz < 40.0 ||
            config.maximumFrequencyHz < config.minimumFrequencyHz ||
            config.maximumFrequencyHz > .45 * rate || !std::isfinite(config.decaySeconds) ||
            config.decaySeconds < .002 || config.decaySeconds > .5 ||
            !std::isfinite(config.maximumEventRateHz) || config.maximumEventRateHz < 0.0 ||
            config.maximumEventRateHz > 2000.0 || !std::isfinite(config.excitationThreshold) ||
            config.excitationThreshold < 0.0 || config.excitationThreshold > 1.0 ||
            !std::isfinite(config.residualGain) || config.residualGain < 0.0 ||
            config.residualGain > .3 || config.voices < 1 ||
            config.voices > detail::EventVoicePool::kCapacity)
            return false;
        if (!pool_.prepare(rate, config.minimumFrequencyHz, config.maximumFrequencyHz,
                           config.decaySeconds, config.voices))
            return false;
        probability_ = -std::expm1(-config.maximumEventRateHz / rate);
        ready_ = true;
        return true;
    }

    void reset() noexcept {
        driver_ = {};
        features_.reset();
        pool_.reset();
        random_.reseed(seed_);
    }

    StereoFrame process(const StereoFrame& input) noexcept {
        driver_ = {};
        if (!ready_)
            return {};
        const auto feature = features_.process(input);
        const double magnitude = std::max(std::abs(static_cast<double>(input[0])),
                                          std::abs(static_cast<double>(input[1])));
        // The current sample must drive excitation as well as the envelope. Silence cannot
        // schedule new events from the detector's release state; existing voices may decay.
        if (magnitude > config_.excitationThreshold &&
            random_.nextUnipolar() < probability_ * feature.slow)
            driver_ = pool_.trigger(input, random_.nextUInt() % detail::EventVoicePool::kFamilies);
        return pool_.process(config_.residualGain);
    }

    const StereoFrame& excitationFrame() const noexcept {
        return driver_;
    }

    std::uint64_t events() const noexcept {
        return pool_.events();
    }
    std::uint64_t steals() const noexcept {
        return pool_.steals();
    }
    std::size_t activeVoices() const noexcept {
        return pool_.activeVoices();
    }

  private:
    BubbleConfig config_{};
    WaterExcitationFeatures features_;
    StereoFrame driver_{};
    detail::EventVoicePool pool_;
    RandomSource random_;
    RandomSource::Seed seed_{RandomSource::kDefaultSeed};
    double probability_{}; // Full-activity per-sample probability, computed at prepare.
    bool ready_{};
};
} // namespace frazil::water::research
