#pragma once

#include "ResearchOperationHistory.h"
#include "TimeValue.h"

namespace frazil::water::preview {
inline juce::String macroTargetsText(const ResearchSessionState& state, MacroId macro) {
    const auto value = [&](ControlId id) { return state.engineering.values[controlIndex(id)]; };
    const auto number = [&](ControlId id) {
        const auto unit = kControls[controlIndex(id)].internalUnit;
        return juce::String(
                   value(id),
                   unit == InternalUnit::hertz || unit == InternalUnit::eventsPerSecond ? 1 : 5)
            .trimCharactersAtEnd("0")
            .trimCharactersAtEnd(".");
    };
    const auto time = [&](ControlId id) {
        const double seconds = value(id);
        return juce::String(seconds >= 1 ? seconds : seconds * 1000, 3)
                   .trimCharactersAtEnd("0")
                   .trimCharactersAtEnd(".") +
               (seconds >= 1 ? " s" : " ms");
    };
    if (state.water.model == WaterModel::resonant) {
        if (macro == MacroId::size)
            return "C root: " + number(ControlId::modalRoot) + " Hz";
        if (macro == MacroId::decay)
            return "C persistence: " + time(ControlId::modalDecay);
        return "C excitation depth: " + number(ControlId::modalMotionDepth) +
               "\nC interval: " + time(ControlId::modalMotionInterval);
    }
    if (macro == MacroId::size)
        return "A: " + number(ControlId::bubbleMinFrequency) + " - " +
               number(ControlId::bubbleMaxFrequency) +
               " Hz\nB: " + number(ControlId::dropletMinFrequency) + " - " +
               number(ControlId::dropletMaxFrequency) + " Hz";
    if (macro == MacroId::decay)
        return "A: " + time(ControlId::bubbleDecay) + "\nB: " + time(ControlId::dropletDecay) +
               "\nD: no Decay destination";
    return "A: " + number(ControlId::bubbleRate) +
           "/s | B thr: " + number(ControlId::dropletThreshold) +
           "\nB gap: " + time(ControlId::dropletRefractory) +
           "\nD: " + time(ControlId::flowTargetInterval) + " / depth " + time(ControlId::flowDepth);
}
inline juce::String operationHistoryText(const ResearchOperationHistory& history) {
    juce::String text = "Completed user operations (newest first; last 50; runtime only)\n";
    const auto number = [](double value) { return juce::String(value, 6); };
    for (std::size_t reverse = history.size(); reverse > 0; --reverse) {
        const auto& op = history.at(reverse - 1);
        text += "#" + juce::String(static_cast<juce::int64>(op.sequence)) + " " +
                juce::String(op.type) + " " + juce::String(op.control) + " | " +
                originName(op.origin) + "\n";
        const auto delta = [&](const juce::String& name, double a, double b) {
            if (a != b)
                text += "  " + name + ": " + number(a) + " -> " + number(b) + "\n";
        };
        delta("Composition", op.before.engineering.mode, op.after.engineering.mode);
        delta("Size", op.before.water.size, op.after.water.size);
        delta("Motion", op.before.water.motion, op.after.water.motion);
        delta("Decay", op.before.water.decay, op.after.water.decay);
        delta("Protect Depth", op.before.engineering.protect.depth,
              op.after.engineering.protect.depth);
        delta("E Trim dB", op.before.auditionETrimDb, op.after.auditionETrimDb);
        delta("Monitor dB", op.before.monitorGainDb, op.after.monitorGainDb);
        delta("Monitor mode", static_cast<int>(op.before.monitor),
              static_cast<int>(op.after.monitor));
        delta("Detector", static_cast<int>(op.before.engineering.protect.gain.score),
              static_cast<int>(op.after.engineering.protect.gain.score));
        delta("Topology", static_cast<int>(op.before.engineering.protect.topology),
              static_cast<int>(op.after.engineering.protect.topology));
        for (const auto& control : kProtectControls)
            if (control.id != ProtectId::depth)
                delta(juce::String("protect.") + control.key,
                      protectValue(op.before.engineering.protect, control.id),
                      protectValue(op.after.engineering.protect, control.id));
        if (op.before.source != op.after.source)
            text += "  Source: " + op.before.source.name + " -> " + op.after.source.name + "\n";
        for (std::size_t i = 0; i < 3; ++i)
            if (op.before.macroMappings[i] != op.after.macroMappings[i])
                text += juce::String("  ") +
                        (i == 0   ? "Size"
                         : i == 1 ? "Motion"
                                  : "Decay") +
                        (op.after.macroMappings[i] == MappingStatus::mapped ? ": RESEARCH_MAPPED\n"
                                                                            : ": CUSTOM\n");
        if (op.before.listeningCalibration != op.after.listeningCalibration)
            text += op.after.listeningCalibration == MappingStatus::mapped
                        ? "  Calibration: MAPPED\n"
                        : "  Calibration: CUSTOM\n";
        for (const auto& descriptor : kControls) {
            const auto i = controlIndex(descriptor.id);
            delta(descriptor.stableId(), op.before.engineering.values[i],
                  op.after.engineering.values[i]);
        }
    }
    return text;
}
} // namespace frazil::water::preview
