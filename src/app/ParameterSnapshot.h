#pragma once

#include <atomic>

// Non-owning APVTS atomics cached by the plugin during construction. The owners outlive each
// processBlock call and the pointers are read-only from the application layer.
struct ParameterSourcePointers final {
    std::atomic<float>* waterEnabled{};
    std::atomic<float>* iceEnabled{};
    std::atomic<float>* routingMode{};
    std::atomic<float>* parallelBalance{};
    std::atomic<float>* waterAmount{};
    std::atomic<float>* iceAmount{};
    std::atomic<float>* inputGainDb{};
    std::atomic<float>* globalMix{};
    std::atomic<float>* outputGainDb{};
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
