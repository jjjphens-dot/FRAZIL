#pragma once

#include <array>
#include <juce_core/juce_core.h>

namespace frazil::water::preview {

// Application values only. These are named research controls, never product macro mappings.
struct ControlSpec {
    const char* module;
    const char* key;
    const char* label;
    double minimum, maximum, step, initial;
};

inline constexpr std::array<ControlSpec, 21> kControls{{
    {"bubble", "minimumFrequencyHz", "Min frequency (Hz)", 40, 19000, 1, 250},
    {"bubble", "maximumFrequencyHz", "Max frequency (Hz)", 40, 19000, 1, 2800},
    {"bubble", "decaySeconds", "Decay (s)", .002, .5, .001, .07},
    {"bubble", "maximumEventRateHz", "Max event rate (/s)", 0, 2000, 1, 120},
    {"bubble", "excitationThreshold", "Excitation threshold", 0, 1, .0001, .0001},
    {"bubble", "residualGain", "Residual gain", 0, .3, .001, .2},
    {"bubble", "voices", "Voices", 1, 16, 1, 16},
    {"droplet", "minimumFrequencyHz", "Min frequency (Hz)", 40, 19000, 1, 600},
    {"droplet", "maximumFrequencyHz", "Max frequency (Hz)", 40, 19000, 1, 4500},
    {"droplet", "decaySeconds", "Decay (s)", .002, .1, .001, .012},
    {"droplet", "transientThreshold", "Transient threshold", .0001, 1, .0001, .015},
    {"droplet", "refractorySeconds", "Refractory (s)", .001, 1, .001, .02},
    {"droplet", "residualGain", "Residual gain", 0, .3, .001, .15},
    {"droplet", "voices", "Voices", 1, 16, 1, 8},
    {"flow", "baseDelaySeconds", "Base delay (s)", .0001, .02, .0001, .004},
    {"flow", "depthSeconds", "Delay depth (s)", 0, .01, .0001, .001},
    {"flow", "targetIntervalSeconds", "Target interval (s)", .02, 10, .01, .25},
    {"flow", "residualGain", "Residual gain", 0, .15, .001, .1},
    {"modal", "rootFrequencyHz", "Root frequency (Hz)", 40, 4700, 1, 260},
    {"modal", "decaySeconds", "Decay (s)", .002, 1, .001, .12},
    {"modal", "residualGain", "Residual gain", 0, .3, .001, .18},
}};

inline constexpr std::array<const char*, 9> kModes{"abd", "c",  "a",  "b",       "d",
                                                   "ab",  "ad", "bd", "baseline"};

struct PreviewSettings final {
    std::array<double, kControls.size()> values{};
    int mode{}; // Index into kModes; independent of Host routing or Water product model IDs.

    PreviewSettings() {
        for (std::size_t i = 0; i < values.size(); ++i)
            values[i] = kControls[i].initial;
    }

    // Message-thread serialization, identical module keys/units to the existing renderer.
    juce::String moduleJson() const {
        juce::var root(new juce::DynamicObject());
        for (std::size_t i = 0; i < values.size(); ++i) {
            const auto& spec = kControls[i];
            if (!root.hasProperty(spec.module))
                root.getDynamicObject()->setProperty(spec.module, new juce::DynamicObject());
            root[spec.module].getDynamicObject()->setProperty(spec.key, values[i]);
        }
        return juce::JSON::toString(root, false);
    }
};

enum class MonitorMode { dry, processed, residual };
} // namespace frazil::water::preview
