#pragma once

#include "../dsp/primitives/LinearSmoother.h"
#include "EngineParameters.h"
#include "ProcessSpec.h"

#include <juce_audio_basics/juce_audio_basics.h>

class AudioEngine {
  public:
    // Prepare allocates all callback storage for the supplied sample rate, maximum block size,
    // and channel count. It must complete before process() is called; false leaves the engine
    // unprepared.
    bool prepare(const ProcessSpec&) noexcept;

    // Reset clears callback state and returns parameter smoothers to their neutral values.
    void reset() noexcept;

    // Process is realtime-safe after prepare(): it performs no I/O, locking, or allocation. The
    // caller supplies finite, mapped EngineParameters and a buffer within the prepared bounds.
    void process(juce::AudioBuffer<float>& buffer, const EngineParameters&) noexcept;

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
