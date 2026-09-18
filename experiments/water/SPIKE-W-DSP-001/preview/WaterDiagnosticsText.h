#pragma once

#include "ProtectDiagnostics.h"
#include "TimeValue.h"

#include <juce_core/juce_core.h>

namespace frazil::water::preview {
// UI-thread formatting only. Levels are measured before audition boost/final monitor gain.
inline juce::String waterDiagnosticsText(const ProtectDiagnosticsSnapshot& snapshot) {
    constexpr std::array names{
        "E total / pre-Protect", "A / Bubble", "B / Droplet", "D / Flow", "C / Modal",
        "E post-Protect"};
    juce::String text = "WATER DIAGNOSTICS | pre-audition | consumed blocks " +
                        juce::String(static_cast<int>(snapshot.blocks)) + " | dropped " +
                        juce::String(static_cast<juce::int64>(snapshot.droppedBlocks)) + "\n";
    const auto level = [](double value) {
        return juce::String(value > 1e-8 ? 20 * std::log10(value) : -160., 1) + " dBFS";
    };
    for (std::size_t i = 0; i < names.size(); ++i) {
        const auto& data = snapshot.water.levels[i];
        text += juce::String(names[i]) +
                (data.samples ? " | peak " + level(data.peak) + " / RMS " + level(data.rms())
                              : " | no new samples") +
                "\n";
    }
    const auto& a = snapshot.water.latest;
    const auto count = [](std::uint64_t value) {
        return juce::String(static_cast<juce::int64>(value));
    };
    const auto time = [](double value) {
        return juce::String(formatTimeValue(value).value_or("--"));
    };
    text += "A events " + count(a.bubbleEvents) + " / active " + count(a.bubbleActive) +
            " / steals " + count(a.bubbleSteals) + " | B events " + count(a.dropletEvents) +
            " / active " + count(a.dropletActive) + " | D delay " + juce::String(a.flowDelayMs, 3) +
            " ms\n";
    text += "C root " + juce::String(a.modalRootHz, 1) + " Hz | decay " +
            time(a.modalDecaySeconds) + " | motion " + juce::String(a.modalMotionDepth, 3) + " / " +
            time(a.modalMotionIntervalSeconds) + " | Protect GR " +
            juce::String(snapshot.latest.reductionDb, 2) + " dB";
    return text;
}
} // namespace frazil::water::preview
