#include "DryWetMixer.h"

float DryWetMixer::mix(float drySample, float wetSample, float mixAmount) noexcept {
    return drySample + mixAmount * (wetSample - drySample);
}
