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
        bool waterEnabled{frazil::parameter_contract::kDefaultWaterEnabled};
        bool iceEnabled{frazil::parameter_contract::kDefaultIceEnabled};
        RoutingMode routing{
            static_cast<RoutingMode>(frazil::parameter_contract::kDefaultRoutingModeIndex)};
        float parallelBalance{frazil::parameter_contract::kDefaultParallelBalance};
        float waterAmount{frazil::parameter_contract::kDefaultWaterAmount};
        float iceAmount{frazil::parameter_contract::kDefaultIceAmount};
        float inputGainDb{frazil::parameter_contract::kDefaultInputGainDb};
        float globalMix{frazil::parameter_contract::kDefaultGlobalMix};
        float outputGainDb{frazil::parameter_contract::kDefaultOutputGainDb};
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
