#include "app/AudioEngine.h"
#include "app/ParameterMapper.h"
#include "app/ParameterSnapshot.h"
#include "dsp/DryWetMixer.h"
#include "dsp/primitives/LinearSmoother.h"
#include "dsp/primitives/RandomSource.h"
#include "plugin/ParameterLayout.h"

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
    parameters.outputGainLinear = 2.0f;

    juce::AudioBuffer<float> firstBlock(1, spec.maximumBlockSize);
    fillBuffer(firstBlock, 1.0f);
    engine.process(firstBlock, parameters);
    // M1 wet is post-input pass-through, so globalMix does not alter identity. This checks that
    // input/output gain use live state from sample 0 rather than ramping from unity defaults.
    expectNear(firstBlock.getSample(0, 0), 1.0f, 1.0e-6f,
               "prepare first block starts at the live input/output gain state");
    expectNear(firstBlock.getSample(0, spec.maximumBlockSize - 1), 1.0f, 1.0e-6f,
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

    std::cout << "FRAZIL unit tests passed (13 groups)\n";
    return 0;
}
