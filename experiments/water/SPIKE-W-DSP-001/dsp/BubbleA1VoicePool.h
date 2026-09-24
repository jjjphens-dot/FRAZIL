#pragma once
#include "BubbleA1Model.h"
#include "WaterExcitationFeatures.h"

#include <cstdint>
#include <limits>

namespace frazil::water::research {
struct BubbleA1Event final {
    BubbleA1Bin physics;
    std::array<double, 2> amplitude{};
    double depthExcitationProxy{}, riseXi{};
    std::size_t bin{};
    BubbleA1RiseModel riseModel{BubbleA1RiseModel::effectiveDampingP1};
};

// Shared acoustic trajectory, separate signed channel amplitudes. Linear complex recurrences
// integrate the frequency ramp; transcendental setup occurs at event start, never per voice/sample.
class BubbleA1Voice final {
  public:
    void start(const BubbleA1Event& e, double rate, double floorDb) noexcept {
        event = e;
        age = 0;
        releaseLeft = 0;
        pending = false;
        real_ = 1;
        imag_ = 0;
        envelope_ = 1;
        rate_ = rate;
        step_ = 2 * std::numbers::pi * e.physics.frequencyHz / rate;
        // P1 uses the rendered decay rate so xi describes rise over an audible lifetime.
        // P0 retains the historical physical-damping slope as an explicit offline ablation.
        const double riseDamping = e.riseModel == BubbleA1RiseModel::effectiveDampingP1
                                       ? 1 / e.physics.tauSeconds
                                       : e.physics.dampingPerSecond;
        delta_ = step_ * e.riseXi * riseDamping / rate;
        cap_ = 2 * std::numbers::pi * std::min(std::sqrt(2.) * e.physics.frequencyHz, .45 * rate) /
               rate;
        // Midpoint integration of f(t), not sin(2*pi*f(t)*t).
        angle_ = std::min(step_ + .5 * delta_, cap_);
        rotReal_ = std::cos(angle_);
        rotImag_ = std::sin(angle_);
        deltaReal_ = std::cos(delta_);
        deltaImag_ = std::sin(delta_);
        floor_ = std::pow(10., floorDb / 20);
        maxAge_ = static_cast<std::uint32_t>(30 * rate);
    }
    std::array<double, 2> process() noexcept {
        const double fade = releaseLeft ? double(releaseLeft) / releaseTotal_ : 1;
        const double signal = imag_ * fade;
        const std::array result{signal * event.amplitude[0], signal * event.amplitude[1]};
        const double nextReal = real_ * rotReal_ - imag_ * rotImag_;
        imag_ = (real_ * rotImag_ + imag_ * rotReal_) * event.physics.poleRadius;
        real_ = nextReal * event.physics.poleRadius;
        envelope_ *= event.physics.poleRadius;
        if (delta_ > 0 && angle_ < cap_) {
            if (angle_ + delta_ >= cap_) {
                angle_ = cap_;
                rotReal_ = std::cos(cap_);
                rotImag_ = std::sin(cap_);
            } else {
                angle_ += delta_;
                const double re = rotReal_ * deltaReal_ - rotImag_ * deltaImag_;
                rotImag_ = rotReal_ * deltaImag_ + rotImag_ * deltaReal_;
                rotReal_ = re;
            }
        }
        ++age;
        if (releaseLeft)
            --releaseLeft;
        return result;
    }
    void release(std::uint32_t samples) noexcept {
        releaseLeft = releaseTotal_ = samples;
    }
    double audibleEnvelope() const noexcept {
        return envelope_ * std::max(std::abs(event.amplitude[0]), std::abs(event.amplitude[1]));
    }
    bool done() const noexcept {
        return audibleEnvelope() <= floor_ || age >= maxAge_ || (pending && releaseLeft == 0);
    }
    double instantaneousFrequency() const noexcept {
        return angle_ * rate_ / (2 * std::numbers::pi);
    }
    BubbleA1Event event{}, replacement{};
    std::uint32_t age{}, releaseLeft{};
    bool pending{};

  private:
    double real_{}, imag_{}, envelope_{}, rotReal_{}, rotImag_{}, deltaReal_{}, deltaImag_{};
    double step_{}, delta_{}, cap_{}, angle_{}, floor_{}, rate_{};
    std::uint32_t maxAge_{}, releaseTotal_{1};
};

struct BubbleA1PoolCounters final {
    std::uint64_t accepted{}, started{}, capacityDrops{}, steals{}, completed{};
    std::array<std::uint64_t, 128> radiusHistogram{};
    // Log-spaced completed lifetime histogram, 1 sample..30 seconds. Numeric fixed storage.
    std::array<std::uint64_t, 128> lifetimeHistogram{};
    std::uint64_t rising{};
};

class BubbleA1VoicePool final {
  public:
    static constexpr std::size_t kStorage = 1024;
    bool prepare(double rate, const BubbleA1Config& c) noexcept {
        ready_ = false;
        reset();
        if (!a1Range(rate, 44100, 96000) || !a1Capacity(c.voiceCapacity) ||
            !a1Range(c.tailFloorDb, -100, -60) || !a1Range(c.stealReleaseMs, .5, 4))
            return false;
        rate_ = rate;
        floorDb_ = c.tailFloorDb;
        capacity_ = c.voiceCapacity;
        releaseSamples_ = static_cast<std::uint32_t>(std::ceil(c.stealReleaseMs * .001 * rate));
        ready_ = true;
        return true;
    }
    void reset() noexcept {
        voices_ = {};
        active_ = {};
        activeCount_ = 0;
        freeCount_ = kStorage;
        counters_ = {};
        for (std::size_t i = 0; i < kStorage; ++i)
            free_[i] = kStorage - i - 1;
    }
    bool setCapacity(std::size_t capacity) noexcept {
        if (!a1Capacity(capacity))
            return false;
        capacity_ = capacity;
        return true; // Existing/releasing voices finish; no destructive downshift.
    }
    // Processing-owner API: event comes from a prepared model, bin < 128, finite amplitudes.
    bool trigger(const BubbleA1Event& e) noexcept {
        if (!ready_)
            return false;
        if (activeCount_ < capacity_) {
            const auto slot = free_[--freeCount_];
            active_[activeCount_++] = slot;
            start(slot, e);
            ++counters_.accepted;
            return true;
        }
        if (activeCount_ > capacity_) {
            ++counters_.capacityDrops;
            return false;
        }
        std::size_t chosen = kStorage;
        double least = std::numeric_limits<double>::infinity();
        for (std::size_t i = 0; i < activeCount_; ++i) {
            const auto slot = active_[i];
            const auto& v = voices_[slot];
            const double level = v.audibleEnvelope();
            if (!v.pending && (level < least || (level == least && slot < chosen))) {
                least = level;
                chosen = slot;
            }
        }
        if (chosen == kStorage) {
            ++counters_.capacityDrops;
            return false;
        }
        auto& v = voices_[chosen];
        v.replacement = e;
        v.pending = true;
        v.release(releaseSamples_);
        ++counters_.accepted;
        ++counters_.steals;
        return true;
    }
    StereoFrame process() noexcept {
        std::array<double, 2> sum{};
        for (std::size_t i = 0; i < activeCount_;) {
            const auto slot = active_[i];
            auto& v = voices_[slot];
            const auto y = v.process();
            sum[0] += y[0];
            sum[1] += y[1];
            if (!v.done()) {
                ++i;
                continue;
            }
            ++counters_.completed;
            const auto bin = static_cast<std::size_t>(127 * std::log(double(std::max(1u, v.age))) /
                                                      std::log(30 * rate_));
            ++counters_.lifetimeHistogram[std::min<std::size_t>(127, bin)];
            if (v.pending && activeCount_ <= capacity_) {
                const auto e = v.replacement;
                start(slot, e);
                ++i;
            } else {
                if (v.pending)
                    ++counters_.capacityDrops;
                free_[freeCount_++] = slot;
                active_[i] = active_[--activeCount_];
            }
        }
        return {static_cast<float>(sum[0]), static_cast<float>(sum[1])};
    }
    std::size_t active() const noexcept {
        return activeCount_;
    }
    std::size_t capacity() const noexcept {
        return capacity_;
    }
    const BubbleA1PoolCounters& counters() const noexcept {
        return counters_;
    }

  private:
    void start(std::size_t slot, const BubbleA1Event& e) noexcept {
        voices_[slot].start(e, rate_, floorDb_);
        ++counters_.started;
        ++counters_.radiusHistogram[e.bin];
        counters_.rising += e.riseXi > 0 ? 1u : 0u;
    }
    std::array<BubbleA1Voice, kStorage> voices_{};
    std::array<std::size_t, kStorage> active_{}, free_{};
    std::size_t activeCount_{}, freeCount_{kStorage}, capacity_{256};
    double rate_{48000}, floorDb_{-80};
    std::uint32_t releaseSamples_{72};
    BubbleA1PoolCounters counters_{};
    bool ready_{};
};
} // namespace frazil::water::research
