#pragma once

#include "DampedResonator.h"
#include "dsp/WaterExcitationFeatures.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace frazil::water::research::detail {

// Fixed 16-voice stereo bank shared only by the two research event mechanisms.
// No coefficient generation in process. Inactive-first, otherwise oldest-age stealing; ties
// select the lowest slot. Replacing an audible voice can click and remains a listening risk.
class EventVoicePool final {
  public:
    static constexpr std::size_t kCapacity = 16;
    static constexpr std::size_t kFamilies = 16;

    // Callers validate acoustic ranges; the pool owns its storage-capacity invariant.
    // Failure clears voices and leaves trigger/process safe and inactive until a valid prepare.
    bool prepare(double rate, double minimumHz, double maximumHz, double decaySeconds,
                 std::size_t voices) noexcept {
        capacity_ = 0;
        reset();
        if (voices == 0 || voices > kCapacity)
            return false;
        capacity_ = voices;
        // Hard expiry at 24 time constants bounds tail and removes inaudible subnormal work.
        lifetime_ = static_cast<std::uint32_t>(std::ceil(24.0 * decaySeconds * rate));
        for (std::size_t i = 0; i < kFamilies; ++i) {
            const double fraction = static_cast<double>(i) / (kFamilies - 1);
            const double frequency = minimumHz * std::pow(maximumHz / minimumHz, fraction);
            coefficients_[i] = makeResonator(rate, frequency, decaySeconds);
            coefficients_[i].excitation = 1.0; // One bounded impulse, never continuous input.
        }
        return true;
    }

    void reset() noexcept {
        voices_ = {};
        events_ = 0;
        steals_ = 0;
    }

    StereoFrame trigger(const StereoFrame& input, std::size_t family) noexcept {
        if (capacity_ == 0)
            return {};
        std::size_t selected{};
        for (std::size_t i = 0; i < capacity_; ++i) {
            if (!voices_[i].active) {
                selected = i;
                break;
            }
            if (voices_[i].age > voices_[selected].age)
                selected = i;
        }
        auto& voice = voices_[selected];
        steals_ += voice.active ? 1u : 0u;
        voice = {};
        voice.active = true;
        voice.family = family % kFamilies;
        StereoFrame driver{};
        for (std::size_t c = 0; c < 2; ++c) {
            driver[c] = std::clamp(input[c], -1.f, 1.f);
            (void)voice.state[c].process(static_cast<double>(driver[c]),
                                         coefficients_[voice.family]);
        }
        ++events_;
        return driver; // Actual bounded impulse submitted to the resonators, for diagnostics only.
    }

    StereoFrame process(double residualGain) noexcept {
        if (capacity_ == 0)
            return {};
        std::array<double, 2> sum{};
        for (std::size_t i = 0; i < capacity_; ++i) {
            auto& voice = voices_[i];
            if (!voice.active)
                continue;
            for (std::size_t c = 0; c < 2; ++c)
                sum[c] += voice.state[c].process(0.0, coefficients_[voice.family]);
            if (++voice.age >= lifetime_)
                voice = {};
        }
        const double gain = residualGain / static_cast<double>(capacity_);
        return {static_cast<float>(sum[0] * gain), static_cast<float>(sum[1] * gain)};
    }

    std::uint64_t events() const noexcept {
        return events_;
    }
    std::uint64_t steals() const noexcept {
        return steals_;
    }
    std::size_t activeVoices() const noexcept {
        std::size_t count{};
        for (std::size_t i = 0; i < capacity_; ++i)
            count += voices_[i].active ? 1u : 0u;
        return count;
    }

  private:
    struct Voice final {
        std::array<DampedResonator, 2> state{};
        std::uint32_t age{}; // Samples since trigger, bounded by lifetime_, reset on steal.
        std::size_t family{};
        bool active{};
    };
    std::array<Voice, kCapacity> voices_{};
    std::array<ResonatorCoefficients, kFamilies> coefficients_{};
    std::size_t capacity_{}; // Zero means unprepared/failed; otherwise 1..kCapacity.
    std::uint32_t lifetime_{};
    std::uint64_t events_{}; // Diagnostic counters only; reset/reprepare starts at zero.
    std::uint64_t steals_{};
};
} // namespace frazil::water::research::detail
