#include "preview/PreviewController.h"
#include "preview/SessionCodec.h"

#include <iostream>

int runSessionCodecTests() {
    using namespace frazil::water::preview;
    int failures{};
    const auto check = [&](bool ok, const char* message) {
        if (!ok) {
            std::cerr << "FAIL session codec: " << message << '\n';
            ++failures;
        }
    };
    ResearchSessionModel session;
    check(session.draft().water.decay == .5, "provisional Decay baseline");
    const auto moduleBaseline = session.draft().engineering.moduleJson();
    session.setMacro(MacroId::decay, .75, ChangeOrigin::soundLeadUI);
    check(session.draft().water.decay == .75 && session.dirty() &&
              session.draft().engineering.moduleJson() != moduleBaseline,
          "Decay maps owned persistence targets");
    session.setModel(WaterModel::resonant, ChangeOrigin::engineeringUI);
    session.setEngineering(ControlId::modalRoot, 500, ChangeOrigin::engineeringUI);
    session.setMonitor(MonitorMode::residual, -20);
    session.applyValidated();
    session.capture(0);
    const auto text = encodeSession(session.applied());
    ResearchSessionState decoded;
    check(decodeSession(text.toStdString(), decoded).isEmpty(), "valid manifest decode");
    check(decoded.water == session.applied().water &&
              decoded.engineering.moduleJson() == session.applied().engineering.moduleJson() &&
              decoded.engineering.mode == 1 && decoded.monitor == MonitorMode::residual &&
              decoded.monitorGainDb == -20 &&
              decoded.ownership[controlIndex(ControlId::modalRoot)].origin ==
                  ChangeOrigin::engineeringUI,
          "four macros config monitor and provenance roundtrip");
    PreviewController controller;
    check(controller.validate(decoded.engineering).isEmpty(),
          "imported config uses existing DSP validation");
    session.reset();
    session.applyValidated();
    check(session.applied().water.decay == .5, "reset includes Decay");
    session.restoreValidated(*session.slot(0));
    check(session.applied().water.decay == .75, "A/B includes Decay");
    session.restoreValidated(decoded);
    check(session.applied().water.decay == .75 &&
              session.draft().lastChange.origin == ChangeOrigin::sessionLoad,
          "import includes Decay and new origin");
    const auto original = encodeSession(decoded);
    auto legacy = juce::JSON::parse(text);
    auto* legacyObject = legacy.getDynamicObject();
    legacyObject->setProperty("version", 1);
    for (const auto* field : {"mappingRevision", "perMacroMappingState",
                              "listeningCalibrationState", "auditionETrimDb"})
        legacyObject->removeProperty(field);
    for (const auto* field : {"motionDepth", "motionIntervalSeconds"}) {
        legacy["configuration"]["modal"].getDynamicObject()->removeProperty(field);
        legacy["ownership"]["targets"].getDynamicObject()->removeProperty(
            juce::Identifier(juce::String("modal.") + field));
    }
    legacy["configuration"]["droplet"].getDynamicObject()->removeProperty("eventsEnabled");
    legacy["ownership"]["targets"].getDynamicObject()->removeProperty("droplet.eventsEnabled");
    legacy["configuration"]["droplet"].getDynamicObject()->removeProperty("eventActivity");
    legacy["ownership"]["targets"].getDynamicObject()->removeProperty("droplet.eventActivity");
    auto legacyExpected = decoded.engineering;
    legacyExpected.values[controlIndex(ControlId::modalMotionDepth)] = 0;
    legacyExpected.values[controlIndex(ControlId::modalMotionInterval)] = .7;
    ResearchSessionState migrated;
    check(
        decodeSession(juce::JSON::toString(legacy, false, 17).toStdString(), migrated).isEmpty() &&
            migrated.engineering.moduleJson() == legacyExpected.moduleJson() &&
            migrated.water == decoded.water && migrated.monitorGainDb == decoded.monitorGainDb &&
            migrated.mappingRevision == "legacy-unmapped" && migrated.auditionETrimDb == 0 &&
            migrated.macroMappings ==
                std::array{MappingStatus::custom, MappingStatus::custom, MappingStatus::custom},
        "v1 preserves all engineering values and never adopts research mapping");
    auto v2 = juce::JSON::parse(text);
    v2.getDynamicObject()->setProperty("version", 2);
    v2.getDynamicObject()->setProperty("mappingRevision", "research-water-mapping-v0.1");
    v2["configuration"]["droplet"].getDynamicObject()->removeProperty("eventsEnabled");
    v2["ownership"]["targets"].getDynamicObject()->removeProperty("droplet.eventsEnabled");
    v2["configuration"]["droplet"].getDynamicObject()->removeProperty("eventActivity");
    v2["ownership"]["targets"].getDynamicObject()->removeProperty("droplet.eventActivity");
    check(decodeSession(juce::JSON::toString(v2, false, 17).toStdString(), migrated).isEmpty() &&
              migrated.engineering.moduleJson() == decoded.engineering.moduleJson() &&
              migrated.mappingRevision == "legacy-research-v0.1" &&
              migrated.macroMappings ==
                  std::array{MappingStatus::custom, MappingStatus::custom, MappingStatus::custom},
          "v0.1 raw targets survive exactly and macros become CUSTOM");
    check(decodeSession(encodeSession(migrated).toStdString(), migrated).isEmpty() &&
              migrated.mappingRevision == "legacy-research-v0.1",
          "migrated legacy v3 can be exported and imported without adoption");
    auto v3 = juce::JSON::parse(text);
    v3.getDynamicObject()->setProperty("version", 3);
    v3["configuration"]["droplet"].getDynamicObject()->removeProperty("eventActivity");
    v3["ownership"]["targets"].getDynamicObject()->removeProperty("droplet.eventActivity");
    check(decodeSession(juce::JSON::toString(v3, false, 17).toStdString(), migrated).isEmpty() &&
              migrated.engineering.values[controlIndex(ControlId::dropletEventActivity)] == 1,
          "v3 defaults activity to legacy one without changing mapping");
    auto probability = decoded;
    probability.engineering.values[controlIndex(ControlId::dropletEventActivity)] = .25;
    check(decodeSession(encodeSession(probability).toStdString(), migrated).isEmpty() &&
              migrated.engineering.moduleJson() == probability.engineering.moduleJson(),
          "v4 retains explicit activity without macro adoption");
    auto invalidMapping = juce::JSON::parse(text);
    invalidMapping.getDynamicObject()->setProperty("mappingRevision", "unknown-revision");
    check(decodeSession(juce::JSON::toString(invalidMapping).toStdString(), decoded).isNotEmpty() &&
              encodeSession(decoded) == original,
          "unknown mapping revision atomic reject");
    invalidMapping = juce::JSON::parse(text);
    invalidMapping.getDynamicObject()->setProperty("auditionETrimDb", 36.1);
    check(decodeSession(juce::JSON::toString(invalidMapping).toStdString(), decoded).isNotEmpty(),
          "out of range monitor trim rejected");
    invalidMapping = juce::JSON::parse(text);
    invalidMapping.getDynamicObject()->setProperty("mappingRevision", "legacy-unmapped");
    check(decodeSession(juce::JSON::toString(invalidMapping).toStdString(), decoded).isNotEmpty(),
          "legacy revision cannot claim mapped macro");
    invalidMapping = juce::JSON::parse(text);
    invalidMapping["configuration"]["droplet"].getDynamicObject()->setProperty("refractorySeconds",
                                                                               .04);
    check(decodeSession(juce::JSON::toString(invalidMapping).toStdString(), decoded).isNotEmpty() &&
              encodeSession(decoded) == original,
          "mapped raw-target contradiction atomic reject");
    for (const auto& bad : {juce::String("{}"), text + "{}", text + "junk",
                            text.replace("\"decay\": 0.75", "\"decay\": 0.75, \"decay\": 0.5"),
                            text.replace("\"decay\": 0.75", "\"decay\": 1.2"),
                            text.replace("\"decay\": 0.75", "\"decay\": NaN"),
                            text.replace("\"decay\": 0.75", "\"decay\": true"),
                            text.replace("\"decay\": 0.75", "\"decay\": 01"),
                            text.replace("\"version\": 4", "\"version\": 5"),
                            text.replace("\"seed\": 42", "\"seed\": 41")}) {
        check(bad != text, "invalid fixture actually mutates manifest");
        check(decodeSession(bad.toStdString(), decoded).isNotEmpty() &&
                  encodeSession(decoded) == original,
              "invalid import leaves candidate unchanged");
    }
    for (const std::string invalid :
         {"{\"a\":1e999}", "{\"a\":9223372036854775808}", "{\"a\":[1]}", "{\"a\":null}",
          R"({"a":"\q"})", "{\"a\":-01}", "{\"a\":1.}", "{\"a\":1e+}"}) {
        SessionJsonSyntax syntax(invalid);
        check(!syntax.valid(), "strict session grammar");
    }
    const auto escapedDuplicate =
        text.replace("\"decay\": 0.75", R"("decay": 0.75, "d\u0065cay": 0.5)");
    check(decodeSession(escapedDuplicate.toStdString(), decoded).isNotEmpty(),
          "decoded-equivalent duplicate keys rejected");
    const auto unknown = text.replace("\"decay\": 0.75", "\"decay\": 0.75, \"unknown\": 0.5");
    check(decodeSession(unknown.toStdString(), decoded).isNotEmpty(),
          "unknown session fields rejected");
    std::cout << "session codec tests failures=" << failures << '\n';
    return failures;
}
