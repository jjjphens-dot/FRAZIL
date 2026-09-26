#pragma once

#include "ResearchSessionModel.h"
#include "TimeValue.h"

namespace frazil::water::preview {
// Message-thread presentation of pending values; no second draft owner.
inline juce::String draftSummary(const ResearchSessionModel& session) {
    const auto& a = session.applied();
    const auto& d = session.draft();
    juce::String text;
    if (a.engineering.core != d.engineering.core)
        text += juce::String("Core Revision: ") + coreName(a.engineering.core) + " -> " +
                coreName(d.engineering.core) + "\n";
    const auto line = [&](const juce::String& name, double oldValue, double value,
                          bool time = false) {
        if (oldValue == value)
            return;
        const auto format = [time](double number) {
            return time ? juce::String(formatTimeValue(number).value_or("invalid"))
                        : juce::String(number, 9);
        };
        text += name + ": " + format(oldValue) + " -> " + format(value) + "\n";
    };
    if (a.engineering.mode != d.engineering.mode)
        text += juce::String("Composition: ") +
                kModes[static_cast<std::size_t>(a.engineering.mode)] + " -> " +
                kModes[static_cast<std::size_t>(d.engineering.mode)] + "\n";
    line("Size", a.water.size, d.water.size);
    line("Motion", a.water.motion, d.water.motion);
    line("Decay", a.water.decay, d.water.decay);
    for (std::size_t i = 0; i < kControls.size(); ++i)
        line(kControls[i].stableId(), a.engineering.values[i], d.engineering.values[i],
             kControls[i].displayPolicy == DisplayPolicy::adaptiveTime);
    for (const auto& spec : kProtectControls)
        line(juce::String("protect.") + spec.key, protectValue(a.engineering.protect, spec.id),
             protectValue(d.engineering.protect, spec.id), spec.unit == ProtectUnit::seconds);
    if (a.engineering.protect.gain.score != d.engineering.protect.gain.score)
        text += "Protect detector changed (each candidate retains its own calibration).\n";
    if (a.engineering.protect.topology != d.engineering.protect.topology)
        text += "Protect topology changed.\n";
    // These retained values can differ even after switching back to the applied detector.
    const auto& oldInactive = d.engineering.protect.gain.score == research::ProtectScore::difference
                                  ? a.protectMemory.logRatio
                                  : a.protectMemory.difference;
    const auto& newInactive = d.engineering.protect.gain.score == research::ProtectScore::difference
                                  ? d.protectMemory.logRatio
                                  : d.protectMemory.difference;
    line("Retained inactive detector Low", oldInactive.low, newInactive.low);
    line("Retained inactive detector High", oldInactive.high, newInactive.high);
    line("Retained Enable depth", a.protectMemory.lastNonzeroDepth,
         d.protectMemory.lastNonzeroDepth);
    if (a.protectMemory.fluidTopology != d.protectMemory.fluidTopology)
        text += "Retained Fluid topology changed.\n";
    return text.isEmpty() ? "No unapplied value changes." : text;
}
} // namespace frazil::water::preview
