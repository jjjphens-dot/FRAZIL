#pragma once

#include "dsp/WaterExcitationFeatures.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>
#include <numbers>
#include <vector>

namespace frazil::water::preview {
// ENGINEERING, monitor boundary only. The producer owns source-rate DSP and returns
// false AFTER its final tail frame. No renderer or source analyzer uses this class.
// prepare may allocate; render/reset use only prepared storage. Single audio owner.
class PreviewMonitorResampler final {
  public:
    bool prepare(double dspRate, double deviceRate) {
        ready_ = std::isfinite(dspRate) && std::isfinite(deviceRate) && dspRate >= 44100 &&
                 dspRate <= 96000 && deviceRate >= 8000 && deviceRate <= 384000;
        if (!ready_)
            return false;
        ratio_ = dspRate / deviceRate;
        active_ = std::abs(dspRate - deviceRate) > .5;
        for (auto& channel : input_)
            channel.resize(static_cast<std::size_t>(std::ceil(ratio_ * kChunk)) + 2);
        antiAlias_ = ratio_ > 1.00001;
        coefficients_.fill(0);
        if (antiAlias_) {
            const double cutoff = .45 / ratio_; // Explicit monitor-only anti-alias bandwidth.
            double sum{};
            for (int i = 0; i < kFilterTaps; ++i) {
                const int x = i - (kFilterTaps - 1) / 2;
                const double sinc =
                    x == 0 ? 2 * cutoff
                           : std::sin(2 * std::numbers::pi * cutoff * x) / (std::numbers::pi * x);
                coefficients_[i] =
                    sinc * (.42 - .5 * std::cos(2 * std::numbers::pi * i / (kFilterTaps - 1)) +
                            .08 * std::cos(4 * std::numbers::pi * i / (kFilterTaps - 1)));
                sum += coefficients_[i];
            }
            for (auto& c : coefficients_)
                c /= sum;
        }
        reset();
        return true;
    }
    void reset() noexcept {
        for (auto& channel : interpolators_)
            channel.reset();
        buffered_ = zeroFrames_ = 0;
        history_ = {};
        filterIndex_ = 0;
        sourceEnded_ = finished_ = false;
        consumed_ = 0;
    }
    template <class Producer>
    void render(float* const* outputs, int channels, int count, Producer&& producer) noexcept {
        for (int c = 0; c < channels; ++c)
            if (outputs[c])
                std::fill_n(outputs[c], count, 0.0f);
        if (!ready_ || finished_)
            return;
        if (!active_) {
            for (int n = 0; n < count; ++n) {
                research::StereoFrame value{};
                if (!producer(value)) {
                    finished_ = true;
                    break;
                }
                ++consumed_;
                for (int c = 0; c < std::min(2, channels); ++c)
                    if (outputs[c])
                        outputs[c][n] = value[static_cast<std::size_t>(c)];
            }
            return;
        }
        for (int offset = 0; offset < count && !finished_;) {
            const int chunk = std::min(kChunk, count - offset);
            const int needed = static_cast<int>(std::ceil(ratio_ * chunk)) + 1;
            for (int n = buffered_; n < needed; ++n) {
                research::StereoFrame value{};
                if (!sourceEnded_) {
                    sourceEnded_ = !producer(value);
                    if (!sourceEnded_)
                        ++consumed_;
                }
                if (sourceEnded_)
                    ++zeroFrames_;
                if (antiAlias_)
                    value = filter(value);
                for (std::size_t c = 0; c < 2; ++c)
                    input_[c][static_cast<std::size_t>(n)] = value[c];
            }
            int used{};
            for (std::size_t c = 0; c < 2; ++c) {
                used =
                    interpolators_[c].process(ratio_, input_[c].data(), output_[c].data(), chunk);
                if (static_cast<int>(c) < channels && outputs[c])
                    std::copy_n(output_[c].data(), chunk, outputs[c] + offset);
                std::move(input_[c].begin() + used, input_[c].begin() + needed, input_[c].begin());
            }
            buffered_ = needed - used;
            // JUCE 9 WindowedSinc uses 200 input history samples, symmetric about
            // getBaseLatency()==100. Drain the full support, not just the group delay.
            // Subtract buffered lookahead: only samples consumed by JUCE drain state.
            finished_ = sourceEnded_ && zeroFrames_ - buffered_ >=
                                            supportFrames() + (antiAlias_ ? kFilterTaps - 1 : 0);
            offset += chunk;
        }
    }
    bool active() const noexcept {
        return active_;
    }
    bool finished() const noexcept {
        return finished_;
    }
    double ratio() const noexcept {
        return ratio_;
    }
    double latencyDeviceFrames() const noexcept {
        return active_ ? (juce::WindowedSincInterpolator::getBaseLatency() +
                          (antiAlias_ ? (kFilterTaps - 1) / 2 : 0)) /
                             ratio_
                       : 0;
    }
    std::uint64_t producedDspFrames() const noexcept {
        return consumed_;
    }
    static constexpr int supportFrames() noexcept {
        return 2 * static_cast<int>(juce::WindowedSincInterpolator::getBaseLatency());
    }

  private:
    research::StereoFrame filter(const research::StereoFrame& value) noexcept {
        research::StereoFrame result{};
        for (std::size_t c = 0; c < 2; ++c) {
            history_[c][filterIndex_] = value[c];
            double sum{};
            for (int i = 0; i < kFilterTaps; ++i)
                sum +=
                    coefficients_[i] * history_[c][(filterIndex_ + kFilterTaps - i) % kFilterTaps];
            result[c] = static_cast<float>(sum);
        }
        filterIndex_ = (filterIndex_ + 1) % kFilterTaps;
        return result;
    }
    static constexpr int kChunk = 256;
    static constexpr int kFilterTaps = 129;
    std::array<double, kFilterTaps> coefficients_{};
    std::array<std::array<float, kFilterTaps>, 2> history_{};
    int filterIndex_{}; // Prepared monitor anti-alias FIR, input-rate circular index.
    bool antiAlias_{};
    std::array<juce::WindowedSincInterpolator, 2> interpolators_;
    std::array<std::vector<float>, 2> input_;
    std::array<std::array<float, kChunk>, 2> output_{};
    // Input-rate lookahead and consumed zero-feed counters, reset on every restart.
    int buffered_{}, zeroFrames_{};
    std::uint64_t consumed_{};
    double ratio_{1};
    bool ready_{}, active_{}, sourceEnded_{}, finished_{};
};

inline research::StereoFrame canonicalStereo(float left, float right,
                                             int originalChannels) noexcept {
    return {left, originalChannels == 1 ? left : right};
}
} // namespace frazil::water::preview
