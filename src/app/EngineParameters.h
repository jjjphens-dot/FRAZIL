#pragma once

#include <cstdint>

enum class RoutingMode : std::uint8_t { parallel = 0, waterIntoIce, iceIntoWater };

struct WaterParameters final {};

struct IceParameters final {};

struct EngineParameters final {
    bool waterEnabled{true};
    bool iceEnabled{true};
    RoutingMode routing{RoutingMode::parallel};

    float parallelBalance{0.5f};
    float waterStageAmount{1.0f};
    float iceStageAmount{1.0f};

    WaterParameters water{};
    IceParameters ice{};

    float inputGainLinear{1.0f};
    float globalMix{1.0f};
    float outputGainLinear{1.0f};
};
