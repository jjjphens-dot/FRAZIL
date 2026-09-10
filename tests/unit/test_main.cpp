#include "app/AudioEngine.h"
#include "app/ParameterMapper.h"
#include "app/ParameterSnapshot.h"
#include "app/StateModel.h"
#include "dsp/DryWetMixer.h"
#include "dsp/primitives/LinearSmoother.h"
#include "dsp/primitives/RandomSource.h"
#include "plugin/ParameterLayout.h"
#include "plugin/StateAdapter.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <juce_audio_processors/juce_audio_processors.h>
#include <limits>

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

void setParameterValue(juce::AudioProcessorValueTreeState& state, const char* id, float value) {
    auto* parameter = state.getParameter(id);
    expect(parameter != nullptr, "state test parameter exists");
    if (parameter != nullptr)
        parameter->setValueNotifyingHost(state.getParameterRange(id).convertTo0to1(value));
}

float getParameterValue(const juce::AudioProcessorValueTreeState& state, const char* id) {
    const auto* value = state.getRawParameterValue(id);
    expect(value != nullptr, "state test parameter value exists");
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

void testProcessSpecValidation() {
    expect(ProcessSpec{48000.0, 128, 2}.isValid(), "valid process spec is accepted");
    expect(!ProcessSpec{0.0, 128, 2}.isValid(), "zero sample rate is rejected");
    expect(!ProcessSpec{48000.0, 0, 2}.isValid(), "zero block size is rejected");
    expect(!ProcessSpec{48000.0, 128, 0}.isValid(), "zero channels are rejected");
}

void testParameterLayoutContract() {
    TestAudioProcessor processor;
    juce::AudioProcessorValueTreeState state(processor, nullptr, "FRAZIL",
                                             frazil::plugin::createParameterLayout());

    constexpr std::array<const char*, 9> expectedIds{
        frazil::plugin::parameterIds::waterEnabled, frazil::plugin::parameterIds::iceEnabled,
        frazil::plugin::parameterIds::routingMode,  frazil::plugin::parameterIds::parallelBalance,
        frazil::plugin::parameterIds::waterAmount,  frazil::plugin::parameterIds::iceAmount,
        frazil::plugin::parameterIds::inputGain,    frazil::plugin::parameterIds::globalMix,
        frazil::plugin::parameterIds::outputGain,
    };
    constexpr std::array<const char*, 9> expectedNames{
        "Water Enabled", "Ice Enabled", "Routing Mode", "Parallel Balance", "Water Amount",
        "Ice Amount",    "Input Gain",  "Global Mix",   "Output Gain",
    };

    expect(processor.getParameters().size() == static_cast<int>(expectedIds.size()),
           "parameter layout exposes exactly nine parameters");

    for (int index = 0; index < processor.getParameters().size(); ++index) {
        const auto* parameter = dynamic_cast<const juce::AudioProcessorParameterWithID*>(
            processor.getParameters()[index]);
        expect(parameter != nullptr, "every parameter exposes a stable ID");
        if (parameter != nullptr) {
            expect(parameter->getParameterID() == expectedIds[static_cast<std::size_t>(index)],
                   "parameter order and ID match the contract");
            expect(parameter->getName(64) == expectedNames[static_cast<std::size_t>(index)],
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
        ExpectedParameter{frazil::plugin::parameterIds::waterEnabled, 0.0f, 1.0f, 1.0f, 1.0f},
        ExpectedParameter{frazil::plugin::parameterIds::iceEnabled, 0.0f, 1.0f, 1.0f, 1.0f},
        ExpectedParameter{frazil::plugin::parameterIds::routingMode, 0.0f, 2.0f, 1.0f, 0.0f},
        ExpectedParameter{frazil::plugin::parameterIds::parallelBalance, 0.0f, 1.0f, 0.001f, 0.5f},
        ExpectedParameter{frazil::plugin::parameterIds::waterAmount, 0.0f, 1.0f, 0.001f, 1.0f},
        ExpectedParameter{frazil::plugin::parameterIds::iceAmount, 0.0f, 1.0f, 0.001f, 1.0f},
        ExpectedParameter{frazil::plugin::parameterIds::inputGain, -24.0f, 24.0f, 0.01f, 0.0f},
        ExpectedParameter{frazil::plugin::parameterIds::globalMix, 0.0f, 1.0f, 0.001f, 1.0f},
        ExpectedParameter{frazil::plugin::parameterIds::outputGain, -24.0f, 24.0f, 0.01f, 0.0f},
    };

    for (const auto& expected : expectedParameters) {
        const auto range = state.getParameterRange(expected.id);
        expectNear(range.start, expected.minimum, 1.0e-6f,
                   "parameter minimum matches the contract");
        expectNear(range.end, expected.maximum, 1.0e-6f, "parameter maximum matches the contract");
        expectNear(range.interval, expected.interval, 1.0e-6f,
                   "parameter step matches the contract");
        const auto* parameter = state.getParameter(expected.id);
        expect(parameter != nullptr, "contract parameter can be retrieved by ID");
        if (parameter != nullptr)
            expectNear(range.convertFrom0to1(parameter->getDefaultValue()), expected.defaultValue,
                       1.0e-6f, "parameter default matches the contract");
    }

    const auto* routing = dynamic_cast<const juce::AudioParameterChoice*>(
        state.getParameter(frazil::plugin::parameterIds::routingMode));
    expect(routing != nullptr, "routing mode is a choice parameter");
    if (routing != nullptr) {
        expect(processor.getParameterNumSteps(2) == 3, "routing mode has three stable choices");
        constexpr std::array<const char*, 3> expectedChoices{
            "Parallel",
            "Water -> Ice",
            "Ice -> Water",
        };
        expect(routing->choices.size() == static_cast<int>(expectedChoices.size()),
               "routing mode exposes the expected choice count");
        for (int index = 0; index < routing->choices.size(); ++index)
            expect(routing->choices[index] == expectedChoices[static_cast<std::size_t>(index)],
                   "routing choice text and index remain stable");
    }

    const auto* waterEnabled = dynamic_cast<const juce::AudioParameterBool*>(
        state.getParameter(frazil::plugin::parameterIds::waterEnabled));
    const auto* iceEnabled = dynamic_cast<const juce::AudioParameterBool*>(
        state.getParameter(frazil::plugin::parameterIds::iceEnabled));
    expect(waterEnabled != nullptr && processor.getParameterNumSteps(0) == 2,
           "water.enabled is an AudioParameterBool");
    expect(iceEnabled != nullptr && processor.getParameterNumSteps(1) == 2,
           "ice.enabled is an AudioParameterBool");

    const auto* inputGain = state.getParameter(frazil::plugin::parameterIds::inputGain);
    const auto* outputGain = state.getParameter(frazil::plugin::parameterIds::outputGain);
    expect(inputGain != nullptr && inputGain->getLabel() == "dB",
           "input.gain exposes the dB host unit");
    expect(outputGain != nullptr && outputGain->getLabel() == "dB",
           "output.gain exposes the dB host unit");
}

void testParameterSnapshotReadsOneCoherentSet() {
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

    expect(!snapshot.waterEnabled, "snapshot retains the captured Water enable value");
    expect(snapshot.routingModeIndex == 2, "snapshot retains the captured routing value");
    expectNear(snapshot.inputGainDb, -6.0f, 1.0e-6f, "snapshot retains the captured gain value");
}

void testParameterMapperClampsAndConverts() {
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
    expect(!parameters.waterEnabled && parameters.iceEnabled, "enable values are mapped");
    expect(parameters.routing == RoutingMode::iceIntoWater, "routing choice maps to enum");
    expectNear(parameters.parallelBalance, 0.0f, 1.0e-6f, "parallel balance is clamped");
    expectNear(parameters.waterStageAmount, 1.0f, 1.0e-6f, "water amount is clamped");
    expectNear(parameters.iceStageAmount, 0.25f, 1.0e-6f, "ice amount is preserved");
    expectNear(parameters.inputGainLinear, 15.8489319f, 1.0e-5f, "+24 dB maps to linear gain");
    expectNear(parameters.globalMix, 1.0f, 1.0e-6f, "invalid mix uses the safe default");
    expectNear(parameters.outputGainLinear, 0.0630957f, 1.0e-5f, "-24 dB maps to linear gain");

    snapshot.routingModeIndex = 99;
    expect(ParameterMapper{}.map(snapshot).routing == RoutingMode::parallel,
           "invalid routing choice falls back to parallel");
}

void testStateModelRoundTrip() {
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
    expect(serialized.schemaVersion == StateModel::kCurrentSchemaVersion,
           "state serialization emits the current schema version");
    const auto result = StateModel::deserialize(serialized);
    expect(result.status == StateModel::DeserializeStatus::current,
           "current state deserializes without migration");
    expect(result.values.waterEnabled == values.waterEnabled,
           "state round-trip retains Water enable");
    expect(result.values.iceEnabled == values.iceEnabled, "state round-trip retains Ice enable");
    expect(result.values.routing == values.routing, "state round-trip retains routing mode");
    expectNear(result.values.parallelBalance, values.parallelBalance, 1.0e-6f,
               "state round-trip retains parallel balance");
    expectNear(result.values.waterAmount, values.waterAmount, 1.0e-6f,
               "state round-trip retains Water amount");
    expectNear(result.values.iceAmount, values.iceAmount, 1.0e-6f,
               "state round-trip retains Ice amount");
    expectNear(result.values.inputGainDb, values.inputGainDb, 1.0e-6f,
               "state round-trip retains input gain");
    expectNear(result.values.globalMix, values.globalMix, 1.0e-6f,
               "state round-trip retains global mix");
    expectNear(result.values.outputGainDb, values.outputGainDb, 1.0e-6f,
               "state round-trip retains output gain");

    for (const auto routing :
         {RoutingMode::parallel, RoutingMode::waterIntoIce, RoutingMode::iceIntoWater}) {
        values.routing = routing;
        const auto routingResult = StateModel::deserialize(StateModel::serialize(values));
        expect(routingResult.status == StateModel::DeserializeStatus::current,
               "each routing mode uses the current state schema");
        expect(routingResult.values.routing == routing, "each routing mode round-trips");
    }
}

void testStateModelMigrationAndFallback() {
    const auto values = StateModel::defaultValues();
    const auto serialized = StateModel::serialize(values);

    auto legacy = serialized;
    legacy.schemaVersion = StateModel::kLegacySchemaVersion;
    const auto migrated = StateModel::deserialize(legacy);
    expect(migrated.status == StateModel::DeserializeStatus::migrated,
           "known legacy schema enters the migration path");

    auto schemaLessLegacy = serialized;
    schemaLessLegacy.schemaVersion.reset();
    const auto schemaLessResult = StateModel::deserialize(schemaLessLegacy);
    expect(schemaLessResult.status == StateModel::DeserializeStatus::migrated,
           "schema-less known parameter state enters the pre-v1 migration path");

    auto unknown = serialized;
    unknown.schemaVersion = StateModel::kCurrentSchemaVersion + 1U;
    const auto unknownResult = StateModel::deserialize(unknown);
    expect(unknownResult.status == StateModel::DeserializeStatus::fallback,
           "unknown schema falls back safely");
    expect(unknownResult.values.routing == values.routing,
           "unknown schema fallback uses safe default routing");

    auto outOfRange = serialized;
    outOfRange.parallelBalance = 2.0f;
    const auto outOfRangeResult = StateModel::deserialize(outOfRange);
    expect(outOfRangeResult.status == StateModel::DeserializeStatus::fallback,
           "out-of-range state value uses safe fallback");
    expectNear(outOfRangeResult.values.parallelBalance, values.parallelBalance, 1.0e-6f,
               "out-of-range state value falls back to the documented default");

    const auto emptyResult = StateModel::deserialize(StateModel::SerializedState{});
    expect(emptyResult.status == StateModel::DeserializeStatus::fallback,
           "empty state uses safe defaults");
    expect(emptyResult.values.waterEnabled && emptyResult.values.iceEnabled,
           "empty state restores default enabled values");
}

void testHostStateAdapterRoundTripAndInactiveRetention() {
    TestAudioProcessor sourceProcessor;
    juce::AudioProcessorValueTreeState source(sourceProcessor, nullptr, "FRAZIL",
                                              frazil::plugin::createParameterLayout());
    setParameterValue(source, frazil::plugin::parameterIds::waterEnabled, 0.0f);
    setParameterValue(source, frazil::plugin::parameterIds::iceEnabled, 1.0f);
    setParameterValue(source, frazil::plugin::parameterIds::routingMode, 2.0f);
    setParameterValue(source, frazil::plugin::parameterIds::parallelBalance, 0.25f);
    setParameterValue(source, frazil::plugin::parameterIds::waterAmount, 0.5f);
    setParameterValue(source, frazil::plugin::parameterIds::iceAmount, 0.75f);
    setParameterValue(source, frazil::plugin::parameterIds::inputGain, -6.0f);
    setParameterValue(source, frazil::plugin::parameterIds::globalMix, 0.4f);
    setParameterValue(source, frazil::plugin::parameterIds::outputGain, 3.0f);

    const auto serialized = frazil::plugin::HostStateAdapter::serialize(source);
    expect(serialized.hasType(juce::Identifier{"FRAZIL"}),
           "host state adapter emits the FRAZIL root type");
    expect(static_cast<int>(serialized.getProperty("schemaVersion")) ==
               static_cast<int>(StateModel::kCurrentSchemaVersion),
           "host state adapter emits schemaVersion");
    expect(serialized.getNumChildren() == 9,
           "host state adapter serializes all nine static parameters");

    TestAudioProcessor restoredProcessor;
    juce::AudioProcessorValueTreeState restored(restoredProcessor, nullptr, "FRAZIL",
                                                frazil::plugin::createParameterLayout());
    expect(frazil::plugin::HostStateAdapter::restore(restored, serialized),
           "current host state restores without fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::waterEnabled), 0.0f,
               1.0e-6f, "restored Water enable matches source");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::routingMode), 2.0f,
               1.0e-6f, "restored routing mode matches source");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::parallelBalance), 0.25f,
               1.0e-6f, "restored parallel balance matches source");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::waterAmount), 0.5f,
               1.0e-6f, "restored inactive Water amount matches source");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::iceAmount), 0.75f, 1.0e-6f,
               "restored inactive Ice amount matches source");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::inputGain), -6.0f, 1.0e-6f,
               "restored input gain matches source");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::globalMix), 0.4f, 1.0e-6f,
               "restored global mix matches source");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::outputGain), 3.0f, 1.0e-6f,
               "restored output gain matches source");
}

void testHostStateAdapterXmlRoundTrip() {
    TestAudioProcessor sourceProcessor;
    juce::AudioProcessorValueTreeState source(sourceProcessor, nullptr, "FRAZIL",
                                              frazil::plugin::createParameterLayout());
    setParameterValue(source, frazil::plugin::parameterIds::waterEnabled, 0.0f);
    setParameterValue(source, frazil::plugin::parameterIds::iceEnabled, 1.0f);
    setParameterValue(source, frazil::plugin::parameterIds::routingMode, 2.0f);
    setParameterValue(source, frazil::plugin::parameterIds::parallelBalance, 0.25f);
    setParameterValue(source, frazil::plugin::parameterIds::waterAmount, 0.5f);
    setParameterValue(source, frazil::plugin::parameterIds::iceAmount, 0.75f);
    setParameterValue(source, frazil::plugin::parameterIds::inputGain, -6.0f);
    setParameterValue(source, frazil::plugin::parameterIds::globalMix, 0.4f);
    setParameterValue(source, frazil::plugin::parameterIds::outputGain, 3.0f);

    const auto serialized = frazil::plugin::HostStateAdapter::serialize(source);
    const auto xml = serialized.createXml();
    expect(xml != nullptr, "host state creates XML for API round-trip");
    if (xml == nullptr)
        return;

    const auto fromXml = juce::ValueTree::fromXml(*xml);
    expect(fromXml.isValid(), "host state XML recreates a valid ValueTree");
    expect(fromXml.getProperty("schemaVersion").isString(),
           "ValueTree XML restore exposes schemaVersion as a string");
    const auto xmlGlobalMix = findParameterNode(fromXml, frazil::plugin::parameterIds::globalMix);
    expect(xmlGlobalMix.getProperty("value").isString(),
           "ValueTree XML restore exposes parameter values as strings");

    TestAudioProcessor restoredProcessor;
    juce::AudioProcessorValueTreeState restored(restoredProcessor, nullptr, "FRAZIL",
                                                frazil::plugin::createParameterLayout());
    expect(frazil::plugin::HostStateAdapter::restore(restored, fromXml),
           "XML/API state round-trip restores without fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::waterEnabled), 0.0f,
               1.0e-6f, "XML/API round-trip restores inactive Water enable");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::iceEnabled), 1.0f, 1.0e-6f,
               "XML/API round-trip restores Ice enable");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::routingMode), 2.0f,
               1.0e-6f, "XML/API round-trip restores routing mode");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::parallelBalance), 0.25f,
               1.0e-6f, "XML/API round-trip restores parallel balance");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::waterAmount), 0.5f,
               1.0e-6f, "XML/API round-trip restores inactive Water amount");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::iceAmount), 0.75f, 1.0e-6f,
               "XML/API round-trip restores inactive Ice amount");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::inputGain), -6.0f, 1.0e-6f,
               "XML/API round-trip restores input gain");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::globalMix), 0.4f, 1.0e-6f,
               "XML/API round-trip restores global mix");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::outputGain), 3.0f, 1.0e-6f,
               "XML/API round-trip restores output gain");
}

void testHostStateAdapterLegacyIdsAndInvalidFallback() {
    TestAudioProcessor sourceProcessor;
    juce::AudioProcessorValueTreeState source(sourceProcessor, nullptr, "FRAZIL",
                                              frazil::plugin::createParameterLayout());
    setParameterValue(source, frazil::plugin::parameterIds::waterEnabled, 0.0f);
    setParameterValue(source, frazil::plugin::parameterIds::iceEnabled, 1.0f);
    setParameterValue(source, frazil::plugin::parameterIds::routingMode, 1.0f);
    setParameterValue(source, frazil::plugin::parameterIds::waterAmount, 0.25f);
    setParameterValue(source, frazil::plugin::parameterIds::iceAmount, 0.75f);

    auto legacy = frazil::plugin::HostStateAdapter::serialize(source);
    legacy.removeProperty("schemaVersion", nullptr);
    for (int index = 0; index < legacy.getNumChildren(); ++index) {
        auto parameter = legacy.getChild(index);
        const auto id = parameter.getProperty("id").toString();
        if (id == frazil::plugin::parameterIds::waterEnabled)
            parameter.setProperty("id", "water.enable", nullptr);
        else if (id == frazil::plugin::parameterIds::iceEnabled)
            parameter.setProperty("id", "ice.enable", nullptr);
    }

    const auto decoded = frazil::plugin::HostStateAdapter::deserialize(legacy);
    expect(decoded.status == StateModel::DeserializeStatus::migrated,
           "legacy parameter IDs are recognized by the migration path");

    TestAudioProcessor restoredProcessor;
    juce::AudioProcessorValueTreeState restored(restoredProcessor, nullptr, "FRAZIL",
                                                frazil::plugin::createParameterLayout());
    expect(frazil::plugin::HostStateAdapter::restore(restored, legacy),
           "legacy state restores without fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::waterEnabled), 0.0f,
               1.0e-6f, "legacy Water ID migrates to the canonical ID");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::iceEnabled), 1.0f, 1.0e-6f,
               "legacy Ice ID migrates to the canonical ID");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::routingMode), 1.0f,
               1.0e-6f, "legacy state retains routing mode");

    auto unknownSchema = legacy;
    unknownSchema.setProperty("schemaVersion", 99, nullptr);
    expect(!frazil::plugin::HostStateAdapter::restore(restored, unknownSchema),
           "unknown schema reports safe fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::routingMode), 0.0f,
               1.0e-6f, "unknown schema restores default routing");

    auto malformed = frazil::plugin::HostStateAdapter::serialize(source);
    for (int index = malformed.getNumChildren() - 1; index >= 0; --index) {
        if (malformed.getChild(index).getProperty("id").toString() ==
            frazil::plugin::parameterIds::globalMix) {
            malformed.removeChild(index, nullptr);
            break;
        }
    }
    expect(!frazil::plugin::HostStateAdapter::restore(restored, malformed),
           "missing parameter reports safe fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::globalMix), 1.0f, 1.0e-6f,
               "missing parameter restores its documented default");

    auto wrongRoot = juce::ValueTree{"NOT_FRAZIL"};
    expect(!frazil::plugin::HostStateAdapter::restore(restored, wrongRoot),
           "malformed root reports safe fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::waterEnabled), 1.0f,
               1.0e-6f, "malformed root restores default Water enable");
}

void testHostStateAdapterParserRegressions() {
    TestAudioProcessor sourceProcessor;
    juce::AudioProcessorValueTreeState source(sourceProcessor, nullptr, "FRAZIL",
                                              frazil::plugin::createParameterLayout());
    setParameterValue(source, frazil::plugin::parameterIds::globalMix, 0.4f);
    const auto valid = frazil::plugin::HostStateAdapter::serialize(source);

    TestAudioProcessor restoredProcessor;
    juce::AudioProcessorValueTreeState restored(restoredProcessor, nullptr, "FRAZIL",
                                                frazil::plugin::createParameterLayout());

    auto duplicate = valid.createCopy();
    auto globalMixNode = findParameterNode(valid, frazil::plugin::parameterIds::globalMix);
    expect(globalMixNode.isValid(), "duplicate regression locates global mix parameter");
    if (globalMixNode.isValid())
        duplicate.appendChild(globalMixNode.createCopy(), nullptr);
    expect(!frazil::plugin::HostStateAdapter::restore(restored, duplicate),
           "duplicate known parameter reports safe fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::globalMix), 1.0f, 1.0e-6f,
               "duplicate known parameter restores the global mix default");

    auto nonnumericSchema = valid.createCopy();
    nonnumericSchema.setProperty("schemaVersion", "abc", nullptr);
    expect(!frazil::plugin::HostStateAdapter::restore(restored, nonnumericSchema),
           "nonnumeric schemaVersion reports safe fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::globalMix), 1.0f, 1.0e-6f,
               "nonnumeric schemaVersion restores the global mix default");

    auto nonnumericParameter = valid.createCopy();
    auto nonnumericParameterNode =
        findParameterNode(nonnumericParameter, frazil::plugin::parameterIds::globalMix);
    expect(nonnumericParameterNode.isValid(), "nonnumeric regression locates global mix parameter");
    if (nonnumericParameterNode.isValid())
        nonnumericParameterNode.setProperty("value", "abc", nullptr);
    expect(!frazil::plugin::HostStateAdapter::restore(restored, nonnumericParameter),
           "nonnumeric parameter value reports safe fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::globalMix), 1.0f, 1.0e-6f,
               "nonnumeric parameter value restores the global mix default");

    auto partialNumericParameter = valid.createCopy();
    auto partialNumericNode =
        findParameterNode(partialNumericParameter, frazil::plugin::parameterIds::globalMix);
    expect(partialNumericNode.isValid(), "partial numeric regression locates global mix parameter");
    if (partialNumericNode.isValid())
        partialNumericNode.setProperty("value", "0.4trailing", nullptr);
    expect(!frazil::plugin::HostStateAdapter::restore(restored, partialNumericParameter),
           "partial numeric parameter value reports safe fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::globalMix), 1.0f, 1.0e-6f,
               "partial numeric parameter value restores the global mix default");

    auto malformedBool = valid.createCopy();
    auto malformedBoolNode =
        findParameterNode(malformedBool, frazil::plugin::parameterIds::waterEnabled);
    expect(malformedBoolNode.isValid(), "malformed bool regression locates Water parameter");
    if (malformedBoolNode.isValid())
        malformedBoolNode.setProperty("value", 0.5f, nullptr);
    expect(!frazil::plugin::HostStateAdapter::restore(restored, malformedBool),
           "malformed bool value reports safe fallback");
    expectNear(getParameterValue(restored, frazil::plugin::parameterIds::waterEnabled), 1.0f,
               1.0e-6f, "malformed bool value restores the Water enable default");
}

void testLinearSmootherReachesTarget() {
    LinearSmoother smoother;
    smoother.prepare(1000.0, 0.010);
    smoother.reset(0.0f);
    smoother.setTarget(1.0f);

    for (int sample = 0; sample < 9; ++sample)
        expect(smoother.getNextValue() < 1.0f, "smoother ramps before its endpoint");

    expectNear(smoother.getNextValue(), 1.0f, 1.0e-6f, "smoother reaches target at ramp end");
    expect(smoother.getRemainingSamples() == 0, "smoother has no remaining samples at endpoint");
}

void testLinearSmootherIsBlockSizeStable() {
    constexpr std::array<int, 6> blockSizes{16, 32, 64, 128, 256, 512};
    constexpr std::array<double, 3> sampleRates{44100.0, 48000.0, 96000.0};

    for (const auto sampleRate : sampleRates) {
        for (const auto blockSize : blockSizes) {
            LinearSmoother smoother;
            smoother.prepare(sampleRate, 0.010);
            smoother.reset(0.0f);
            smoother.setTarget(1.0f);

            const auto rampSamples = static_cast<int>(std::lround(sampleRate * 0.010));
            int processedSamples = 0;
            while (processedSamples < rampSamples) {
                const auto samplesThisBlock = std::min(blockSize, rampSamples - processedSamples);
                const auto remainingBeforeRepeatedTarget = smoother.getRemainingSamples();
                smoother.setTarget(1.0f);
                expect(smoother.getRemainingSamples() == remainingBeforeRepeatedTarget,
                       "repeated block target does not restart an in-flight ramp");

                for (int sample = 0; sample < samplesThisBlock; ++sample) {
                    const auto value = smoother.getNextValue();
                    ++processedSamples;
                    expect(std::isfinite(value), "block-ramped smoother output is finite");
                    if (processedSamples < rampSamples)
                        expect(value < 1.0f, "block-ramped smoother reaches target only at end");
                    else
                        expectNear(value, 1.0f, 1.0e-6f,
                                   "block-ramped smoother reaches target at sample duration");
                }
            }

            expect(smoother.getRemainingSamples() == 0,
                   "block-ramped smoother has no remaining samples at endpoint");
        }
    }
}

void testLinearSmootherRetargetsFromCurrentValue() {
    constexpr int rampSamples = 480;
    LinearSmoother smoother;
    smoother.prepare(48000.0, 0.010);
    smoother.reset(0.0f);
    smoother.setTarget(1.0f);

    for (int sample = 0; sample < 100; ++sample)
        static_cast<void>(smoother.getNextValue());

    const auto currentBeforeRetarget = smoother.getCurrentValue();
    smoother.setTarget(0.25f);
    expectNear(smoother.getCurrentValue(), currentBeforeRetarget, 1.0e-6f,
               "retarget keeps the current value as its new ramp origin");
    expect(smoother.getRemainingSamples() == rampSamples, "retarget establishes a fresh full ramp");

    for (int sample = 0; sample < rampSamples; ++sample) {
        const auto value = smoother.getNextValue();
        expect(std::isfinite(value), "retargeted smoother output is finite");
        if (sample == rampSamples - 1)
            expectNear(value, 0.25f, 1.0e-6f, "retargeted smoother reaches its new target");
    }
    expect(smoother.getRemainingSamples() == 0,
           "retargeted smoother has no remaining samples at endpoint");
}

void testDryWetMixerEndpointsAndMonotonicity() {
    expectNear(DryWetMixer::mix(0.25f, 0.75f, 0.0f), 0.25f, 1.0e-6f,
               "dry/wet mix zero is the dry endpoint");
    expectNear(DryWetMixer::mix(0.25f, 0.75f, 1.0f), 0.75f, 1.0e-6f,
               "dry/wet mix one is the wet endpoint");
    expect(DryWetMixer::mix(0.0f, 1.0f, 0.25f) < DryWetMixer::mix(0.0f, 1.0f, 0.75f),
           "dry/wet mix is monotonic between endpoints");
}

void testRandomSourceIsDeterministicAndInstanceLocal() {
    RandomSource first{42u};
    RandomSource second{42u};
    for (int index = 0; index < 8; ++index)
        expect(first.nextUInt() == second.nextUInt(), "fixed seed produces a repeatable sequence");

    first.reseed(42u);
    second.reseed(42u);
    expect(first.nextUInt() == second.nextUInt(), "reseed restores the deterministic sequence");
    expect(RandomSource::deriveInstanceSeed(42u, 0) != RandomSource::deriveInstanceSeed(42u, 1),
           "instance seed derivation decorrelates instances");

    const auto value = first.nextUnipolar();
    expect(value >= 0.0f && value < 1.0f, "random unipolar value stays in the documented range");
}

void testAudioEngineAppliesGainStaging() {
    AudioEngine engine;
    expect(engine.prepare(ProcessSpec{48000.0, 480, 1}), "valid engine prepare succeeds");

    juce::AudioBuffer<float> inputGainBuffer(1, 480);
    for (int sample = 0; sample < inputGainBuffer.getNumSamples(); ++sample)
        inputGainBuffer.setSample(0, sample, 1.0f);

    EngineParameters parameters;
    parameters.inputGainLinear = 2.0f;
    parameters.globalMix = 0.0f;
    engine.process(inputGainBuffer, parameters);
    expectNear(inputGainBuffer.getSample(0, 479), 2.0f, 1.0e-5f,
               "input gain reaches its target before the dry/wet mix");

    engine.reset();
    juce::AudioBuffer<float> outputGainBuffer(1, 480);
    for (int sample = 0; sample < outputGainBuffer.getNumSamples(); ++sample)
        outputGainBuffer.setSample(0, sample, 1.0f);
    parameters.inputGainLinear = 1.0f;
    parameters.globalMix = 1.0f;
    parameters.outputGainLinear = 2.0f;
    engine.process(outputGainBuffer, parameters);
    expectNear(outputGainBuffer.getSample(0, 479), 2.0f, 1.0e-5f,
               "output gain is applied after the global mix");
}

void fillBuffer(juce::AudioBuffer<float>& buffer, float value) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, value);
}

bool isFiniteBuffer(const juce::AudioBuffer<float>& buffer) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            if (!std::isfinite(buffer.getSample(channel, sample)))
                return false;

    return true;
}

void testAudioEnginePrimesParametersOnFirstBlock() {
    AudioEngine engine;
    const ProcessSpec spec{48000.0, 64, 1};
    expect(engine.prepare(spec), "engine prepare succeeds for first-block priming");

    EngineParameters parameters;
    parameters.inputGainLinear = 0.5f;
    parameters.globalMix = 0.25f;
    parameters.outputGainLinear = 1.5f;

    juce::AudioBuffer<float> firstBlock(1, spec.maximumBlockSize);
    fillBuffer(firstBlock, 1.0f);
    engine.process(firstBlock, parameters);
    // M1 wet is post-input pass-through, so globalMix does not alter identity. This checks that
    // input/output gain use live state from sample 0 rather than ramping from unity defaults.
    expectNear(firstBlock.getSample(0, 0), 0.75f, 1.0e-6f,
               "prepare first block starts at the live input/output gain state");
    expectNear(firstBlock.getSample(0, spec.maximumBlockSize - 1), 0.75f, 1.0e-6f,
               "prepare first block remains at the live gain state");

    engine.reset();
    parameters.inputGainLinear = 0.25f;
    parameters.globalMix = 0.75f;
    parameters.outputGainLinear = 0.5f;
    juce::AudioBuffer<float> resetBlock(1, spec.maximumBlockSize);
    fillBuffer(resetBlock, 1.0f);
    engine.process(resetBlock, parameters);
    expectNear(resetBlock.getSample(0, 0), 0.125f, 1.0e-6f,
               "reset first block starts at the new live gain state");
}

void testAudioEngineHandlesRuntimeBufferInvariantViolations() {
    AudioEngine engine;
    expect(engine.prepare(ProcessSpec{48000.0, 32, 1}),
           "engine prepare succeeds for runtime buffer invariant test");

    EngineParameters parameters;
    parameters.inputGainLinear = 0.5f;
    parameters.globalMix = 0.25f;
    parameters.outputGainLinear = 1.0f;

    juce::AudioBuffer<float> oversizedSamples(1, 64);
    fillBuffer(oversizedSamples, 1.0f);
    engine.process(oversizedSamples, parameters);
    expect(isFiniteBuffer(oversizedSamples),
           "oversized sample buffer remains finite without audio-thread resize");
    expectNear(oversizedSamples.getSample(0, 0), 0.5f, 1.0e-6f,
               "oversized sample buffer uses the documented deterministic fallback");

    juce::AudioBuffer<float> oversizedChannels(2, 32);
    fillBuffer(oversizedChannels, 1.0f);
    engine.process(oversizedChannels, parameters);
    expect(isFiniteBuffer(oversizedChannels),
           "oversized channel buffer remains finite without audio-thread resize");
}

void testAudioEngineResetAndZeroLengthAreSafe() {
    AudioEngine engine;
    expect(!engine.prepare(ProcessSpec{0.0, 32, 1}), "invalid engine spec is rejected");

    juce::AudioBuffer<float> buffer(1, 0);
    engine.process(buffer, EngineParameters{});
    expect(true, "zero-length buffer is accepted without processing");
}
} // namespace

int main() {
    testProcessSpecValidation();
    testParameterLayoutContract();
    testParameterSnapshotReadsOneCoherentSet();
    testParameterMapperClampsAndConverts();
    testStateModelRoundTrip();
    testStateModelMigrationAndFallback();
    testHostStateAdapterRoundTripAndInactiveRetention();
    testHostStateAdapterXmlRoundTrip();
    testHostStateAdapterLegacyIdsAndInvalidFallback();
    testHostStateAdapterParserRegressions();
    testLinearSmootherReachesTarget();
    testLinearSmootherIsBlockSizeStable();
    testLinearSmootherRetargetsFromCurrentValue();
    testDryWetMixerEndpointsAndMonotonicity();
    testRandomSourceIsDeterministicAndInstanceLocal();
    testAudioEngineAppliesGainStaging();
    testAudioEnginePrimesParametersOnFirstBlock();
    testAudioEngineHandlesRuntimeBufferInvariantViolations();
    testAudioEngineResetAndZeroLengthAreSafe();

    if (failures != 0)
        return 1;

    std::cout << "FRAZIL unit tests passed (19 groups)\n";
    return 0;
}
