#pragma once

#include "dsp/WaterExcitationFeatures.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace frazil::water::preview {
enum class WaterSignal : std::size_t {
    totalPreProtect,
    bubble,
    droplet,
    flow,
    modal,
    postProtect,
    count
};
struct WaterFrameReadout final {
    std::array<research::StereoFrame, static_cast<std::size_t>(WaterSignal::count)> signals{};
    research::StereoFrame& at(WaterSignal signal) noexcept {
        return signals[static_cast<std::size_t>(signal)];
    }
};
struct WaterActivity final {
    std::uint64_t bubbleEvents{}, dropletEvents{}, bubbleSteals{};
    std::size_t bubbleActive{}, dropletActive{};
    double flowDelayMs{}, modalRootHz{}, modalDecaySeconds{}, modalMotionDepth{},
        modalMotionIntervalSeconds{};
};
struct WaterLevel final {
    double peak{}, squares{};
    std::uint64_t samples{};
    void include(const research::StereoFrame& frame, int channels) noexcept {
        for (int i = 0; i < channels; ++i) {
            const double value = frame[static_cast<std::size_t>(i)];
            peak = std::max(peak, std::abs(value));
            squares += value * value;
            ++samples;
        }
    }
    void merge(const WaterLevel& other) noexcept {
        peak = std::max(peak, other.peak);
        squares += other.squares;
        samples += other.samples;
    }
    double rms() const noexcept {
        return samples ? std::sqrt(squares / samples) : 0;
    }
};
// Fixed-size numeric block payload, carried by the existing bounded Protect SPSC queue.
// Levels are pre-audition; activity counters are cumulative since restart, voice/delay values
// latest.
struct WaterDiagnostics final {
    std::array<WaterLevel, static_cast<std::size_t>(WaterSignal::count)> levels{};
    WaterActivity latest;
    void include(const WaterFrameReadout& frame, int channels) noexcept {
        for (std::size_t i = 0; i < levels.size(); ++i)
            levels[i].include(frame.signals[i], channels);
    }
    void merge(const WaterDiagnostics& block) noexcept {
        for (std::size_t i = 0; i < levels.size(); ++i)
            levels[i].merge(block.levels[i]);
        latest = block.latest;
    }
};
} // namespace frazil::water::preview
