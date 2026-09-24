#pragma once
#include "DropletB1BubbleVoice.h"
#include "WaterExcitationFeatures.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace frazil::water::research {
struct DropletB1VoiceCounters final {
    std::uint64_t started{}, droppedByVoiceCapacity{}, steals{}, completed{};
};
// ENGINEERING storage/lifecycle only. No source analysis, physical selection or RNG.
class DropletB1VoicePool final {
  public:
    static constexpr std::size_t kCapacity = 256;
    void prepare(double rate, std::size_t capacity) noexcept {
        rate_ = rate;
        capacity_ = capacity;
        reset();
    }
    void reset() noexcept {
        slots_ = {};
        counters_ = {};
        lastStarted_ = {};
        active_ = 0;
    }
    bool request(const DropletB1Event& e) noexcept {
        for (std::size_t i = 0; i < capacity_; ++i)
            if (!slots_[i].voice.active() && !slots_[i].remaining) {
                start(slots_[i], e);
                return true;
            }
        std::size_t victim = capacity_;
        double least = std::numeric_limits<double>::infinity();
        for (std::size_t i = 0; i < capacity_; ++i)
            if (!slots_[i].remaining && slots_[i].voice.priority() < least) {
                victim = i;
                least = slots_[i].voice.priority();
            }
        if (victim == capacity_) {
            ++counters_.droppedByVoiceCapacity;
            return false;
        }
        auto& slot = slots_[victim];
        slot.replacement = e;
        slot.remaining = slot.releaseSamples =
            static_cast<std::uint32_t>(std::ceil(rate_ * e.releaseMs * .001));
        ++counters_.steals;
        return true;
    }
    StereoFrame process() noexcept {
        std::array<double, 2> sum{};
        active_ = 0;
        for (std::size_t i = 0; i < capacity_; ++i) {
            auto& slot = slots_[i];
            const bool wasActive = slot.voice.active();
            auto value = slot.voice.process();
            if (slot.remaining) {
                const double fade = double(slot.remaining - 1) / slot.releaseSamples;
                for (auto& channel : value)
                    channel *= fade;
                if (--slot.remaining == 0) {
                    // The outgoing contribution is exactly zero on this sample; the captured
                    // replacement begins on the following sample (bounded extra onset delay).
                    if (wasActive)
                        ++counters_.completed;
                    start(slot, slot.replacement);
                } else if (wasActive && !slot.voice.active())
                    ++counters_.completed;
            } else if (wasActive && !slot.voice.active())
                ++counters_.completed;
            for (std::size_t ch = 0; ch < 2; ++ch)
                sum[ch] += value[ch];
            active_ += slot.voice.active();
        }
        return {static_cast<float>(sum[0]), static_cast<float>(sum[1])};
    }
    const DropletB1VoiceCounters& counters() const noexcept {
        return counters_;
    }
    const DropletB1Event& lastStarted() const noexcept {
        return lastStarted_;
    }
    std::size_t active() const noexcept {
        return active_;
    }

  private:
    struct Slot final {
        DropletB1BubbleVoice voice;
        DropletB1Event replacement; // Already captured, never read from future source audio.
        std::uint32_t remaining{}, releaseSamples{}; // Linear steal release, samples.
    };
    void start(Slot& slot, const DropletB1Event& event) noexcept {
        slot.voice.start(event, rate_);
        lastStarted_ = event;
        ++counters_.started;
    }
    std::array<Slot, kCapacity> slots_{};
    DropletB1VoiceCounters counters_{};
    DropletB1Event lastStarted_{}; // Bounded audio-owner readout; no trace allocation.
    double rate_{};
    std::size_t capacity_{32}, active_{};
};
} // namespace frazil::water::research
