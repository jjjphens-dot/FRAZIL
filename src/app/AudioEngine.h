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
    // When dryReferenceOnly is true, use the prepared reference path instead of the wet path.
    // This is a developer comparison hook; the default preserves the production processing path.
    void process(juce::AudioBuffer<float>& buffer, const EngineParameters&,
                 bool dryReferenceOnly) noexcept;

  private:
    static constexpr double kParameterRampSeconds = 0.010;

    juce::AudioBuffer<float> dryReference_;
    LinearSmoother inputGainSmoother_;
    LinearSmoother globalMixSmoother_;
    LinearSmoother outputGainSmoother_;
    // Set false by prepare/reset; the first valid process block primes smoothers from live
    // EngineParameters so restored values do not ramp from arbitrary defaults.
    bool parameterStatePrimed_{};
    bool prepared_{};
};
