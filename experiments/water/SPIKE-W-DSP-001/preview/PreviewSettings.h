#pragma once

#include "ControlDescriptor.h"
#include "ProtectControls.h"

#include <array>
#include <juce_core/juce_core.h>

namespace frazil::water::preview {

inline constexpr std::array<const char*, 9> kModes{"abd", "c",  "a",  "b",       "d",
                                                   "ab",  "ad", "bd", "baseline"};

struct PreviewSettings final {
    std::array<double, kControls.size()> values{};
    int mode{}; // Index into kModes; independent of Host routing or Water product model IDs.
    ProtectSettings protect;

    PreviewSettings() {
        for (std::size_t i = 0; i < values.size(); ++i)
            values[i] = kControls[i].initial;
    }

    // Typed config -> UI value adapter. No DSP processor reads descriptor IDs or strings.
    void assignConfigs(const research::FluidConfig& fluid, const research::ModalConfig& modal,
                       const ProtectSettings& protection) {
        values = {fluid.bubble.minimumFrequencyHz,
                  fluid.bubble.maximumFrequencyHz,
                  fluid.bubble.decaySeconds,
                  fluid.bubble.maximumEventRateHz,
                  fluid.bubble.excitationThreshold,
                  fluid.bubble.residualGain,
                  static_cast<double>(fluid.bubble.voices),
                  fluid.droplet.minimumFrequencyHz,
                  fluid.droplet.maximumFrequencyHz,
                  fluid.droplet.decaySeconds,
                  fluid.droplet.transientThreshold,
                  fluid.droplet.refractorySeconds,
                  fluid.droplet.residualGain,
                  static_cast<double>(fluid.droplet.voices),
                  fluid.flow.baseDelaySeconds,
                  fluid.flow.depthSeconds,
                  fluid.flow.targetIntervalSeconds,
                  fluid.flow.residualGain,
                  modal.rootFrequencyHz,
                  modal.decaySeconds,
                  modal.residualGain,
                  modal.motionDepth,
                  modal.motionIntervalSeconds};
        protect = protection;
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
        juce::var protection(new juce::DynamicObject());
        for (const auto& spec : kProtectControls)
            protection.getDynamicObject()->setProperty(spec.key, protectValue(protect, spec.id));
        protection.getDynamicObject()->setProperty("detector",
                                                   static_cast<int>(protect.gain.score));
        protection.getDynamicObject()->setProperty("topology", static_cast<int>(protect.topology));
        root.getDynamicObject()->setProperty("protect", protection);
        return juce::JSON::toString(root, false, 17);
    }
};

enum class MonitorMode { dry, processed, residual };
} // namespace frazil::water::preview
