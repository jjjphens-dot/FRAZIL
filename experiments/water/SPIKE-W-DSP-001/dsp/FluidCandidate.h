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
        // Clear previously enabled state even when the next prepare disables that component.
        reset();
        if (!std::isfinite(research.sampleRateHz) || research.sampleRateHz < 44100.0 ||
            research.sampleRateHz > 96000.0 ||
            (config.bubbleEnabled && !bubble_.prepare(research, config.bubble)) ||
            (config.dropletEnabled && !droplet_.prepare(research, config.droplet)) ||
            (config.flowEnabled && !flow_.prepare(research, config.flow)))
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
    StereoFrame bubbleExcitation() const noexcept {
        return ready_ && config_.bubbleEnabled ? bubble_.excitationFrame() : StereoFrame{};
    }
    StereoFrame dropletExcitation() const noexcept {
        return ready_ && config_.dropletEnabled ? droplet_.excitationFrame() : StereoFrame{};
    }
    std::uint64_t bubbleEvents() const noexcept {
        return bubble_.events();
    }
    std::uint64_t dropletEvents() const noexcept {
        return droplet_.events();
    }
    std::uint64_t bubbleSteals() const noexcept {
        return bubble_.steals();
    }
    std::size_t bubbleActive() const noexcept {
        return bubble_.activeVoices();
    }
    std::size_t dropletActive() const noexcept {
        return droplet_.activeVoices();
    }
    double flowDelaySamples() const noexcept {
        return config_.flowEnabled ? flow_.lastDelaySamples() : 0;
    }

  private:
    FluidConfig config_{};
    BubbleEnsemble bubble_;
    DropletImpactExciter droplet_;
    FlowModulator flow_;
    bool ready_{};
};
} // namespace frazil::water::research
