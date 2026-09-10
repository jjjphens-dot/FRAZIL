#include "plugin/ParameterLayout.h"
#include "plugin/PluginProcessor.h"

#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <juce_audio_basics/juce_audio_basics.h>

namespace {
constexpr double kCanonicalSampleRateHz = 48000.0;
constexpr int kStereoChannelCount = 2;

struct TestContext {
    int failures{};
};

void expect(TestContext& context, bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++context.failures;
    }
}

void expectNear(TestContext& context, float actual, float expected, float tolerance,
                const char* description) {
    expect(context, std::abs(actual - expected) <= tolerance, description);
}

void setParameterValue(TestContext& context, FRAZILAudioProcessor& processor, const char* id,
                       float value) {
    auto* parameter = processor.parameter(id);
    expect(context, parameter != nullptr, "integration parameter exists");
    if (parameter != nullptr) {
        parameter->setValueNotifyingHost(processor.parameterRange(id).convertTo0to1(value));
    }
}

float getParameterValue(TestContext& context, const FRAZILAudioProcessor& processor,
                        const char* id) {
    const auto* value = processor.rawParameterValue(id);
    expect(context, value != nullptr, "integration parameter value exists");
    return value != nullptr ? value->load(std::memory_order_relaxed) : 0.0f;
}

void fillBuffer(juce::AudioBuffer<float>& buffer, float value) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, value);
}

void testParameterAutomationReachesAudioPath(TestContext& context) {
    FRAZILAudioProcessor processor;
    constexpr int kBlockSize = 64;
    processor.prepareToPlay(kCanonicalSampleRateHz, kBlockSize);

    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buffer(kStereoChannelCount, kBlockSize);
    fillBuffer(buffer, 1.0f);
    processor.processBlock(buffer, midi);
    expectNear(context, buffer.getSample(0, kBlockSize - 1), 1.0f, 1.0e-6f,
               "default plugin block is unity pass-through");

    constexpr float kTargetGainDb = 6.0f;
    const auto targetGain = std::pow(10.0f, kTargetGainDb / 20.0f);
    setParameterValue(context, processor, frazil::plugin::parameterIds::kInputGain, kTargetGainDb);

    fillBuffer(buffer, 1.0f);
    processor.processBlock(buffer, midi);
    const auto firstRampedSample = buffer.getSample(0, 0);
    const auto lastRampedSample = buffer.getSample(0, kBlockSize - 1);
    expect(context, firstRampedSample > 1.0f && firstRampedSample < targetGain,
           "host parameter write enters the next block's input-gain ramp");
    expect(context, lastRampedSample > firstRampedSample && lastRampedSample < targetGain,
           "input-gain ramp progresses sample by sample within a block");

    for (int block = 0; block < 7; ++block) {
        fillBuffer(buffer, 1.0f);
        processor.processBlock(buffer, midi);
    }
    expectNear(context, buffer.getSample(0, kBlockSize - 1), targetGain, 1.0e-5f,
               "automated input gain reaches its final value without restarting the ramp");
}

void testStateRestoreAfterPrepareReachesAudioPath(TestContext& context) {
    constexpr int kBlockSize = 64;
    FRAZILAudioProcessor source;
    setParameterValue(context, source, frazil::plugin::parameterIds::kInputGain, -4.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kOutputGain, 7.0f);

    juce::MemoryBlock serializedState;
    source.getStateInformation(serializedState);
    expect(context, serializedState.getSize() > 0, "lifecycle state save produces an XML payload");

    FRAZILAudioProcessor restored;
    restored.prepareToPlay(kCanonicalSampleRateHz, kBlockSize);
    restored.setStateInformation(serializedState.getData(),
                                 static_cast<int>(serializedState.getSize()));

    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kInputGain),
               -4.0f, 1.0e-6f, "state restore after prepare retains input gain");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kOutputGain),
               7.0f, 1.0e-6f, "state restore after prepare retains output gain");

    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buffer(kStereoChannelCount, kBlockSize);
    fillBuffer(buffer, 1.0f);
    restored.processBlock(buffer, midi);

    constexpr float kExpectedNetGainDb = 3.0f;
    const auto expectedGain = std::pow(10.0f, kExpectedNetGainDb / 20.0f);
    expect(context, std::isfinite(buffer.getSample(0, 0)),
           "state restore after prepare produces finite audio");
    expectNear(context, buffer.getSample(0, kBlockSize - 1), expectedGain, 1.0e-5f,
               "state restore after prepare is applied to the first audio block");
}

void testModeSwitchRetainsInactiveValuesAcrossStateReopen(TestContext& context) {
    constexpr int kBlockSize = 64;
    FRAZILAudioProcessor source;
    source.prepareToPlay(kCanonicalSampleRateHz, kBlockSize);
    setParameterValue(context, source, frazil::plugin::parameterIds::kWaterEnabled, 0.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kIceEnabled, 1.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kParallelBalance, 0.2f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kWaterAmount, 0.35f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kIceAmount, 0.8f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kInputGain, -3.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kGlobalMix, 0.6f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kOutputGain, 4.0f);

    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buffer(kStereoChannelCount, kBlockSize);
    constexpr std::array<float, 4> kRoutingModes{0.0f, 1.0f, 2.0f, 0.0f};
    constexpr float kWaterAmount = 0.35f;
    constexpr float kIceAmount = 0.8f;
    for (const auto routingMode : kRoutingModes) {
        setParameterValue(context, source, frazil::plugin::parameterIds::kRoutingMode, routingMode);
        fillBuffer(buffer, 1.0f);
        source.processBlock(buffer, midi);

        expectNear(context,
                   getParameterValue(context, source, frazil::plugin::parameterIds::kWaterAmount),
                   kWaterAmount, 1.0e-6f, "Water amount survives every routing mode switch");
        expectNear(context,
                   getParameterValue(context, source, frazil::plugin::parameterIds::kIceAmount),
                   kIceAmount, 1.0e-6f, "Ice amount survives every routing mode switch");
    }

    juce::MemoryBlock serializedState;
    source.getStateInformation(serializedState);
    expect(context, serializedState.getSize() > 0, "plugin state save produces an XML payload");

    FRAZILAudioProcessor restored;
    restored.setStateInformation(serializedState.getData(),
                                 static_cast<int>(serializedState.getSize()));
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kWaterEnabled),
               0.0f, 1.0e-6f, "state reopen retains Water enable");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kIceEnabled),
               1.0f, 1.0e-6f, "state reopen retains Ice enable");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kRoutingMode),
               0.0f, 1.0e-6f, "state reopen retains the latest routing mode");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kParallelBalance),
               0.2f, 1.0e-6f, "state reopen retains parallel balance");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kWaterAmount),
               0.35f, 1.0e-6f, "state reopen retains inactive Water amount");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kIceAmount), 0.8f,
               1.0e-6f, "state reopen retains inactive Ice amount");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kInputGain),
               -3.0f, 1.0e-6f, "state reopen retains input gain");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kGlobalMix), 0.6f,
               1.0e-6f, "state reopen retains global mix");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kOutputGain),
               4.0f, 1.0e-6f, "state reopen retains output gain");

    restored.prepareToPlay(kCanonicalSampleRateHz, kBlockSize);
    fillBuffer(buffer, 1.0f);
    restored.processBlock(buffer, midi);
    // M1's wet path is post-input pass-through, so global.mix does not change the identity here.
    constexpr float kExpectedNetGainDb = 1.0f;
    const auto expectedGain = std::pow(10.0f, kExpectedNetGainDb / 20.0f);
    expect(context, std::isfinite(buffer.getSample(0, 0)),
           "restored state enters the audio path with finite output");
    expectNear(context, buffer.getSample(0, 63), expectedGain, 1.0e-5f,
               "restored gain values are applied from the first prepared block");
}
} // namespace

int main() {
    TestContext context;
    testParameterAutomationReachesAudioPath(context);
    testStateRestoreAfterPrepareReachesAudioPath(context);
    testModeSwitchRetainsInactiveValuesAcrossStateReopen(context);

    if (context.failures != 0)
        return 1;

    std::cout << "FRAZIL plugin integration tests passed (3 groups)\n";
    return 0;
}
