#pragma once

class LinearSmoother final {
  public:
    void prepare(double sampleRate, double rampSeconds) noexcept;
    void reset(float value) noexcept;
    // Repeated targets preserve an in-flight ramp; a changed target retargets from currentValue_.
    void setTarget(float target) noexcept;
    float getNextValue() noexcept;

    float getCurrentValue() const noexcept {
        return currentValue_;
    }
    int getRemainingSamples() const noexcept {
        return remainingSamples_;
    }

  private:
    // All state is owned by the audio engine instance; prepare/reset happen outside process().
    float currentValue_{};
    float targetValue_{};
    float step_{};
    int rampSamples_{1};
    int remainingSamples_{};
};
