#pragma once
#include "DropletB1EntrainmentModel.h"

#include <array>
#include <optional>

namespace frazil::water::research {
// Fixed ENGINEERING causal queue. No audio access: the source state is already captured.
class DropletB1PendingQueue final {
  public:
    static constexpr std::size_t kCapacity = 16;
    void reset() noexcept {
        events_ = {};
        size_ = 0;
    }
    bool push(const DropletB1Event& event) noexcept {
        if (size_ == kCapacity)
            return false;
        events_[size_++] = event;
        return true;
    }
    std::optional<DropletB1Event> popDue(std::uint64_t sample) noexcept {
        if (!size_)
            return std::nullopt;
        std::size_t first = 0;
        for (std::size_t i = 1; i < size_; ++i)
            if (events_[i].dueSample < events_[first].dueSample ||
                (events_[i].dueSample == events_[first].dueSample &&
                 events_[i].eligibleId < events_[first].eligibleId))
                first = i;
        if (events_[first].dueSample > sample)
            return std::nullopt;
        const auto result = events_[first];
        events_[first] = events_[--size_];
        return result;
    }
    std::size_t size() const noexcept {
        return size_;
    }

  private:
    std::array<DropletB1Event, kCapacity> events_{}; // Audio-owner immutable captured events.
    std::size_t size_{};
};
} // namespace frazil::water::research
