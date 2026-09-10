#pragma once

#include <atomic>

// Non-owning APVTS atomics cached by the plugin during construction. The owners outlive each
// processBlock call and the pointers are read-only from the application layer.
struct ParameterSourcePointers final {
    const std::atomic<float>* waterEnabled{};
    const std::atomic<float>* iceEnabled{};
    const std::atomic<float>* routingMode{};
    const std::atomic<float>* parallelBalance{};
    const std::atomic<float>* waterAmount{};
    const std::atomic<float>* iceAmount{};
    const std::atomic<float>* inputGainDb{};
    const std::atomic<float>* globalMix{};
    const std::atomic<float>* outputGainDb{};
};

struct ParameterSnapshot final {
    bool waterEnabled{true};
    bool iceEnabled{true};
    int routingModeIndex{};
    float parallelBalance{0.5f};
    float waterAmount{1.0f};
    float iceAmount{1.0f};
    float inputGainDb{};
    float globalMix{1.0f};
    float outputGainDb{};

    // Reads each cached source exactly once. The caller owns the atomics and must keep them
    // alive for the lifetime of the capture boundary.
    static ParameterSnapshot capture(const ParameterSourcePointers&) noexcept;
};
