#pragma once

#include "DraftSummary.h"
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
    std::function<void()> onLayoutChange;
    explicit PreviewPanel(const juce::String& sourceArgument)
        : operations_(session_), sound_(session_, ChangeOrigin::soundLeadUI, operations_),
          engineering_(session_, operations_), protect_(session_, operations_, [this] {
              return tabs_.getCurrentTabIndex() == 0 ? ChangeOrigin::soundLeadUI
                                                     : ChangeOrigin::engineeringUI;
          }) {
        operations_.onPrepareBegin = [this] { controller_.stop(); };
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
        protect_.onLayoutChange = engineering_.onLayoutChange =
            tabs_.onChange = [this] { updateLayout(); };
        for (auto* toggle : {&showDraft_, &showDiagnostics_}) {
            addAndMakeVisible(toggle);
            toggle->onClick = [this] { updateLayout(); };
        }
        draftDetails_.setMultiLine(true);
        draftDetails_.setReadOnly(true);
        draftDetails_.setTitle("Unapplied changes: applied to draft");
        addChildComponent(draftDetails_);
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
            const auto value = gain_.getValue();
            operations_.edit("Monitor gain", ChangeOrigin::soundLeadUI, false);
            session_.setMonitor(session_.draft().monitor, value);
        };
        gain_.onMouseBegin = [this] {
            operations_.edit("Monitor gain", ChangeOrigin::soundLeadUI, false, false, true);
        };
        gain_.onMouseEnd = [this] { operations_.finish(); };
        load_.onClick = [this] { chooseSource(); };
        play_.onClick = [this] {
            operations_.finish(false);
            const auto error = controller_.play(session_.applied().engineering);
            setStatus(error.isEmpty() ? "Playing from start; fixed seed 42; source + 30 s tail."
                                      : error);
        };
        stop_.onClick = [this] {
            operations_.finish(false);
            controller_.stop();
            setStatus("Stopped. Play restarts source and seed.");
        };
        apply_.onClick = [this] {
            operations_.finish(false);
            applyDraft();
        };
        dry_.onClick = [this] { setMonitorMode(MonitorMode::dry); };
        processed_.onClick = [this] { setMonitorMode(MonitorMode::processed); };
        residual_.onClick = [this] { setMonitorMode(MonitorMode::residual); };
        captureA_.onClick = [this] { capture(0); };
        captureB_.onClick = [this] { capture(1); };
        applyA_.onClick = [this] { recall(0); };
        applyB_.onClick = [this] { recall(1); };
        reset_.onClick = [this] {
            operations_.action("Reset Baseline", ChangeOrigin::reset, false, false, [this] {
                session_.reset();
                discardPendingText();
                applyDraft();
            });
        };
        copy_.onClick = [this] {
            juce::SystemClipboard::copyTextToClipboard(session_.applied().engineering.moduleJson());
            setStatus("Copied APPLIED module config; composition/seed/monitor are separate.");
        };
        export_.onClick = [this] { chooseExport(); };
        exportSession_.onClick = [this] { chooseExport(true); };
        importSession_.onClick = [this] { chooseSessionImport(); };
        importModule_.onClick = [this] { chooseSessionImport(false); };
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
        setSize(1180, preferredHeight());
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
    int preferredHeight() const noexcept {
        return 420 + tabHeight() + protect_.preferredHeight() +
               (showDraft_.getToggleState() ? 110 : 0) +
               (showDiagnostics_.getToggleState() ? 230 : 0);
    }
    void resized() override {
        auto area = getLocalBounds().reduced(20);
        title_.setBounds(area.removeFromTop(32));
        note_.setBounds(area.removeFromTop(25));
        source_.setBounds(area.removeFromTop(25));
        auto transport = area.removeFromTop(38);
        for (auto* button : {&load_, &play_, &stop_, &apply_})
            button->setBounds(transport.removeFromLeft(140).reduced(3));
        showDraft_.setBounds(transport.removeFromLeft(160));
        showDiagnostics_.setBounds(transport.removeFromLeft(160));
        status_.setBounds(area.removeFromTop(44));
        appliedLabel_.setBounds(area.removeFromTop(30));
        draftDetails_.setVisible(showDraft_.getToggleState());
        if (showDraft_.getToggleState())
            draftDetails_.setBounds(area.removeFromTop(110).reduced(3));
        tabs_.setBounds(area.removeFromTop(tabHeight()));
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
        for (auto* button : {&importModule_, &importSession_, &exportSession_, &copySession_})
            button->setBounds(sessions.removeFromLeft(170).reduced(3));
        diagnostics_.setVisible(showDiagnostics_.getToggleState());
        if (showDiagnostics_.getToggleState())
            diagnostics_.setBounds(area.removeFromTop(230).reduced(4));
    }

  private:
    void setMonitorMode(MonitorMode mode) {
        operations_.action("Monitor mode", ChangeOrigin::soundLeadUI, false, false, [this, mode] {
            session_.setMonitor(mode, session_.draft().monitorGainDb);
        });
    }
    class ViewTabs final : public juce::TabbedComponent {
      public:
        ViewTabs() : juce::TabbedComponent(juce::TabbedButtonBar::TabsAtTop) {}
        std::function<void()> onChange;
        void currentTabChanged(int, const juce::String&) override {
            if (onChange)
                onChange();
        }
    };
    int tabHeight() const noexcept {
        return 32 + (tabs_.getCurrentTabIndex() == 0 ? 205 : engineering_.preferredHeight());
    }
    void updateLayout() {
        resized();
        if (onLayoutChange)
            onLayoutChange();
    }
    std::array<juce::TextButton*, 18> buttons() {
        return {&load_,          &play_,        &stop_,        &apply_,  &dry_,
                &processed_,     &residual_,    &captureA_,    &applyA_, &captureB_,
                &applyB_,        &reset_,       &copy_,        &export_, &importSession_,
                &exportSession_, &copySession_, &importModule_};
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
    void discardPendingText() {
        engineering_.discardPendingText();
        protect_.discardPendingText();
    }
    void capture(std::size_t slot) {
        operations_.finish();
        session_.capture(slot);
        refresh();
        setStatus(juce::String("Captured ") + (slot == 0 ? "A" : "B") +
                  ": APPLIED experiment, engineering and monitor state.");
    }
    void recall(std::size_t slot) {
        if (!session_.slot(slot))
            return;
        const auto& candidate = *session_.slot(slot);
        const auto error = controller_.validate(
            candidate.engineering,
            candidate.source.sampleRate > 0 ? candidate.source.sampleRate : 48000);
        if (error.isNotEmpty()) {
            setStatus(error);
            return;
        }
        operations_.action(slot == 0 ? "Apply A" : "Apply B", ChangeOrigin::sessionLoad, true,
                           false, [this, &candidate] {
                               session_.restoreValidated(candidate);
                               discardPendingText();
                           });
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
        const auto actualSource = controller_.sourceMetadata();
        const auto& requiredSource = session_.applied().source;
        const bool sourceMatches = requiredSource.name.isEmpty() || actualSource == requiredSource;
        play_.setEnabled(!session_.dspDirty() && actualSource.name.isNotEmpty() && sourceMatches);
        source_.setText(controller_.sourceDescription() +
                            (sourceMatches ? "" : " | Session requires: " + requiredSource.name),
                        juce::dontSendNotification);
        source_.setTooltip(
            controller_.sourceDescription() + " | Session source: " + requiredSource.name +
            "; filename/rate/channels/frames are descriptive, not a content identity check.");
        applyA_.setEnabled(session_.slot(0).has_value());
        applyB_.setEnabled(session_.slot(1).has_value());
        refreshApplied();
        draftDetails_.setText(draftSummary(session_), false);
    }
    void refreshApplied() {
        appliedLabel_.setText(
            juce::String(session_.dspDirty() ? "DSP DIRTY | " : "DSP APPLIED | ") +
                (session_.sessionDirty() ? "SESSION DIRTY | " : "SESSION APPLIED | ") +
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
        operations_.finish(false);
        const auto error = controller_.load(file);
        if (error.isEmpty())
            operations_.action("Load Source", ChangeOrigin::soundLeadUI, false, false,
                               [this] { session_.setSource(controller_.sourceMetadata()); });
        source_.setText(controller_.sourceDescription(), juce::dontSendNotification);
        setStatus(error.isEmpty() ? "Source loaded; no resampling. Play uses APPLIED config."
                                  : error);
    }
    void chooseSource() {
        operations_.finish(false);
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
    void chooseSessionImport(bool session = true) {
        chooser_ = std::make_unique<juce::FileChooser>(session ? "Import research session"
                                                               : "Import renderer module config",
                                                       juce::File{}, "*.json");
        chooser_->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [safe = juce::Component::SafePointer<PreviewPanel>(this),
             session](const juce::FileChooser& result) {
                if (!safe || !result.getResult().existsAsFile())
                    return;
                const auto file = result.getResult();
                juce::MemoryBlock bytes;
                if (file.getSize() > 1024 * 1024 || !file.loadFileAsData(bytes)) {
                    safe->setStatus("Session: cannot read file (maximum 1 MiB).");
                    return;
                }
                ResearchSessionState candidate;
                const std::string_view text{static_cast<const char*>(bytes.getData()),
                                            bytes.getSize()};
                auto error = session ? decodeSession(text, candidate)
                                     : decodeModuleConfig(text, safe->session_.draft(), candidate);
                if (error.isEmpty())
                    error = session ? safe->controller_.validate(candidate.engineering,
                                                                 candidate.source.sampleRate > 0
                                                                     ? candidate.source.sampleRate
                                                                     : 48000)
                                    : safe->controller_.validate(candidate.engineering);
                if (error.isNotEmpty()) {
                    safe->setStatus(error);
                    return;
                }
                safe->operations_.action(session ? "Import Session" : "Import Module",
                                         ChangeOrigin::sessionLoad, true, false, [&] {
                                             safe->session_.restoreValidated(candidate);
                                             safe->discardPendingText();
                                         });
                safe->setStatus(session
                                    ? "Imported session. If source metadata differs, load the "
                                      "matching WAV before Play; audio is not embedded."
                                    : "Imported module config using renderer defaults for omitted "
                                      "fields; macros/source retained; engineering CUSTOM.");
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
        operations_.tick(ResearchOperations::clockNow());
        protect_.updateDiagnostics(controller_.protectDiagnostics());
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
    ResearchOperations operations_;
    PreviewController controller_;
    WaterMacroView sound_;
    EngineeringView engineering_;
    ProtectView protect_;
    ViewTabs tabs_;
    juce::ToggleButton showDraft_{"Draft details"}, showDiagnostics_{"Audio diagnostics"};
    juce::TextEditor draftDetails_;
    juce::TooltipWindow tooltips_{this, 500};
    juce::Label title_, note_, source_, status_, appliedLabel_, gainLabel_;
    ResearchSlider gain_;
    juce::TextButton load_{"Load WAV"}, play_{"Play / Restart"}, stop_{"Stop"},
        apply_{"Apply config"};
    juce::TextButton dry_{"Source / x"}, processed_{"Full / x+E"}, residual_{"Water only / E"};
    juce::TextButton captureA_{"Capture A"}, applyA_{"Apply A"}, captureB_{"Capture B"},
        applyB_{"Apply B"};
    juce::TextButton reset_{"Reset baseline"}, copy_{"Copy config"}, export_{"Export config"};
    juce::TextButton importSession_{"Import Session"}, exportSession_{"Export Session"},
        copySession_{"Copy Session"};
    juce::TextButton importModule_{"Import Module Config"};
    ui::DeveloperDiagnosticsView diagnostics_;
    std::unique_ptr<juce::FileChooser> chooser_;
};
} // namespace frazil::water::preview
