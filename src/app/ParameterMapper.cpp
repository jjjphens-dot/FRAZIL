#include "ParameterMapper.h"

#include <algorithm>
#include <cmath>

namespace parameter_contract = frazil::parameter_contract;

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
    if (routingIndex >= 0 && routingIndex < parameter_contract::kRoutingModeCount)
        parameters.routing = static_cast<RoutingMode>(routingIndex);

    parameters.parallelBalance = clampFinite(
        snapshot.parallelBalance, parameter_contract::kAmountRange.minimum,
        parameter_contract::kAmountRange.maximum, parameter_contract::kDefaultParallelBalance);
    parameters.waterStageAmount = clampFinite(
        snapshot.waterAmount, parameter_contract::kAmountRange.minimum,
        parameter_contract::kAmountRange.maximum, parameter_contract::kDefaultWaterAmount);
    parameters.iceStageAmount = clampFinite(
        snapshot.iceAmount, parameter_contract::kAmountRange.minimum,
        parameter_contract::kAmountRange.maximum, parameter_contract::kDefaultIceAmount);
    parameters.inputGainLinear = decibelsToLinear(clampFinite(
        snapshot.inputGainDb, parameter_contract::kGainDbRange.minimum,
        parameter_contract::kGainDbRange.maximum, parameter_contract::kDefaultInputGainDb));
    parameters.globalMix = clampFinite(snapshot.globalMix, parameter_contract::kAmountRange.minimum,
                                       parameter_contract::kAmountRange.maximum,
                                       parameter_contract::kDefaultGlobalMix);
    parameters.outputGainLinear = decibelsToLinear(clampFinite(
        snapshot.outputGainDb, parameter_contract::kGainDbRange.minimum,
        parameter_contract::kGainDbRange.maximum, parameter_contract::kDefaultOutputGainDb));
    return parameters;
}
