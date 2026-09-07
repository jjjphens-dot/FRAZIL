#include "plugin/ParameterLayout.h"
#include "plugin/PluginProcessor.h"

#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <juce_audio_basics/juce_audio_basics.h>

namespace {
int failures = 0;

void expect(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

void expectNear(float actual, float expected, float tolerance, const char* description) {
    expect(std::abs(actual - expected) <= tolerance, description);
}

void setParameterValue(FRAZILAudioProcessor& processor, const char* id, float value) {
    auto* parameter = processor.parameters.getParameter(id);
    expect(parameter != nullptr, "integration parameter exists");
    if (parameter != nullptr) {
        parameter->setValueNotifyingHost(
            processor.parameters.getParameterRange(id).convertTo0to1(value));
    }
}

float getParameterValue(const FRAZILAudioProcessor& processor, const char* id) {
    const auto* value = processor.parameters.getRawParameterValue(id);
    expect(value != nullptr, "integration parameter value exists");
    return value != nullptr ? value->load(std::memory_order_relaxed) : 0.0f;
}

void fillBuffer(juce::AudioBuffer<float>& buffer, float value) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, value);
}

void testParameterAutomationReachesAudioPath() {
    FRAZILAudioProcessor processor;
    constexpr int kBlockSize = 64;
    processor.prepareToPlay(48000.0, kBlockSize);

    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buffer(2, kBlockSize);
    fillBuffer(buffer, 1.0f);
    processor.processBlock(buffer, midi);
    expectNear(buffer.getSample(0, kBlockSize - 1), 1.0f, 1.0e-6f,
               "default plugin block is unity pass-through");

    constexpr float kTargetGainDb = 6.0f;
    const auto targetGain = std::pow(10.0f, kTargetGainDb / 20.0f);
    setParameterValue(processor, frazil::plugin::parameterIds::inputGain, kTargetGainDb);

    fillBuffer(buffer, 1.0f);
    processor.processBlock(buffer, midi);
    const auto firstRampedSample = buffer.getSample(0, 0);
    const auto lastRampedSample = buffer.getSample(0, kBlockSize - 1);
    expect(firstRampedSample > 1.0f && firstRampedSample < targetGain,
           "host parameter write enters the next block's input-gain ramp");
    expect(lastRampedSample > firstRampedSample && lastRampedSample < targetGain,
           "input-gain ramp progresses sample by sample within a block");

    for (int block = 0; block < 7; ++block) {
        fillBuffer(buffer, 1.0f);
        processor.processBlock(buffer, midi);
    }
    expectNear(buffer.getSample(0, kBlockSize - 1), targetGain, 1.0e-5f,
               "automated input gain reaches its final value without restarting the ramp");
}

void testModeSwitchRetainsInactiveValuesAcrossStateReopen() {
    constexpr int kBlockSize = 64;
    FRAZILAudioProcessor source;
    source.prepareToPlay(48000.0, kBlockSize);
    setParameterValue(source, frazil::plugin::parameterIds::waterEnabled, 0.0f);
    setParameterValue(source, frazil::plugin::parameterIds::iceEnabled, 1.0f);
    setParameterValue(source, frazil::plugin::parameterIds::parallelBalance, 0.2f);
    setParameterValue(source, frazil::plugin::parameterIds::waterAmount, 0.35f);
    setParameterValue(source, frazil::plugin::parameterIds::iceAmount, 0.8f);
    setParameterValue(source, frazil::plugin::parameterIds::inputGain, -3.0f);
    setParameterValue(source, frazil::plugin::parameterIds::globalMix, 0.6f);
    setParameterValue(source, frazil::plugin::parameterIds::outputGain, 4.0f);

    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buffer(2, kBlockSize);
    constexpr std::array<float, 4> routingModes{0.0f, 1.0f, 2.0f, 0.0f};
    constexpr std::array<float, 4> waterAmounts{0.35f, 0.45f, 0.65f, 0.35f};
    constexpr std::array<float, 4> iceAmounts{0.8f, 0.55f, 0.25f, 0.8f};
    for (std::size_t index = 0; index < routingModes.size(); ++index) {
        setParameterValue(source, frazil::plugin::parameterIds::routingMode, routingModes[index]);
        setParameterValue(source, frazil::plugin::parameterIds::waterAmount, waterAmounts[index]);
        setParameterValue(source, frazil::plugin::parameterIds::iceAmount, iceAmounts[index]);
        fillBuffer(buffer, 1.0f);
        source.processBlock(buffer, midi);

        expectNear(getParameterValue(source, frazil::plugin::parameterIds::waterAmount),
                   waterAmounts[index], 1.0e-6f, "Water amount survives every routing mode switch");
        expectNear(getParameterValue(source, frazil::plugin::parameterIds::iceAmount),
                   iceAmounts[index], 1.0e-6f, "Ice amount survives every routing mode switch");
    }

    juce::MemoryBlock serializedState;
    source.getStateInformation(serializedState);
    expect(serializedState.getSize() > 0, "plugin state save produces an XML payload");

    FRAZILAudioProcessor restored;
    restored.setStateInformation(serializedState.getData(),
                                 static_cast<int>(serializedState.getSize()));
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::waterEnabled), 0.0f,
               1.0e-6f, "state reopen retains Water enable");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::iceEnabled), 1.0f, 1.0e-6f,
               "state reopen retains Ice enable");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::routingMode), 0.0f,
               1.0e-6f, "state reopen retains the latest routing mode");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::parallelBalance), 0.2f,
               1.0e-6f, "state reopen retains parallel balance");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::waterAmount), 0.35f,
               1.0e-6f, "state reopen retains inactive Water amount");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::iceAmount), 0.8f, 1.0e-6f,
               "state reopen retains inactive Ice amount");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::inputGain), -3.0f, 1.0e-6f,
               "state reopen retains input gain");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::globalMix), 0.6f, 1.0e-6f,
               "state reopen retains global mix");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::outputGain), 4.0f, 1.0e-6f,
               "state reopen retains output gain");

    restored.prepareToPlay(48000.0, kBlockSize);
    fillBuffer(buffer, 1.0f);
    restored.processBlock(buffer, midi);
    // M1's wet path is post-input pass-through, so global.mix does not change the identity here.
    constexpr float kExpectedNetGainDb = 1.0f;
    const auto expectedGain = std::pow(10.0f, kExpectedNetGainDb / 20.0f);
    expect(std::isfinite(buffer.getSample(0, 0)),
           "restored state enters the audio path with finite output");
    expectNear(buffer.getSample(0, 63), expectedGain, 1.0e-5f,
               "restored gain values are applied from the first prepared block");
}
} // namespace

int main() {
    testParameterAutomationReachesAudioPath();
    testModeSwitchRetainsInactiveValuesAcrossStateReopen();

    if (failures != 0)
        return 1;

    std::cout << "FRAZIL plugin integration tests passed (2 groups)\n";
    return 0;
}
