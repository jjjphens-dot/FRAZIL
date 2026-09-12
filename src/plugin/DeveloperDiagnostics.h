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

// A bounded, lock-free transport from processBlock() to the developer editor. It intentionally
// carries only the latest block summary; it is not a logging queue or a persistent state store.
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
        const auto active = activeSlot_.load(std::memory_order_relaxed);
        const auto target = active == 0 ? 1 : 0;
        auto& slot = slots_[target];
        slot.sequence.fetch_add(1, std::memory_order_release);
        slot.sampleRateHz.store(writerState_.sampleRateHz, std::memory_order_relaxed);
        slot.preparedBlockSize.store(writerState_.preparedBlockSize, std::memory_order_relaxed);
        slot.latestBlockSize.store(writerState_.latestBlockSize, std::memory_order_relaxed);
        slot.channelCount.store(writerState_.channelCount, std::memory_order_relaxed);
        slot.inputPeak.store(writerState_.inputPeak, std::memory_order_relaxed);
        slot.outputPeak.store(writerState_.outputPeak, std::memory_order_relaxed);
        slot.inputRms.store(writerState_.inputRms, std::memory_order_relaxed);
        slot.outputRms.store(writerState_.outputRms, std::memory_order_relaxed);
        slot.finite.store(writerState_.finite, std::memory_order_relaxed);
        slot.sequence.fetch_add(1, std::memory_order_release);
        activeSlot_.store(target, std::memory_order_release);
    }

    bool readPublishedSnapshot(DeveloperDiagnosticsSnapshot& destination) const noexcept {
        for (int attempt = 0; attempt < 2; ++attempt) {
            const auto active = activeSlot_.load(std::memory_order_acquire);
            if (active < 0 || active > 1)
                continue;

            const auto sequenceBefore = slots_[active].sequence.load(std::memory_order_acquire);
            if ((sequenceBefore & 1u) != 0u)
                continue;

            destination.sampleRateHz = slots_[active].sampleRateHz.load(std::memory_order_relaxed);
            destination.preparedBlockSize =
                slots_[active].preparedBlockSize.load(std::memory_order_relaxed);
            destination.latestBlockSize =
                slots_[active].latestBlockSize.load(std::memory_order_relaxed);
            destination.channelCount = slots_[active].channelCount.load(std::memory_order_relaxed);
            destination.inputPeak = slots_[active].inputPeak.load(std::memory_order_relaxed);
            destination.outputPeak = slots_[active].outputPeak.load(std::memory_order_relaxed);
            destination.inputRms = slots_[active].inputRms.load(std::memory_order_relaxed);
            destination.outputRms = slots_[active].outputRms.load(std::memory_order_relaxed);
            destination.finite = slots_[active].finite.load(std::memory_order_relaxed);

            const auto sequenceAfter = slots_[active].sequence.load(std::memory_order_acquire);
            if (sequenceBefore == sequenceAfter && (sequenceAfter & 1u) == 0u &&
                activeSlot_.load(std::memory_order_acquire) == active)
                return true;
        }

        return false;
    }

    std::array<AtomicSnapshot, 2> slots_{};
    std::atomic<int> activeSlot_{};
    DeveloperDiagnosticsSnapshot writerState_{};
    mutable DeveloperDiagnosticsSnapshot lastSnapshot_{};
};

} // namespace frazil::plugin
