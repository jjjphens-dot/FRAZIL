#pragma once
#include "dsp/BubbleA1VoicePool.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <type_traits>

namespace frazil::water::preview {
struct PreviewEventRecord final {
    research::BubbleA1Observation a1{};
    std::uint64_t frame{}, eligibleId{};
    std::uint64_t startCount{}; // Last-start payload; multiplicity is explicit if starts coalesce.
    int module{}, bin{};        // 1=A1, 2=B2; flags describe this record, not cumulative counters.
    bool requested{}, eligible{}, admitted{}, started{};
    double noveltyDb{}, positiveSlope{}, sourceExcitation{}, mappedExcitation{};
    double radiusMm{}, centerFrequencyHz{}, depthExcitationProxy{};
    double detuneLeftCents{}, detuneRightCents{}, renderFrequencyL{}, renderFrequencyR{};
    double renderAmplitudeL{}, renderAmplitudeR{}, pathMeters{};
};
static_assert(std::is_trivially_copyable_v<PreviewEventRecord>);
// Single audio producer / message consumer. Full queue drops NEW records and counts them;
// loss is explicit. No allocation, locks or file access. Reset only with callback detached.
class PreviewEventTrace final {
  public:
    // The same bounded transport is used by the native offline renderer. No serialization here.
    static void captureA1(void* context, const research::BubbleA1Observation& value) noexcept {
        PreviewEventRecord record;
        record.module = 1;
        record.a1 = value;
        static_cast<PreviewEventTrace*>(context)->push(record);
    }
    bool push(const PreviewEventRecord& record) noexcept {
        const auto write = write_.load(std::memory_order_relaxed);
        if (write - read_.load(std::memory_order_acquire) == kCapacity) {
            dropped_.fetch_add(1, std::memory_order_relaxed);
            return false;
        }
        records_[write % kCapacity] = record;
        write_.store(write + 1, std::memory_order_release);
        return true;
    }
    bool pop(PreviewEventRecord& record) noexcept {
        const auto read = read_.load(std::memory_order_relaxed);
        if (read == write_.load(std::memory_order_acquire))
            return false;
        record = records_[read % kCapacity];
        read_.store(read + 1, std::memory_order_release);
        return true;
    }
    std::uint64_t takeDropped() noexcept {
        return dropped_.exchange(0);
    }
    static constexpr std::size_t kCapacity = 512;

  private:
    std::array<PreviewEventRecord, kCapacity> records_{};
    std::atomic<std::uint64_t> read_{}, write_{}, dropped_{};
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
};
} // namespace frazil::water::preview
