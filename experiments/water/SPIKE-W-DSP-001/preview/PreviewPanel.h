#pragma once

#include "PreviewController.h"
#include "ProtectView.h"
#include "ResearchViews.h"
#include "SessionCodec.h"
#include "ui/DeveloperDiagnosticsView.h"

namespace frazil::water::preview {

// Message-thread coordinator: owns the session and device controller, validates lifecycle commands,
// and renders both views from the session. Widgets do not own draft/applied/A/B state.
class PreviewPanel final : public juce::Component, private juce::Timer {
  public:
    explicit PreviewPanel(const juce::String& sourceArgument)
        : sound_(session_, ChangeOrigin::soundLeadUI, [this] { controller_.stop(); }),
          engineering_(session_, [this] { controller_.stop(); }),
          protect_(session_, [this] { controller_.stop(); }) {
        researchLabel(*this, title_, "FRAZIL / WATER RESEARCH PREVIEW");
        title_.setFont(juce::FontOptions(22));
        researchLabel(*this, note_,
                      "Research only | shared experiment state | no Host automation | seed 42");
        for (auto* label : {&source_, &status_, &appliedLabel_, &gainLabel_})
            researchLabel(*this, *label, "");
        gainLabel_.setText("Monitor output (dB)", juce::dontSendNotification);
        tabs_.addTab("Sound Lead", juce::Colour(0xff182b36), &sound_, false);
        tabs_.addTab("Engineering", juce::Colour(0xff182b36), &engineering_, false);
        addAndMakeVisible(tabs_);
        addAndMakeVisible(protect_);
        protect_.onLayoutChange = [this] { resized(); };
        for (auto* button : buttons()) {
            addAndMakeVisible(*button);
            button->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff304d5a));
            button->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        }
        gain_.setRange(-60, 0, .1);
        gain_.setValue(-12, juce::dontSendNotification);
        gain_.setSliderStyle(juce::Slider::LinearHorizontal);
        gain_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 24);
        gain_.setTooltip("Monitor gain only; no auto makeup and no module-config field.");
        addAndMakeVisible(gain_);
        gain_.onValueChange = [this] {
            session_.setMonitor(session_.draft().monitor, gain_.getValue());
        };
        load_.onClick = [this] { chooseSource(); };
        play_.onClick = [this] {
            const auto error = controller_.play(session_.applied().engineering);
            setStatus(error.isEmpty() ? "Playing from start; fixed seed 42; source + 30 s tail."
                                      : error);
        };
        stop_.onClick = [this] {
            controller_.stop();
            setStatus("Stopped. Play restarts source and seed.");
        };
        apply_.onClick = [this] { applyDraft(); };
        dry_.onClick = [this] {
            session_.setMonitor(MonitorMode::dry, session_.draft().monitorGainDb);
        };
        processed_.onClick = [this] {
            session_.setMonitor(MonitorMode::processed, session_.draft().monitorGainDb);
        };
        residual_.onClick = [this] {
            session_.setMonitor(MonitorMode::residual, session_.draft().monitorGainDb);
        };
        captureA_.onClick = [this] { capture(0); };
        captureB_.onClick = [this] { capture(1); };
        applyA_.onClick = [this] { recall(0); };
        applyB_.onClick = [this] { recall(1); };
        reset_.onClick = [this] {
            controller_.stop();
            session_.reset();
            applyDraft();
        };
        copy_.onClick = [this] {
            juce::SystemClipboard::copyTextToClipboard(session_.applied().engineering.moduleJson());
            setStatus("Copied APPLIED module config; composition/seed/monitor are separate.");
        };
        export_.onClick = [this] { chooseExport(); };
        exportSession_.onClick = [this] { chooseExport(true); };
        importSession_.onClick = [this] { chooseSessionImport(); };
        copySession_.onClick = [this] {
            juce::SystemClipboard::copyTextToClipboard(encodeSession(session_.applied()));
            setStatus("Copied APPLIED research session, including four experiment macros.");
        };
        session_.onChange = [this] { refresh(); };
        addAndMakeVisible(diagnostics_);
        refresh();
        source_.setText(controller_.sourceDescription(), juce::dontSendNotification);
        setStatus("Load WAV -> edit -> Apply config -> Play. Size/Motion/Decay are UNMAPPED "
                  "experiment state.");
        if (sourceArgument.isNotEmpty())
            loadSource(
                juce::File::getCurrentWorkingDirectory().getChildFile(sourceArgument.unquoted()));
        setSize(1180, 1495);
        startTimerHz(10);
    }
    ~PreviewPanel() override {
        stopTimer();
        session_.onChange = {};
        controller_.stop();
    }
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff0d181f));
    }
    void resized() override {
        auto area = getLocalBounds().reduced(20);
        title_.setBounds(area.removeFromTop(32));
        note_.setBounds(area.removeFromTop(25));
        source_.setBounds(area.removeFromTop(25));
        auto transport = area.removeFromTop(38);
        for (auto* button : {&load_, &play_, &stop_, &apply_})
            button->setBounds(transport.removeFromLeft(140).reduced(3));
        appliedLabel_.setBounds(area.removeFromTop(30));
        tabs_.setBounds(area.removeFromTop(645));
        protect_.setBounds(area.removeFromTop(protect_.preferredHeight()));
        auto monitor = area.removeFromTop(38);
        for (auto* button : {&dry_, &processed_, &residual_})
            button->setBounds(monitor.removeFromLeft(145).reduced(3));
        gainLabel_.setBounds(monitor.removeFromLeft(145));
        gain_.setBounds(monitor);
        auto workflow = area.removeFromTop(38);
        const int width = workflow.getWidth() / 7;
        for (auto* button : {&captureA_, &applyA_, &captureB_, &applyB_, &reset_, &copy_, &export_})
            button->setBounds(workflow.removeFromLeft(width).reduced(3));
        auto sessions = area.removeFromTop(38);
        for (auto* button : {&importSession_, &exportSession_, &copySession_})
            button->setBounds(sessions.removeFromLeft(170).reduced(3));
        status_.setBounds(area.removeFromBottom(52));
        diagnostics_.setBounds(area.reduced(4));
    }

  private:
    std::array<juce::TextButton*, 17> buttons() {
        return {&load_,     &play_,     &stop_,          &apply_,         &dry_,        &processed_,
                &residual_, &captureA_, &applyA_,        &captureB_,      &applyB_,     &reset_,
                &copy_,     &export_,   &importSession_, &exportSession_, &copySession_};
    }
    void applyDraft() {
        controller_.stop();
        if (!validProtectMemory(session_.draft().protectMemory)) {
            setStatus(
                "Protect: retained D0/D1 calibrations require Low < High in their own units.");
            return;
        }
        const auto error = controller_.validate(session_.draft().engineering);
        if (error.isNotEmpty()) {
            setStatus(error);
            return;
        }
        session_.applyValidated();
        setStatus("Applied. Play starts a fresh run. Unmapped macros retain values without "
                  "modifying DSP.");
    }
    void capture(std::size_t slot) {
        session_.capture(slot);
        refresh();
        setStatus(juce::String("Captured ") + (slot == 0 ? "A" : "B") +
                  ": APPLIED experiment, engineering and monitor state.");
    }
    void recall(std::size_t slot) {
        if (!session_.slot(slot))
            return;
        controller_.stop();
        const auto& candidate = *session_.slot(slot);
        const auto error = controller_.validate(candidate.engineering);
        if (error.isNotEmpty()) {
            setStatus(error);
            return;
        }
        session_.restoreValidated(candidate);
        setStatus("Recalled snapshot. Play restarts; source position is not restored.");
    }
    void refresh() {
        sound_.refresh();
        engineering_.refresh();
        protect_.refresh();
        const auto& state = session_.draft();
        gain_.setValue(state.monitorGainDb, juce::dontSendNotification);
        controller_.setMonitor(state.monitor, static_cast<float>(state.monitorGainDb));
        controller_.setProtectDepth(session_.applied().engineering.protect.depth);
        dry_.setToggleState(state.monitor == MonitorMode::dry, juce::dontSendNotification);
        processed_.setToggleState(state.monitor == MonitorMode::processed,
                                  juce::dontSendNotification);
        residual_.setToggleState(state.monitor == MonitorMode::residual,
                                 juce::dontSendNotification);
        play_.setEnabled(!session_.dirty());
        applyA_.setEnabled(session_.slot(0).has_value());
        applyB_.setEnabled(session_.slot(1).has_value());
        refreshApplied();
    }
    void refreshApplied() {
        appliedLabel_.setText(
            juce::String(session_.dirty() ? "DRAFT — " : "APPLIED — ") +
                juce::String(static_cast<int>(session_.unappliedChanges())) +
                " unapplied changes | applied " +
                kModes[static_cast<std::size_t>(session_.applied().engineering.mode)] + " | rev " +
                juce::String(static_cast<juce::int64>(session_.revision())) + " / " +
                originName(session_.draft().lastChange.origin) + " | " +
                juce::String(controller_.positionSeconds(), 1) + " s",
            juce::dontSendNotification);
    }
    void setStatus(const juce::String& text) {
        status_.setText(text, juce::dontSendNotification);
    }
    void loadSource(const juce::File& file) {
        const auto error = controller_.load(file);
        source_.setText(controller_.sourceDescription(), juce::dontSendNotification);
        setStatus(error.isEmpty() ? "Source loaded; no resampling. Play uses APPLIED config."
                                  : error);
    }
    void chooseSource() {
        controller_.stop();
        chooser_ = std::make_unique<juce::FileChooser>("Load finite mono/stereo WAV", juce::File{},
                                                       "*.wav");
        chooser_->launchAsync(juce::FileBrowserComponent::openMode |
                                  juce::FileBrowserComponent::canSelectFiles,
                              [safe = juce::Component::SafePointer<PreviewPanel>(this)](
                                  const juce::FileChooser& result) {
                                  if (safe && result.getResult().existsAsFile())
                                      safe->loadSource(result.getResult());
                              });
    }
    void chooseSessionImport() {
        chooser_ =
            std::make_unique<juce::FileChooser>("Import research session", juce::File{}, "*.json");
        chooser_->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [safe = juce::Component::SafePointer<PreviewPanel>(this)](
                const juce::FileChooser& result) {
                if (!safe || !result.getResult().existsAsFile())
                    return;
                const auto file = result.getResult();
                juce::MemoryBlock bytes;
                if (file.getSize() > 1024 * 1024 || !file.loadFileAsData(bytes)) {
                    safe->setStatus("Session: cannot read file (maximum 1 MiB).");
                    return;
                }
                ResearchSessionState candidate;
                auto error = decodeSession(
                    {static_cast<const char*>(bytes.getData()), bytes.getSize()}, candidate);
                if (error.isEmpty())
                    error = safe->controller_.validate(candidate.engineering);
                if (error.isNotEmpty()) {
                    safe->setStatus(error);
                    return;
                }
                safe->controller_.stop();
                safe->session_.restoreValidated(candidate);
                safe->setStatus("Imported research session. Current WAV retained; Play restarts "
                                "from its beginning.");
            });
    }
    void chooseExport(bool session = false) {
        chooser_ = std::make_unique<juce::FileChooser>(
            session ? "Export applied research session" : "Export applied research config",
            juce::File::getCurrentWorkingDirectory().getChildFile(session ? "water-session.json"
                                                                          : "water-research.json"),
            "*.json");
        const auto json = session ? encodeSession(session_.applied())
                                  : session_.applied().engineering.moduleJson();
        chooser_->launchAsync(
            juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles |
                juce::FileBrowserComponent::warnAboutOverwriting,
            [safe = juce::Component::SafePointer<PreviewPanel>(this), json,
             session](const juce::FileChooser& result) {
                if (!safe || result.getResult() == juce::File{})
                    return;
                safe->setStatus(
                    result.getResult().replaceWithText(json)
                        ? (session ? "Exported APPLIED research session; source audio is separate."
                                   : "Exported APPLIED module JSON; supply composition and seed "
                                     "separately.")
                        : "Export failed.");
            });
    }
    void timerCallback() override {
        diagnostics_.update(controller_.diagnostics(),
                            kModes[static_cast<std::size_t>(session_.applied().engineering.mode)]);
        refreshApplied();
        if (controller_.playing() && (controller_.finished() || controller_.deviceRateMismatch())) {
            const bool mismatch = controller_.deviceRateMismatch();
            controller_.stop();
            setStatus(mismatch ? "Device rate changed; stopped. Play reopens at source rate."
                               : "Finished source + 30 s tail.");
        }
    }
    ResearchSessionModel session_;
    PreviewController controller_;
    WaterMacroView sound_;
    EngineeringView engineering_;
    ProtectView protect_;
    juce::TabbedComponent tabs_{juce::TabbedButtonBar::TabsAtTop};
    juce::Label title_, note_, source_, status_, appliedLabel_, gainLabel_;
    juce::Slider gain_;
    juce::TextButton load_{"Load WAV"}, play_{"Play / Restart"}, stop_{"Stop"},
        apply_{"Apply config"};
    juce::TextButton dry_{"Source / x"}, processed_{"Full / x+E"}, residual_{"Water only / E"};
    juce::TextButton captureA_{"Capture A"}, applyA_{"Apply A"}, captureB_{"Capture B"},
        applyB_{"Apply B"};
    juce::TextButton reset_{"Reset baseline"}, copy_{"Copy config"}, export_{"Export config"};
    juce::TextButton importSession_{"Import Session"}, exportSession_{"Export Session"},
        copySession_{"Copy Session"};
    ui::DeveloperDiagnosticsView diagnostics_;
    std::unique_ptr<juce::FileChooser> chooser_;
};
} // namespace frazil::water::preview
