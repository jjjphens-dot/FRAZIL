#include "PreviewController.h"
#include "ui/DeveloperDiagnosticsView.h"

#include <optional>

namespace frazil::water::preview {
class PreviewPanel final : public juce::Component, private juce::Timer {
  public:
    explicit PreviewPanel(const juce::String& sourceArgument) {
        title.setText("FRAZIL / WATER RESEARCH PREVIEW", juce::dontSendNotification);
        title.setFont(juce::FontOptions(22.0f));
        note.setText("SPIKE-W-DSP-001 | engineering controls, not Size/Motion macros | seed 42 | "
                     "not production / no Host automation",
                     juce::dontSendNotification);
        for (auto* label : {&title, &note, &source, &status, &appliedLabel, &gainLabel}) {
            addAndMakeVisible(*label);
            label->setColour(juce::Label::textColourId, juce::Colour(0xffbed4dc));
        }
        gainLabel.setText("Monitor output (dB)", juce::dontSendNotification);
        gain.setRange(-60, 0, .1);
        gain.setValue(-12, juce::dontSendNotification);
        gain.setSliderStyle(juce::Slider::LinearHorizontal);
        gain.setTextBoxStyle(juce::Slider::TextBoxRight, false, 68, 24);
        gain.setTooltip(
            "Monitor-only output gain; excluded from module config export. No auto makeup.");
        addAndMakeVisible(gain);
        gain.onValueChange = [this] { updateMonitor(); };
        const std::array<const char*, 9> names{"Fluid / A+B+D", "Resonant / C", "A / Bubble",
                                               "B / Droplet",   "D / Flow",     "A+B",
                                               "A+D",           "B+D",          "Baseline / x"};
        for (std::size_t i = 0; i < names.size(); ++i)
            mode.addItem(names[i], static_cast<int>(i) + 1);
        mode.setSelectedId(1, juce::dontSendNotification);
        mode.setTooltip(
            "Draft research composition. Apply config before Play; no live model transition.");
        addAndMakeVisible(mode);
        mode.onChange = [this] { edited(); };

        for (std::size_t i = 0; i < kControls.size(); ++i) {
            const auto& spec = kControls[i];
            labels[i].setText(spec.label, juce::dontSendNotification);
            labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffbed4dc));
            sliders[i].setRange(spec.minimum, spec.maximum, spec.step);
            sliders[i].setSliderStyle(juce::Slider::LinearHorizontal);
            sliders[i].setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 22);
            sliders[i].setValue(spec.initial, juce::dontSendNotification);
            sliders[i].setTooltip(
                juce::String(spec.module) + "." + spec.key +
                " | draft, prepare-time engineering value; inactive modules retain values");
            sliders[i].onValueChange = [this] { edited(); };
            addAndMakeVisible(labels[i]);
            addAndMakeVisible(sliders[i]);
        }
        const std::array<const char*, 4> groupNames{"A / BUBBLE", "B / DROPLET", "D / FLOW",
                                                    "C / MODAL"};
        for (std::size_t i = 0; i < headings.size(); ++i) {
            headings[i].setText(groupNames[i], juce::dontSendNotification);
            headings[i].setColour(juce::Label::textColourId, juce::Colour(0xff5ed0ba));
            addAndMakeVisible(headings[i]);
        }
        for (auto* button : buttons()) {
            addAndMakeVisible(*button);
            button->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff304d5a));
            button->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        }
        load.onClick = [this] { chooseSource(); };
        play.onClick = [this] {
            const auto error = controller.play(applied);
            setStatus(error.isEmpty() ? "Playing from start; fixed seed 42. Source + 30 s tail."
                                      : error);
        };
        stop.onClick = [this] {
            controller.stop();
            setStatus("Stopped. Play restarts source and DSP seed.");
        };
        apply.onClick = [this] { applyDraft(); };
        dry.onClick = [this] {
            monitor = MonitorMode::dry;
            updateMonitor();
        };
        processed.onClick = [this] {
            monitor = MonitorMode::processed;
            updateMonitor();
        };
        residual.onClick = [this] {
            monitor = MonitorMode::residual;
            updateMonitor();
        };
        captureA.onClick = [this] { capture(0); };
        captureB.onClick = [this] { capture(1); };
        applyA.onClick = [this] { recall(0); };
        applyB.onClick = [this] { recall(1); };
        reset.onClick = [this] {
            showSettings({});
            monitor = MonitorMode::processed;
            gain.setValue(-12, juce::dontSendNotification);
            updateMonitor();
            applyDraft();
        };
        copy.onClick = [this] {
            juce::SystemClipboard::copyTextToClipboard(applied.moduleJson());
            setStatus("Copied APPLIED module config. Renderer mode shown above; seed 42. Monitor "
                      "gain excluded.");
        };
        exportConfig.onClick = [this] { chooseExport(); };
        applyA.setEnabled(false);
        applyB.setEnabled(false);
        addAndMakeVisible(diagnosticsView);
        setSize(1180, 920);
        updateMonitor();
        refreshApplied();
        source.setText(controller.sourceDescription(), juce::dontSendNotification);
        setStatus("Load WAV -> edit engineering values -> Apply config -> Play. Capture A/B stores "
                  "applied values.");
        if (sourceArgument.isNotEmpty())
            loadSource(
                juce::File::getCurrentWorkingDirectory().getChildFile(sourceArgument.unquoted()));
        startTimerHz(10);
    }

    ~PreviewPanel() override {
        stopTimer();
        controller.stop();
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff0d181f));
    }

    void resized() override {
        auto area = getLocalBounds().reduced(20);
        title.setBounds(area.removeFromTop(32));
        note.setBounds(area.removeFromTop(25));
        source.setBounds(area.removeFromTop(25));
        auto transport = area.removeFromTop(38);
        for (auto* button : {&load, &play, &stop, &apply})
            button->setBounds(transport.removeFromLeft(128).reduced(3));
        mode.setBounds(transport.reduced(3));
        appliedLabel.setBounds(area.removeFromTop(24));
        area.removeFromTop(6);
        auto controls = area.removeFromTop(355);
        const int width = controls.getWidth() / 4;
        const std::array<std::size_t, 5> offsets{0, 7, 14, 18, 21};
        for (std::size_t group = 0; group < 4; ++group) {
            auto column = controls.removeFromLeft(width).reduced(7, 0);
            headings[group].setBounds(column.removeFromTop(25));
            for (auto i = offsets[group]; i < offsets[group + 1]; ++i) {
                labels[i].setBounds(column.removeFromTop(21));
                sliders[i].setBounds(column.removeFromTop(25));
            }
        }
        auto comparison = area.removeFromTop(38);
        for (auto* button : {&dry, &processed, &residual})
            button->setBounds(comparison.removeFromLeft(112).reduced(3));
        gainLabel.setBounds(comparison.removeFromLeft(145));
        gain.setBounds(comparison);
        auto workflow = area.removeFromTop(38);
        const auto buttonWidth = workflow.getWidth() / 7;
        for (auto* button : {&captureA, &applyA, &captureB, &applyB, &reset, &copy, &exportConfig})
            button->setBounds(workflow.removeFromLeft(buttonWidth).reduced(3));
        status.setBounds(area.removeFromBottom(52));
        diagnosticsView.setBounds(area.reduced(4));
    }

  private:
    struct CapturedState {
        PreviewSettings settings;
        MonitorMode monitor;
        double gain;
    };
    std::array<juce::TextButton*, 14> buttons() {
        return {&load,     &play,   &stop,     &apply,  &dry,   &processed, &residual,
                &captureA, &applyA, &captureB, &applyB, &reset, &copy,      &exportConfig};
    }
    PreviewSettings draft() const {
        PreviewSettings settings;
        settings.mode = mode.getSelectedId() - 1;
        for (std::size_t i = 0; i < settings.values.size(); ++i)
            settings.values[i] = sliders[i].getValue();
        return settings;
    }
    void showSettings(const PreviewSettings& settings) {
        mode.setSelectedId(settings.mode + 1, juce::dontSendNotification);
        for (std::size_t i = 0; i < settings.values.size(); ++i)
            sliders[i].setValue(settings.values[i], juce::dontSendNotification);
    }
    void edited() {
        controller.stop();
        dirty = true;
        play.setEnabled(false);
        refreshApplied();
        setStatus("Draft changed; playback stopped. Apply config before Play. Capture/export still "
                  "use APPLIED values.");
    }
    void applyDraft() {
        controller.stop();
        const auto settings = draft();
        const auto error = controller.validate(settings);
        if (error.isNotEmpty()) {
            setStatus(error);
            return;
        }
        applied = settings;
        dirty = false;
        play.setEnabled(true);
        refreshApplied();
        setStatus(
            "Config applied. Play starts a fresh deterministic run; this is not live automation.");
    }
    void capture(std::size_t slot) {
        slots[slot] = CapturedState{applied, monitor, gain.getValue()};
        (slot == 0 ? applyA : applyB).setEnabled(true);
        setStatus(juce::String("Captured ") + (slot == 0 ? "A" : "B") +
                  ": APPLIED config, composition and monitor state; source/position excluded.");
    }
    void recall(std::size_t slot) {
        if (!slots[slot])
            return;
        showSettings(slots[slot]->settings);
        monitor = slots[slot]->monitor;
        gain.setValue(slots[slot]->gain, juce::dontSendNotification);
        updateMonitor();
        applyDraft();
    }
    void updateMonitor() {
        controller.setMonitor(monitor, static_cast<float>(gain.getValue()));
        dry.setToggleState(monitor == MonitorMode::dry, juce::dontSendNotification);
        processed.setToggleState(monitor == MonitorMode::processed, juce::dontSendNotification);
        residual.setToggleState(monitor == MonitorMode::residual, juce::dontSendNotification);
    }
    void loadSource(const juce::File& file) {
        const auto error = controller.load(file);
        source.setText(controller.sourceDescription(), juce::dontSendNotification);
        setStatus(error.isEmpty() ? "Source loaded; no resampling. Play uses the applied config."
                                  : error);
    }
    void chooseSource() {
        controller.stop();
        chooser = std::make_unique<juce::FileChooser>("Load finite mono/stereo WAV", juce::File{},
                                                      "*.wav");
        chooser->launchAsync(juce::FileBrowserComponent::openMode |
                                 juce::FileBrowserComponent::canSelectFiles,
                             [safe = juce::Component::SafePointer<PreviewPanel>(this)](
                                 const juce::FileChooser& result) {
                                 if (safe != nullptr && result.getResult().existsAsFile())
                                     safe->loadSource(result.getResult());
                             });
    }
    void chooseExport() {
        chooser = std::make_unique<juce::FileChooser>(
            "Export applied research config",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("water-research.json"),
            "*.json");
        // Snapshot at command time; edits while the chooser is open cannot change the export.
        const auto json = applied.moduleJson();
        chooser->launchAsync(
            juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles |
                juce::FileBrowserComponent::warnAboutOverwriting,
            [safe = juce::Component::SafePointer<PreviewPanel>(this),
             json](const juce::FileChooser& result) {
                if (safe == nullptr || result.getResult() == juce::File{})
                    return;
                const bool ok = result.getResult().replaceWithText(json);
                safe->setStatus(ok ? "Exported module JSON for existing renderer. Supply "
                                     "composition + seed 42 separately; monitor gain excluded."
                                   : "Export failed.");
            });
    }
    void refreshApplied() {
        appliedLabel.setText(juce::String("Applied: ") +
                                 kModes[static_cast<std::size_t>(applied.mode)] + " | seed 42 | " +
                                 (dirty ? "DRAFT NOT APPLIED" : "ready") + " | Monitor: " +
                                 (monitor == MonitorMode::dry         ? "x"
                                  : monitor == MonitorMode::processed ? "x+E"
                                                                      : "E") +
                                 " | " + juce::String(controller.positionSeconds(), 1) + " s",
                             juce::dontSendNotification);
    }
    void setStatus(const juce::String& text) {
        status.setText(text, juce::dontSendNotification);
    }
    void timerCallback() override {
        diagnosticsView.update(controller.diagnostics(),
                               kModes[static_cast<std::size_t>(applied.mode)]);
        refreshApplied();
        if (controller.playing() && (controller.finished() || controller.deviceRateMismatch())) {
            const bool mismatch = controller.deviceRateMismatch();
            controller.stop();
            setStatus(mismatch
                          ? "Device rate changed; stopped. Play reopens at the source sample rate."
                          : "Finished source and 30 s tail. Play restarts seed and source.");
        }
    }
    PreviewController controller;
    PreviewSettings applied;
    std::array<std::optional<CapturedState>, 2> slots;
    MonitorMode monitor{MonitorMode::processed};
    bool dirty{};
    juce::Label title, note, source, status, appliedLabel, gainLabel;
    std::array<juce::Label, 4> headings;
    std::array<juce::Label, kControls.size()> labels;
    std::array<juce::Slider, kControls.size()> sliders;
    juce::ComboBox mode;
    juce::Slider gain;
    juce::TextButton load{"Load WAV"}, play{"Play / Restart"}, stop{"Stop"}, apply{"Apply config"};
    juce::TextButton dry{"Dry / x"}, processed{"Processed / x+E"}, residual{"Residual / E"};
    juce::TextButton captureA{"Capture A"}, applyA{"Apply A"}, captureB{"Capture B"},
        applyB{"Apply B"};
    juce::TextButton reset{"Reset config"}, copy{"Copy config"}, exportConfig{"Export config"};
    ui::DeveloperDiagnosticsView diagnosticsView;
    std::unique_ptr<juce::FileChooser> chooser;
};

class PreviewApplication final : public juce::JUCEApplication {
  public:
    const juce::String getApplicationName() override {
        return "FRAZIL Water Research Preview";
    }
    const juce::String getApplicationVersion() override {
        return "0.1.0";
    }
    void initialise(const juce::String& commandLine) override {
        window = std::make_unique<Window>(getApplicationName(), commandLine.trim());
    }
    void shutdown() override {
        window.reset();
    }

  private:
    // Keep all engineering controls and meter readouts legible on smaller displays.
    class ScrollablePanel final : public juce::Viewport {
      public:
        explicit ScrollablePanel(const juce::String& source) {
            setScrollBarsShown(true, false);
            setViewedComponent(new PreviewPanel(source), true);
            setSize(1180, 840);
        }
        void resized() override {
            juce::Viewport::resized();
            if (auto* panel = getViewedComponent())
                panel->setSize(getWidth() - getScrollBarThickness(), std::max(920, getHeight()));
        }
    };
    class Window final : public juce::DocumentWindow {
      public:
        Window(const juce::String& name, const juce::String& source)
            : DocumentWindow(name, juce::Colour(0xff0d181f), allButtons) {
            setUsingNativeTitleBar(true);
            setContentOwned(new ScrollablePanel(source), true);
            setResizable(true, false);
            setResizeLimits(1080, 700, 1600, 1100);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }
        void closeButtonPressed() override {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };
    std::unique_ptr<Window> window;
};
} // namespace frazil::water::preview

START_JUCE_APPLICATION(frazil::water::preview::PreviewApplication)
