#include "StateAdapter.h"

#include "ParameterLayout.h"

#include <array>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace frazil::plugin {
namespace {
constexpr char kStateRootType[] = "FRAZIL";
constexpr char kParameterNodeType[] = "PARAM";
constexpr char kSchemaVersionProperty[] = "schemaVersion";
constexpr char kParameterIdProperty[] = "id";
constexpr char kParameterValueProperty[] = "value";
constexpr std::size_t kKnownParameterCount = 9;

enum class KnownParameter : std::uint8_t {
    none,
    waterEnabled,
    iceEnabled,
    routingMode,
    parallelBalance,
    waterAmount,
    iceAmount,
    inputGain,
    globalMix,
    outputGain,
};

bool isNativeNumeric(const juce::var& value) noexcept {
    return value.isInt() || value.isInt64() || value.isDouble();
}

// ValueTree::fromXml() recreates XML attributes as strings, so accept only the strict decimal
// representation emitted by the state writer while rejecting partial or non-finite input.
bool parseNumericString(const juce::String& text, double& result) {
    const auto* cursor = text.toRawUTF8();
    if (cursor == nullptr || *cursor == '\0')
        return false;

    if (*cursor == '+' || *cursor == '-')
        ++cursor;

    bool hasMantissaDigit = false;
    while (std::isdigit(static_cast<unsigned char>(*cursor)) != 0) {
        hasMantissaDigit = true;
        ++cursor;
    }

    if (*cursor == '.') {
        ++cursor;
        while (std::isdigit(static_cast<unsigned char>(*cursor)) != 0) {
            hasMantissaDigit = true;
            ++cursor;
        }
    }

    if (!hasMantissaDigit)
        return false;

    if (*cursor == 'e' || *cursor == 'E') {
        ++cursor;
        if (*cursor == '+' || *cursor == '-')
            ++cursor;

        bool hasExponentDigit = false;
        while (std::isdigit(static_cast<unsigned char>(*cursor)) != 0) {
            hasExponentDigit = true;
            ++cursor;
        }

        if (!hasExponentDigit)
            return false;
    }

    if (*cursor != '\0')
        return false;

    errno = 0;
    char* end = nullptr;
    result = std::strtod(text.toRawUTF8(), &end);
    return end != nullptr && *end == '\0' && errno != ERANGE;
}

bool readNumericValue(const juce::var& value, double& result) {
    if (isNativeNumeric(value)) {
        result = static_cast<double>(value);
        return true;
    }

    return value.isString() && parseNumericString(value.toString(), result);
}

KnownParameter identifyParameter(const juce::String& id) {
    if (id == parameterIds::waterEnabled || id == "water.enable")
        return KnownParameter::waterEnabled;
    if (id == parameterIds::iceEnabled || id == "ice.enable")
        return KnownParameter::iceEnabled;
    if (id == parameterIds::routingMode)
        return KnownParameter::routingMode;
    if (id == parameterIds::parallelBalance)
        return KnownParameter::parallelBalance;
    if (id == parameterIds::waterAmount)
        return KnownParameter::waterAmount;
    if (id == parameterIds::iceAmount)
        return KnownParameter::iceAmount;
    if (id == parameterIds::inputGain)
        return KnownParameter::inputGain;
    if (id == parameterIds::globalMix)
        return KnownParameter::globalMix;
    if (id == parameterIds::outputGain)
        return KnownParameter::outputGain;
    return KnownParameter::none;
}

StateModel::DeserializeResult fallbackResult() noexcept {
    StateModel::SerializedState invalid;
    invalid.schemaVersion = std::numeric_limits<std::uint32_t>::max();
    return StateModel::deserialize(invalid);
}

juce::ValueTree makeValueTree(const StateModel::SerializedState& state) {
    juce::ValueTree tree{juce::Identifier{kStateRootType}};
    tree.setProperty(
        kSchemaVersionProperty,
        static_cast<int>(state.schemaVersion.value_or(StateModel::kCurrentSchemaVersion)), nullptr);

    const auto defaults = StateModel::defaultValues();
    const auto append = [&tree](const char* id, float value) {
        juce::ValueTree parameter{juce::Identifier{kParameterNodeType}};
        parameter.setProperty(kParameterIdProperty, id, nullptr);
        parameter.setProperty(kParameterValueProperty, value, nullptr);
        tree.appendChild(parameter, nullptr);
    };

    append(parameterIds::waterEnabled,
           state.waterEnabled.value_or(defaults.waterEnabled) ? 1.0f : 0.0f);
    append(parameterIds::iceEnabled, state.iceEnabled.value_or(defaults.iceEnabled) ? 1.0f : 0.0f);
    append(parameterIds::routingMode,
           static_cast<float>(state.routingMode.value_or(static_cast<int>(defaults.routing))));
    append(parameterIds::parallelBalance, state.parallelBalance.value_or(defaults.parallelBalance));
    append(parameterIds::waterAmount, state.waterAmount.value_or(defaults.waterAmount));
    append(parameterIds::iceAmount, state.iceAmount.value_or(defaults.iceAmount));
    append(parameterIds::inputGain, state.inputGainDb.value_or(defaults.inputGainDb));
    append(parameterIds::globalMix, state.globalMix.value_or(defaults.globalMix));
    append(parameterIds::outputGain, state.outputGainDb.value_or(defaults.outputGainDb));
    return tree;
}

void setInvalidSchema(StateModel::SerializedState& state) noexcept {
    state.schemaVersion = std::numeric_limits<std::uint32_t>::max();
}
} // namespace

juce::ValueTree HostStateAdapter::serialize(juce::AudioProcessorValueTreeState& parameters) {
    // copyState() provides APVTS' coherent, lock-protected message-boundary snapshot. It is never
    // called by the audio path.
    const auto currentState = parameters.copyState();
    const auto decoded = deserialize(currentState);
    return makeValueTree(StateModel::serialize(decoded.values));
}

StateModel::DeserializeResult HostStateAdapter::deserialize(const juce::ValueTree& tree) {
    if (!tree.isValid() || !tree.hasType(juce::Identifier{kStateRootType}))
        return fallbackResult();

    StateModel::SerializedState state;
    bool parseError = false;

    if (tree.hasProperty(kSchemaVersionProperty)) {
        const auto schema = tree.getProperty(kSchemaVersionProperty);
        double value{};
        if (!readNumericValue(schema, value)) {
            parseError = true;
        } else {
            constexpr auto kMaximumSchemaVersion =
                static_cast<double>(std::numeric_limits<std::uint32_t>::max());
            if (!std::isfinite(value) || value < 0.0 || value > kMaximumSchemaVersion ||
                std::floor(value) != value) {
                parseError = true;
            } else {
                state.schemaVersion = static_cast<std::uint32_t>(value);
            }
        }
    }

    std::array<bool, kKnownParameterCount> seen{};
    for (int index = 0; index < tree.getNumChildren(); ++index) {
        const auto parameter = tree.getChild(index);
        if (!parameter.hasType(juce::Identifier{kParameterNodeType}))
            continue;

        const auto known =
            identifyParameter(parameter.getProperty(kParameterIdProperty).toString());
        if (known == KnownParameter::none)
            continue;

        const auto knownIndex = static_cast<std::size_t>(known) - 1U;
        if (seen[knownIndex]) {
            parseError = true;
            continue;
        }
        seen[knownIndex] = true;

        const auto rawValue = parameter.getProperty(kParameterValueProperty);
        double value{};
        if (!readNumericValue(rawValue, value)) {
            parseError = true;
            continue;
        }

        if (!std::isfinite(value) ||
            value < static_cast<double>(std::numeric_limits<float>::lowest()) ||
            value > static_cast<double>(std::numeric_limits<float>::max())) {
            parseError = true;
            continue;
        }

        const auto floatValue = static_cast<float>(value);
        switch (known) {
        case KnownParameter::waterEnabled:
            if (floatValue == 0.0f)
                state.waterEnabled = false;
            else if (floatValue == 1.0f)
                state.waterEnabled = true;
            else
                parseError = true;
            break;
        case KnownParameter::iceEnabled:
            if (floatValue == 0.0f)
                state.iceEnabled = false;
            else if (floatValue == 1.0f)
                state.iceEnabled = true;
            else
                parseError = true;
            break;
        case KnownParameter::routingMode:
            if (std::floor(value) == value &&
                value >= static_cast<double>(std::numeric_limits<int>::min()) &&
                value <= static_cast<double>(std::numeric_limits<int>::max()))
                state.routingMode = static_cast<int>(value);
            else
                parseError = true;
            break;
        case KnownParameter::parallelBalance:
            state.parallelBalance = floatValue;
            break;
        case KnownParameter::waterAmount:
            state.waterAmount = floatValue;
            break;
        case KnownParameter::iceAmount:
            state.iceAmount = floatValue;
            break;
        case KnownParameter::inputGain:
            state.inputGainDb = floatValue;
            break;
        case KnownParameter::globalMix:
            state.globalMix = floatValue;
            break;
        case KnownParameter::outputGain:
            state.outputGainDb = floatValue;
            break;
        case KnownParameter::none:
            break;
        }
    }

    if (parseError)
        setInvalidSchema(state);
    return StateModel::deserialize(state);
}

bool HostStateAdapter::restore(juce::AudioProcessorValueTreeState& parameters,
                               const juce::ValueTree& tree) {
    const auto decoded = deserialize(tree);
    // replaceState is a non-realtime restore boundary. Future EditHistoryManager integration must
    // clear plugin history here; Host restore is never a UI undo transaction.
    parameters.replaceState(makeValueTree(StateModel::serialize(decoded.values)));
    return decoded.status != StateModel::DeserializeStatus::fallback;
}
} // namespace frazil::plugin
