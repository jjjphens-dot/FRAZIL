#pragma once

#include <atomic>

namespace frazil::water::preview {
// Single audio producer / message-thread consumer. Latch every block's actual output peak:
// a brief over-range event must survive the gap between UI polls. Never changes audio or history.
class MonitorOverRange final {
  public:
    void observe(float outputPeak) noexcept {
        if (outputPeak > 1.f)
            pending_.store(true, std::memory_order_relaxed);
    }
    bool consume() noexcept {
        return pending_.exchange(false, std::memory_order_relaxed);
    }

  private:
    static_assert(std::atomic<bool>::is_always_lock_free);
    std::atomic<bool> pending_{};
};
} // namespace frazil::water::preview
