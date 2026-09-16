#pragma once

#include "WaterDspConfig.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <span>

namespace frazil::water::research {

// LOCAL-WDSP-00 baseline only: emits E(x)=0. The renderer owns the single carrier addition.
// All calls belong to one processing owner; spans are borrowed for the duration of the call.
// No audio state, allocation, tail, feature extraction or sonic algorithm exists yet.
class ResearchBaseline final {
  public:
    [[nodiscard]] bool prepare(const ResearchConfig& config) noexcept {
        prepared_ = std::isfinite(config.sampleRateHz) && config.sampleRateHz >= 44100.0 &&
                    config.sampleRateHz <= 96000.0;
        return prepared_;
    }

    // There is no persistent signal state to clear. Preparation remains valid after reset.
    void reset() noexcept {}

    // Separate, equally sized spans are required; exact in-place operation is also supported.
    // Failure leaves the caller's output unchanged. Empty prepared callbacks are valid.
    [[nodiscard]] bool processResidual(std::span<const float> input,
                                       std::span<float> residual) const noexcept {
        if (!prepared_ || input.size() != residual.size())
            return false;
        std::fill(residual.begin(), residual.end(), 0.0f);
        return true;
    }

  private:
    // Lifecycle validity only; failed reprepare invalidates prior preparation.
    bool prepared_{};
};

} // namespace frazil::water::research
