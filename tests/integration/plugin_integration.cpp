#include "plugin/ParameterLayout.h"
#include "plugin/PluginProcessor.h"

#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <juce_audio_basics/juce_audio_basics.h>

namespace {
struct TestContext {
    int failures{};
};

void expect(TestContext& context, bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++context.failures;
    }
}

void expectNear(TestContext& context,
                float actual,
                float expected,
                float tolerance,
                const char* description) {
    expect(context, std::abs(actual - expected) <= tolerance, description);
}

void setParameterValue(TestContext& context,
                       FRAZILAudioProcessor& processor,
                       const char* id,
                       float value) {
    auto* parameter = processor.parameters.getParameter(id);
    expect(context, parameter != nullptr, "integration parameter exists");
    if (parameter != nullptr) {
        parameter->setValueNotifyingHost(
            processor.parameters.getParameterRange(id).convertTo0to1(value));
    }
}

float getParameterValue(TestContext& context,
                        const FRAZILAudioProcessor& processor,
                        const char* id) {
    const auto* value = processor.parameters.getRawParameterValue(id);
    expect(context, value != nullptr, "integration parameter value exists");
    return value != nullptr ? value->load(std::memory_order_relaxed) : 0.0f;
}

void fillBuffer(juce::AudioBuffer<float>& buffer, float value) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, value);
}

#if FRAZIL_ENABLE_DEVELOPER_UI
struct ParameterEventListener final : juce::AudioProcessorParameter::Listener {
    void parameterValueChanged(int, float) override { ++valueChanges; }
    void parameterGestureChanged(int, bool) override { ++gestureChanges; }

    int valueChanges{};
    int gestureChanges{};
};

frazil::plugin::DeveloperHostParameterSnapshot makeDeveloperHostSnapshot() {
    frazil::plugin::DeveloperHostParameterSnapshot snapshot;
    snapshot.rawValues = {1.0f, 1.0f, 0.0f, 0.25f, 0.4f, 0.8f, 6.0f, 0.3f, -3.0f};
    return snapshot;
}
#endif

void testParameterAutomationReachesAudioPath(TestContext& context) {
    FRAZILAudioProcessor processor;
    constexpr int kBlockSize = 64;
    processor.prepareToPlay(48000.0, kBlockSize);

    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buffer(2, kBlockSize);
    fillBuffer(buffer, 1.0f);
    processor.processBlock(buffer, midi);
    expectNear(context, buffer.getSample(0, kBlockSize - 1), 1.0f, 1.0e-6f,
               "default plugin block is unity pass-through");
#if FRAZIL_ENABLE_DEVELOPER_UI
    const auto diagnostics = processor.getDeveloperDiagnosticsSnapshot();
    expectNear(context, diagnostics.sampleRateHz, 48000.0f, 1.0e-6f,
               "developer diagnostics retain prepared sample rate");
    expect(context, diagnostics.preparedBlockSize == kBlockSize &&
                       diagnostics.latestBlockSize == kBlockSize && diagnostics.channelCount == 2,
           "developer diagnostics distinguish prepared and latest block dimensions");
    expectNear(context, diagnostics.inputPeak, 1.0f, 1.0e-6f,
               "developer diagnostics measure input peak");
    expectNear(context, diagnostics.outputPeak, 1.0f, 1.0e-6f,
               "developer diagnostics measure output peak");
    expect(context, diagnostics.finite, "developer diagnostics report finite audio");
#endif

    constexpr float kTargetGainDb = 6.0f;
    const auto targetGain = std::pow(10.0f, kTargetGainDb / 20.0f);
    setParameterValue(context, processor, frazil::plugin::parameterIds::inputGain, kTargetGainDb);

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

#if FRAZIL_ENABLE_DEVELOPER_UI
    juce::AudioBuffer<float> shortBuffer(2, kBlockSize / 2);
    fillBuffer(shortBuffer, 1.0f);
    processor.processBlock(shortBuffer, midi);
    const auto shortDiagnostics = processor.getDeveloperDiagnosticsSnapshot();
    expect(context, shortDiagnostics.preparedBlockSize == kBlockSize &&
                       shortDiagnostics.latestBlockSize == kBlockSize / 2,
           "developer diagnostics publish the latest callback block size");
#endif
}

#if FRAZIL_ENABLE_DEVELOPER_UI
void testDeveloperComparisonBoundary(TestContext& context) {
    constexpr int kBlockSize = 64;
    FRAZILAudioProcessor processor;
    processor.prepareToPlay(48000.0, kBlockSize);

    auto* inputGain = processor.parameters.getParameter(frazil::plugin::parameterIds::inputGain);
    expect(context, inputGain != nullptr, "developer boundary parameter exists");
    ParameterEventListener listener;
    if (inputGain != nullptr)
        inputGain->addListener(&listener);

    const auto hostBefore = processor.getDeveloperHostParameterSnapshot();
    const auto developerState = makeDeveloperHostSnapshot();
    processor.setDeveloperHostParameterOverride(developerState);
    expect(context, processor.isDeveloperHostParameterOverrideActive(),
           "developer override becomes active");
    const auto effective = processor.getDeveloperHostParameterSnapshot();
    expectNear(context,
               effective.rawValues[static_cast<std::size_t>(
                   frazil::plugin::DeveloperHostParameter::inputGainDb)],
               6.0f, 1.0e-6f, "developer override changes the effective audio snapshot");
    expectNear(context,
               hostBefore.rawValues[static_cast<std::size_t>(
                   frazil::plugin::DeveloperHostParameter::inputGainDb)],
               0.0f, 1.0e-6f, "developer override leaves the APVTS Host value untouched");
    expect(context, listener.valueChanges == 0 && listener.gestureChanges == 0,
           "developer override emits no Host value or gesture notifications");

    setParameterValue(context, processor, frazil::plugin::parameterIds::inputGain, -3.0f);
    expectNear(context, getParameterValue(context, processor,
                                          frazil::plugin::parameterIds::inputGain),
               -3.0f, 1.0e-6f, "Host/APVTS accepts automation while Developer override is active");
    const auto effectiveDuringHostChange = processor.getDeveloperHostParameterSnapshot();
    expectNear(context,
               effectiveDuringHostChange.rawValues[static_cast<std::size_t>(
                   frazil::plugin::DeveloperHostParameter::inputGainDb)],
               6.0f, 1.0e-6f,
               "Developer effective input gain remains active during Host automation");
    expect(context, listener.valueChanges == 1 && listener.gestureChanges == 0,
           "Host automation emits its value notification but no gesture notification");

    juce::MemoryBlock serializedState;
    processor.getStateInformation(serializedState);
    const auto stateText = juce::String::fromUTF8(
        static_cast<const char*>(serializedState.getData()),
        static_cast<int>(serializedState.getSize()));
    expect(context, !stateText.contains("waterExperiment"),
           "developer experiment state is absent from production state XML");

    processor.setDeveloperComparisonMode(frazil::plugin::DeveloperComparisonMode::dry);
    expect(context, processor.getDeveloperComparisonMode() ==
                       frazil::plugin::DeveloperComparisonMode::dry,
           "developer Dry comparison mode is selected without a Host parameter");
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buffer(2, kBlockSize);
    fillBuffer(buffer, 1.0f);
    processor.processBlock(buffer, midi);
    expect(context, std::isfinite(buffer.getSample(0, kBlockSize - 1)),
           "developer Dry comparison produces finite audio");

    processor.setStateInformation(serializedState.getData(),
                                  static_cast<int>(serializedState.getSize()));
    expect(context, !processor.isDeveloperHostParameterOverrideActive(),
           "state restore clears the temporary Developer override");
    expect(context, processor.getDeveloperComparisonMode() ==
                       frazil::plugin::DeveloperComparisonMode::processed,
           "state restore returns comparison mode to Processed");
    const auto effectiveAfterStateRestore = processor.getDeveloperHostParameterSnapshot();
    expectNear(context,
               effectiveAfterStateRestore.rawValues[static_cast<std::size_t>(
                   frazil::plugin::DeveloperHostParameter::inputGainDb)],
               -3.0f, 1.0e-6f, "state restore makes the current Host value effective");

    processor.setDeveloperHostParameterOverride(developerState);
    const auto notificationsBeforeClear = listener.valueChanges;
    processor.clearDeveloperHostParameterOverride();
    expect(context, !processor.isDeveloperHostParameterOverrideActive(),
           "developer override can be cleared");
    const auto hostAfter = processor.getDeveloperHostParameterSnapshot();
    expectNear(context,
               hostAfter.rawValues[static_cast<std::size_t>(
                   frazil::plugin::DeveloperHostParameter::inputGainDb)],
               -3.0f, 1.0e-6f, "clearing override restores the current APVTS Host snapshot");
    expect(context, listener.valueChanges == notificationsBeforeClear &&
                       listener.gestureChanges == 0,
           "clearing developer override emits no Host notifications");

    if (inputGain != nullptr)
        inputGain->removeListener(&listener);
}
#endif

void testStateRestoreAfterPrepareReachesAudioPath(TestContext& context) {
    constexpr int kBlockSize = 64;
    FRAZILAudioProcessor source;
    setParameterValue(context, source, frazil::plugin::parameterIds::inputGain, -4.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::outputGain, 7.0f);

    juce::MemoryBlock serializedState;
    source.getStateInformation(serializedState);
    expect(context, serializedState.getSize() > 0, "lifecycle state save produces an XML payload");

    FRAZILAudioProcessor restored;
    restored.prepareToPlay(48000.0, kBlockSize);
    restored.setStateInformation(serializedState.getData(),
                                 static_cast<int>(serializedState.getSize()));

    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::inputGain), -4.0f,
               1.0e-6f, "state restore after prepare retains input gain");
    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::outputGain), 7.0f,
               1.0e-6f, "state restore after prepare retains output gain");

    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buffer(2, kBlockSize);
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
    source.prepareToPlay(48000.0, kBlockSize);
    setParameterValue(context, source, frazil::plugin::parameterIds::waterEnabled, 0.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::iceEnabled, 1.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::parallelBalance, 0.2f);
    setParameterValue(context, source, frazil::plugin::parameterIds::waterAmount, 0.35f);
    setParameterValue(context, source, frazil::plugin::parameterIds::iceAmount, 0.8f);
    setParameterValue(context, source, frazil::plugin::parameterIds::inputGain, -3.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::globalMix, 0.6f);
    setParameterValue(context, source, frazil::plugin::parameterIds::outputGain, 4.0f);

    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buffer(2, kBlockSize);
    constexpr std::array<float, 4> routingModes{0.0f, 1.0f, 2.0f, 0.0f};
    constexpr float kWaterAmount = 0.35f;
    constexpr float kIceAmount = 0.8f;
    for (const auto routingMode : routingModes) {
        setParameterValue(context, source, frazil::plugin::parameterIds::routingMode, routingMode);
        fillBuffer(buffer, 1.0f);
        source.processBlock(buffer, midi);

        expectNear(context, getParameterValue(context, source, frazil::plugin::parameterIds::waterAmount),
                   kWaterAmount, 1.0e-6f, "Water amount survives every routing mode switch");
        expectNear(context, getParameterValue(context, source, frazil::plugin::parameterIds::iceAmount),
                   kIceAmount, 1.0e-6f, "Ice amount survives every routing mode switch");
    }

    juce::MemoryBlock serializedState;
    source.getStateInformation(serializedState);
    expect(context, serializedState.getSize() > 0, "plugin state save produces an XML payload");

    FRAZILAudioProcessor restored;
    restored.setStateInformation(serializedState.getData(),
                                 static_cast<int>(serializedState.getSize()));
    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::waterEnabled), 0.0f,
               1.0e-6f, "state reopen retains Water enable");
    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::iceEnabled), 1.0f, 1.0e-6f,
               "state reopen retains Ice enable");
    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::routingMode), 0.0f,
               1.0e-6f, "state reopen retains the latest routing mode");
    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::parallelBalance), 0.2f,
               1.0e-6f, "state reopen retains parallel balance");
    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::waterAmount), 0.35f,
               1.0e-6f, "state reopen retains inactive Water amount");
    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::iceAmount), 0.8f, 1.0e-6f,
               "state reopen retains inactive Ice amount");
    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::inputGain), -3.0f, 1.0e-6f,
               "state reopen retains input gain");
    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::globalMix), 0.6f, 1.0e-6f,
               "state reopen retains global mix");
    expectNear(context, getParameterValue(context, restored, frazil::plugin::parameterIds::outputGain), 4.0f, 1.0e-6f,
               "state reopen retains output gain");

    restored.prepareToPlay(48000.0, kBlockSize);
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
#if FRAZIL_ENABLE_DEVELOPER_UI
    testDeveloperComparisonBoundary(context);
#endif
    testStateRestoreAfterPrepareReachesAudioPath(context);
    testModeSwitchRetainsInactiveValuesAcrossStateReopen(context);

    if (context.failures != 0)
        return 1;

#if FRAZIL_ENABLE_DEVELOPER_UI
    std::cout << "FRAZIL plugin integration tests passed (4 groups)\n";
#else
    std::cout << "FRAZIL plugin integration tests passed (3 groups)\n";
#endif
    return 0;
}
