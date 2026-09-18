#pragma once

#include "ResearchSessionModel.h"
#include "SessionJsonSyntax.h"
#include "render/ReadConfig.h"

#include <initializer_list>

namespace frazil::water::preview {
namespace sessionDetail {
inline juce::var object() {
    return juce::var(new juce::DynamicObject());
}
inline void put(juce::var& object, const char* key, const juce::var& value) {
    object.getDynamicObject()->setProperty(key, value);
}
inline bool fields(const juce::var& value, std::initializer_list<const char*> names) {
    const auto* object = value.getDynamicObject();
    if (!object || object->getProperties().size() != static_cast<int>(names.size()))
        return false;
    for (const auto* name : names)
        if (!value.hasProperty(name))
            return false;
    return true;
}
inline bool number(const juce::var& value, double low, double high, double& result) {
    if (!value.isInt() && !value.isInt64() && !value.isDouble())
        return false;
    const double candidate = static_cast<double>(value);
    if (!std::isfinite(candidate) || candidate < low || candidate > high)
        return false;
    result = candidate;
    return true;
}
inline bool integer(const juce::var& value, int low, int high, int& result) {
    double numberValue{};
    if (!number(value, low, high, numberValue) || numberValue != std::floor(numberValue))
        return false;
    result = static_cast<int>(numberValue);
    return true;
}
inline juce::var ownership(const ControlOwnership& state) {
    auto value = object();
    put(value, "origin", static_cast<int>(state.origin));
    put(value, "revision", static_cast<juce::int64>(state.revision));
    return value;
}
inline bool readOwnership(const juce::var& value, ControlOwnership& output) {
    int origin{};
    if (!fields(value, {"origin", "revision"}) || !integer(value["origin"], 0, 4, origin) ||
        (!value["revision"].isInt() && !value["revision"].isInt64()))
        return false;
    const auto revision = static_cast<juce::int64>(value["revision"]);
    if (revision < 0)
        return false;
    output = {static_cast<ChangeOrigin>(origin), static_cast<std::uint64_t>(revision)};
    return true;
}
} // namespace sessionDetail

// Separate research manifest. It is never plugin state or renderer module schema. Encoding and
// decoding run on the message thread; the caller validates prepared DSP config before restoring.
inline juce::String encodeSession(const ResearchSessionState& state) {
    using namespace sessionDetail;
    auto root = object();
    put(root, "format", "frazil.water-research-session");
    put(root, "version", 1);
    auto water = object();
    put(water, "model", static_cast<int>(state.water.model));
    put(water, "size", state.water.size);
    put(water, "motion", state.water.motion);
    put(water, "decay", state.water.decay);
    put(root, "water", water);
    put(root, "configuration", juce::JSON::parse(state.engineering.moduleJson()));
    put(root, "composition", state.engineering.mode);
    put(root, "seed", 42);
    auto monitor = object();
    put(monitor, "mode", static_cast<int>(state.monitor));
    put(monitor, "gainDb", state.monitorGainDb);
    put(root, "monitor", monitor);
    auto control = object();
    put(control, "modelMapping", static_cast<int>(state.modelMapping));
    put(control, "customEngineering", state.customEngineering);
    put(control, "lastChange", ownership(state.lastChange));
    auto targets = object();
    for (std::size_t i = 0; i < kControls.size(); ++i)
        targets.getDynamicObject()->setProperty(
            juce::Identifier(juce::String(kControls[i].stableId())), ownership(state.ownership[i]));
    put(control, "targets", targets);
    put(root, "ownership", control);
    return juce::JSON::toString(root, false, 17);
}

// Atomic candidate decode: no session mutation, output assigned only after complete schema checks.
// Actual prepare-time coupled/rate constraints remain with the existing DSP/controller authority.
inline juce::String decodeSession(std::string_view text, ResearchSessionState& output) {
    using namespace sessionDetail;
    SessionJsonSyntax syntax(text);
    if (!syntax.valid())
        return "Session: invalid JSON syntax, size or nesting.";
    if (text.starts_with("\xef\xbb\xbf"))
        text.remove_prefix(3);
    juce::var root;
    if (juce::JSON::parse(juce::String::fromUTF8(text.data(), static_cast<int>(text.size())), root)
            .failed() ||
        SessionJsonSyntax::decodedProperties(root) != syntax.properties())
        return "Session: duplicate or invalid fields.";
    int version{}, seed{};
    if (!fields(root, {"format", "version", "water", "configuration", "composition", "seed",
                       "monitor", "ownership"}) ||
        !root["format"].isString() ||
        root["format"].toString() != "frazil.water-research-session" ||
        !integer(root["version"], 1, 1, version) || !integer(root["seed"], 42, 42, seed))
        return "Session: unsupported format/version/seed.";
    ResearchSessionState candidate;
    int model{}, mode{}, monitorMode{}, mapping{};
    const auto& water = root["water"];
    if (!fields(water, {"model", "size", "motion", "decay"}) ||
        !integer(water["model"], 0, 1, model) ||
        !number(water["size"], 0, 1, candidate.water.size) ||
        !number(water["motion"], 0, 1, candidate.water.motion) ||
        !number(water["decay"], 0, 1, candidate.water.decay) ||
        !integer(root["composition"], 0, 8, mode))
        return "Session: invalid Water experiment state.";
    candidate.water.model = static_cast<WaterModel>(model);
    candidate.engineering.mode = mode;
    const auto& monitor = root["monitor"];
    if (!fields(monitor, {"mode", "gainDb"}) || !integer(monitor["mode"], 0, 2, monitorMode) ||
        !number(monitor["gainDb"], -60, 0, candidate.monitorGainDb))
        return "Session: invalid monitor state.";
    candidate.monitor = static_cast<MonitorMode>(monitorMode);
    const auto& control = root["ownership"];
    if (!fields(control, {"modelMapping", "customEngineering", "lastChange", "targets"}) ||
        !integer(control["modelMapping"], 1, 2, mapping) ||
        !control["customEngineering"].isBool() ||
        !readOwnership(control["lastChange"], candidate.lastChange))
        return "Session: invalid ownership metadata.";
    candidate.modelMapping = static_cast<MappingStatus>(mapping);
    candidate.customEngineering = static_cast<bool>(control["customEngineering"]);
    if (model != (mode == 1 ? 1 : 0) || mapping != (mode <= 1 ? 1 : 2))
        return "Session: Model/composition mapping is inconsistent.";
    const auto& targets = control["targets"];
    if (!targets.isObject() ||
        targets.getDynamicObject()->getProperties().size() != static_cast<int>(kControls.size()))
        return "Session: incomplete target provenance.";
    const auto& config = root["configuration"];
    research::FluidConfig fluid;
    research::ModalConfig modal;
    if (!research::readConfigText(juce::JSON::toString(config).toStdString(), fluid, modal))
        return "Session: invalid module config.";
    for (std::size_t i = 0; i < kControls.size(); ++i) {
        const auto& spec = kControls[i];
        if (!readOwnership(targets[juce::Identifier(juce::String(spec.stableId()))],
                           candidate.ownership[i]) ||
            !config[spec.module].hasProperty(spec.key) ||
            !number(config[spec.module][spec.key], spec.minimum, spec.maximum,
                    candidate.engineering.values[i]) ||
            (spec.valueType == ControlValueType::integer &&
             candidate.engineering.values[i] != std::floor(candidate.engineering.values[i])))
            return "Session: invalid field " + juce::String(spec.stableId());
    }
    output = std::move(candidate);
    return {};
}
} // namespace frazil::water::preview
