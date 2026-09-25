#pragma once
#include "FlowD1FractionalDelay.h"
#include "FlowD1Trajectory.h"

namespace frazil::water::research {
struct FlowD1Result final {
    FlowD1WideFrame transferred{}, correction{};
};
// Consumes A1+B1 emission ONLY. Carrier ownership stays with the offline composition caller.
class FlowD1 final {
  public:
    bool prepare(const ResearchConfig& research, const FlowD1Config& config = {}) noexcept {
        ready_ = false;
        reset();
        if (!trajectory_.prepare(research, config) ||
            !delay_.prepare(research.sampleRateHz, config.maxExcessPathMeters))
            return false;
        rate_ = research.sampleRateHz;
        ready_ = true;
        return true;
    }
    void reset() noexcept {
        trajectory_.reset();
        delay_.reset();
    }
    FlowD1Result process(const StereoFrame& emission) noexcept {
        if (!ready_)
            return {};
        const double path = trajectory_.process();
        FlowD1Result result;
        result.transferred = delay_.process(emission, FlowD1Model::delaySeconds(path) * rate_);
        for (std::size_t c = 0; c < 2; ++c)
            result.correction[c] = result.transferred[c] - static_cast<double>(emission[c]);
        return result;
    }
    double pathMeters() const noexcept {
        return trajectory_.pathMeters();
    }
    std::size_t drainSamples() const noexcept {
        return ready_ ? delay_.drainSamples() : 0;
    }

  private:
    FlowD1Trajectory trajectory_;
    FlowD1FractionalDelay delay_;
    double rate_{};
    bool ready_{};
};
} // namespace frazil::water::research
