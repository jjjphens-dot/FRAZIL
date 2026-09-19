#pragma once

#include "PreviewSettings.h"

namespace frazil::water::preview {
// Independent listening starting point, not a macro curve or renderer/production default.
struct ResearchListeningCalibration final {
    static constexpr const char* revision = "Research Listening Calibration v0.1";
    static constexpr std::array controls{ControlId::bubbleGain, ControlId::dropletGain,
                                         ControlId::flowGain, ControlId::modalGain};
    static constexpr std::array values{.26, .24, .06, .30};
    static constexpr bool owns(ControlId id) noexcept {
        for (const auto gain : controls)
            if (id == gain)
                return true;
        return false;
    }
    static void apply(PreviewSettings& settings) noexcept {
        for (std::size_t i = 0; i < controls.size(); ++i)
            settings.values[controlIndex(controls[i])] = values[i];
    }
    static bool matches(const PreviewSettings& settings) noexcept {
        for (std::size_t i = 0; i < controls.size(); ++i)
            if (settings.values[controlIndex(controls[i])] != values[i])
                return false;
        return true;
    }
};
} // namespace frazil::water::preview
