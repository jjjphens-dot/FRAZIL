#pragma once

#include "ParameterContract.h"

#include <atomic>

// Non-owning APVTS atomics cached by the plugin during construction. The owners outlive each
// processBlock call and the pointers are read-only from the application layer.
struct ParameterSourcePointers final {
    const std::atomic<float>* waterEnabled{};
    const std::atomic<float>* iceEnabled{};
    const std::atomic<float>* routingMode{};
    const std::atomic<float>* parallelBalance{};
    const std::atomic<float>* waterAmount{};
    const std::atomic<float>* iceAmount{};
    const std::atomic<float>* inputGainDb{};
    const std::atomic<float>* globalMix{};
    const std::atomic<float>* outputGainDb{};
};

struct ParameterSnapshot final {
    bool waterEnabled{frazil::parameter_contract::kDefaultWaterEnabled};
    bool iceEnabled{frazil::parameter_contract::kDefaultIceEnabled};
    int routingModeIndex{frazil::parameter_contract::kDefaultRoutingModeIndex};
    float parallelBalance{frazil::parameter_contract::kDefaultParallelBalance};
    float waterAmount{frazil::parameter_contract::kDefaultWaterAmount};
    float iceAmount{frazil::parameter_contract::kDefaultIceAmount};
    float inputGainDb{frazil::parameter_contract::kDefaultInputGainDb};
    float globalMix{frazil::parameter_contract::kDefaultGlobalMix};
    float outputGainDb{frazil::parameter_contract::kDefaultOutputGainDb};

    // Reads each cached source exactly once. The caller owns the atomics and must keep them
    // alive for the lifetime of the capture boundary.
    static ParameterSnapshot capture(const ParameterSourcePointers&) noexcept;
};
