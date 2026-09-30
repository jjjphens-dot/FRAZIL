#pragma once
#include <algorithm>
#include <cmath>
namespace frazil::water::research {
// REDUCED_PHYSICAL_MODEL; no measured population or source-amplitude radius inference.
struct DropletB2RadiusModel final {
    static double radiusMm(double center, double spreadPct, double draw) noexcept {
        if (spreadPct == 0)
            return center;
        return std::clamp(center * std::exp((2 * draw - 1) * std::log1p(spreadPct / 100)), .2, 7.);
    }
};
} // namespace frazil::water::research
