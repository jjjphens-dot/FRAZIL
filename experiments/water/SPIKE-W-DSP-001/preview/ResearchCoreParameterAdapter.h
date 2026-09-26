#pragma once

#include "ResearchCoreTuningState.h"
#include "dsp/BubbleA1Model.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <span>
#include <string>

namespace frazil::water::preview {
enum class ResearchModule { bubble, droplet, flow };
inline constexpr std::array kResearchModules{ResearchModule::bubble, ResearchModule::droplet,
                                             ResearchModule::flow};

// Non-owning immutable view: every numeric field/choice refers to a canonical static spec.
struct ResearchDebugParameter final {
    std::string_view name, unit, classification;
    const double &minimum, &maximum, &initial;
    std::span<const double> choices;
    bool accepts(double value) const noexcept {
        if (!std::isfinite(value) || value < minimum || value > maximum)
            return false;
        return choices.empty() || std::find(choices.begin(), choices.end(), value) != choices.end();
    }
};

// Preview-only candidate adaptation. All lookup/conversion runs on the message/prepare thread.
// A future candidate adds state/spec/config adaptation and DSP dispatch; the view stays generic.
struct ResearchCoreParameterAdapter final {
    static const char* identifier(ResearchModule module) noexcept {
        return module == ResearchModule::bubble    ? "A1"
               : module == ResearchModule::droplet ? "B1"
                                                   : "D1";
    }
    static const char* displayName(ResearchModule module) noexcept {
        return module == ResearchModule::bubble    ? "A1 / BUBBLE"
               : module == ResearchModule::droplet ? "B1 / DROPLET"
                                                   : "D1 / FLOW";
    }
    static const char* configKey(ResearchModule module) noexcept {
        return module == ResearchModule::bubble    ? "bubbleA1"
               : module == ResearchModule::droplet ? "dropletB1"
                                                   : "flowD1";
    }
    static std::span<const double> values(const ResearchCoreTuningState& state,
                                          ResearchModule module) noexcept {
        switch (module) {
        case ResearchModule::bubble:
            return state.bubble;
        case ResearchModule::droplet:
            return state.droplet;
        case ResearchModule::flow:
            return state.flow;
        }
        return {};
    }
    static std::span<double> values(ResearchCoreTuningState& state,
                                    ResearchModule module) noexcept {
        switch (module) {
        case ResearchModule::bubble:
            return state.bubble;
        case ResearchModule::droplet:
            return state.droplet;
        case ResearchModule::flow:
            return state.flow;
        }
        return {};
    }
    static std::size_t parameterCount(ResearchModule module) noexcept {
        switch (module) {
        case ResearchModule::bubble:
            return research::kA1Parameters.size();
        case ResearchModule::droplet:
            return research::kB1Parameters.size();
        case ResearchModule::flow:
            return research::kD1Parameters.size();
        }
        return 0;
    }
    // Callers enumerate parameterCount first; static specs outlive every view.
    static ResearchDebugParameter parameter(ResearchModule module, std::size_t index) {
        const auto adapt = [](const auto& spec) -> ResearchDebugParameter {
            return {spec.name,
                    spec.unit,
                    spec.classification,
                    spec.minimum,
                    spec.maximum,
                    spec.initial,
                    {spec.choices.data(), spec.choiceCount}};
        };
        if (module == ResearchModule::bubble)
            return adapt(research::kA1Parameters.at(index));
        if (module == ResearchModule::droplet)
            return adapt(research::kB1Parameters.at(index));
        return adapt(research::kD1Parameters.at(index));
    }
    static std::string key(ResearchModule module, std::size_t index) {
        return std::string(identifier(module)) + "." + std::string(parameter(module, index).name);
    }
    static bool primary(ResearchModule module, std::size_t index) {
        const auto name = parameter(module, index).name;
        if (module == ResearchModule::flow)
            return true;
        if (module == ResearchModule::bubble)
            return name == "radiusMinMm" || name == "radiusMaxMm" || name == "populationGamma" ||
                   name == "persistenceScale" || name == "maxEventRateHz" ||
                   name == "motionFactor" || name == "residualGain";
        return name == "equivalentBubbleRadiusMm" || name == "persistenceScale" ||
               name == "entrainmentProbability" || name == "pinchOffDelayMs" ||
               name == "onsetRatioDb" || name == "minOnsetSpacingMs" || name == "residualGain";
    }
    static double getValue(const ResearchCoreTuningState& state, ResearchModule module,
                           std::size_t index) {
        return values(state, module)[index];
    }
    static bool setValue(ResearchCoreTuningState& state, ResearchModule module, std::size_t index,
                         double value) {
        if (index >= parameterCount(module) || !parameter(module, index).accepts(value))
            return false;
        values(state, module)[index] = value;
        return true;
    }
    static void resetModule(ResearchCoreTuningState& state, ResearchModule module) {
        for (std::size_t i = 0; i < parameterCount(module); ++i)
            values(state, module)[i] = parameter(module, i).initial;
    }
    static void resetAll(ResearchCoreTuningState& state) {
        state = {};
    }
    static bool validate(const ResearchCoreTuningState& state) {
        for (auto module : kResearchModules)
            for (std::size_t i = 0; i < parameterCount(module); ++i)
                if (!parameter(module, i).accepts(getValue(state, module, i)))
                    return false;
        return namedValue(state, ResearchModule::bubble, "radiusMinMm") <
               namedValue(state, ResearchModule::bubble, "radiusMaxMm");
    }
    // Named authority lookups avoid a second positional field-order contract. Never called in
    // process.
    static double namedValue(const ResearchCoreTuningState& state, ResearchModule module,
                             std::string_view name) {
        for (std::size_t i = 0; i < parameterCount(module); ++i)
            if (parameter(module, i).name == name)
                return getValue(state, module, i);
        return std::numeric_limits<double>::quiet_NaN();
    }
    // Precondition: validate(state), including choices before integer conversion.
    static research::BubbleA1Config makeBubbleA1Config(const ResearchCoreTuningState& state) {
        const auto v = [&](std::string_view name) {
            return namedValue(state, ResearchModule::bubble, name);
        };
        research::BubbleA1Config config;
        config.radiusMinMm = v("radiusMinMm");
        config.radiusMaxMm = v("radiusMaxMm");
        config.populationGamma = v("populationGamma");
        config.amplitudeRadiusExponent = v("amplitudeRadiusExponent");
        config.depthExponent = v("depthExponent");
        config.persistenceScale = v("persistenceScale");
        config.maxEventRateHz = v("maxEventRateHz");
        config.motionFactor = v("motionFactor");
        config.riseXi = v("riseXi");
        config.riseCutoff = v("riseCutoff");
        config.tailFloorDb = v("tailFloorDb");
        config.stealReleaseMs = v("stealReleaseMs");
        config.residualGain = v("residualGain");
        config.voiceCapacity = static_cast<std::size_t>(v("voiceCapacity"));
        config.riseModel =
            static_cast<research::BubbleA1RiseModel>(static_cast<int>(v("riseModel")));
        config.sourceEnergyAmplitude = v("sourceEnergyAmplitude") == 1;
        return config;
    }
    static research::SharedExcitationConfig
    makeSharedExcitationConfig(const ResearchCoreTuningState& state) {
        const auto v = [&](std::string_view name) {
            return namedValue(state, ResearchModule::bubble, name);
        };
        return {v("fastAttackMs"),  v("fastReleaseMs"),     v("slowAttackMs"),
                v("slowReleaseMs"), v("activityFloorDbFS"), v("activityKneeDb")};
    }
    static research::DropletB1Config makeDropletB1Config(const ResearchCoreTuningState& state) {
        return {state.droplet};
    }
    static research::FlowD1Config makeFlowD1Config(const ResearchCoreTuningState& state) {
        return {namedValue(state, ResearchModule::flow, "velocityScaleMps"),
                namedValue(state, ResearchModule::flow, "virtualStructureLengthMeters"),
                namedValue(state, ResearchModule::flow, "maxExcessPathMeters")};
    }
};
} // namespace frazil::water::preview
