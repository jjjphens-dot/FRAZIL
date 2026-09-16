#pragma once

#include "BubbleEnsemble.h"
#include "DropletImpactExciter.h"
#include "FlowModulator.h"

namespace frazil::water::research {

struct FluidConfig final {
    BubbleConfig bubble;
    DropletConfig droplet;
    FlowConfig flow;
    bool bubbleEnabled{true};
    bool dropletEnabled{true};
    bool flowEnabled{true};
};

struct FluidResiduals final {
    StereoFrame bubble{}, droplet{}, flow{};

    StereoFrame sum() const noexcept {
        return {static_cast<float>(static_cast<double>(bubble[0]) + droplet[0] + flow[0]),
                static_cast<float>(static_cast<double>(bubble[1]) + droplet[1] + flow[1])};
    }
};

// Experiment composition only. Flags/config are fixed at prepare, not live controls; production
// transitions are outside scope. Each enabled component retains its own RNG and detector state.
// Disabling a component does not advance it, and cannot perturb the other components.
class FluidCandidate final {
  public:
    bool prepare(const ResearchConfig& research, const FluidConfig& config = {}) {
        ready_ = false;
        config_ = config;
        if (!bubble_.prepare(research, config.bubble) ||
            !droplet_.prepare(research, config.droplet) || !flow_.prepare(research, config.flow))
            return false;
        ready_ = true;
        return true;
    }

    void reset() noexcept {
        bubble_.reset();
        droplet_.reset();
        flow_.reset();
    }

    FluidResiduals processComponents(const StereoFrame& input) noexcept {
        FluidResiduals result;
        if (ready_) {
            if (config_.bubbleEnabled)
                result.bubble = bubble_.process(input);
            if (config_.dropletEnabled)
                result.droplet = droplet_.process(input);
            if (config_.flowEnabled)
                result.flow = flow_.process(input);
        }
        return result;
    }

    StereoFrame process(const StereoFrame& input) noexcept {
        return processComponents(input).sum();
    }
    std::uint64_t bubbleEvents() const noexcept {
        return bubble_.events();
    }
    std::uint64_t dropletEvents() const noexcept {
        return droplet_.events();
    }

  private:
    FluidConfig config_{};
    BubbleEnsemble bubble_;
    DropletImpactExciter droplet_;
    FlowModulator flow_;
    bool ready_{};
};
} // namespace frazil::water::research
