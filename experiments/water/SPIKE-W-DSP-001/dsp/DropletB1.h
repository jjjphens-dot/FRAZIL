#pragma once
#include "DropletB1OnsetDetector.h"
#include "DropletB1PendingQueue.h"
#include "DropletB1VoicePool.h"

namespace frazil::water::research {
struct DropletB1Counters final {
    std::uint64_t eligible{}, admitted{}, queued{}, rejectedByAdmission{},
        droppedByPendingCapacity{};
    std::size_t pendingPeak{};
};
// Orchestration only. One audio owner; prepare/reset/retarget/process must not race.
// Returns E_B1, never a copy of the source. All storage is fixed before processing.
class DropletB1 final {
  public:
    bool prepare(const ResearchConfig& research, const DropletB1Config& config = {}) noexcept {
        ready_ = false;
        reset();
        if (!config.valid() || !std::isfinite(research.sampleRateHz) ||
            research.sampleRateHz < 44100 || research.sampleRateHz > 96000)
            return false;
        rate_ = research.sampleRateHz;
        config_ = config;
        physics_ = DropletB1Model::make(config);
        if (physics_.frequencyHz > .45 * rate_ || !analysis_.prepare(rate_))
            return false;
        detector_.prepare(rate_, config);
        entrainment_.prepare(research);
        if (!pool_.prepare(rate_, static_cast<std::size_t>(config[B1Parameter::capacity])))
            return false;
        ready_ = true;
        return true;
    }
    void reset() noexcept {
        analysis_.reset();
        detector_.reset();
        entrainment_.reset();
        pending_.reset();
        pool_.reset();
        counters_ = {};
        sample_ = 0;
        lastEligible_ = {};
        noveltyDb_ = 0;
    }
    StereoFrame process(const StereoFrame& input) noexcept {
        if (!ready_)
            return {};
        const auto onset = detector_.process(analysis_.process(input));
        noveltyDb_ = onset.noveltyDb;
        if (onset.eligible) {
            ++counters_.eligible;
            const auto impact =
                DropletB1SourceCoupler::capture(sample_, onset.noveltyDb, analysis_);
            if (entrainment_.define(lastEligible_, impact, physics_, config_, rate_,
                                    counters_.eligible)) {
                ++counters_.admitted;
                if (pending_.push(lastEligible_))
                    ++counters_.queued;
                else
                    ++counters_.droppedByPendingCapacity;
                counters_.pendingPeak = std::max(counters_.pendingPeak, pending_.size());
            } else
                ++counters_.rejectedByAdmission;
        }
        // Explicit bounded drain, even if a future fixture makes all 16 events due together.
        for (std::size_t i = 0; i < DropletB1PendingQueue::kCapacity; ++i) {
            const auto event = pending_.popDue(sample_);
            if (!event)
                break;
            pool_.request(*event);
        }
        ++sample_;
        return pool_.process();
    }
    bool setEntrainmentProbability(double probability) noexcept {
        if (!kB1Parameters[static_cast<std::size_t>(B1Parameter::admission)].accepts(probability))
            return false;
        config_[B1Parameter::admission] = probability;
        return true; // Existing queued events/tails and identity/admission streams survive.
    }
    const DropletB1Counters& counters() const noexcept {
        return counters_;
    }
    const DropletB1VoicePool& pool() const noexcept {
        return pool_;
    }
    const DropletB1Event& lastEligible() const noexcept {
        return lastEligible_;
    }
    const DropletB1PhysicalState& physics() const noexcept {
        return physics_;
    }
    double noveltyDb() const noexcept {
        return noveltyDb_;
    }

  private:
    DropletB1Config config_{};
    DropletB1PhysicalState physics_{};
    SharedExcitationAnalyzer analysis_;
    DropletB1OnsetDetector detector_;
    DropletB1EntrainmentModel entrainment_;
    DropletB1PendingQueue pending_;
    DropletB1VoicePool pool_;
    DropletB1Counters counters_{};
    DropletB1Event
        lastEligible_{}; // Includes rejected identities, inspect only on eligible count change.
    double rate_{}, noveltyDb_{};
    std::uint64_t sample_{}; // Absolute source sample since reset; independent of block boundaries.
    bool ready_{};
};
} // namespace frazil::water::research
