#pragma once

class DryWetMixer final {
  public:
    // mixAmount is expected in [0, 1]: 0 returns drySample and 1 returns wetSample. The law is
    // linear interpolation without clamping; callers validate parameter ranges before entering
    // this realtime-safe, allocation-free primitive.
    static float mix(float drySample, float wetSample, float mixAmount) noexcept;
};
