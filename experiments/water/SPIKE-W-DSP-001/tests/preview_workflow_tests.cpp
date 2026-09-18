#include "preview/PreviewController.h"
#include "preview/SessionCodec.h"

#include <iostream>

int runWorkflowTests() {
    using namespace frazil::water::preview;
    int failures{};
    const auto check = [&](bool ok, const char* message) {
        if (!ok) {
            std::cerr << "FAIL workflow: " << message << '\n';
            ++failures;
        }
    };
    ResearchSessionModel session;
    session.setSource({"impulse.wav", 48000, 1, 48000});
    session.setMacro(MacroId::decay, .8, ChangeOrigin::soundLeadUI);
    session.setEngineering(ControlId::bubbleMinFrequency, 300, ChangeOrigin::engineeringUI);
    session.applyValidated();
    const auto manifest = encodeSession(session.applied());
    ResearchSessionState candidate;
    check(decodeSession(manifest.toStdString(), candidate).isEmpty() &&
              candidate.source == session.applied().source &&
              candidate.build.commit == kPreviewGitCommit,
          "source and build metadata roundtrip");
    auto invalidSource = juce::JSON::parse(manifest);
    invalidSource["source"].getDynamicObject()->setProperty("channels", 3);
    const auto before = encodeSession(candidate);
    check(
        decodeSession(juce::JSON::toString(invalidSource).toStdString(), candidate).isNotEmpty() &&
            encodeSession(candidate) == before,
        "invalid source metadata is atomic");
    session.setMonitor(MonitorMode::dry, -18);
    check(session.applied().lastChange.revision == session.revision(), "live monitor provenance");
    session.setProtect(ProtectId::depth, .4, ChangeOrigin::soundLeadUI);
    check(session.applied().lastChange.revision == session.revision(), "live depth provenance");
    session.reset();
    check(session.draft().source.name == "impulse.wav", "reset retains loaded source");
    session.setModel(WaterModel::resonant, ChangeOrigin::engineeringUI);
    PreviewController controller;
    check(decodeModuleConfig(R"({"modal":{"rootFrequencyHz":400}})", session.draft(), candidate)
                  .isEmpty() &&
              candidate.engineering.values[controlIndex(ControlId::modalRoot)] == 400 &&
              candidate.engineering.values[controlIndex(ControlId::bubbleMinFrequency)] == 250 &&
              candidate.water == session.draft().water && candidate.customEngineering &&
              candidate.source == session.draft().source &&
              controller.validate(candidate.engineering).isEmpty(),
          "partial module import uses renderer defaults and preserves session context");
    session.restoreValidated(candidate);
    check(session.applied().importedBuild.has_value() &&
              session.applied().build.commit == kPreviewGitCommit &&
              session.applied().lastChange.origin == ChangeOrigin::sessionLoad,
          "restore preserves imported provenance and stamps current build");
    ResearchSessionState roundtrip;
    check(decodeModuleConfig(candidate.engineering.moduleJson().toStdString(), candidate, roundtrip)
                  .isEmpty() &&
              roundtrip.engineering.moduleJson() == candidate.engineering.moduleJson(),
          "module config roundtrip");
    session.setModel(WaterModel::fluid, ChangeOrigin::engineeringUI);
    session.applyValidated();
    const auto original = encodeSession(session.applied());
    for (const auto* invalid : {R"({"droplet":{"voices":true}})", R"({"water":{"size":0.5}})",
                                R"({"flow":{"baseDelaySeconds":0.001,"depthSeconds":0.005}})",
                                R"({"protect":{"epsilon":0.01}})"}) {
        auto error = decodeModuleConfig(invalid, session.draft(), candidate);
        if (error.isEmpty())
            error = controller.validate(candidate.engineering);
        if (error.isEmpty())
            session.restoreValidated(candidate);
        check(error.isNotEmpty() && encodeSession(session.applied()) == original,
              "invalid module never commits partial state");
    }
    auto settings = PreviewSettings{};
    settings.values[controlIndex(ControlId::flowBaseDelay)] = .001;
    settings.values[controlIndex(ControlId::flowDepth)] = .005;
    check(controller.validate(settings).contains("FLOW"), "coupled Flow error identifies module");
    settings = {};
    settings.values[controlIndex(ControlId::bubbleMinFrequency)] = 9000;
    check(controller.validate(settings).contains("BUBBLE"), "Bubble error identifies module");
    settings = {};
    settings.mode = 1;
    settings.values[controlIndex(ControlId::modalRoot)] = 9000;
    check(controller.validate(settings).contains("MODAL"),
          "sample-rate Modal error identifies module");
    settings = {};
    settings.protect.gain.epsilon = .01;
    check(controller.validate(settings).contains("PROTECT"), "Protect error identifies module");
    return failures;
}
