#include "ParameterLayout.h"

namespace frazil::plugin {
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    using Range = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterBool>(parameterIds::waterEnabled,
                                                          "Water Enabled", true));
    layout.add(
        std::make_unique<juce::AudioParameterBool>(parameterIds::iceEnabled, "Ice Enabled", true));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        parameterIds::routingMode, "Routing Mode",
        juce::StringArray{"Parallel", "Water -> Ice", "Ice -> Water"}, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::parallelBalance, "Parallel Balance", Range{0.0f, 1.0f, 0.001f}, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::waterAmount, "Water Amount", Range{0.0f, 1.0f, 0.001f}, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(parameterIds::iceAmount, "Ice Amount",
                                                           Range{0.0f, 1.0f, 0.001f}, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::inputGain, "Input Gain", Range{-24.0f, 24.0f, 0.01f}, 0.0f,
        juce::AudioParameterFloatAttributes{}.withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(parameterIds::globalMix, "Global Mix",
                                                           Range{0.0f, 1.0f, 0.001f}, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::outputGain, "Output Gain", Range{-24.0f, 24.0f, 0.01f}, 0.0f,
        juce::AudioParameterFloatAttributes{}.withLabel("dB")));

    return layout;
}
} // namespace frazil::plugin
