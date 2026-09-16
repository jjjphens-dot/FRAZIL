#pragma once

#include "WaterExcitationFeatures.h"
#include "detail/DampedResonator.h"

#include <array>

namespace frazil::water::research {

struct ModalConfig final {
    double rootFrequencyHz{260.0};
    double decaySeconds{0.12};
    double residualGain{0.18};
};

// Research Resonant C: fixed, mildly irregular family. Ratios are experiment choices, not
// measured water modes. Coefficients are shared, channel state is isolated. Output is residual.
class LiquidModalResonator final {
  public:
    bool prepare(double rate, const ModalConfig& config = {}) noexcept {
        ready_ = false;
        reset();
        if (!std::isfinite(rate) || rate < 44100.0 || rate > 96000.0 ||
            !std::isfinite(config.rootFrequencyHz) || config.rootFrequencyHz < 40.0 ||
            config.rootFrequencyHz * kRatios.back() > 0.45 * rate ||
            !std::isfinite(config.decaySeconds) || config.decaySeconds < 0.002 ||
            config.decaySeconds > 1.0 || !std::isfinite(config.residualGain) ||
            config.residualGain < 0.0 || config.residualGain > 0.3)
            return false;
        for (std::size_t i = 0; i < kRatios.size(); ++i)
            coefficients_[i] = detail::makeResonator(rate, config.rootFrequencyHz * kRatios[i],
                                                     config.decaySeconds);
        gainPerMode_ = config.residualGain / static_cast<double>(kRatios.size());
        ready_ = true;
        return true;
    }

    void reset() noexcept {
        for (auto& channel : modes_)
            for (auto& mode : channel)
                mode.reset();
    }

    StereoFrame process(const StereoFrame& input) noexcept {
        StereoFrame output{};
        if (ready_)
            for (std::size_t channel = 0; channel < output.size(); ++channel) {
                double sum{};
                for (std::size_t i = 0; i < kRatios.size(); ++i)
                    sum += modes_[channel][i].process(input[channel], coefficients_[i]);
                output[channel] = static_cast<float>(gainPerMode_ * sum);
            }
        return output;
    }

  private:
    static constexpr std::array kRatios{1.0, 1.41, 1.93, 2.57, 3.31, 4.17};
    std::array<detail::ResonatorCoefficients, kRatios.size()> coefficients_{};
    // Each mode has l1 impulse bound <=1 via (1-r); weights sum to residualGain <= .3.
    std::array<std::array<detail::DampedResonator, kRatios.size()>, 2> modes_{};
    double gainPerMode_{};
    bool ready_{};
};
} // namespace frazil::water::research
