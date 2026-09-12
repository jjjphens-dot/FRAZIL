#pragma once

#include <array>
#include <atomic>
#include <cstdint>

#ifndef FRAZIL_ENABLE_DEVELOPER_UI
#define FRAZIL_ENABLE_DEVELOPER_UI 0
#endif

namespace frazil::plugin {

struct DeveloperDiagnosticsSnapshot final {
    float sampleRateHz{};
    int preparedBlockSize{};
    int latestBlockSize{};
    int channelCount{};
    float inputPeak{};
    float outputPeak{};
    float inputRms{};
    float outputRms{};
    bool finite{true};
};

// A bounded, lock-free transport from processBlock() to the developer editor. The producer is
// single-writer (the processor lifecycle/audio path) and the consumer is single-reader (the
// editor/message thread). It intentionally carries only the latest block summary; it is not a
// logging queue or a persistent state store.
class DeveloperDiagnostics final {
  public:
    void setPrepared(float sampleRateHz, int blockSize, int channelCount) noexcept {
        writerState_.sampleRateHz = sampleRateHz;
        writerState_.preparedBlockSize = blockSize;
        writerState_.latestBlockSize = 0;
        writerState_.channelCount = channelCount;
        publishWriterState();
    }

    void reset() noexcept {
        writerState_ = {};
        writerState_.finite = true;
        publishWriterState();
    }

    void publish(int latestBlockSize, int channelCount, float inputPeak, float outputPeak,
                 float inputRms, float outputRms, bool finite) noexcept {
        writerState_.latestBlockSize = latestBlockSize;
        writerState_.channelCount = channelCount;
        writerState_.inputPeak = inputPeak;
        writerState_.outputPeak = outputPeak;
        writerState_.inputRms = inputRms;
        writerState_.outputRms = outputRms;
        writerState_.finite = finite;
        publishWriterState();
    }

    DeveloperDiagnosticsSnapshot snapshot() const noexcept {
        DeveloperDiagnosticsSnapshot current;
        if (readPublishedSnapshot(current))
            lastSnapshot_ = current;
        return lastSnapshot_;
    }

  private:
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free,
                  "Developer diagnostics publication token must be lock-free");
    static_assert(std::atomic<std::uint32_t>::is_always_lock_free,
                  "Developer diagnostics sequence must be lock-free");
    static_assert(std::atomic<float>::is_always_lock_free,
                  "Developer diagnostics float fields must be lock-free");
    static_assert(std::atomic<int>::is_always_lock_free,
                  "Developer diagnostics integer fields must be lock-free");
    static_assert(std::atomic<bool>::is_always_lock_free,
                  "Developer diagnostics finite flag must be lock-free");

    struct AtomicSnapshot final {
        std::atomic<std::uint32_t> sequence{};
        std::atomic<float> sampleRateHz{};
        std::atomic<int> preparedBlockSize{};
        std::atomic<int> latestBlockSize{};
        std::atomic<int> channelCount{};
        std::atomic<float> inputPeak{};
        std::atomic<float> outputPeak{};
        std::atomic<float> inputRms{};
        std::atomic<float> outputRms{};
        std::atomic<bool> finite{true};
    };

    void publishWriterState() noexcept {
        const auto activePublication = activePublication_.load(std::memory_order_seq_cst);
        const auto target = static_cast<int>((activePublication + 1u) & 1u);
        auto& slot = slots_[target];
        // A single total order for the marker and active index prevents accepting an ABA reuse.
        slot.sequence.fetch_add(1, std::memory_order_seq_cst);
        slot.sampleRateHz.store(writerState_.sampleRateHz, std::memory_order_seq_cst);
        slot.preparedBlockSize.store(writerState_.preparedBlockSize, std::memory_order_seq_cst);
        slot.latestBlockSize.store(writerState_.latestBlockSize, std::memory_order_seq_cst);
        slot.channelCount.store(writerState_.channelCount, std::memory_order_seq_cst);
        slot.inputPeak.store(writerState_.inputPeak, std::memory_order_seq_cst);
        slot.outputPeak.store(writerState_.outputPeak, std::memory_order_seq_cst);
        slot.inputRms.store(writerState_.inputRms, std::memory_order_seq_cst);
        slot.outputRms.store(writerState_.outputRms, std::memory_order_seq_cst);
        slot.finite.store(writerState_.finite, std::memory_order_seq_cst);
        slot.sequence.fetch_add(1, std::memory_order_seq_cst);
        activePublication_.store(activePublication + 1u, std::memory_order_seq_cst);
    }

    bool readPublishedSnapshot(DeveloperDiagnosticsSnapshot& destination) const noexcept {
        for (int attempt = 0; attempt < 2; ++attempt) {
            const auto activePublication = activePublication_.load(std::memory_order_seq_cst);
            const auto active = static_cast<int>(activePublication & 1u);

            const auto sequenceBefore = slots_[active].sequence.load(std::memory_order_seq_cst);
            if ((sequenceBefore & 1u) != 0u)
                continue;

            destination.sampleRateHz = slots_[active].sampleRateHz.load(std::memory_order_seq_cst);
            destination.preparedBlockSize =
                slots_[active].preparedBlockSize.load(std::memory_order_seq_cst);
            destination.latestBlockSize =
                slots_[active].latestBlockSize.load(std::memory_order_seq_cst);
            destination.channelCount = slots_[active].channelCount.load(std::memory_order_seq_cst);
            destination.inputPeak = slots_[active].inputPeak.load(std::memory_order_seq_cst);
            destination.outputPeak = slots_[active].outputPeak.load(std::memory_order_seq_cst);
            destination.inputRms = slots_[active].inputRms.load(std::memory_order_seq_cst);
            destination.outputRms = slots_[active].outputRms.load(std::memory_order_seq_cst);
            destination.finite = slots_[active].finite.load(std::memory_order_seq_cst);

            const auto sequenceAfter = slots_[active].sequence.load(std::memory_order_seq_cst);
            if (sequenceBefore == sequenceAfter && (sequenceAfter & 1u) == 0u &&
                activePublication_.load(std::memory_order_seq_cst) == activePublication)
                return true;
        }

        return false;
    }

    std::array<AtomicSnapshot, 2> slots_{};
    std::atomic<std::uint64_t> activePublication_{};
    DeveloperDiagnosticsSnapshot writerState_{};
    mutable DeveloperDiagnosticsSnapshot lastSnapshot_{};
};

} // namespace frazil::plugin
