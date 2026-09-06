#include "AudioEngine.h"

#include "../dsp/DryWetMixer.h"

bool AudioEngine::prepare(const ProcessSpec& spec) noexcept {
    if (!spec.isValid()) {
        dryReference_.setSize(0, 0, false, false, true);
        prepared_ = false;
        return false;
    }

    dryReference_.setSize(spec.numChannels, spec.maximumBlockSize, false, true, false);
    inputGainSmoother_.prepare(spec.sampleRate, kParameterRampSeconds);
    globalMixSmoother_.prepare(spec.sampleRate, kParameterRampSeconds);
    outputGainSmoother_.prepare(spec.sampleRate, kParameterRampSeconds);
    reset();
    prepared_ = true;
    return true;
}

void AudioEngine::reset() noexcept {
    inputGainSmoother_.reset(1.0f);
    globalMixSmoother_.reset(1.0f);
    outputGainSmoother_.reset(1.0f);
    dryReference_.clear();
}

void AudioEngine::process(juce::AudioBuffer<float>& buffer,
                          const EngineParameters& parameters) noexcept {
    if (!prepared_ || buffer.getNumSamples() <= 0 || buffer.getNumChannels() <= 0)
        return;

    inputGainSmoother_.setTarget(parameters.inputGainLinear);
    globalMixSmoother_.setTarget(parameters.globalMix);
    outputGainSmoother_.setTarget(parameters.outputGainLinear);

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    const bool canStoreDryReference = numChannels <= dryReference_.getNumChannels() &&
                                      numSamples <= dryReference_.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample) {
        const auto inputGain = inputGainSmoother_.getNextValue();
        for (int channel = 0; channel < numChannels; ++channel)
            buffer.getWritePointer(channel)[sample] *= inputGain;

        if (canStoreDryReference) {
            for (int channel = 0; channel < numChannels; ++channel)
                dryReference_.getWritePointer(channel)[sample] = buffer.getSample(channel, sample);
        }
    }

    // M1 has no Water/Ice transform yet, so the wet path is the post-input signal. Keeping the
    // dry reference and mix law here makes the future RoutingEngine insertion explicit.
    for (int sample = 0; sample < numSamples; ++sample) {
        const auto globalMix = globalMixSmoother_.getNextValue();
        const auto outputGain = outputGainSmoother_.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel) {
            const auto wetSample = buffer.getSample(channel, sample);
            const auto drySample =
                canStoreDryReference ? dryReference_.getSample(channel, sample) : wetSample;
            const auto mixedSample = DryWetMixer::mix(drySample, wetSample, globalMix);
            buffer.setSample(channel, sample, mixedSample * outputGain);
        }
    }
}
