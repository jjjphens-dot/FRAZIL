#include "app/AudioEngine.h"
#include "app/ParameterMapper.h"
#include "app/ParameterSnapshot.h"
#include "app/StateModel.h"
#include "dsp/DryWetMixer.h"
#include "dsp/primitives/LinearSmoother.h"
#include "dsp/primitives/RandomSource.h"
#include "plugin/ParameterLayout.h"
#include "plugin/StateAdapter.h"
#include "test_support.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <juce_audio_processors/juce_audio_processors.h>
#include <limits>

namespace {

class TestAudioProcessor final : public juce::AudioProcessor {
  public:
    const juce::String getName() const override {
        return {};
    }
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    double getTailLengthSeconds() const override {
        return {};
    }
    bool acceptsMidi() const override {
        return {};
    }
    bool producesMidi() const override {
        return {};
    }
    juce::AudioProcessorEditor* createEditor() override {
        return {};
    }
    bool hasEditor() const override {
        return {};
    }
    int getNumPrograms() override {
        return 1;
    }
    int getCurrentProgram() override {
        return {};
    }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override {
        return {};
    }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
};

void setParameterValue(TestContext& context, juce::AudioProcessorValueTreeState& state,
                       const char* id, float value) {
    auto* parameter = state.getParameter(id);
    expect(context, parameter != nullptr, "state test parameter exists");
    if (parameter != nullptr)
        parameter->setValueNotifyingHost(state.getParameterRange(id).convertTo0to1(value));
}

float getParameterValue(TestContext& context, const juce::AudioProcessorValueTreeState& state,
                        const char* id) {
    const auto* value = state.getRawParameterValue(id);
    expect(context, value != nullptr, "state test parameter value exists");
    return value != nullptr ? value->load() : 0.0f;
}

juce::ValueTree findParameterNode(const juce::ValueTree& state, const char* id) {
    for (int index = 0; index < state.getNumChildren(); ++index) {
        const auto parameter = state.getChild(index);
        if (parameter.getProperty("id").toString() == id)
            return parameter;
    }
    return {};
}

void testProcessSpecValidation(TestContext& context) {
    expect(context, ProcessSpec{48000.0, 128, 2}.isValid(), "valid process spec is accepted");
    expect(context, !ProcessSpec{0.0, 128, 2}.isValid(), "zero sample rate is rejected");
    expect(context, !ProcessSpec{48000.0, 0, 2}.isValid(), "zero block size is rejected");
    expect(context, !ProcessSpec{48000.0, 128, 0}.isValid(), "zero channels are rejected");
}

void testParameterLayoutContract(TestContext& context) {
    TestAudioProcessor processor;
    juce::AudioProcessorValueTreeState state(processor, nullptr, "FRAZIL",
                                             frazil::plugin::createParameterLayout());

    constexpr std::array<const char*, 9> expectedIds{
        frazil::plugin::parameterIds::kWaterEnabled, frazil::plugin::parameterIds::kIceEnabled,
        frazil::plugin::parameterIds::kRoutingMode,  frazil::plugin::parameterIds::kParallelBalance,
        frazil::plugin::parameterIds::kWaterAmount,  frazil::plugin::parameterIds::kIceAmount,
        frazil::plugin::parameterIds::kInputGain,    frazil::plugin::parameterIds::kGlobalMix,
        frazil::plugin::parameterIds::kOutputGain,
    };
    constexpr std::array<const char*, 9> expectedNames{
        "Water Enabled", "Ice Enabled", "Routing Mode", "Parallel Balance", "Water Amount",
        "Ice Amount",    "Input Gain",  "Global Mix",   "Output Gain",
    };

    expect(context, processor.getParameters().size() == static_cast<int>(expectedIds.size()),
           "parameter layout exposes exactly nine parameters");

    for (int index = 0; index < processor.getParameters().size(); ++index) {
        const auto* parameter = dynamic_cast<const juce::AudioProcessorParameterWithID*>(
            processor.getParameters()[index]);
        expect(context, parameter != nullptr, "every parameter exposes a stable ID");
        if (parameter != nullptr) {
            expect(context,
                   parameter->getParameterID() == expectedIds[static_cast<std::size_t>(index)],
                   "parameter order and ID match the contract");
            expect(context,
                   parameter->getName(64) == expectedNames[static_cast<std::size_t>(index)],
                   "host-visible parameter name matches the contract");
        }
    }

    struct ExpectedParameter {
        const char* id;
        float minimum;
        float maximum;
        float interval;
        float defaultValue;
    };
    constexpr std::array<ExpectedParameter, 9> expectedParameters{
        ExpectedParameter{frazil::plugin::parameterIds::kWaterEnabled, 0.0f, 1.0f, 1.0f, 1.0f},
        ExpectedParameter{frazil::plugin::parameterIds::kIceEnabled, 0.0f, 1.0f, 1.0f, 1.0f},
        ExpectedParameter{frazil::plugin::parameterIds::kRoutingMode, 0.0f, 2.0f, 1.0f, 0.0f},
        ExpectedParameter{frazil::plugin::parameterIds::kParallelBalance, 0.0f, 1.0f, 0.001f, 0.5f},
        ExpectedParameter{frazil::plugin::parameterIds::kWaterAmount, 0.0f, 1.0f, 0.001f, 1.0f},
        ExpectedParameter{frazil::plugin::parameterIds::kIceAmount, 0.0f, 1.0f, 0.001f, 1.0f},
        ExpectedParameter{frazil::plugin::parameterIds::kInputGain, -24.0f, 24.0f, 0.01f, 0.0f},
        ExpectedParameter{frazil::plugin::parameterIds::kGlobalMix, 0.0f, 1.0f, 0.001f, 1.0f},
        ExpectedParameter{frazil::plugin::parameterIds::kOutputGain, -24.0f, 24.0f, 0.01f, 0.0f},
    };

    for (const auto& expected : expectedParameters) {
        const auto range = state.getParameterRange(expected.id);
        expectNear(context, range.start, expected.minimum, 1.0e-6f,
                   "parameter minimum matches the contract");
        expectNear(context, range.end, expected.maximum, 1.0e-6f,
                   "parameter maximum matches the contract");
        expectNear(context, range.interval, expected.interval, 1.0e-6f,
                   "parameter step matches the contract");
        const auto* parameter = state.getParameter(expected.id);
        expect(context, parameter != nullptr, "contract parameter can be retrieved by ID");
        if (parameter != nullptr)
            expectNear(context, range.convertFrom0to1(parameter->getDefaultValue()),
                       expected.defaultValue, 1.0e-6f, "parameter default matches the contract");
    }

    const auto* routing = dynamic_cast<const juce::AudioParameterChoice*>(
        state.getParameter(frazil::plugin::parameterIds::kRoutingMode));
    expect(context, routing != nullptr, "routing mode is a choice parameter");
    if (routing != nullptr) {
        expect(context, processor.getParameterNumSteps(2) == 3,
               "routing mode has three stable choices");
        constexpr std::array<const char*, 3> expectedChoices{
            "Parallel",
            "Water -> Ice",
            "Ice -> Water",
        };
        expect(context, routing->choices.size() == static_cast<int>(expectedChoices.size()),
               "routing mode exposes the expected choice count");
        for (int index = 0; index < routing->choices.size(); ++index)
            expect(context,
                   routing->choices[index] == expectedChoices[static_cast<std::size_t>(index)],
                   "routing choice text and index remain stable");
    }

    const auto* waterEnabled = dynamic_cast<const juce::AudioParameterBool*>(
        state.getParameter(frazil::plugin::parameterIds::kWaterEnabled));
    const auto* iceEnabled = dynamic_cast<const juce::AudioParameterBool*>(
        state.getParameter(frazil::plugin::parameterIds::kIceEnabled));
    expect(context, waterEnabled != nullptr && processor.getParameterNumSteps(0) == 2,
           "water.enabled is an AudioParameterBool");
    expect(context, iceEnabled != nullptr && processor.getParameterNumSteps(1) == 2,
           "ice.enabled is an AudioParameterBool");

    const auto* inputGain = state.getParameter(frazil::plugin::parameterIds::kInputGain);
    const auto* outputGain = state.getParameter(frazil::plugin::parameterIds::kOutputGain);
    expect(context, inputGain != nullptr && inputGain->getLabel() == "dB",
           "input.gain exposes the dB host unit");
    expect(context, outputGain != nullptr && outputGain->getLabel() == "dB",
           "output.gain exposes the dB host unit");
}

void testParameterSnapshotReadsOneCoherentSet(TestContext& context) {
    std::atomic<float> waterEnabled{0.0f};
    std::atomic<float> iceEnabled{1.0f};
    std::atomic<float> routingMode{2.0f};
    std::atomic<float> parallelBalance{0.25f};
    std::atomic<float> waterAmount{0.5f};
    std::atomic<float> iceAmount{0.75f};
    std::atomic<float> inputGainDb{-6.0f};
    std::atomic<float> globalMix{0.4f};
    std::atomic<float> outputGainDb{3.0f};

    const ParameterSourcePointers sources{&waterEnabled,    &iceEnabled,  &routingMode,
                                          &parallelBalance, &waterAmount, &iceAmount,
                                          &inputGainDb,     &globalMix,   &outputGainDb};
    const auto snapshot = ParameterSnapshot::capture(sources);

    waterEnabled.store(1.0f);
    routingMode.store(0.0f);
    inputGainDb.store(24.0f);

    expect(context, !snapshot.waterEnabled, "snapshot retains the captured Water enable value");
    expect(context, snapshot.routingModeIndex == 2, "snapshot retains the captured routing value");
    expectNear(context, snapshot.inputGainDb, -6.0f, 1.0e-6f,
               "snapshot retains the captured gain value");
}

void testParameterMapperClampsAndConverts(TestContext& context) {
    ParameterSnapshot snapshot;
    snapshot.waterEnabled = false;
    snapshot.iceEnabled = true;
    snapshot.routingModeIndex = 2;
    snapshot.parallelBalance = -1.0f;
    snapshot.waterAmount = 2.0f;
    snapshot.iceAmount = 0.25f;
    snapshot.inputGainDb = 24.0f;
    snapshot.globalMix = std::numeric_limits<float>::quiet_NaN();
    snapshot.outputGainDb = -24.0f;

    const auto parameters = ParameterMapper{}.map(snapshot);
    expect(context, !parameters.waterEnabled && parameters.iceEnabled, "enable values are mapped");
    expect(context, parameters.routing == RoutingMode::iceIntoWater, "routing choice maps to enum");
    expectNear(context, parameters.parallelBalance, 0.0f, 1.0e-6f, "parallel balance is clamped");
    expectNear(context, parameters.waterStageAmount, 1.0f, 1.0e-6f, "water amount is clamped");
    expectNear(context, parameters.iceStageAmount, 0.25f, 1.0e-6f, "ice amount is preserved");
    expectNear(context, parameters.inputGainLinear, 15.8489319f, 1.0e-5f,
               "+24 dB maps to linear gain");
    expectNear(context, parameters.globalMix, 1.0f, 1.0e-6f, "invalid mix uses the safe default");
    expectNear(context, parameters.outputGainLinear, 0.0630957f, 1.0e-5f,
               "-24 dB maps to linear gain");

    snapshot.routingModeIndex = 99;
    expect(context, ParameterMapper{}.map(snapshot).routing == RoutingMode::parallel,
           "invalid routing choice falls back to parallel");
}

void testStateModelRoundTrip(TestContext& context) {
    auto values = StateModel::defaultValues();
    values.waterEnabled = false;
    values.iceEnabled = true;
    values.routing = RoutingMode::iceIntoWater;
    values.parallelBalance = 0.25f;
    values.waterAmount = 0.5f;
    values.iceAmount = 0.75f;
    values.inputGainDb = -6.0f;
    values.globalMix = 0.4f;
    values.outputGainDb = 3.0f;

    const auto serialized = StateModel::serialize(values);
    expect(context, serialized.schemaVersion == StateModel::kCurrentSchemaVersion,
           "state serialization emits the current schema version");
    const auto result = StateModel::deserialize(serialized);
    expect(context, result.status == StateModel::DeserializeStatus::current,
           "current state deserializes without migration");
    expect(context, result.values.waterEnabled == values.waterEnabled,
           "state round-trip retains Water enable");
    expect(context, result.values.iceEnabled == values.iceEnabled,
           "state round-trip retains Ice enable");
    expect(context, result.values.routing == values.routing,
           "state round-trip retains routing mode");
    expectNear(context, result.values.parallelBalance, values.parallelBalance, 1.0e-6f,
               "state round-trip retains parallel balance");
    expectNear(context, result.values.waterAmount, values.waterAmount, 1.0e-6f,
               "state round-trip retains Water amount");
    expectNear(context, result.values.iceAmount, values.iceAmount, 1.0e-6f,
               "state round-trip retains Ice amount");
    expectNear(context, result.values.inputGainDb, values.inputGainDb, 1.0e-6f,
               "state round-trip retains input gain");
    expectNear(context, result.values.globalMix, values.globalMix, 1.0e-6f,
               "state round-trip retains global mix");
    expectNear(context, result.values.outputGainDb, values.outputGainDb, 1.0e-6f,
               "state round-trip retains output gain");

    for (const auto routing :
         {RoutingMode::parallel, RoutingMode::waterIntoIce, RoutingMode::iceIntoWater}) {
        values.routing = routing;
        const auto routingResult = StateModel::deserialize(StateModel::serialize(values));
        expect(context, routingResult.status == StateModel::DeserializeStatus::current,
               "each routing mode uses the current state schema");
        expect(context, routingResult.values.routing == routing, "each routing mode round-trips");
    }
}

void testStateModelMigrationAndFallback(TestContext& context) {
    const auto values = StateModel::defaultValues();
    const auto serialized = StateModel::serialize(values);

    auto legacy = serialized;
    legacy.schemaVersion = StateModel::kLegacySchemaVersion;
    const auto migrated = StateModel::deserialize(legacy);
    expect(context, migrated.status == StateModel::DeserializeStatus::migrated,
           "known legacy schema enters the migration path");

    auto schemaLessLegacy = serialized;
    schemaLessLegacy.schemaVersion.reset();
    const auto schemaLessResult = StateModel::deserialize(schemaLessLegacy);
    expect(context, schemaLessResult.status == StateModel::DeserializeStatus::migrated,
           "schema-less known parameter state enters the pre-v1 migration path");

    auto unknown = serialized;
    unknown.schemaVersion = StateModel::kCurrentSchemaVersion + 1U;
    const auto unknownResult = StateModel::deserialize(unknown);
    expect(context, unknownResult.status == StateModel::DeserializeStatus::fallback,
           "unknown schema falls back safely");
    expect(context, unknownResult.values.routing == values.routing,
           "unknown schema fallback uses safe default routing");

    auto outOfRange = serialized;
    outOfRange.parallelBalance = 2.0f;
    const auto outOfRangeResult = StateModel::deserialize(outOfRange);
    expect(context, outOfRangeResult.status == StateModel::DeserializeStatus::fallback,
           "out-of-range state value uses safe fallback");
    expectNear(context, outOfRangeResult.values.parallelBalance, values.parallelBalance, 1.0e-6f,
               "out-of-range state value falls back to the documented default");

    const auto emptyResult = StateModel::deserialize(StateModel::SerializedState{});
    expect(context, emptyResult.status == StateModel::DeserializeStatus::fallback,
           "empty state uses safe defaults");
    expect(context, emptyResult.values.waterEnabled && emptyResult.values.iceEnabled,
           "empty state restores default enabled values");
}

void testHostStateAdapterRoundTripAndInactiveRetention(TestContext& context) {
    TestAudioProcessor sourceProcessor;
    juce::AudioProcessorValueTreeState source(sourceProcessor, nullptr, "FRAZIL",
                                              frazil::plugin::createParameterLayout());
    setParameterValue(context, source, frazil::plugin::parameterIds::kWaterEnabled, 0.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kIceEnabled, 1.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kRoutingMode, 2.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kParallelBalance, 0.25f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kWaterAmount, 0.5f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kIceAmount, 0.75f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kInputGain, -6.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kGlobalMix, 0.4f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kOutputGain, 3.0f);

    const auto serialized = frazil::plugin::HostStateAdapter::serialize(source);
    expect(context, serialized.hasType(juce::Identifier{"FRAZIL"}),
           "host state adapter emits the FRAZIL root type");
    expect(context,
           static_cast<int>(serialized.getProperty("schemaVersion")) ==
               static_cast<int>(StateModel::kCurrentSchemaVersion),
           "host state adapter emits schemaVersion");
    expect(context, serialized.getNumChildren() == 9,
           "host state adapter serializes all nine static parameters");

    TestAudioProcessor restoredProcessor;
    juce::AudioProcessorValueTreeState restored(restoredProcessor, nullptr, "FRAZIL",
                                                frazil::plugin::createParameterLayout());
    expect(context, frazil::plugin::HostStateAdapter::restore(restored, serialized),
           "current host state restores without fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kWaterEnabled),
               0.0f, 1.0e-6f, "restored Water enable matches source");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kRoutingMode),
               2.0f, 1.0e-6f, "restored routing mode matches source");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kParallelBalance),
               0.25f, 1.0e-6f, "restored parallel balance matches source");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kWaterAmount),
               0.5f, 1.0e-6f, "restored inactive Water amount matches source");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kIceAmount),
               0.75f, 1.0e-6f, "restored inactive Ice amount matches source");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kInputGain),
               -6.0f, 1.0e-6f, "restored input gain matches source");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kGlobalMix), 0.4f,
               1.0e-6f, "restored global mix matches source");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kOutputGain),
               3.0f, 1.0e-6f, "restored output gain matches source");
}

void testHostStateAdapterXmlRoundTrip(TestContext& context) {
    TestAudioProcessor sourceProcessor;
    juce::AudioProcessorValueTreeState source(sourceProcessor, nullptr, "FRAZIL",
                                              frazil::plugin::createParameterLayout());
    setParameterValue(context, source, frazil::plugin::parameterIds::kWaterEnabled, 0.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kIceEnabled, 1.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kRoutingMode, 2.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kParallelBalance, 0.25f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kWaterAmount, 0.5f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kIceAmount, 0.75f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kInputGain, -6.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kGlobalMix, 0.4f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kOutputGain, 3.0f);

    const auto serialized = frazil::plugin::HostStateAdapter::serialize(source);
    const auto xml = serialized.createXml();
    expect(context, xml != nullptr, "host state creates XML for API round-trip");
    if (xml == nullptr)
        return;

    const auto fromXml = juce::ValueTree::fromXml(*xml);
    expect(context, fromXml.isValid(), "host state XML recreates a valid ValueTree");
    expect(context, fromXml.getProperty("schemaVersion").isString(),
           "ValueTree XML restore exposes schemaVersion as a string");
    const auto xmlGlobalMix = findParameterNode(fromXml, frazil::plugin::parameterIds::kGlobalMix);
    expect(context, xmlGlobalMix.getProperty("value").isString(),
           "ValueTree XML restore exposes parameter values as strings");

    TestAudioProcessor restoredProcessor;
    juce::AudioProcessorValueTreeState restored(restoredProcessor, nullptr, "FRAZIL",
                                                frazil::plugin::createParameterLayout());
    expect(context, frazil::plugin::HostStateAdapter::restore(restored, fromXml),
           "XML/API state round-trip restores without fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kWaterEnabled),
               0.0f, 1.0e-6f, "XML/API round-trip restores inactive Water enable");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kIceEnabled),
               1.0f, 1.0e-6f, "XML/API round-trip restores Ice enable");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kRoutingMode),
               2.0f, 1.0e-6f, "XML/API round-trip restores routing mode");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kParallelBalance),
               0.25f, 1.0e-6f, "XML/API round-trip restores parallel balance");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kWaterAmount),
               0.5f, 1.0e-6f, "XML/API round-trip restores inactive Water amount");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kIceAmount),
               0.75f, 1.0e-6f, "XML/API round-trip restores inactive Ice amount");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kInputGain),
               -6.0f, 1.0e-6f, "XML/API round-trip restores input gain");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kGlobalMix), 0.4f,
               1.0e-6f, "XML/API round-trip restores global mix");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kOutputGain),
               3.0f, 1.0e-6f, "XML/API round-trip restores output gain");
}

void testHostStateAdapterLegacyIdsAndInvalidFallback(TestContext& context) {
    TestAudioProcessor sourceProcessor;
    juce::AudioProcessorValueTreeState source(sourceProcessor, nullptr, "FRAZIL",
                                              frazil::plugin::createParameterLayout());
    setParameterValue(context, source, frazil::plugin::parameterIds::kWaterEnabled, 0.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kIceEnabled, 1.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kRoutingMode, 1.0f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kWaterAmount, 0.25f);
    setParameterValue(context, source, frazil::plugin::parameterIds::kIceAmount, 0.75f);

    auto legacy = frazil::plugin::HostStateAdapter::serialize(source);
    legacy.removeProperty("schemaVersion", nullptr);
    for (int index = 0; index < legacy.getNumChildren(); ++index) {
        auto parameter = legacy.getChild(index);
        const auto id = parameter.getProperty("id").toString();
        if (id == frazil::plugin::parameterIds::kWaterEnabled)
            parameter.setProperty("id", "water.enable", nullptr);
        else if (id == frazil::plugin::parameterIds::kIceEnabled)
            parameter.setProperty("id", "ice.enable", nullptr);
    }

    const auto decoded = frazil::plugin::HostStateAdapter::deserialize(legacy);
    expect(context, decoded.status == StateModel::DeserializeStatus::migrated,
           "legacy parameter IDs are recognized by the migration path");

    TestAudioProcessor restoredProcessor;
    juce::AudioProcessorValueTreeState restored(restoredProcessor, nullptr, "FRAZIL",
                                                frazil::plugin::createParameterLayout());
    expect(context, frazil::plugin::HostStateAdapter::restore(restored, legacy),
           "legacy state restores without fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kWaterEnabled),
               0.0f, 1.0e-6f, "legacy Water ID migrates to the canonical ID");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kIceEnabled),
               1.0f, 1.0e-6f, "legacy Ice ID migrates to the canonical ID");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kRoutingMode),
               1.0f, 1.0e-6f, "legacy state retains routing mode");

    auto unknownSchema = legacy;
    unknownSchema.setProperty("schemaVersion", 99, nullptr);
    expect(context, !frazil::plugin::HostStateAdapter::restore(restored, unknownSchema),
           "unknown schema reports safe fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kRoutingMode),
               0.0f, 1.0e-6f, "unknown schema restores default routing");

    auto malformed = frazil::plugin::HostStateAdapter::serialize(source);
    for (int index = malformed.getNumChildren() - 1; index >= 0; --index) {
        if (malformed.getChild(index).getProperty("id").toString() ==
            frazil::plugin::parameterIds::kGlobalMix) {
            malformed.removeChild(index, nullptr);
            break;
        }
    }
    expect(context, !frazil::plugin::HostStateAdapter::restore(restored, malformed),
           "missing parameter reports safe fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kGlobalMix), 1.0f,
               1.0e-6f, "missing parameter restores its documented default");

    auto wrongRoot = juce::ValueTree{"NOT_FRAZIL"};
    expect(context, !frazil::plugin::HostStateAdapter::restore(restored, wrongRoot),
           "malformed root reports safe fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kWaterEnabled),
               1.0f, 1.0e-6f, "malformed root restores default Water enable");
}

void testHostStateAdapterParserRegressions(TestContext& context) {
    TestAudioProcessor sourceProcessor;
    juce::AudioProcessorValueTreeState source(sourceProcessor, nullptr, "FRAZIL",
                                              frazil::plugin::createParameterLayout());
    setParameterValue(context, source, frazil::plugin::parameterIds::kGlobalMix, 0.4f);
    const auto valid = frazil::plugin::HostStateAdapter::serialize(source);

    TestAudioProcessor restoredProcessor;
    juce::AudioProcessorValueTreeState restored(restoredProcessor, nullptr, "FRAZIL",
                                                frazil::plugin::createParameterLayout());

    auto duplicate = valid.createCopy();
    auto globalMixNode = findParameterNode(valid, frazil::plugin::parameterIds::kGlobalMix);
    expect(context, globalMixNode.isValid(), "duplicate regression locates global mix parameter");
    if (globalMixNode.isValid())
        duplicate.appendChild(globalMixNode.createCopy(), nullptr);
    expect(context, !frazil::plugin::HostStateAdapter::restore(restored, duplicate),
           "duplicate known parameter reports safe fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kGlobalMix), 1.0f,
               1.0e-6f, "duplicate known parameter restores the global mix default");

    auto nonnumericSchema = valid.createCopy();
    nonnumericSchema.setProperty("schemaVersion", "abc", nullptr);
    expect(context, !frazil::plugin::HostStateAdapter::restore(restored, nonnumericSchema),
           "nonnumeric schemaVersion reports safe fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kGlobalMix), 1.0f,
               1.0e-6f, "nonnumeric schemaVersion restores the global mix default");

    auto nonnumericParameter = valid.createCopy();
    auto nonnumericParameterNode =
        findParameterNode(nonnumericParameter, frazil::plugin::parameterIds::kGlobalMix);
    expect(context, nonnumericParameterNode.isValid(),
           "nonnumeric regression locates global mix parameter");
    if (nonnumericParameterNode.isValid())
        nonnumericParameterNode.setProperty("value", "abc", nullptr);
    expect(context, !frazil::plugin::HostStateAdapter::restore(restored, nonnumericParameter),
           "nonnumeric parameter value reports safe fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kGlobalMix), 1.0f,
               1.0e-6f, "nonnumeric parameter value restores the global mix default");

    auto partialNumericParameter = valid.createCopy();
    auto partialNumericNode =
        findParameterNode(partialNumericParameter, frazil::plugin::parameterIds::kGlobalMix);
    expect(context, partialNumericNode.isValid(),
           "partial numeric regression locates global mix parameter");
    if (partialNumericNode.isValid())
        partialNumericNode.setProperty("value", "0.4trailing", nullptr);
    expect(context, !frazil::plugin::HostStateAdapter::restore(restored, partialNumericParameter),
           "partial numeric parameter value reports safe fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kGlobalMix), 1.0f,
               1.0e-6f, "partial numeric parameter value restores the global mix default");

    auto malformedBool = valid.createCopy();
    auto malformedBoolNode =
        findParameterNode(malformedBool, frazil::plugin::parameterIds::kWaterEnabled);
    expect(context, malformedBoolNode.isValid(),
           "malformed bool regression locates Water parameter");
    if (malformedBoolNode.isValid())
        malformedBoolNode.setProperty("value", 0.5f, nullptr);
    expect(context, !frazil::plugin::HostStateAdapter::restore(restored, malformedBool),
           "malformed bool value reports safe fallback");
    expectNear(context,
               getParameterValue(context, restored, frazil::plugin::parameterIds::kWaterEnabled),
               1.0f, 1.0e-6f, "malformed bool value restores the Water enable default");
}

} // namespace

void runParameterStateTests(TestContext& context) {
    testProcessSpecValidation(context);
    testParameterLayoutContract(context);
    testParameterSnapshotReadsOneCoherentSet(context);
    testParameterMapperClampsAndConverts(context);
    testStateModelRoundTrip(context);
    testStateModelMigrationAndFallback(context);
    testHostStateAdapterRoundTripAndInactiveRetention(context);
    testHostStateAdapterXmlRoundTrip(context);
    testHostStateAdapterLegacyIdsAndInvalidFallback(context);
    testHostStateAdapterParserRegressions(context);
}
