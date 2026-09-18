#pragma once

#include "ResearchSessionModel.h"
#include "SessionJsonSyntax.h"
#include "render/ReadConfig.h"

#include <initializer_list>

namespace frazil::water::preview {
namespace sessionDetail {
inline bool textValue(const juce::var& value, juce::String& output, int maximum) {
    if (!value.isString() || value.toString().length() > maximum)
        return false;
    output = value.toString();
    return true;
}
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
inline juce::var buildValue(const BuildMetadata& build) {
    auto value = object();
    put(value, "commit", build.commit);
    put(value, "state", build.state);
    put(value, "variant", build.variant);
    put(value, "compiler", build.compiler);
    return value;
}
inline bool readBuild(const juce::var& value, BuildMetadata& build) {
    return fields(value, {"commit", "state", "variant", "compiler"}) &&
           textValue(value["commit"], build.commit, 64) &&
           (build.commit == "unknown" ||
            (build.commit.length() == 40 && build.commit.containsOnly("0123456789abcdefABCDEF"))) &&
           textValue(value["state"], build.state, 16) &&
           (build.state == "clean" || build.state == "dirty" || build.state == "unknown") &&
           textValue(value["variant"], build.variant, 64) &&
           textValue(value["compiler"], build.compiler, 128);
}
} // namespace sessionDetail

// Separate research manifest. It is never plugin state or renderer module schema. Encoding and
// decoding run on the message thread; the caller validates prepared DSP config before restoring.
inline juce::String encodeSession(const ResearchSessionState& state) {
    using namespace sessionDetail;
    auto root = object();
    put(root, "format", "frazil.water-research-session");
    put(root, "version", 2);
    put(root, "mappingRevision", state.mappingRevision);
    auto mappings = object();
    put(mappings, "size", static_cast<int>(state.macroMappings[0]));
    put(mappings, "motion", static_cast<int>(state.macroMappings[1]));
    put(mappings, "decay", static_cast<int>(state.macroMappings[2]));
    put(root, "perMacroMappingState", mappings);
    put(root, "listeningCalibrationState", static_cast<int>(state.listeningCalibration));
    put(root, "auditionETrimDb", state.auditionETrimDb);
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
    auto memory = object();
    put(memory, "d0Low", state.protectMemory.difference.low);
    put(memory, "d0High", state.protectMemory.difference.high);
    put(memory, "d1Low", state.protectMemory.logRatio.low);
    put(memory, "d1High", state.protectMemory.logRatio.high);
    put(memory, "lastNonzeroDepth", state.protectMemory.lastNonzeroDepth);
    put(memory, "fluidTopology", static_cast<int>(state.protectMemory.fluidTopology));
    put(root, "protectMemory", memory);
    auto source = object();
    put(source, "name", state.source.name);
    put(source, "sampleRate", state.source.sampleRate);
    put(source, "channels", state.source.channels);
    put(source, "frames", static_cast<juce::int64>(state.source.frames));
    put(root, "source", source);
    put(root, "build", buildValue(state.build));
    put(root, "importedBuild", state.importedBuild ? buildValue(*state.importedBuild) : object());
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
    if (!integer(root["version"], 1, 2, version))
        return "Session: unsupported version.";
    const bool knownFields =
        version == 1
            ? fields(root,
                     {"format", "version", "water", "configuration", "composition", "seed",
                      "monitor", "ownership", "protectMemory", "source", "build", "importedBuild"})
            : fields(root, {"format", "version", "water", "configuration", "composition", "seed",
                            "monitor", "ownership", "protectMemory", "source", "build",
                            "importedBuild", "mappingRevision", "perMacroMappingState",
                            "listeningCalibrationState", "auditionETrimDb"});
    if (!knownFields || !root["format"].isString() ||
        root["format"].toString() != "frazil.water-research-session" ||
        !integer(root["seed"], 42, 42, seed))
        return "Session: unsupported format/version/seed.";
    ResearchSessionState candidate;
    candidate.mappingRevision = "legacy-unmapped";
    candidate.macroMappings.fill(MappingStatus::custom);
    candidate.listeningCalibration = MappingStatus::custom;
    candidate.auditionETrimDb = 0;
    if (version == 2) {
        const auto& mappings = root["perMacroMappingState"];
        int calibration{};
        if (!textValue(root["mappingRevision"], candidate.mappingRevision, 64) ||
            (candidate.mappingRevision != "legacy-unmapped" &&
             candidate.mappingRevision != "research-water-mapping-v0.1") ||
            !fields(mappings, {"size", "motion", "decay"}) ||
            !integer(root["listeningCalibrationState"], 1, 2, calibration) ||
            !number(root["auditionETrimDb"], 0, 36, candidate.auditionETrimDb))
            return "Session: invalid research mapping/calibration/trim.";
        candidate.listeningCalibration = static_cast<MappingStatus>(calibration);
        constexpr std::array names{"size", "motion", "decay"};
        for (std::size_t i = 0; i < names.size(); ++i) {
            int status{};
            if (!integer(mappings[names[i]], 1, 2, status) ||
                (candidate.mappingRevision == "legacy-unmapped" && status != 2))
                return "Session: invalid per-macro mapping state.";
            candidate.macroMappings[i] = static_cast<MappingStatus>(status);
        }
    }
    const auto& source = root["source"];
    int channels{}, frames{};
    if (!fields(source, {"name", "sampleRate", "channels", "frames"}) ||
        !textValue(source["name"], candidate.source.name, 255) ||
        candidate.source.name.containsAnyOf("/\\") ||
        !number(source["sampleRate"], 0, 96000, candidate.source.sampleRate) ||
        !integer(source["channels"], 0, 2, channels) ||
        !integer(source["frames"], 0, 11520000, frames))
        return "Session: invalid source metadata.";
    candidate.source.channels = channels;
    candidate.source.frames = frames;
    if (candidate.source.name.isEmpty()
            ? (candidate.source.sampleRate != 0 || channels != 0 || frames != 0)
            : (candidate.source.sampleRate < 44100 || channels < 1 || frames < 1 ||
               frames > 120 * candidate.source.sampleRate))
        return "Session: inconsistent source metadata.";
    if (!readBuild(root["build"], candidate.build) || !root["importedBuild"].isObject())
        return "Session: invalid build provenance.";
    if (root["importedBuild"].getDynamicObject()->getProperties().size() != 0) {
        BuildMetadata imported;
        if (!readBuild(root["importedBuild"], imported))
            return "Session: invalid imported build provenance.";
        candidate.importedBuild = imported;
    }
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
    if (!targets.isObject() || targets.getDynamicObject()->getProperties().size() !=
                                   (version == 1 ? 21 : static_cast<int>(kControls.size())))
        return "Session: incomplete target provenance.";
    const auto& config = root["configuration"];
    research::FluidConfig fluid;
    research::ModalConfig modal;
    if (!research::readConfigText(juce::JSON::toString(config, false, 17).toStdString(), fluid,
                                  modal, &candidate.engineering.protect))
        return "Session: invalid module config.";
    const auto& protect = config["protect"];
    if (!protect.isObject() || protect.getDynamicObject()->getProperties().size() != 13 ||
        !protect.hasProperty("detector") || !protect.hasProperty("topology"))
        return "Session: incomplete Protect config.";
    for (const auto& spec : kProtectControls) {
        double value{};
        if (!protect.hasProperty(spec.key) ||
            !number(protect[spec.key], spec.minimum,
                    protectMaximum(spec, candidate.engineering.protect.gain.score), value))
            return "Session: invalid Protect field " + juce::String(spec.key);
    }
    const auto& memory = root["protectMemory"];
    auto& restored = candidate.protectMemory;
    int topology{};
    if (!fields(memory,
                {"d0Low", "d0High", "d1Low", "d1High", "lastNonzeroDepth", "fluidTopology"}) ||
        !number(memory["d0Low"], 0, 1, restored.difference.low) ||
        !number(memory["d0High"], 0, 1, restored.difference.high) ||
        !number(memory["d1Low"], 0, 100, restored.logRatio.low) ||
        !number(memory["d1High"], 0, 100, restored.logRatio.high) ||
        !number(memory["lastNonzeroDepth"], std::numeric_limits<double>::denorm_min(), 1,
                restored.lastNonzeroDepth) ||
        !integer(memory["fluidTopology"], 1, 3, topology) ||
        restored.difference.low >= restored.difference.high ||
        restored.logRatio.low >= restored.logRatio.high)
        return "Session: invalid Protect calibration memory.";
    restored.fluidTopology = static_cast<research::FluidProtectTopology>(topology);
    const auto& active =
        candidate.engineering.protect.gain.score == research::ProtectScore::difference
            ? restored.difference
            : restored.logRatio;
    const auto& settings = candidate.engineering.protect;
    if (settings.gain.thresholdLow != active.low || settings.gain.thresholdHigh != active.high ||
        (mode == 1 ? settings.topology != research::FluidProtectTopology::whole
                   : settings.topology != restored.fluidTopology))
        return "Session: Protect config and retained state disagree.";
    for (std::size_t i = 0; i < kControls.size(); ++i) {
        const auto& spec = kControls[i];
        if (version == 1 && i >= 21) {
            // v1 predates Modal motion. Fill typed legacy defaults, never mapped defaults.
            candidate.engineering.values[i] = spec.initial;
            candidate.ownership[i] = {ChangeOrigin::sessionLoad, 0};
            if (config[spec.module].hasProperty(spec.key))
                return "Session: v1 cannot contain v2 Modal motion fields.";
            continue;
        }
        if (!readOwnership(targets[juce::Identifier(juce::String(spec.stableId()))],
                           candidate.ownership[i]) ||
            !config[spec.module].hasProperty(spec.key) ||
            !number(config[spec.module][spec.key], spec.minimum, spec.maximum,
                    candidate.engineering.values[i]) ||
            (spec.valueType == ControlValueType::integer &&
             candidate.engineering.values[i] != std::floor(candidate.engineering.values[i])))
            return "Session: invalid field " + juce::String(spec.stableId());
    }
    if (version == 2 && candidate.listeningCalibration == MappingStatus::mapped &&
        !ResearchListeningCalibration::matches(candidate.engineering))
        return "Session: listening calibration claim contradicts gains.";
    if (version == 2 && candidate.mappingRevision == ResearchWaterMacroMapper::revision.data()) {
        const auto mapped = ResearchWaterMacroMapper::map(candidate.water);
        for (const auto& spec : kControls) {
            const auto owner = macroOwner(spec.id);
            if (owner && candidate.macroMappings[static_cast<std::size_t>(*owner)] ==
                             MappingStatus::mapped) {
                const double expected = mappedTarget(spec.id, *mapped);
                if (std::abs(candidate.engineering.values[controlIndex(spec.id)] - expected) >
                    1e-12 * std::max(1.0, std::abs(expected)))
                    return "Session: mapped target contradicts macro: " +
                           juce::String(spec.stableId());
            }
        }
    }
    output = std::move(candidate);
    return {};
}
// Renderer module import uses that parser's default-for-omitted-fields semantics. It retains the
// current composition/macros/source, marks engineering CUSTOM, and never changes state on failure.
inline juce::String decodeModuleConfig(std::string_view text, const ResearchSessionState& current,
                                       ResearchSessionState& output) {
    if (text.size() > 1024 * 1024)
        return "Module config: maximum size is 1 MiB.";
    research::FluidConfig fluid;
    research::ModalConfig modal;
    ProtectSettings protect;
    if (!research::readConfigText(text, fluid, modal, &protect))
        return "Module config: invalid, unknown or duplicate fields.";
    auto candidate = current;
    candidate.engineering.assignConfigs(fluid, modal, protect);
    for (std::size_t i = 0; i < kControls.size(); ++i) {
        const auto& spec = kControls[i];
        const double value = candidate.engineering.values[i];
        if (!std::isfinite(value) || value < spec.minimum || value > spec.maximum ||
            (spec.valueType == ControlValueType::integer && value != std::floor(value)))
            return "Module config: outside UI range for " + juce::String(spec.stableId());
    }
    for (const auto& spec : kProtectControls) {
        const double value = protectValue(protect, spec.id);
        if (!std::isfinite(value) || value < spec.minimum ||
            value > protectMaximum(spec, protect.gain.score))
            return "Module config: invalid protect." + juce::String(spec.key);
    }
    if (candidate.engineering.mode == 1 &&
        protect.topology != research::FluidProtectTopology::whole)
        return "Protect: Resonant C requires Whole topology; select Fluid before importing F2/F3.";
    auto& calibration = protect.gain.score == research::ProtectScore::difference
                            ? candidate.protectMemory.difference
                            : candidate.protectMemory.logRatio;
    calibration = {protect.gain.thresholdLow, protect.gain.thresholdHigh};
    if (candidate.engineering.mode != 1)
        candidate.protectMemory.fluidTopology = protect.topology;
    if (protect.depth > 0)
        candidate.protectMemory.lastNonzeroDepth = protect.depth;
    if (!validProtectMemory(candidate.protectMemory))
        return "Protect: Low must be less than High in each retained detector domain.";
    candidate.customEngineering = true;
    candidate.macroMappings.fill(MappingStatus::custom);
    candidate.listeningCalibration = MappingStatus::custom;
    output = std::move(candidate);
    return {};
}
} // namespace frazil::water::preview
