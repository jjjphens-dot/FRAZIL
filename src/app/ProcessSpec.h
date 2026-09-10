#pragma once

#include <cmath>

struct ProcessSpec final {
    double sampleRate{};    // Hz; finite and greater than zero.
    int maximumBlockSize{}; // Samples per callback; greater than zero.
    int numChannels{};      // Prepared audio channels; greater than zero.

    bool isValid() const noexcept {
        return std::isfinite(sampleRate) && sampleRate > 0.0 && maximumBlockSize > 0 &&
               numChannels > 0;
    }
};
