#pragma once
#include "FlowD1Model.h"
#include "WaterDspConfig.h"

namespace frazil::water::research {
// Instance-owned virtual path; advances independently of source samples and block partition.
class FlowD1Trajectory final {
  public:
    bool prepare(const ResearchConfig& research, const FlowD1Config& config) noexcept {
        ready_ = false;
        reset();
        if (!FlowD1Model::validRate(research.sampleRateHz) || !config.valid())
            return false;
        config_ = config;
        rate_ = research.sampleRateHz;
        seed_ = research.seedFor(RandomDomain::flowD1);
        ready_ = true;
        reset();
        return true;
    }
    void reset() noexcept {
        random_.reseed(seed_);
        from_ = to_ = phase_ = step_ = path_ = 0;
        if (ready_ && config_.velocityScaleMps > 0 && config_.maxExcessPathMeters > 0)
            chooseTarget();
    }
    double process() noexcept {
        if (!ready_ || config_.velocityScaleMps == 0 || config_.maxExcessPathMeters == 0)
            return 0;
        const double q = phase_ * phase_ * (3 - 2 * phase_);
        path_ = from_ + (to_ - from_) * q;
        if (phase_ == 1) {
            from_ = to_;
            chooseTarget();
        } else {
            phase_ = std::min(1.0, phase_ + step_);
        }
        return path_;
    }
    double pathMeters() const noexcept {
        return path_;
    }

  private:
    void chooseTarget() noexcept {
        to_ = config_.maxExcessPathMeters * random_.nextUnipolar();
        phase_ = 0;
        step_ = FlowD1Model::phaseStep(config_, to_ - from_, rate_);
    }
    FlowD1Config config_{};
    RandomSource random_;
    RandomSource::Seed seed_{RandomSource::kDefaultSeed};
    double rate_{48000}, from_{}, to_{}, phase_{}, step_{}, path_{};
    bool ready_{};
};
} // namespace frazil::water::research
