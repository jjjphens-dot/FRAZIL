#include "ParameterLayout.h"

#include "../app/ParameterContract.h"

namespace frazil::plugin {
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    using namespace frazil::parameter_contract;
    using Range = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterBool>(parameterIds::kWaterEnabled,
                                                          "Water Enabled", kDefaultWaterEnabled));
    layout.add(std::make_unique<juce::AudioParameterBool>(parameterIds::kIceEnabled, "Ice Enabled",
                                                          kDefaultIceEnabled));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        parameterIds::kRoutingMode, "Routing Mode",
        juce::StringArray{"Parallel", "Water -> Ice", "Ice -> Water"}, kDefaultRoutingModeIndex));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::kParallelBalance, "Parallel Balance",
        Range{kAmountRange.minimum, kAmountRange.maximum, kAmountRange.step},
        kDefaultParallelBalance));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::kWaterAmount, "Water Amount",
        Range{kAmountRange.minimum, kAmountRange.maximum, kAmountRange.step}, kDefaultWaterAmount));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::kIceAmount, "Ice Amount",
        Range{kAmountRange.minimum, kAmountRange.maximum, kAmountRange.step}, kDefaultIceAmount));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::kInputGain, "Input Gain",
        Range{kGainDbRange.minimum, kGainDbRange.maximum, kGainDbRange.step}, kDefaultInputGainDb,
        juce::AudioParameterFloatAttributes{}.withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::kGlobalMix, "Global Mix",
        Range{kAmountRange.minimum, kAmountRange.maximum, kAmountRange.step}, kDefaultGlobalMix));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::kOutputGain, "Output Gain",
        Range{kGainDbRange.minimum, kGainDbRange.maximum, kGainDbRange.step}, kDefaultOutputGainDb,
        juce::AudioParameterFloatAttributes{}.withLabel("dB")));

    return layout;
}
} // namespace frazil::plugin
