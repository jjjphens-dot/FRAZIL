#pragma once

#include "dsp/FluidCandidate.h"
#include "dsp/LiquidModalResonator.h"

#include <initializer_list>
#include <juce_core/juce_core.h>
#include <utility>

namespace frazil::water::research {

// Offline-only strict configuration reader. Unknown fields and nonnumeric values are errors;
// omitted values retain the versioned C++ research defaults, never production macro mappings.
inline bool readNumbers(const juce::var& value,
                        std::initializer_list<std::pair<const char*, double*>> fields) {
    auto* object = value.getDynamicObject();
    if (!object)
        return false;
    for (const auto& property : object->getProperties()) {
        bool found{};
        for (const auto& [name, destination] : fields)
            if (property.name.toString() == name) {
                if (!property.value.isInt() && !property.value.isInt64() &&
                    !property.value.isDouble())
                    return false;
                *destination = static_cast<double>(property.value);
                if (!std::isfinite(*destination))
                    return false;
                found = true;
            }
        if (!found)
            return false;
    }
    return true;
}

inline bool readConfig(const juce::File& file, FluidConfig& fluid, ModalConfig& modal) {
    if (!file.existsAsFile())
        return false;
    juce::var root;
    if (juce::JSON::parse(file.loadFileAsString(), root).failed() || !root.isObject())
        return false;
    for (const auto& property : root.getDynamicObject()->getProperties()) {
        const auto name = property.name.toString();
        if (name == "bubble") {
            auto& c = fluid.bubble;
            double voices = static_cast<double>(c.voices);
            if (!readNumbers(property.value, {{"minimumFrequencyHz", &c.minimumFrequencyHz},
                                              {"maximumFrequencyHz", &c.maximumFrequencyHz},
                                              {"decaySeconds", &c.decaySeconds},
                                              {"maximumEventRateHz", &c.maximumEventRateHz},
                                              {"excitationThreshold", &c.excitationThreshold},
                                              {"residualGain", &c.residualGain},
                                              {"voices", &voices}}) ||
                voices < 1 || voices > 16 || std::floor(voices) != voices)
                return false;
            c.voices = static_cast<std::size_t>(voices);
        } else if (name == "droplet") {
            auto& c = fluid.droplet;
            double voices = static_cast<double>(c.voices);
            if (!readNumbers(property.value, {{"minimumFrequencyHz", &c.minimumFrequencyHz},
                                              {"maximumFrequencyHz", &c.maximumFrequencyHz},
                                              {"decaySeconds", &c.decaySeconds},
                                              {"transientThreshold", &c.transientThreshold},
                                              {"refractorySeconds", &c.refractorySeconds},
                                              {"residualGain", &c.residualGain},
                                              {"voices", &voices}}) ||
                voices < 1 || voices > 16 || std::floor(voices) != voices)
                return false;
            c.voices = static_cast<std::size_t>(voices);
        } else if (name == "flow") {
            auto& c = fluid.flow;
            if (!readNumbers(property.value, {{"baseDelaySeconds", &c.baseDelaySeconds},
                                              {"depthSeconds", &c.depthSeconds},
                                              {"targetIntervalSeconds", &c.targetIntervalSeconds},
                                              {"residualGain", &c.residualGain}}))
                return false;
        } else if (name == "modal") {
            if (!readNumbers(property.value, {{"rootFrequencyHz", &modal.rootFrequencyHz},
                                              {"decaySeconds", &modal.decaySeconds},
                                              {"residualGain", &modal.residualGain}}))
                return false;
        } else {
            return false;
        }
    }
    return true;
}
} // namespace frazil::water::research
