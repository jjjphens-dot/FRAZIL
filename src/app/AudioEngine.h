#pragma once

#include "../dsp/primitives/LinearSmoother.h"
#include "EngineParameters.h"
#include "ProcessSpec.h"

#include <juce_audio_basics/juce_audio_basics.h>

class AudioEngine {
  public:
    bool prepare(const ProcessSpec&) noexcept;
    void reset() noexcept;
    void process(juce::AudioBuffer<float>& buffer, const EngineParameters&) noexcept;

  private:
    static constexpr double kParameterRampSeconds = 0.010;

    juce::AudioBuffer<float> dryReference_;
    LinearSmoother inputGainSmoother_;
    LinearSmoother globalMixSmoother_;
    LinearSmoother outputGainSmoother_;
    bool prepared_{};
};
