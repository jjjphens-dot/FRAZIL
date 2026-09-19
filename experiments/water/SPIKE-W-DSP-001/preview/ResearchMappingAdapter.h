#pragma once

#include "PreviewSettings.h"
#include "ResearchWaterMacroMapper.h"

namespace frazil::water::preview {
// Explicit destination ownership. Unowned engineering values (including gains, Flow base and
// Bubble sensitivity) survive any macro/Return-to-Mapped operation.
constexpr std::optional<MacroId> macroOwner(ControlId id) noexcept {
    switch (id) {
    case ControlId::bubbleMinFrequency:
    case ControlId::bubbleMaxFrequency:
    case ControlId::dropletMinFrequency:
    case ControlId::dropletMaxFrequency:
    case ControlId::modalRoot:
        return MacroId::size;
    case ControlId::bubbleRate:
    case ControlId::dropletThreshold:
    case ControlId::dropletRefractory:
    case ControlId::dropletEventsEnabled:
    case ControlId::flowDepth:
    case ControlId::flowTargetInterval:
    case ControlId::modalMotionDepth:
    case ControlId::modalMotionInterval:
        return MacroId::motion;
    case ControlId::bubbleDecay:
    case ControlId::dropletDecay:
    case ControlId::modalDecay:
        return MacroId::decay;
    default:
        return std::nullopt;
    }
}

inline double mappedTarget(ControlId id, const ResearchWaterTargets& target) noexcept {
    const auto& f = target.fluid;
    const auto& c = target.resonant;
    switch (id) {
    case ControlId::bubbleMinFrequency:
        return f.bubbleMinimumHz;
    case ControlId::bubbleMaxFrequency:
        return f.bubbleMaximumHz;
    case ControlId::dropletMinFrequency:
        return f.dropletMinimumHz;
    case ControlId::dropletMaxFrequency:
        return f.dropletMaximumHz;
    case ControlId::modalRoot:
        return c.rootHz;
    case ControlId::modalMotionDepth:
        return c.motionDepth;
    case ControlId::modalMotionInterval:
        return c.motionIntervalSeconds;
    case ControlId::bubbleRate:
        return f.bubbleRateHz;
    case ControlId::dropletEventsEnabled:
        return f.dropletEventsEnabled;
    case ControlId::dropletThreshold:
        return f.dropletThreshold;
    case ControlId::dropletRefractory:
        return f.dropletRefractorySeconds;
    case ControlId::flowDepth:
        return f.flowDepthSeconds;
    case ControlId::flowTargetInterval:
        return f.flowIntervalSeconds;
    case ControlId::bubbleDecay:
        return f.bubbleDecaySeconds;
    case ControlId::dropletDecay:
        return f.dropletDecaySeconds;
    case ControlId::modalDecay:
        return c.decaySeconds;
    default:
        return 0; // Callers must check ownership first.
    }
}
inline void applyResearchMacro(PreviewSettings& settings, const WaterExperimentState& state,
                               MacroId macro) {
    const auto targets = ResearchWaterMacroMapper::map(state);
    if (!targets)
        return;
    for (const auto& descriptor : kControls)
        if (macroOwner(descriptor.id) == macro)
            settings.values[controlIndex(descriptor.id)] = mappedTarget(descriptor.id, *targets);
}
} // namespace frazil::water::preview
