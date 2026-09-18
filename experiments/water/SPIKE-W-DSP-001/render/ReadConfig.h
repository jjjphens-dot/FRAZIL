#pragma once

#include "dsp/FluidCandidate.h"
#include "dsp/LiquidModalResonator.h"

#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
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

namespace detail {
// Offline syntax gate for this two-level numeric config schema, not a general JSON parser.
// JUCE accepts trailing documents and some non-JSON tokens. Validate RFC 8259 object/string/
// number grammar and consume the entire input before JUCE decodes keys and numeric values.
class ConfigJsonSyntax {
  public:
    explicit ConfigJsonSyntax(std::string_view text) noexcept : remaining_(text) {}

    bool valid() noexcept {
        const bool parsed = object(true);
        whitespace();
        return parsed && remaining_.empty();
    }

    std::size_t propertyCount() const noexcept {
        return propertyCount_;
    }

  private:
    bool take(char c) noexcept {
        if (remaining_.empty() || remaining_.front() != c)
            return false;
        remaining_.remove_prefix(1);
        return true;
    }

    void whitespace() noexcept {
        while (!remaining_.empty() &&
               std::string_view(" \t\r\n").find(remaining_.front()) != std::string_view::npos)
            remaining_.remove_prefix(1);
    }

    bool digits() noexcept {
        const auto before = remaining_.size();
        while (!remaining_.empty() && remaining_.front() >= '0' && remaining_.front() <= '9')
            remaining_.remove_prefix(1);
        return remaining_.size() != before;
    }

    bool number() noexcept {
        const auto start = remaining_;
        take('-');
        if (!take('0') && !digits())
            return false;
        bool integer = true;
        if (take('.')) {
            integer = false;
            if (!digits())
                return false;
        }
        if (take('e') || take('E')) {
            integer = false;
            if (!take('+'))
                take('-');
            if (!digits())
                return false;
        }
        // JUCE's integer accumulation wraps outside int64; keep the existing global guard.
        if (integer) {
            const auto token = start.substr(0, start.size() - remaining_.size());
            std::int64_t value{};
            const auto result = std::from_chars(token.data(), token.data() + token.size(), value);
            return result.ec == std::errc{} && result.ptr == token.data() + token.size();
        }
        return true;
    }

    bool key() noexcept {
        if (!take('"'))
            return false;
        while (!remaining_.empty()) {
            const auto c = static_cast<unsigned char>(remaining_.front());
            remaining_.remove_prefix(1);
            if (c == '"')
                return true;
            if (c < 0x20)
                return false;
            if (c != '\\')
                continue;
            if (take('u')) {
                if (remaining_.size() < 4 || remaining_.substr(0, 4) == "0000")
                    return false; // NUL cannot be part of any supported configuration key.
                for (const char hex : remaining_.substr(0, 4))
                    if (std::string_view("0123456789abcdefABCDEF").find(hex) ==
                        std::string_view::npos)
                        return false;
                remaining_.remove_prefix(4);
            } else {
                if (remaining_.empty() || std::string_view("\"\\/bfnrt").find(remaining_.front()) ==
                                              std::string_view::npos)
                    return false;
                remaining_.remove_prefix(1);
            }
        }
        return false;
    }

    bool object(bool root) noexcept {
        whitespace();
        if (!take('{'))
            return false;
        whitespace();
        if (take('}'))
            return true;
        for (;;) {
            whitespace();
            if (!key())
                return false;
            ++propertyCount_;
            whitespace();
            if (!take(':'))
                return false;
            whitespace();
            // Fixed schema depth: modules contain numbers, never another container.
            if (!(root ? object(false) : number()))
                return false;
            whitespace();
            if (take('}'))
                return true;
            if (!take(','))
                return false;
        }
    }

    // Borrowed file bytes; cursor lives only during offline validation.
    std::string_view remaining_;
    // Count both levels before JUCE can overwrite a repeated decoded key.
    std::size_t propertyCount_{};
};
} // namespace detail

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

// Non-realtime entry point shared by the offline renderer and standalone research preview.
inline bool readConfigText(std::string_view text, FluidConfig& fluid, ModalConfig& modal) {
    if (text.empty() || text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        return false;
    if (text.starts_with("\xef\xbb\xbf"))
        text.remove_prefix(3); // Tolerate a UTF-8 BOM, but never discard trailing file bytes.
    detail::ConfigJsonSyntax syntax(text);
    if (!syntax.valid() ||
        !juce::CharPointer_UTF8::isValidString(text.data(), static_cast<int>(text.size())))
        return false;
    juce::var root;
    if (juce::JSON::parse(juce::String::fromUTF8(text.data(), static_cast<int>(text.size())), root)
            .failed() ||
        !root.isObject())
        return false;
    auto decodedProperties =
        static_cast<std::size_t>(root.getDynamicObject()->getProperties().size());
    for (const auto& property : root.getDynamicObject()->getProperties()) {
        const auto* module = property.value.getDynamicObject();
        if (!module)
            return false;
        decodedProperties += static_cast<std::size_t>(module->getProperties().size());
    }
    // In this two-level schema, any duplicate loses at least one property on decoding.
    // Reject it before applying fields so it cannot hide unknown/nonfinite representations.
    if (decodedProperties != syntax.propertyCount())
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

inline bool readConfig(const juce::File& file, FluidConfig& fluid, ModalConfig& modal) {
    juce::MemoryBlock bytes;
    if (!file.existsAsFile() || !file.loadFileAsData(bytes))
        return false;
    return readConfigText({static_cast<const char*>(bytes.getData()), bytes.getSize()}, fluid,
                          modal);
}
} // namespace frazil::water::research
