#include "StateModel.h"

#include <cmath>

namespace {
bool hasAnyParameter(const StateModel::SerializedState& state) noexcept {
    return state.waterEnabled.has_value() || state.iceEnabled.has_value() ||
           state.routingMode.has_value() || state.parallelBalance.has_value() ||
           state.waterAmount.has_value() || state.iceAmount.has_value() ||
           state.inputGainDb.has_value() || state.globalMix.has_value() ||
           state.outputGainDb.has_value();
}

bool assignFiniteInRange(const std::optional<float>& source, float minimum, float maximum,
                         float fallback, float& destination) noexcept {
    if (!source.has_value() || !std::isfinite(*source) || *source < minimum || *source > maximum) {
        destination = fallback;
        return false;
    }

    destination = *source;
    return true;
}
} // namespace

StateModel::Values StateModel::defaultValues() noexcept {
    return {};
}

StateModel::SerializedState StateModel::serialize(const Values& values) noexcept {
    SerializedState state;
    state.schemaVersion = kCurrentSchemaVersion;
    state.waterEnabled = values.waterEnabled;
    state.iceEnabled = values.iceEnabled;
    state.routingMode = static_cast<int>(values.routing);
    state.parallelBalance = values.parallelBalance;
    state.waterAmount = values.waterAmount;
    state.iceAmount = values.iceAmount;
    state.inputGainDb = values.inputGainDb;
    state.globalMix = values.globalMix;
    state.outputGainDb = values.outputGainDb;
    return state;
}

StateModel::DeserializeResult StateModel::deserialize(const SerializedState& state) noexcept {
    const auto defaults = defaultValues();
    DeserializeResult result{defaults, DeserializeStatus::fallback};

    const bool isLegacy =
        !state.schemaVersion.has_value() || *state.schemaVersion == kLegacySchemaVersion;
    if (!isLegacy && *state.schemaVersion != kCurrentSchemaVersion)
        return result;

    // A schema-less state with known parameter content is the pre-v1 migration entry. An empty
    // schema-less envelope is malformed input and therefore uses the complete safe default.
    if (isLegacy && !hasAnyParameter(state))
        return result;

    result.status = isLegacy ? DeserializeStatus::migrated : DeserializeStatus::current;
    bool usedFallback = false;

    if (state.waterEnabled.has_value())
        result.values.waterEnabled = *state.waterEnabled;
    else
        usedFallback = true;

    if (state.iceEnabled.has_value())
        result.values.iceEnabled = *state.iceEnabled;
    else
        usedFallback = true;

    if (state.routingMode.has_value() && *state.routingMode >= 0 &&
        *state.routingMode < frazil::parameter_contract::kRoutingModeCount)
        result.values.routing = static_cast<RoutingMode>(*state.routingMode);
    else
        usedFallback = true;

    usedFallback |= !assignFiniteInRange(state.parallelBalance,
                                         frazil::parameter_contract::kAmountRange.minimum,
                                         frazil::parameter_contract::kAmountRange.maximum,
                                         defaults.parallelBalance, result.values.parallelBalance);
    usedFallback |=
        !assignFiniteInRange(state.waterAmount, frazil::parameter_contract::kAmountRange.minimum,
                             frazil::parameter_contract::kAmountRange.maximum, defaults.waterAmount,
                             result.values.waterAmount);
    usedFallback |=
        !assignFiniteInRange(state.iceAmount, frazil::parameter_contract::kAmountRange.minimum,
                             frazil::parameter_contract::kAmountRange.maximum, defaults.iceAmount,
                             result.values.iceAmount);
    usedFallback |=
        !assignFiniteInRange(state.inputGainDb, frazil::parameter_contract::kGainDbRange.minimum,
                             frazil::parameter_contract::kGainDbRange.maximum, defaults.inputGainDb,
                             result.values.inputGainDb);
    usedFallback |=
        !assignFiniteInRange(state.globalMix, frazil::parameter_contract::kAmountRange.minimum,
                             frazil::parameter_contract::kAmountRange.maximum, defaults.globalMix,
                             result.values.globalMix);
    usedFallback |=
        !assignFiniteInRange(state.outputGainDb, frazil::parameter_contract::kGainDbRange.minimum,
                             frazil::parameter_contract::kGainDbRange.maximum,
                             defaults.outputGainDb, result.values.outputGainDb);

    if (usedFallback)
        result.status = DeserializeStatus::fallback;

    return result;
}
