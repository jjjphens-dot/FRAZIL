#pragma once

#include "ExactValueControl.h"
#include "ProtectDiagnostics.h"
#include "ResearchSessionModel.h"

namespace frazil::water::preview {

// A distinct research module, not a fifth accepted Water macro. All controls use the session owner.
class ProtectView final : public juce::Component {
  public:
    std::function<void()> onLayoutChange;
    ProtectView(ResearchSessionModel& session, std::function<void()> beforePrepareEdit)
        : session_(session), beforePrepareEdit_(std::move(beforePrepareEdit)) {
        title_.setText("PROTECT / RESEARCH — residual only", juce::dontSendNotification);
        title_.setColour(juce::Label::textColourId, juce::Colour(0xff5ed0ba));
        addAndMakeVisible(title_);
        addAndMakeVisible(enabled_);
        addAndMakeVisible(detector_);
        addAndMakeVisible(topology_);
        addAndMakeVisible(advanced_);
        addAndMakeVisible(note_);
        enabled_.setTooltip(
            "LIVE: OFF targets Depth 0; ON restores last nonzero Depth (initial convenience 0.5).");
        enabled_.onClick = [this] {
            session_.setProtectEnabled(enabled_.getToggleState(), ChangeOrigin::soundLeadUI);
        };
        detector_.addItem("D0 / Difference", 1);
        detector_.addItem("D1 / Log Ratio", 2);
        detector_.setTooltip(
            "APPLY; neither candidate is a perceptual ranking. Each retains its own thresholds.");
        detector_.onChange = [this] {
            beforePrepareEdit_();
            session_.setDetector(detector_.getSelectedId() == 1 ? research::ProtectScore::difference
                                                                : research::ProtectScore::logRatio,
                                 ChangeOrigin::engineeringUI);
        };
        topology_.addItem("Whole / F1", 1);
        topology_.addItem("Droplet Exempt / F2", 2);
        topology_.addItem("Droplet Half / F3", 3);
        topology_.onChange = [this] {
            beforePrepareEdit_();
            session_.setTopology(
                static_cast<research::FluidProtectTopology>(topology_.getSelectedId()),
                ChangeOrigin::engineeringUI);
        };
        advanced_.onClick = [this] {
            refresh();
            if (onLayoutChange)
                onLayoutChange();
        };
        for (std::size_t i = 0; i < controls_.size(); ++i) {
            addAndMakeVisible(controls_[i]);
            controls_[i].onEdit = [this, i](double value) {
                if (kProtectControls[i].lifecycle == ControlLifecycle::prepareRequired)
                    beforePrepareEdit_();
                return session_.setProtect(kProtectControls[i].id, value,
                                           ChangeOrigin::engineeringUI);
            };
        }
        note_.setColour(juce::Label::textColourId, juce::Colour(0xffbed4dc));
        for (auto& label : readouts_) {
            label.setColour(juce::Label::textColourId, juce::Colour(0xffbed4dc));
            label.setJustificationType(juce::Justification::topLeft);
            addAndMakeVisible(label);
        }
        observation_.setColour(juce::Label::textColourId, juce::Colour(0xffbed4dc));
        addAndMakeVisible(observation_);
        updateDiagnostics({});
        refresh();
    }
    int preferredHeight() const noexcept {
        return advanced_.getToggleState() ? 455 : 290;
    }
    void updateDiagnostics(const ProtectDiagnosticsSnapshot& snapshot) {
        const std::array<const char*, 5> names{"Fast (amplitude)", "Slow (amplitude)",
                                               "D0 (amplitude)", "D1 (dB ratio)",
                                               "GR (dB attenuation)"};
        const std::array<double, 5> current{snapshot.latest.fast, snapshot.latest.slow,
                                            snapshot.latest.difference, snapshot.latest.logRatioDb,
                                            snapshot.latest.reductionDb};
        const std::array<double, 5> peak{snapshot.peak.fast, snapshot.peak.slow,
                                         snapshot.peak.difference, snapshot.peak.logRatioDb,
                                         snapshot.peak.reductionDb};
        for (std::size_t i = 0; i < readouts_.size(); ++i)
            readouts_[i].setText(juce::String(names[i]) + "\nlast " + juce::String(current[i], 6) +
                                     "\npeak " + juce::String(peak[i], 6),
                                 juce::dontSendNotification);
        observation_.setText("UI interval: " + juce::String(static_cast<int>(snapshot.blocks)) +
                                 " blocks | dropped since Play: " +
                                 juce::String(static_cast<juce::int64>(snapshot.droppedBlocks)) +
                                 " | peaks are not co-timed samples",
                             juce::dontSendNotification);
    }
    void refresh() {
        const auto& settings = session_.draft().engineering.protect;
        enabled_.setToggleState(settings.depth > 0, juce::dontSendNotification);
        detector_.setSelectedId(static_cast<int>(settings.gain.score) + 1,
                                juce::dontSendNotification);
        const bool modal = session_.draft().engineering.mode == 1;
        topology_.setItemEnabled(2, !modal);
        topology_.setItemEnabled(3, !modal);
        topology_.setSelectedId(static_cast<int>(settings.topology), juce::dontSendNotification);
        topology_.setTooltip(
            modal ? "APPLY: Resonant C supports Whole only; Fluid topology is retained separately."
                  : "APPLY: F2/F3 can reduce cancellation and increase summed residual energy.");
        for (std::size_t i = 0; i < controls_.size(); ++i) {
            const auto& spec = kProtectControls[i];
            const bool threshold = spec.unit == ProtectUnit::detectorThreshold;
            const auto suffix =
                threshold
                    ? (settings.gain.score == research::ProtectScore::difference ? " (amplitude)"
                                                                                 : " (dB ratio)")
                    : "";
            const double baseline =
                threshold && settings.gain.score == research::ProtectScore::difference
                    ? (spec.id == ProtectId::low ? .010 : .120)
                    : spec.initial;
            controls_[i].configure(
                juce::String(spec.label) + suffix +
                    (spec.lifecycle == ControlLifecycle::live ? " / LIVE" : " / APPLY"),
                spec.minimum, protectMaximum(spec, settings.gain.score), baseline,
                spec.unit == ProtectUnit::seconds);
            controls_[i].setHelp(
                juce::String("protect.") + spec.key +
                " | PROTECT-EXP-001 research baseline | units retain existing DSP semantics");
            controls_[i].refreshValue(protectValue(settings, spec.id));
            controls_[i].setVisible(spec.visibility == ControlVisibility::primary ||
                                    advanced_.getToggleState());
        }
        note_.setText("Depth 0 = OFF after finite transition. GR is residual attenuation, not "
                      "output level reduction. D0/D1 are research candidates.",
                      juce::dontSendNotification);
        resized();
    }
    void resized() override {
        auto area = getLocalBounds().reduced(8);
        title_.setBounds(area.removeFromTop(25));
        auto header = area.removeFromTop(32);
        enabled_.setBounds(header.removeFromLeft(100));
        detector_.setBounds(header.removeFromLeft(200).reduced(2));
        topology_.setBounds(header.removeFromLeft(235).reduced(2));
        advanced_.setBounds(header.removeFromLeft(160));
        auto primary = area.removeFromTop(76);
        const int width = primary.getWidth() / 4;
        for (std::size_t i = 0; i < 4; ++i)
            controls_[i].setBounds(primary.removeFromLeft(width).reduced(4));
        if (advanced_.getToggleState()) {
            for (std::size_t row = 0; row < 2; ++row) {
                auto line = area.removeFromTop(78);
                for (std::size_t column = 0; column < 4; ++column) {
                    const auto index = 4 + row * 4 + column;
                    if (index < controls_.size())
                        controls_[index].setBounds(line.removeFromLeft(width).reduced(4));
                }
            }
        }
        auto readings = area.removeFromTop(70);
        const int readingWidth = readings.getWidth() / 5;
        for (auto& readout : readouts_)
            readout.setBounds(readings.removeFromLeft(readingWidth));
        observation_.setBounds(area.removeFromTop(28));
        note_.setBounds(area.removeFromTop(35));
    }

  private:
    ResearchSessionModel& session_;
    std::function<void()> beforePrepareEdit_;
    juce::Label title_, note_;
    std::array<juce::Label, 5> readouts_;
    juce::Label observation_;
    juce::ToggleButton enabled_{"Enable"}, advanced_{"Advanced"};
    juce::ComboBox detector_, topology_;
    std::array<ExactValueControl, kProtectControls.size()> controls_;
};
} // namespace frazil::water::preview
