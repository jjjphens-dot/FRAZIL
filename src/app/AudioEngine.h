#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

class AudioEngine
{
public:
    void prepare(double sampleRate, int maximumBlockSize, int numChannels) noexcept;
    void reset() noexcept;
    void process(juce::AudioBuffer<float>& buffer) noexcept;

private:
    double sampleRate_ {};
    int maximumBlockSize_ {};
    int numChannels_ {};
};
