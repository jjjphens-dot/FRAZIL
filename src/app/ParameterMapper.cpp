#include "ParameterMapper.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kAmountMinimum = 0.0f;
constexpr float kAmountMaximum = 1.0f;
constexpr float kGainMinimumDb = -24.0f;
constexpr float kGainMaximumDb = 24.0f;
constexpr float kDefaultGainDb = 0.0f;
constexpr int kRoutingModeCount = 3;
} // namespace

float ParameterMapper::clampFinite(float value, float minimum, float maximum,
                                   float fallback) noexcept {
    if (!std::isfinite(value))
        value = fallback;

    return std::clamp(value, minimum, maximum);
}

float ParameterMapper::decibelsToLinear(float decibels) noexcept {
    return std::pow(10.0f, decibels / 20.0f);
}

EngineParameters ParameterMapper::map(const ParameterSnapshot& snapshot) const noexcept {
    EngineParameters parameters;
    parameters.waterEnabled = snapshot.waterEnabled;
    parameters.iceEnabled = snapshot.iceEnabled;

    const auto routingIndex = snapshot.routingModeIndex;
    if (routingIndex >= 0 && routingIndex < kRoutingModeCount)
        parameters.routing = static_cast<RoutingMode>(routingIndex);

    parameters.parallelBalance =
        clampFinite(snapshot.parallelBalance, kAmountMinimum, kAmountMaximum, 0.5f);
    parameters.waterStageAmount =
        clampFinite(snapshot.waterAmount, kAmountMinimum, kAmountMaximum, 1.0f);
    parameters.iceStageAmount =
        clampFinite(snapshot.iceAmount, kAmountMinimum, kAmountMaximum, 1.0f);
    parameters.inputGainLinear = decibelsToLinear(
        clampFinite(snapshot.inputGainDb, kGainMinimumDb, kGainMaximumDb, kDefaultGainDb));
    parameters.globalMix = clampFinite(snapshot.globalMix, kAmountMinimum, kAmountMaximum, 1.0f);
    parameters.outputGainLinear = decibelsToLinear(
        clampFinite(snapshot.outputGainDb, kGainMinimumDb, kGainMaximumDb, kDefaultGainDb));
    return parameters;
}
