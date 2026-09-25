#pragma once
#include <array>
#include <cmath>
#include <cstddef>
#include <string_view>

namespace frazil::water::research {
// Immutable numeric authority for shared analysis and A1 research configuration.
// No runtime lookup, JSON or UI dependency enters audio processing.
struct ResearchParameterSpec final {
    std::string_view name, unit, classification;
    double minimum, maximum, initial;
    std::array<double, 5> choices{};
    std::size_t choiceCount{};
    bool accepts(double value) const noexcept {
        if (!std::isfinite(value) || value < minimum || value > maximum)
            return false;
        if (!choiceCount)
            return true;
        for (std::size_t i = 0; i < choiceCount; ++i)
            if (value == choices[i])
                return true;
        return false;
    }
};
} // namespace frazil::water::research
