#include "LinearSmoother.h"

#include <algorithm>
#include <cmath>

void LinearSmoother::prepare(double sampleRate, double rampSeconds) noexcept {
    if (!std::isfinite(sampleRate) || sampleRate <= 0.0 || !std::isfinite(rampSeconds) ||
        rampSeconds <= 0.0) {
        rampSamples_ = 1;
        return;
    }

    const auto requestedSamples = static_cast<int>(std::lround(sampleRate * rampSeconds));
    rampSamples_ = std::max(1, requestedSamples);
}

void LinearSmoother::reset(float value) noexcept {
    currentValue_ = std::isfinite(value) ? value : 0.0f;
    targetValue_ = currentValue_;
    step_ = 0.0f;
    remainingSamples_ = 0;
}

void LinearSmoother::setTarget(float target) noexcept {
    targetValue_ = std::isfinite(target) ? target : currentValue_;

    if (rampSamples_ <= 1 || targetValue_ == currentValue_) {
        currentValue_ = targetValue_;
        step_ = 0.0f;
        remainingSamples_ = 0;
        return;
    }

    step_ = (targetValue_ - currentValue_) / static_cast<float>(rampSamples_);
    remainingSamples_ = rampSamples_;
}

float LinearSmoother::getNextValue() noexcept {
    if (remainingSamples_ > 0) {
        --remainingSamples_;
        if (remainingSamples_ == 0)
            currentValue_ = targetValue_;
        else
            currentValue_ += step_;
    }

    return currentValue_;
}
