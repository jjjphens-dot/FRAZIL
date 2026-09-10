#pragma once

#include "ParameterContract.h"

#include <cstdint>

enum class RoutingMode : std::uint8_t { parallel = 0, waterIntoIce, iceIntoWater };

struct WaterParameters final {};

struct IceParameters final {};

struct EngineParameters final {
    bool waterEnabled{frazil::parameter_contract::kDefaultWaterEnabled};
    bool iceEnabled{frazil::parameter_contract::kDefaultIceEnabled};
    RoutingMode routing{
        static_cast<RoutingMode>(frazil::parameter_contract::kDefaultRoutingModeIndex)};

    float parallelBalance{frazil::parameter_contract::kDefaultParallelBalance};
    float waterStageAmount{frazil::parameter_contract::kDefaultWaterAmount};
    float iceStageAmount{frazil::parameter_contract::kDefaultIceAmount};

    WaterParameters water{};
    IceParameters ice{};

    float inputGainLinear{1.0f};
    float globalMix{frazil::parameter_contract::kDefaultGlobalMix};
    float outputGainLinear{1.0f};
};
