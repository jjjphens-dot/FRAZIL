#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace frazil::plugin {
namespace parameterIds {
inline constexpr char kWaterEnabled[] = "water.enabled";
inline constexpr char kIceEnabled[] = "ice.enabled";
inline constexpr char kRoutingMode[] = "routing.mode";
inline constexpr char kParallelBalance[] = "parallel.balance";
inline constexpr char kWaterAmount[] = "water.amount";
inline constexpr char kIceAmount[] = "ice.amount";
inline constexpr char kInputGain[] = "input.gain";
inline constexpr char kGlobalMix[] = "global.mix";
inline constexpr char kOutputGain[] = "output.gain";
} // namespace parameterIds

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
} // namespace frazil::plugin
