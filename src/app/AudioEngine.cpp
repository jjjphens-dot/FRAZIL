#include "AudioEngine.h"

void AudioEngine::prepare(double sampleRate, int maximumBlockSize, int numChannels) noexcept
{
    sampleRate_ = sampleRate;
    maximumBlockSize_ = maximumBlockSize;
    numChannels_ = numChannels;
}

void AudioEngine::reset() noexcept
{
}

void AudioEngine::process(juce::AudioBuffer<float>&) noexcept
{
    // M0 pass-through. Water/Ice execution belongs in src/dsp/ in later milestones.
}
