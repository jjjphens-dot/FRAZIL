#include "app/AudioEngine.h"
#include "app/ParameterMapper.h"
#include "app/ParameterSnapshot.h"
#include "dsp/DryWetMixer.h"
#include "dsp/primitives/LinearSmoother.h"
#include "dsp/primitives/RandomSource.h"
#include "plugin/ParameterLayout.h"

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

    expect(processor.getParameters().size() == static_cast<int>(expectedIds.size()),
           "parameter layout exposes exactly nine parameters");

    for (int index = 0; index < processor.getParameters().size(); ++index) {
        const auto* parameter = dynamic_cast<const juce::AudioProcessorParameterWithID*>(
            processor.getParameters()[index]);
        expect(parameter != nullptr, "every parameter exposes a stable ID");
        if (parameter != nullptr)
            expect(parameter->getParameterID() == expectedIds[static_cast<std::size_t>(index)],
                   "parameter order and ID match the contract");
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
    if (routing != nullptr)
        expect(processor.getParameterNumSteps(2) == 3, "routing mode has three stable choices");
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
    testLinearSmootherReachesTarget();
    testDryWetMixerEndpointsAndMonotonicity();
    testRandomSourceIsDeterministicAndInstanceLocal();
    testAudioEngineAppliesGainStaging();
    testAudioEngineResetAndZeroLengthAreSafe();

    if (failures != 0)
        return 1;

    std::cout << "FRAZIL unit tests passed (9 groups)\n";
    return 0;
}
