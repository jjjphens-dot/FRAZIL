#pragma once

#include "WaterDiagnostics.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace frazil::water::preview {

struct ProtectReadout final {
    double fast{}, slow{}, difference{}, logRatioDb{}, reductionDb{};
    void includePeak(const ProtectReadout& value) noexcept {
        fast = std::max(fast, value.fast);
        slow = std::max(slow, value.slow);
        difference = std::max(difference, value.difference);
        logRatioDb = std::max(logRatioDb, value.logRatioDb);
        reductionDb = std::max(reductionDb, value.reductionDb);
    }
};
struct ProtectBlockReadout final {
    ProtectReadout latest, peak;
    WaterDiagnostics water;
};
struct ProtectDiagnosticsSnapshot final {
    ProtectReadout latest, peak;
    std::size_t blocks{};
    std::uint64_t droppedBlocks{};
    WaterDiagnostics water;
};

// Fixed SPSC block-summary queue. Audio publishes one value per callback; UI drains at most 256
// entries per poll and formats there. Non-atomic slots are safe because release/acquire head/tail
// transfer exclusive slot ownership. Full queue drops new summaries, never waits or overwrites.
// Peaks cover consumed blocks (not co-timed samples); drop count makes observation loss explicit.
class ProtectDiagnostics final {
  public:
    static constexpr std::size_t kCapacity = 256;
    bool publish(const ProtectBlockReadout& value) noexcept {
        const auto head = head_.load(std::memory_order_relaxed);
        if (head - tail_.load(std::memory_order_acquire) >= kCapacity) {
            dropped_.fetch_add(1, std::memory_order_relaxed);
            return false;
        }
        slots_[head % kCapacity] = value;
        head_.store(head + 1, std::memory_order_release);
        return true;
    }
    // Sole message-thread reader. A copied snapshot has no shared ownership with the queue.
    ProtectDiagnosticsSnapshot snapshot() noexcept {
        ProtectDiagnosticsSnapshot result;
        result.latest = last_;
        result.water.latest = lastWater_;
        auto tail = tail_.load(std::memory_order_relaxed);
        const auto end = head_.load(std::memory_order_acquire);
        while (tail != end && result.blocks < kCapacity) {
            const auto& slot = slots_[tail % kCapacity];
            result.latest = slot.latest;
            result.peak.includePeak(slot.peak);
            result.water.merge(slot.water);
            ++tail;
            ++result.blocks;
        }
        tail_.store(tail, std::memory_order_release);
        last_ = result.latest;
        lastWater_ = result.water.latest;
        result.droppedBlocks = dropped_.load(std::memory_order_relaxed);
        return result;
    }
    // Message-thread lifecycle only, after callback detach; no concurrent producer/consumer.
    void clear() noexcept {
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
        dropped_.store(0, std::memory_order_relaxed);
        last_ = {};
        lastWater_ = {};
    }

  private:
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
    std::array<ProtectBlockReadout, kCapacity> slots_{}; // Constructed off the audio thread.
    std::atomic<std::uint64_t> head_{}, tail_{}, dropped_{};
    ProtectReadout last_; // Reader-local last observation, cleared only while producer detached.
    WaterActivity lastWater_;
};
} // namespace frazil::water::preview
