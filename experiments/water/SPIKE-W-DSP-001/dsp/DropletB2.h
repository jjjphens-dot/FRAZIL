#pragma once
#include "DropletB2OnsetDetector.h"
#include "DropletB2PendingQueue.h"
#include "DropletB2RadiusModel.h"
#include "DropletB2VoicePool.h"
namespace frazil::water::research {
struct DropletB2Counters final {
    std::uint64_t eligible{}, admitted{}, queued{}, rejectedByAdmission{},
        droppedByPendingCapacity{};
};
// Bounded source-linked B2 orchestration. All processing state has one audio owner.
class DropletB2 final {
  public:
    bool prepare(const ResearchConfig& research, const DropletB2Config& config = {}) noexcept {
        ready_ = false;
        reset();
        config_ = config;
        rate_ = research.sampleRateHz;
        if (!config.valid() || !std::isfinite(rate_) || rate_ < 44100 || rate_ > 96000)
            return false;
        baseline_ = config.baseline();
        detector_.prepare(rate_, config);
        entrainment_.prepare(research); // Same admission/identity domains permit exact B1 ablation.
        radiusSeed_ = research.seedFor(RandomDomain::dropletB2Radius);
        spatialSeed_ = research.seedFor(RandomDomain::dropletB2Spatial);
        ready_ = analysis_.prepare(rate_) &&
                 pool_.prepare(rate_, static_cast<std::size_t>(baseline_[B1Parameter::capacity]));
        reset();
        return ready_;
    }
    void reset() noexcept {
        analysis_.reset();
        detector_.reset();
        entrainment_.reset();
        pending_.reset();
        pool_.reset();
        radius_.reseed(radiusSeed_);
        spatial_.reseed(spatialSeed_);
        counters_ = {};
        sample_ = 0;
        last_ = {};
        novelty_ = slope_ = 0;
    }
    StereoFrame process(const StereoFrame& input) noexcept {
        if (!ready_)
            return {};
        const auto onset = detector_.process(analysis_.process(input));
        novelty_ = onset.noveltyDb;
        slope_ = onset.positiveSlope;
        if (onset.eligible) {
            ++counters_.eligible;
            last_ = {};
            const auto impact = DropletB1SourceCoupler::capture(sample_, novelty_, analysis_);
            const bool admitted =
                entrainment_.define(last_.center, impact, DropletB1Model::make(baseline_),
                                    baseline_, rate_, counters_.eligible);
            last_.sourceExcitation = impact.sourceExcitation;
            last_.mappedExcitation = config_[B2Parameter::gamma] == 1
                                         ? impact.sourceExcitation
                                         : std::pow(std::clamp(impact.sourceExcitation, 0., 1.),
                                                    config_[B2Parameter::gamma]);
            last_.positiveSlope = slope_;
            last_.releaseMs = last_.center.releaseMs;
            last_.dueSample = last_.center.dueSample;
            if (admitted) {
                ++counters_.admitted;
                auto physicalConfig = baseline_;
                physicalConfig[B1Parameter::radius] = DropletB2RadiusModel::radiusMm(
                    baseline_[B1Parameter::radius], config_[B2Parameter::radiusSpread],
                    radius_.nextUnipolar());
                last_.center.physics = DropletB1Model::make(physicalConfig);
                last_.polarity = spatial_.nextUnipolar() < .5 ? -1 : 1;
                last_.detuneCents = config_[B2Parameter::detune];
                last_.maximumBeatHz = config_[B2Parameter::beat];
                if (pending_.push(last_))
                    ++counters_.queued;
                else
                    ++counters_.droppedByPendingCapacity;
            } else
                ++counters_.rejectedByAdmission;
        }
        for (std::size_t i = 0; i < DropletB2PendingQueue::kCapacity; ++i) {
            const auto e = pending_.popDue(sample_);
            if (!e)
                break;
            pool_.request(*e);
        }
        ++sample_;
        return pool_.process();
    }
    const DropletB2Counters& counters() const noexcept {
        return counters_;
    }
    const DropletB2VoicePool& pool() const noexcept {
        return pool_;
    }
    const DropletB2Event& lastEligible() const noexcept {
        return last_;
    }
    double noveltyDb() const noexcept {
        return novelty_;
    }
    double positiveSlope() const noexcept {
        return slope_;
    }

  private:
    DropletB2Config config_;
    DropletB1Config baseline_;
    SharedExcitationAnalyzer analysis_;
    DropletB2OnsetDetector detector_;
    DropletB1EntrainmentModel entrainment_;
    DropletB2PendingQueue pending_;
    DropletB2VoicePool pool_;
    DropletB2Counters counters_;
    DropletB2Event last_{};
    RandomSource radius_, spatial_; // Independent event-only draws; reset restarts both domains.
    RandomSource::Seed radiusSeed_{}, spatialSeed_{};
    double rate_{}, novelty_{}, slope_{};
    std::uint64_t sample_{};
    bool ready_{};
};
} // namespace frazil::water::research
