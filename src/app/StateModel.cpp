#include "StateModel.h"

#include <cmath>

namespace {
constexpr int kRoutingModeCount = 3;
constexpr float kAmountMinimum = 0.0f;
constexpr float kAmountMaximum = 1.0f;
constexpr float kGainMinimumDb = -24.0f;
constexpr float kGainMaximumDb = 24.0f;

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
        *state.routingMode < kRoutingModeCount)
        result.values.routing = static_cast<RoutingMode>(*state.routingMode);
    else
        usedFallback = true;

    usedFallback |= !assignFiniteInRange(state.parallelBalance, kAmountMinimum, kAmountMaximum,
                                         defaults.parallelBalance, result.values.parallelBalance);
    usedFallback |= !assignFiniteInRange(state.waterAmount, kAmountMinimum, kAmountMaximum,
                                         defaults.waterAmount, result.values.waterAmount);
    usedFallback |= !assignFiniteInRange(state.iceAmount, kAmountMinimum, kAmountMaximum,
                                         defaults.iceAmount, result.values.iceAmount);
    usedFallback |= !assignFiniteInRange(state.inputGainDb, kGainMinimumDb, kGainMaximumDb,
                                         defaults.inputGainDb, result.values.inputGainDb);
    usedFallback |= !assignFiniteInRange(state.globalMix, kAmountMinimum, kAmountMaximum,
                                         defaults.globalMix, result.values.globalMix);
    usedFallback |= !assignFiniteInRange(state.outputGainDb, kGainMinimumDb, kGainMaximumDb,
                                         defaults.outputGainDb, result.values.outputGainDb);

    if (usedFallback)
        result.status = DeserializeStatus::fallback;

    return result;
}
