#pragma once

#include "dsp/FluidCandidate.h"
#include "dsp/LiquidModalResonator.h"

#include <charconv>
#include <initializer_list>
#include <juce_core/juce_core.h>
#include <limits>
#include <string_view>
#include <utility>

namespace frazil::water::research {

// Offline-only strict configuration reader. Unknown fields and nonnumeric values are errors;
// omitted values retain the versioned C++ research defaults, never production macro mappings.
// Type/representation checks apply globally; only active DSP validates semantic ranges.
inline bool validVoiceRepresentation(double voices) noexcept {
    // Comparing with 2^digits avoids rounding SIZE_MAX upward before an unsafe integer cast.
    return std::isfinite(voices) && voices >= 0.0 &&
           voices < std::ldexp(1.0, std::numeric_limits<std::size_t>::digits) &&
           std::floor(voices) == voices;
}

// JUCE accumulates integer literals into int64 without reporting overflow. Reject literals
// outside that parser representation before parsing; strings/escaped quotes are not numbers.
// Decimal/exponent values still go through the finite and size_t checks after JSON parsing.
inline bool validJsonIntegerLiterals(std::string_view json) noexcept {
    bool quoted{};
    for (std::size_t i = 0; i < json.size(); ++i) {
        if (quoted) {
            if (json[i] == '\\')
                ++i;
            else if (json[i] == '"')
                quoted = false;
        } else if (json[i] == '"') {
            quoted = true;
        } else if (json[i] == '-' || (json[i] >= '0' && json[i] <= '9')) {
            const auto start = i;
            while (i < json.size() && json[i] != ',' && json[i] != '}' && json[i] != ']' &&
                   json[i] != ' ' && json[i] != '\n' && json[i] != '\r' && json[i] != '\t')
                ++i;
            const auto token = json.substr(start, i - start);
            if (token.find_first_of(".eE") == std::string_view::npos) {
                std::int64_t integer{};
                const auto result =
                    std::from_chars(token.data(), token.data() + token.size(), integer);
                if (result.ec != std::errc{} || result.ptr != token.data() + token.size())
                    return false;
            }
            --i;
        }
    }
    return true;
}

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
    const auto text = file.loadFileAsString();
    if (!validJsonIntegerLiterals(text.toStdString()))
        return false;
    juce::var root;
    if (juce::JSON::parse(text, root).failed() || !root.isObject())
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
                !validVoiceRepresentation(voices))
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
                !validVoiceRepresentation(voices))
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
