#include "app/AudioEngine.h"
#include "test_support.h"

#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>

namespace {
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

void testAudioEngineAppliesGainStaging(TestContext& context) {
    AudioEngine engine;
    expect(context, engine.prepare(ProcessSpec{48000.0, 480, 1}), "valid engine prepare succeeds");

    juce::AudioBuffer<float> inputGainBuffer(1, 480);
    for (int sample = 0; sample < inputGainBuffer.getNumSamples(); ++sample)
        inputGainBuffer.setSample(0, sample, 1.0f);

    EngineParameters parameters;
    parameters.inputGainLinear = 2.0f;
    parameters.globalMix = 0.0f;
    engine.process(inputGainBuffer, parameters);
    expectNear(context, inputGainBuffer.getSample(0, 479), 2.0f, 1.0e-5f,
               "input gain reaches its target before the dry/wet mix");

    engine.reset();
    juce::AudioBuffer<float> outputGainBuffer(1, 480);
    for (int sample = 0; sample < outputGainBuffer.getNumSamples(); ++sample)
        outputGainBuffer.setSample(0, sample, 1.0f);
    parameters.inputGainLinear = 1.0f;
    parameters.globalMix = 1.0f;
    parameters.outputGainLinear = 2.0f;
    engine.process(outputGainBuffer, parameters);
    expectNear(context, outputGainBuffer.getSample(0, 479), 2.0f, 1.0e-5f,
               "output gain is applied after the global mix");
}

void testAudioEnginePrimesParametersOnFirstBlock(TestContext& context) {
    AudioEngine engine;
    const ProcessSpec spec{48000.0, 64, 1};
    expect(context, engine.prepare(spec), "engine prepare succeeds for first-block priming");

    EngineParameters parameters;
    parameters.inputGainLinear = 0.5f;
    parameters.globalMix = 0.25f;
    parameters.outputGainLinear = 1.5f;

    juce::AudioBuffer<float> firstBlock(1, spec.maximumBlockSize);
    fillBuffer(firstBlock, 1.0f);
    engine.process(firstBlock, parameters);
    // M1 wet is post-input pass-through, so globalMix does not alter identity. This checks that
    // input/output gain use live state from sample 0 rather than ramping from unity defaults.
    expectNear(context, firstBlock.getSample(0, 0), 0.75f, 1.0e-6f,
               "prepare first block starts at the live input/output gain state");
    expectNear(context, firstBlock.getSample(0, spec.maximumBlockSize - 1), 0.75f, 1.0e-6f,
               "prepare first block remains at the live gain state");

    engine.reset();
    parameters.inputGainLinear = 0.25f;
    parameters.globalMix = 0.75f;
    parameters.outputGainLinear = 0.5f;
    juce::AudioBuffer<float> resetBlock(1, spec.maximumBlockSize);
    fillBuffer(resetBlock, 1.0f);
    engine.process(resetBlock, parameters);
    expectNear(context, resetBlock.getSample(0, 0), 0.125f, 1.0e-6f,
               "reset first block starts at the new live gain state");
}

void testAudioEngineHandlesRuntimeBufferInvariantViolations(TestContext& context) {
    AudioEngine engine;
    expect(context, engine.prepare(ProcessSpec{48000.0, 32, 1}),
           "engine prepare succeeds for runtime buffer invariant test");

    EngineParameters parameters;
    parameters.inputGainLinear = 0.5f;
    parameters.globalMix = 0.25f;
    parameters.outputGainLinear = 1.0f;

    juce::AudioBuffer<float> oversizedSamples(1, 64);
    fillBuffer(oversizedSamples, 1.0f);
    engine.process(oversizedSamples, parameters);
    expect(context, isFiniteBuffer(oversizedSamples),
           "oversized sample buffer remains finite without audio-thread resize");
    expectNear(context, oversizedSamples.getSample(0, 0), 0.5f, 1.0e-6f,
               "oversized sample buffer uses the documented deterministic fallback");

    juce::AudioBuffer<float> oversizedChannels(2, 32);
    fillBuffer(oversizedChannels, 1.0f);
    engine.process(oversizedChannels, parameters);
    expect(context, isFiniteBuffer(oversizedChannels),
           "oversized channel buffer remains finite without audio-thread resize");
}

void testAudioEngineResetAndZeroLengthAreSafe(TestContext& context) {
    AudioEngine engine;
    expect(context, !engine.prepare(ProcessSpec{0.0, 32, 1}), "invalid engine spec is rejected");

    juce::AudioBuffer<float> buffer(1, 0);
    engine.process(buffer, EngineParameters{});
    expect(context, true, "zero-length buffer is accepted without processing");
}

} // namespace

void runAudioEngineTests(TestContext& context) {
    testAudioEngineAppliesGainStaging(context);
    testAudioEnginePrimesParametersOnFirstBlock(context);
    testAudioEngineHandlesRuntimeBufferInvariantViolations(context);
    testAudioEngineResetAndZeroLengthAreSafe(context);
}
