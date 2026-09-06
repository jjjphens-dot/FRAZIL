#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace frazil::plugin {
namespace parameterIds {
inline constexpr char waterEnabled[] = "water.enabled";
inline constexpr char iceEnabled[] = "ice.enabled";
inline constexpr char routingMode[] = "routing.mode";
inline constexpr char parallelBalance[] = "parallel.balance";
inline constexpr char waterAmount[] = "water.amount";
inline constexpr char iceAmount[] = "ice.amount";
inline constexpr char inputGain[] = "input.gain";
inline constexpr char globalMix[] = "global.mix";
inline constexpr char outputGain[] = "output.gain";
} // namespace parameterIds

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
} // namespace frazil::plugin
