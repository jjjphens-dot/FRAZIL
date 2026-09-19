#pragma once

#include "WaterDspConfig.h"
#include "WaterExcitationFeatures.h"
#include "detail/EventVoicePool.h"

namespace frazil::water::research {

struct DropletConfig final {
    double minimumFrequencyHz{600.0};
    double maximumFrequencyHz{4500.0};
    double decaySeconds{0.012};
    double transientThreshold{0.015};
    double refractorySeconds{0.02};
    double residualGain{0.15};
    std::size_t voices{8};
    double eventsEnabled{1}; // Exactly 0/1; omitted raw configs retain legacy scheduling.
    double eventActivity{1}; // Probability per otherwise-valid onset, range [0,1].
};

// B: hysteretic transient threshold with refractory interval, not a free-running event clock.
// The first candidate uses deterministic onset timing; its own PRNG chooses frequency family.
class DropletImpactExciter final {
  public:
    bool prepare(const ResearchConfig& research, const DropletConfig& config = {}) noexcept {
        ready_ = false;
        config_ = config;
        seed_ = research.seedFor(RandomDomain::droplet);
        activitySeed_ = research.seedFor(RandomDomain::dropletActivity);
        reset();
        const double rate = research.sampleRateHz;
        if (!features_.prepare(rate) || !std::isfinite(config.minimumFrequencyHz) ||
            !std::isfinite(config.maximumFrequencyHz) || config.minimumFrequencyHz < 40.0 ||
            config.maximumFrequencyHz < config.minimumFrequencyHz ||
            config.maximumFrequencyHz > .45 * rate || !std::isfinite(config.decaySeconds) ||
            config.decaySeconds < .002 || config.decaySeconds > .1 ||
            !std::isfinite(config.transientThreshold) || config.transientThreshold < .0001 ||
            config.transientThreshold > 1.0 || !std::isfinite(config.refractorySeconds) ||
            config.refractorySeconds < .001 || config.refractorySeconds > 1.0 ||
            !std::isfinite(config.residualGain) || config.residualGain < 0.0 ||
            config.residualGain > .3 || config.voices < 1 ||
            config.voices > detail::EventVoicePool::kCapacity ||
            (config.eventsEnabled != 0 && config.eventsEnabled != 1) ||
            !std::isfinite(config.eventActivity) || config.eventActivity < 0 ||
            config.eventActivity > 1)
            return false;
        refractorySamples_ = static_cast<std::uint32_t>(std::ceil(config.refractorySeconds * rate));
        if (!pool_.prepare(rate, config.minimumFrequencyHz, config.maximumFrequencyHz,
                           config.decaySeconds, config.voices))
            return false;
        ready_ = true;
        return true;
    }

    void reset() noexcept {
        features_.reset();
        pool_.reset();
        random_.reseed(seed_);
        activityRandom_.reseed(activitySeed_);
        remaining_ = 0;
        armed_ = true;
    }

    StereoFrame process(const StereoFrame& input) noexcept {
        if (!ready_)
            return {};
        const auto feature = features_.process(input);
        if (remaining_ > 0)
            --remaining_;
        if (feature.transient < config_.transientThreshold * .5)
            armed_ = true;
        const double magnitude = std::max(std::abs(static_cast<double>(input[0])),
                                          std::abs(static_cast<double>(input[1])));
        if (config_.eventsEnabled != 0 && armed_ && remaining_ == 0 &&
            feature.transient > config_.transientThreshold && magnitude > 0.0) {
            // Consume the family on each eligible onset, even if activity rejects the event.
            // Thus changing probability neither shifts family choices at shared onsets nor
            // retries the same transient each sample. Activity=1 preserves legacy output exactly.
            const auto family = random_.nextUInt() % detail::EventVoicePool::kFamilies;
            if (config_.eventActivity >= 1 ||
                activityRandom_.nextUnipolar() < config_.eventActivity)
                pool_.trigger(input, family);
            remaining_ = refractorySamples_;
            armed_ = false;
        }
        return pool_.process(config_.residualGain);
    }

    // Audio-owner-only scheduling gate. Existing voices and detector state are preserved.
    void setEventsEnabled(bool enabled) noexcept {
        config_.eventsEnabled = enabled ? 1 : 0;
    }

    // Audio-owner gate; pending voices/detector/family state survive an activity change.
    bool setEventActivity(double activity) noexcept {
        if (!std::isfinite(activity) || activity < 0 || activity > 1)
            return false;
        config_.eventActivity = activity;
        return true;
    }

    std::uint64_t events() const noexcept {
        return pool_.events();
    }
    std::size_t activeVoices() const noexcept {
        return pool_.activeVoices();
    }

  private:
    DropletConfig config_{};
    WaterExcitationFeatures features_;
    detail::EventVoicePool pool_;
    RandomSource random_, activityRandom_;
    RandomSource::Seed seed_{RandomSource::kDefaultSeed};
    RandomSource::Seed activitySeed_{RandomSource::kDefaultSeed};
    std::uint32_t refractorySamples_{};
    std::uint32_t remaining_{}; // Samples until another onset can trigger, cleared by reset.
    bool armed_{true}; // Re-arm below half threshold, rather than retrigger on sustained activity.
    bool ready_{};
};
} // namespace frazil::water::research
