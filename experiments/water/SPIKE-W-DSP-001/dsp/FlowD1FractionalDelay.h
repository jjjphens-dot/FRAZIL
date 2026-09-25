#pragma once
#include "FlowD1Model.h"
#include "WaterExcitationFeatures.h"

#include <array>
#include <cstddef>

namespace frazil::water::research {
using FlowD1WideFrame = std::array<double, 2>;
// Local causal four-tap Lagrange. Float history, double arithmetic/output avoid overflow
// for finite-float adversaries. No clipping; interpolation is not amplitude-contractive.
class FlowD1FractionalDelay final {
  public:
    static constexpr std::size_t storageSamples = 8;
    bool prepare(double rate, double maximumPathMeters) noexcept {
        ready_ = false;
        reset();
        if (!FlowD1Model::validRate(rate) || !kD1ExcessPath.accepts(maximumPathMeters))
            return false;
        maxDelay_ = FlowD1Model::delaySeconds(maximumPathMeters) * rate;
        ready_ = true;
        return true;
    }
    void reset() noexcept {
        history_ = {};
        write_ = 0;
    }
    FlowD1WideFrame process(const StereoFrame& input, double delaySamples) noexcept {
        if (!ready_ || !std::isfinite(delaySamples) || delaySamples < 0 || delaySamples > maxDelay_)
            return {};
        const auto integer = static_cast<std::size_t>(delaySamples);
        const auto base = integer > 0 ? integer - 1 : 0;
        const double d = delaySamples - static_cast<double>(base);
        const std::array<double, 4> weights{-(d - 1) * (d - 2) * (d - 3) / 6,
                                            d * (d - 2) * (d - 3) / 2, -d * (d - 1) * (d - 3) / 2,
                                            d * (d - 1) * (d - 2) / 6};
        FlowD1WideFrame out{};
        for (std::size_t channel = 0; channel < 2; ++channel) {
            history_[channel][write_] = input[channel];
            if (delaySamples == 0) {
                out[channel] = input[channel];
            } else {
                for (std::size_t tap = 0; tap < 4; ++tap)
                    out[channel] +=
                        weights[tap] *
                        history_[channel][(write_ + storageSamples - base - tap) % storageSamples];
            }
        }
        write_ = (write_ + 1) % storageSamples;
        return out;
    }
    // Conservative causal drain bound, including the interpolation support beyond tau.
    std::size_t drainSamples() const noexcept {
        return ready_ ? static_cast<std::size_t>(std::ceil(maxDelay_)) + 3 : 0;
    }

  private:
    std::array<std::array<float, storageSamples>, 2> history_{};
    std::size_t write_{};
    double maxDelay_{};
    bool ready_{};
};
} // namespace frazil::water::research
