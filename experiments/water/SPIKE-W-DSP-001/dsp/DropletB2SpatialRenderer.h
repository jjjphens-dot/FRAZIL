#pragma once
#include <algorithm>
#include <array>
#include <cmath>
namespace frazil::water::research {
// PRODUCT_MAPPING spatial presentation. Center remains physical; the cap is ENGINEERING.
struct DropletB2SpatialRenderer final {
    static double boundedCents(double centerHz, double requested, double maximumBeatHz) noexcept {
        return std::min(requested,
                        1200. / std::log(2.) * std::asinh(maximumBeatHz / (2 * centerHz)));
    }
    static std::array<double, 2> frequencies(double centerHz, double cents, int polarity) noexcept {
        const double ratio = std::exp2(polarity * cents / 1200.);
        return {centerHz / ratio, centerHz * ratio};
    }
};
} // namespace frazil::water::research
