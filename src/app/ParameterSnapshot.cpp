#include "ParameterSnapshot.h"

#include <cmath>

namespace {
float loadOrDefault(const std::atomic<float>* source, float defaultValue) noexcept {
    return source != nullptr ? source->load(std::memory_order_relaxed) : defaultValue;
}

bool loadBoolOrDefault(const std::atomic<float>* source, bool defaultValue) noexcept {
    if (source == nullptr)
        return defaultValue;

    return source->load(std::memory_order_relaxed) >=
           frazil::parameter_contract::kEnabledOnThreshold;
}

int loadRoutingModeOrDefault(const std::atomic<float>* source, int defaultValue) noexcept {
    const auto value = loadOrDefault(source, static_cast<float>(defaultValue));
    if (!std::isfinite(value) || value < 0.0f || value > 2.0f)
        return defaultValue;

    return static_cast<int>(value);
}
} // namespace

ParameterSnapshot ParameterSnapshot::capture(const ParameterSourcePointers& sources) noexcept {
    ParameterSnapshot snapshot;
    snapshot.waterEnabled = loadBoolOrDefault(sources.waterEnabled, snapshot.waterEnabled);
    snapshot.iceEnabled = loadBoolOrDefault(sources.iceEnabled, snapshot.iceEnabled);
    snapshot.routingModeIndex =
        loadRoutingModeOrDefault(sources.routingMode, snapshot.routingModeIndex);
    snapshot.parallelBalance = loadOrDefault(sources.parallelBalance, snapshot.parallelBalance);
    snapshot.waterAmount = loadOrDefault(sources.waterAmount, snapshot.waterAmount);
    snapshot.iceAmount = loadOrDefault(sources.iceAmount, snapshot.iceAmount);
    snapshot.inputGainDb = loadOrDefault(sources.inputGainDb, snapshot.inputGainDb);
    snapshot.globalMix = loadOrDefault(sources.globalMix, snapshot.globalMix);
    snapshot.outputGainDb = loadOrDefault(sources.outputGainDb, snapshot.outputGainDb);
    return snapshot;
}
