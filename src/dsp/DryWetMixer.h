#pragma once

class DryWetMixer final {
  public:
    static float mix(float drySample, float wetSample, float mixAmount) noexcept;
};
