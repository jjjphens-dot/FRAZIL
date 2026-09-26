#pragma once

#include "dsp/BubbleA1ConfigSpec.h"
#include "dsp/DropletB1Config.h"
#include "dsp/FlowD1Config.h"

#include <array>

namespace frazil::water::preview {
template <typename Spec, std::size_t N>
constexpr std::array<double, N> researchDefaults(const std::array<Spec, N>& specs) noexcept {
    std::array<double, N> values{};
    for (std::size_t i = 0; i < N; ++i)
        values[i] = specs[i].initial;
    return values;
}

// Message-thread value state, copied with Draft/Applied/A/B. No DSP or metadata ownership.
struct ResearchCoreTuningState final {
    std::array<double, research::kA1Parameters.size()> bubble =
        researchDefaults(research::kA1Parameters);
    std::array<double, research::kB1Parameters.size()> droplet =
        researchDefaults(research::kB1Parameters);
    std::array<double, research::kD1Parameters.size()> flow =
        researchDefaults(research::kD1Parameters);
    bool operator==(const ResearchCoreTuningState&) const = default;
};
} // namespace frazil::water::preview
