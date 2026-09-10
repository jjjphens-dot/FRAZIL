#pragma once

namespace frazil::parameter_contract {
struct FloatRange final {
    float minimum;
    float maximum;
    float step;
};

// JUCE-free Host-facing value contract. ParameterLayout owns IDs, display names, and registration
// order; app consumers use these shared ranges/defaults for mapping and state validation.
inline constexpr FloatRange kAmountRange{0.0f, 1.0f, 0.001f};
inline constexpr FloatRange kGainDbRange{-24.0f, 24.0f, 0.01f};
inline constexpr int kRoutingModeCount = 3;
inline constexpr int kDefaultRoutingModeIndex = 0;
inline constexpr bool kDefaultWaterEnabled = true;
inline constexpr bool kDefaultIceEnabled = true;
inline constexpr float kDefaultParallelBalance = 0.5f;
inline constexpr float kDefaultWaterAmount = 1.0f;
inline constexpr float kDefaultIceAmount = 1.0f;
inline constexpr float kDefaultInputGainDb = 0.0f;
inline constexpr float kDefaultGlobalMix = 1.0f;
inline constexpr float kDefaultOutputGainDb = 0.0f;
inline constexpr float kEnabledOnThreshold = 0.5f;
} // namespace frazil::parameter_contract
