#pragma once

#include <array>
#include <cstddef>
#include <string>

namespace frazil::water::preview {

enum class ControlGroup { bubble, droplet, flow, modal, protect };
enum class ControlValueType { continuous, integer };
enum class InternalUnit { hertz, seconds, eventsPerSecond, linearAmplitude, linearGain, count };
enum class DisplayPolicy { numeric, adaptiveTime };
enum class ControlLifecycle { prepareRequired, live };
enum class ControlVisibility { primary, advanced };
enum class DefaultStatus { researchBaseline };
enum class ControlId {
    bubbleMinFrequency,
    bubbleMaxFrequency,
    bubbleDecay,
    bubbleRate,
    bubbleThreshold,
    bubbleGain,
    bubbleVoices,
    dropletMinFrequency,
    dropletMaxFrequency,
    dropletDecay,
    dropletThreshold,
    dropletRefractory,
    dropletGain,
    dropletVoices,
    flowBaseDelay,
    flowDepth,
    flowTargetInterval,
    flowGain,
    modalRoot,
    modalDecay,
    modalGain,
    modalMotionDepth,
    modalMotionInterval,
    dropletEventsEnabled,
    dropletEventActivity,
    modalExcitation,
    modalNormalization,
    modalMotionModel,
    count
};

constexpr const char* moduleName(ControlGroup group) noexcept {
    switch (group) {
    case ControlGroup::bubble:
        return "bubble";
    case ControlGroup::droplet:
        return "droplet";
    case ControlGroup::flow:
        return "flow";
    case ControlGroup::modal:
        return "modal";
    case ControlGroup::protect:
        return "protect";
    }
    return "";
}

// UI metadata only. Numerical values retain their original DSP units and validated research
// ranges. Descriptors never enter DSP processing or define product macro mappings.
struct ControlDescriptor final {
    ControlId id;
    ControlGroup group;
    const char* module;
    const char* key;
    const char* label;
    double minimum, maximum, step, initial;
    InternalUnit internalUnit;
    ControlValueType valueType;
    DisplayPolicy displayPolicy;
    const char* defaultSource{"SPIKE-W-DSP-001"};
    DefaultStatus defaultStatus{DefaultStatus::researchBaseline};
    ControlLifecycle lifecycle{ControlLifecycle::prepareRequired};
    ControlVisibility visibility{ControlVisibility::primary};

    constexpr ControlDescriptor(ControlId controlId, ControlGroup controlGroup, const char* field,
                                const char* name, double low, double high, double increment,
                                double baseline, InternalUnit unit)
        : id(controlId), group(controlGroup), module(moduleName(controlGroup)), key(field),
          label(name), minimum(low), maximum(high), step(increment), initial(baseline),
          internalUnit(unit), valueType(unit == InternalUnit::count ? ControlValueType::integer
                                                                    : ControlValueType::continuous),
          displayPolicy(unit == InternalUnit::seconds ? DisplayPolicy::adaptiveTime
                                                      : DisplayPolicy::numeric) {}

    std::string stableId() const {
        return std::string(module) + "." + key;
    }
};

inline constexpr std::array<ControlDescriptor, static_cast<std::size_t>(ControlId::count)>
    kControls{{
        {ControlId::bubbleMinFrequency, ControlGroup::bubble, "minimumFrequencyHz",
         "Min frequency (Hz)", 40, 19000, 1, 250, InternalUnit::hertz},
        {ControlId::bubbleMaxFrequency, ControlGroup::bubble, "maximumFrequencyHz",
         "Max frequency (Hz)", 40, 19000, 1, 2800, InternalUnit::hertz},
        {ControlId::bubbleDecay, ControlGroup::bubble, "decaySeconds", "Decay (s)", .002, .5, .001,
         .07, InternalUnit::seconds},
        {ControlId::bubbleRate, ControlGroup::bubble, "maximumEventRateHz", "Max event rate (/s)",
         0, 2000, 1, 120, InternalUnit::eventsPerSecond},
        {ControlId::bubbleThreshold, ControlGroup::bubble, "excitationThreshold",
         "Excitation threshold", 0, 1, .0001, .0001, InternalUnit::linearAmplitude},
        {ControlId::bubbleGain, ControlGroup::bubble, "residualGain", "Residual gain", 0, .3, .001,
         .2, InternalUnit::linearGain},
        {ControlId::bubbleVoices, ControlGroup::bubble, "voices", "Voices", 1, 16, 1, 16,
         InternalUnit::count},
        {ControlId::dropletMinFrequency, ControlGroup::droplet, "minimumFrequencyHz",
         "Min frequency (Hz)", 40, 19000, 1, 600, InternalUnit::hertz},
        {ControlId::dropletMaxFrequency, ControlGroup::droplet, "maximumFrequencyHz",
         "Max frequency (Hz)", 40, 19000, 1, 4500, InternalUnit::hertz},
        {ControlId::dropletDecay, ControlGroup::droplet, "decaySeconds", "Decay (s)", .002, .1,
         .001, .012, InternalUnit::seconds},
        {ControlId::dropletThreshold, ControlGroup::droplet, "transientThreshold",
         "Transient threshold", .0001, 1, .0001, .015, InternalUnit::linearAmplitude},
        {ControlId::dropletRefractory, ControlGroup::droplet, "refractorySeconds", "Refractory (s)",
         .001, 1, .001, .02, InternalUnit::seconds},
        {ControlId::dropletGain, ControlGroup::droplet, "residualGain", "Residual gain", 0, .3,
         .001, .15, InternalUnit::linearGain},
        {ControlId::dropletVoices, ControlGroup::droplet, "voices", "Voices", 1, 16, 1, 8,
         InternalUnit::count},
        {ControlId::flowBaseDelay, ControlGroup::flow, "baseDelaySeconds", "Base delay (s)", .0001,
         .02, .0001, .004, InternalUnit::seconds},
        {ControlId::flowDepth, ControlGroup::flow, "depthSeconds", "Delay depth (s)", 0, .01, .0001,
         .001, InternalUnit::seconds},
        {ControlId::flowTargetInterval, ControlGroup::flow, "targetIntervalSeconds",
         "Target interval (s)", .02, 10, .01, .25, InternalUnit::seconds},
        {ControlId::flowGain, ControlGroup::flow, "residualGain", "Residual gain", 0, .15, .001, .1,
         InternalUnit::linearGain},
        {ControlId::modalRoot, ControlGroup::modal, "rootFrequencyHz", "Root frequency (Hz)", 40,
         4700, 1, 260, InternalUnit::hertz},
        {ControlId::modalDecay, ControlGroup::modal, "decaySeconds", "Decay (s)", .002, 1, .001,
         .12, InternalUnit::seconds},
        {ControlId::modalGain, ControlGroup::modal, "residualGain", "Residual gain", 0, .3, .001,
         .18, InternalUnit::linearGain},
        {ControlId::modalMotionDepth, ControlGroup::modal, "motionDepth", "Excitation motion depth",
         0, .35, .001, 0, InternalUnit::linearAmplitude},
        {ControlId::modalMotionInterval, ControlGroup::modal, "motionIntervalSeconds",
         "Motion interval (s)", .02, 10, .01, .7, InternalUnit::seconds},
        {ControlId::dropletEventsEnabled, ControlGroup::droplet, "eventsEnabled",
         "New events (0 off / 1 on)", 0, 1, 1, 1, InternalUnit::count},
        {ControlId::dropletEventActivity, ControlGroup::droplet, "eventActivity",
         "Onset probability (0..1)", 0, 1, .001, 1, InternalUnit::linearAmplitude},
        {ControlId::modalExcitation, ControlGroup::modal, "excitation", "Carrier (0..4)", 0, 4, 1,
         0, InternalUnit::count},
        {ControlId::modalNormalization, ControlGroup::modal, "normalization",
         "Normalization (0 C0 / 1 C3)", 0, 1, 1, 0, InternalUnit::count},
        {ControlId::modalMotionModel, ControlGroup::modal, "motionModel",
         "Motion (0 independent / 1 structured)", 0, 1, 1, 0, InternalUnit::count},
    }};

constexpr std::size_t controlIndex(ControlId id) noexcept {
    return static_cast<std::size_t>(id);
}

static_assert(
    [] {
        for (std::size_t i = 0; i < kControls.size(); ++i)
            if (controlIndex(kControls[i].id) != i)
                return false;
        return true;
    }(),
    "Every stable control ID must have one ordered descriptor");

} // namespace frazil::water::preview
