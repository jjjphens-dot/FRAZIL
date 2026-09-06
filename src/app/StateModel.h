#pragma once

#include "EngineParameters.h"

#include <cstdint>
#include <optional>

// Application-owned state values. This type deliberately stores Host-facing values (including
// dB gains) rather than mapped DSP values, and has no dependency on JUCE or APVTS.
class StateModel final {
  public:
    static constexpr std::uint32_t kLegacySchemaVersion = 0;
    static constexpr std::uint32_t kCurrentSchemaVersion = 1;

    struct Values final {
        bool waterEnabled{true};
        bool iceEnabled{true};
        RoutingMode routing{RoutingMode::parallel};
        float parallelBalance{0.5f};
        float waterAmount{1.0f};
        float iceAmount{1.0f};
        float inputGainDb{};
        float globalMix{1.0f};
        float outputGainDb{};
    };

    // A parsed, transport-neutral envelope. The optional fields let the plugin adapter represent
    // missing properties before validation; no parsing or migration occurs on the audio thread.
    struct SerializedState final {
        std::optional<std::uint32_t> schemaVersion;
        std::optional<bool> waterEnabled;
        std::optional<bool> iceEnabled;
        std::optional<int> routingMode;
        std::optional<float> parallelBalance;
        std::optional<float> waterAmount;
        std::optional<float> iceAmount;
        std::optional<float> inputGainDb;
        std::optional<float> globalMix;
        std::optional<float> outputGainDb;
    };

    enum class DeserializeStatus : std::uint8_t { current, migrated, fallback };

    struct DeserializeResult final {
        Values values{};
        DeserializeStatus status{DeserializeStatus::fallback};
    };

    static Values defaultValues() noexcept;
    static SerializedState serialize(const Values&) noexcept;
    static DeserializeResult deserialize(const SerializedState&) noexcept;
};
