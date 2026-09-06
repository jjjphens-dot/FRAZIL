#pragma once

#include <cmath>

struct ProcessSpec final {
    double sampleRate{};
    int maximumBlockSize{};
    int numChannels{};

    bool isValid() const noexcept {
        return std::isfinite(sampleRate) && sampleRate > 0.0 && maximumBlockSize > 0 &&
               numChannels > 0;
    }
};
