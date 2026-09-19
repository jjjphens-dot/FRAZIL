#pragma once
#include "dsp/WaterExcitationFeatures.h"

#include <array>
#include <string_view>
namespace frazil::water::preview {
// Temporary monitor selection, never a composition/config/session/Host parameter.
enum class DiagnosticSignal : std::size_t {
    none,
    bubble,
    droplet,
    flow,
    modal,
    bubbleDriver,
    dropletDriver,
    modalDriver,
    count
};
inline constexpr std::array kDiagnosticLabels{
    "Normal monitoring",        "Solo Bubble (pre-Protect)",   "Solo Droplet (pre-Protect)",
    "Solo Flow (pre-Protect)",  "Solo Modal (pre-Protect)",    "Audition Bubble trigger",
    "Audition Droplet trigger", "Audition Resonant excitation"};
static_assert(kDiagnosticLabels.size() == static_cast<std::size_t>(DiagnosticSignal::count));
constexpr bool diagnosticAvailable(std::string_view mode, DiagnosticSignal signal) noexcept {
    if (signal == DiagnosticSignal::none)
        return true;
    if (mode == "baseline")
        return false;
    switch (signal) {
    case DiagnosticSignal::bubble:
    case DiagnosticSignal::bubbleDriver:
        return mode.find('a') != std::string_view::npos;
    case DiagnosticSignal::droplet:
    case DiagnosticSignal::dropletDriver:
        return mode.find('b') != std::string_view::npos;
    case DiagnosticSignal::flow:
        return mode.find('d') != std::string_view::npos;
    case DiagnosticSignal::modal:
    case DiagnosticSignal::modalDriver:
        return mode == "c";
    default:
        return false;
    }
}
struct DiagnosticFrames final {
    std::array<research::StereoFrame, static_cast<std::size_t>(DiagnosticSignal::count)> signals{};
    research::StereoFrame& at(DiagnosticSignal signal) noexcept {
        return signals[static_cast<std::size_t>(signal)];
    }
};
constexpr bool isDriver(DiagnosticSignal signal) noexcept {
    return signal >= DiagnosticSignal::bubbleDriver && signal <= DiagnosticSignal::modalDriver;
}
} // namespace frazil::water::preview
