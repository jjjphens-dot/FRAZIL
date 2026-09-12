#pragma once

#include <atomic>

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
        sampleRateHz_.store(sampleRateHz, std::memory_order_relaxed);
        preparedBlockSize_.store(blockSize, std::memory_order_relaxed);
        latestBlockSize_.store(0, std::memory_order_relaxed);
        channelCount_.store(channelCount, std::memory_order_relaxed);
    }

    void reset() noexcept {
        sampleRateHz_.store(0.0f, std::memory_order_relaxed);
        preparedBlockSize_.store(0, std::memory_order_relaxed);
        latestBlockSize_.store(0, std::memory_order_relaxed);
        channelCount_.store(0, std::memory_order_relaxed);
        inputPeak_.store(0.0f, std::memory_order_relaxed);
        outputPeak_.store(0.0f, std::memory_order_relaxed);
        inputRms_.store(0.0f, std::memory_order_relaxed);
        outputRms_.store(0.0f, std::memory_order_relaxed);
        finite_.store(true, std::memory_order_relaxed);
    }

    void publish(int latestBlockSize, int channelCount, float inputPeak, float outputPeak,
                 float inputRms, float outputRms, bool finite) noexcept {
        latestBlockSize_.store(latestBlockSize, std::memory_order_relaxed);
        channelCount_.store(channelCount, std::memory_order_relaxed);
        inputPeak_.store(inputPeak, std::memory_order_relaxed);
        outputPeak_.store(outputPeak, std::memory_order_relaxed);
        inputRms_.store(inputRms, std::memory_order_relaxed);
        outputRms_.store(outputRms, std::memory_order_relaxed);
        finite_.store(finite, std::memory_order_relaxed);
    }

    DeveloperDiagnosticsSnapshot snapshot() const noexcept {
        return {sampleRateHz_.load(std::memory_order_relaxed),
                preparedBlockSize_.load(std::memory_order_relaxed),
                latestBlockSize_.load(std::memory_order_relaxed),
                channelCount_.load(std::memory_order_relaxed),
                inputPeak_.load(std::memory_order_relaxed),
                outputPeak_.load(std::memory_order_relaxed),
                inputRms_.load(std::memory_order_relaxed),
                outputRms_.load(std::memory_order_relaxed),
                finite_.load(std::memory_order_relaxed)};
    }

  private:
    std::atomic<float> sampleRateHz_{};
    std::atomic<int> preparedBlockSize_{};
    std::atomic<int> latestBlockSize_{};
    std::atomic<int> channelCount_{};
    std::atomic<float> inputPeak_{};
    std::atomic<float> outputPeak_{};
    std::atomic<float> inputRms_{};
    std::atomic<float> outputRms_{};
    std::atomic<bool> finite_{true};
};

} // namespace frazil::plugin
