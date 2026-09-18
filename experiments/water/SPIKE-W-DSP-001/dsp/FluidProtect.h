#pragma once

#include "FluidCandidate.h"
#include "ResidualProtect.h"

#include <cstddef>

namespace frazil::water::research {
enum class FluidProtectTopology { whole = 1, dropletExempt = 2, dropletHalf = 3 };

// Pure residual composition. F2/F3 do not guarantee contraction of the summed residual:
// changing relative branch gains may remove cancellation. No component state is accessed.
inline StereoFrame applyFluidProtect(const FluidResiduals& parts, double gain,
                                     FluidProtectTopology topology) noexcept {
    if (gain == 1.0)
        return parts.sum();
    if (topology == FluidProtectTopology::whole)
        return ResidualProtect::apply(parts.sum(), gain);
    const double dropletGain =
        topology == FluidProtectTopology::dropletExempt ? 1.0 : 1.0 - .5 * (1.0 - gain);
    StereoFrame result{};
    for (std::size_t c = 0; c < result.size(); ++c)
        result[c] = static_cast<float>(gain * parts.bubble[c] + dropletGain * parts.droplet[c] +
                                       gain * parts.flow[c]);
    return result;
}
} // namespace frazil::water::research
