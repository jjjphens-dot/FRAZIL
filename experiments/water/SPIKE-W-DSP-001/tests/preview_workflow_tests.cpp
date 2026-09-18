#include "preview/DraftSummary.h"
#include "preview/ExactValueControl.h"
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
    ExactValueControl widget;
    widget.configure("Test decay", .001, 2, .1, true);
    double edited = .1;
    widget.onEdit = [&](double value) {
        edited = value;
        return true;
    };
    widget.refreshValue(edited);
    juce::TextEditor* entry{};
    juce::Slider* slider{};
    for (auto* child : widget.getChildren()) {
        if (auto* text = dynamic_cast<juce::TextEditor*>(child))
            entry = text;
        if (auto* control = dynamic_cast<juce::Slider*>(child))
            slider = control;
    }
    check(entry && slider, "exact-entry composition");
    if (entry && slider) {
        const auto type = [&](const juce::String& value) {
            entry->setText(value, false);
            entry->onTextChange();
            entry->onReturnKey();
        };
        type("70ms");
        check(edited == .07 && entry->getText() == "70 ms", "text commits in seconds");
        for (const auto* invalid : {"0.07seconds", "NaN", "3s", "70ms junk"}) {
            type(invalid);
            check(edited == .07 && slider->getValue() == .07,
                  "invalid exact text preserves model and slider");
        }
        entry->onEscapeKey();
        check(entry->getText() == "70 ms", "Escape restores formatted value");
        type("bad");
        widget.refreshValue(.08);
        check(entry->getText() == "80 ms", "external recall replaces stale text");
        type("bad");
        widget.discardPendingText();
        check(entry->getText() == "80 ms",
              "explicit recall also clears stale text for unchanged values");
        type("1.001s");
        check(edited == 1.001 && entry->getText() == "1.001 s", "precision across time boundary");
        widget.configure("Depth", 0, 1, .5, false);
        widget.refreshValue(.5);
        const auto event = [&](float x) {
            return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(), {x, 10},
                                    juce::ModifierKeys(juce::ModifierKeys::shiftModifier |
                                                       juce::ModifierKeys::leftButtonModifier),
                                    1, 0, 0, 0, 0, slider, slider, juce::Time::getCurrentTime(),
                                    {100, 10}, juce::Time::getCurrentTime(), 1, x != 100);
        };
        slider->mouseDown(event(100));
        slider->mouseDrag(event(135));
        slider->mouseUp(event(135));
        check(std::abs(edited - .51) < 1e-12, "Shift drag fine adjustment in normalized space");
        widget.configure("Voices", 1, 16, 8, false, true);
        widget.refreshValue(8);
        edited = 8;
        type("2.5");
        check(edited == 8, "integer entry rejects fractional value");
    }
    session.setEngineering(ControlId::bubbleDecay, .081, ChangeOrigin::engineeringUI);
    check(draftSummary(session).contains("81 ms"), "draft differences use display units");
    return failures;
}
