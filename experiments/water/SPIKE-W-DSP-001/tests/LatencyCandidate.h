#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

// Test-only streaming realization of EXP-W-FD-002's offline candidates. Coefficients
// are prepared by the independent Python study, never designed on the audio path.
// No production or research renderer target includes this resource prototype.
namespace frazil::water::research::latencytest {
class LatencyCandidate {
  public:
    static constexpr std::size_t kTableCells = 4096;
    static constexpr std::size_t kMaximumFirTaps = 129;
    static constexpr std::size_t kMaximumSosSections = 128;
    static constexpr int kMaximumFilterLatency = 64;
    static constexpr double kMaximumPhysicalDelaySamples = .05 / 1484 * 96000;
    using Stereo = std::array<double, 2>;
    struct Configuration {
        int guard{}, filterLatency{};
        std::vector<double> fir;
        std::vector<std::array<double, 6>> sos;
        std::vector<double> table; // 4097 fractional positions, 2*guard+1 taps each.
    };
    bool prepare(Configuration configuration) {
        const int guard = configuration.guard;
        if ((guard != 8 && guard != 16 && guard != 32 && guard != 64) ||
            configuration.filterLatency < 0 ||
            configuration.filterLatency > kMaximumFilterLatency ||
            configuration.table.size() != (kTableCells + 1) * (2u * guard + 1u) ||
            configuration.fir.size() > kMaximumFirTaps ||
            configuration.sos.size() > kMaximumSosSections ||
            configuration.fir.empty() == configuration.sos.empty() ||
            (!configuration.fir.empty() &&
             configuration.filterLatency != static_cast<int>((configuration.fir.size() - 1) / 2)) ||
            (!configuration.sos.empty() && configuration.filterLatency != 0))
            return false;
        for (auto x : configuration.table)
            if (!std::isfinite(x))
                return false;
        for (auto x : configuration.fir)
            if (!std::isfinite(x))
                return false;
        for (const auto& section : configuration.sos) {
            if (section[3] != 1.)
                return false;
            for (auto x : section)
                if (!std::isfinite(x))
                    return false;
        }
        config_ = std::move(configuration);
        sections_.resize(config_.sos.size());
        reset();
        return true;
    }
    void reset() noexcept {
        input_ = {};
        filtered_ = {};
        paths_ = {};
        std::fill(sections_.begin(), sections_.end(), SectionState{});
        cursor_ = 0;
    }
    int latency() const noexcept {
        return config_.guard + config_.filterLatency;
    }
    std::size_t stateBytes() const noexcept {
        return sizeof(*this) + config_.table.capacity() * sizeof(double) +
               config_.fir.capacity() * sizeof(double) +
               config_.sos.capacity() * sizeof(std::array<double, 6>) +
               sections_.capacity() * sizeof(SectionState);
    }
    Stereo process(Stereo sample, double physicalDelay) noexcept {
        // Contract: float-range finite stereo input (double internal arithmetic),
        // physical delay in [0, .05/1484*96000], after successful prepare.
        // The path FIFO preserves physical time n-totalLatency. The filtered audio
        // contains FIR delay already; its history needs only guard+physicalDelay.
        paths_[cursor_] = physicalDelay;
        input_[cursor_] = sample;
        Stereo value{};
        if (!config_.fir.empty()) {
            for (std::size_t tap = 0; tap < config_.fir.size(); ++tap)
                for (int channel = 0; channel < 2; ++channel)
                    value[channel] += config_.fir[tap] * input_[index(tap)][channel];
        } else {
            value = sample;
            for (std::size_t i = 0; i < config_.sos.size(); ++i) {
                const auto& c = config_.sos[i];
                for (int channel = 0; channel < 2; ++channel) {
                    auto& s = sections_[i][channel];
                    const double y = c[0] * value[channel] + s[0];
                    s[0] = c[1] * value[channel] - c[4] * y + s[1];
                    s[1] = c[2] * value[channel] - c[5] * y;
                    value[channel] = y;
                }
            }
        }
        filtered_[cursor_] = value;
        const double delay = paths_[index(static_cast<std::size_t>(latency()))];
        const auto integer = static_cast<std::size_t>(std::floor(delay));
        const double location = (delay - integer) * static_cast<double>(kTableCells);
        const auto row = static_cast<std::size_t>(location);
        const double fraction = location - row;
        const auto taps = static_cast<std::size_t>(2 * config_.guard + 1);
        Stereo output{};
        for (std::size_t j = 0; j < taps; ++j) {
            const double a = config_.table[row * taps + j];
            const double b = config_.table[(row + 1) * taps + j];
            const double weight = a + fraction * (b - a);
            for (int channel = 0; channel < 2; ++channel)
                output[channel] += weight * filtered_[index(integer + j)][channel];
        }
        cursor_ = (cursor_ + 1) % kHistorySize;
        return output;
    }

  private:
    static constexpr std::size_t kHistorySize = 136; // 129 taps + ceil(max tau*96k).
    using SectionState = std::array<std::array<double, 2>, 2>;
    std::size_t index(std::size_t offset) const noexcept {
        return (cursor_ + kHistorySize - offset) % kHistorySize;
    }
    Configuration config_;
    std::vector<SectionState> sections_;
    std::array<Stereo, kHistorySize> input_{}, filtered_{};
    std::array<double, kHistorySize> paths_{};
    std::size_t cursor_{};
};
} // namespace frazil::water::research::latencytest
