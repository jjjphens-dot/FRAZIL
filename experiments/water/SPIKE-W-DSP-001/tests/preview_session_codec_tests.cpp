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
              session.draft().engineering.moduleJson() == moduleBaseline,
          "Decay is shared experiment state without DSP mapping");
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
    ResearchSessionState migrated;
    check(
        decodeSession(juce::JSON::toString(legacy, false, 17).toStdString(), migrated).isEmpty() &&
            migrated.engineering.moduleJson() == decoded.engineering.moduleJson() &&
            migrated.water == decoded.water && migrated.monitorGainDb == decoded.monitorGainDb &&
            migrated.mappingRevision == "legacy-unmapped" && migrated.auditionETrimDb == 0 &&
            migrated.macroMappings ==
                std::array{MappingStatus::custom, MappingStatus::custom, MappingStatus::custom},
        "v1 preserves all engineering values and never adopts research mapping");
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
    invalidMapping["perMacroMappingState"].getDynamicObject()->setProperty("motion", 1);
    check(decodeSession(juce::JSON::toString(invalidMapping).toStdString(), decoded).isNotEmpty(),
          "legacy revision cannot claim mapped macro");
    for (const auto& bad : {juce::String("{}"), text + "{}", text + "junk",
                            text.replace("\"decay\": 0.75", "\"decay\": 0.75, \"decay\": 0.5"),
                            text.replace("\"decay\": 0.75", "\"decay\": 1.2"),
                            text.replace("\"decay\": 0.75", "\"decay\": NaN"),
                            text.replace("\"decay\": 0.75", "\"decay\": true"),
                            text.replace("\"decay\": 0.75", "\"decay\": 01"),
                            text.replace("\"version\": 2", "\"version\": 3"),
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
