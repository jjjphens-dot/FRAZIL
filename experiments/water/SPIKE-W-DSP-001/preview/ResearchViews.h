#pragma once

#include "ResearchSessionModel.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace frazil::water::preview {

inline void researchLabel(juce::Component& owner, juce::Label& label, const juce::String& text) {
    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colour(0xffbed4dc));
    owner.addAndMakeVisible(label);
}

// Reused representations of the same macro state. Refresh is notification-free: only user
// commands write to the session, preventing observer feedback and redundant revisions.
class WaterMacroView final : public juce::Component {
  public:
    WaterMacroView(ResearchSessionModel& session, ChangeOrigin origin,
                   std::function<void()> beforeEdit)
        : session_(session), origin_(origin), beforeEdit_(std::move(beforeEdit)) {
        model_.addItem("Fluid", 1);
        model_.addItem("Resonant", 2);
        addAndMakeVisible(model_);
        model_.onChange = [this] {
            beforeEdit_();
            session_.setModel(
                model_.getSelectedId() == 1 ? WaterModel::fluid : WaterModel::resonant, origin_);
        };
        for (std::size_t i = 0; i < knobs_.size(); ++i) {
            auto& knob = knobs_[i];
            knob.setRange(0, 1, .001);
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 22);
            knob.setDoubleClickReturnValue(true, .5);
            knob.setTooltip("Experiment baseline 0.5; UNMAPPED: does not change DSP targets.");
            knob.onValueChange = [this, i] {
                beforeEdit_();
                session_.setMacro(i == 0 ? MacroId::size : MacroId::motion, knobs_[i].getValue(),
                                  origin_);
            };
            addAndMakeVisible(knob);
            researchLabel(*this, names_[i], i == 0 ? "Size / UNMAPPED" : "Motion / UNMAPPED");
        }
        researchLabel(*this, mapping_, "");
        researchLabel(*this, notice_,
                      "Experiment state only. No Size / Motion DSP mapping has been accepted.");
        addAndMakeVisible(returnMapped_);
        returnMapped_.onClick = [this] {
            beforeEdit_();
            session_.returnModelToMapped();
        };
        refresh();
    }
    void refresh() {
        const auto& state = session_.draft();
        model_.setSelectedId(state.water.model == WaterModel::fluid ? 1 : 2,
                             juce::dontSendNotification);
        knobs_[0].setValue(state.water.size, juce::dontSendNotification);
        knobs_[1].setValue(state.water.motion, juce::dontSendNotification);
        mapping_.setText(state.modelMapping == MappingStatus::mapped ? "Model: MAPPED"
                                                                     : "Model: CUSTOM composition",
                         juce::dontSendNotification);
        returnMapped_.setEnabled(state.modelMapping == MappingStatus::custom);
    }
    void resized() override {
        auto area = getLocalBounds().reduced(8);
        notice_.setBounds(area.removeFromBottom(28));
        auto left = area.removeFromLeft(250);
        model_.setBounds(left.removeFromTop(32));
        mapping_.setBounds(left.removeFromTop(32));
        returnMapped_.setBounds(left.removeFromTop(30));
        for (std::size_t i = 0; i < knobs_.size(); ++i) {
            auto cell = area.removeFromLeft(180).reduced(8, 0);
            names_[i].setBounds(cell.removeFromTop(24));
            knobs_[i].setBounds(cell);
        }
    }

  private:
    ResearchSessionModel& session_; // Borrowed message-thread owner outlives the view.
    ChangeOrigin origin_;
    std::function<void()> beforeEdit_;
    juce::ComboBox model_;
    std::array<juce::Slider, 2> knobs_;
    std::array<juce::Label, 2> names_;
    juce::Label mapping_, notice_;
    juce::TextButton returnMapped_{"Return Model to Mapped"};
};

class EngineeringView final : public juce::Component {
  public:
    EngineeringView(ResearchSessionModel& session, std::function<void()> beforeEdit)
        : session_(session), beforeEdit_(std::move(beforeEdit)),
          macros_(session, ChangeOrigin::engineeringUI, beforeEdit_) {
        addAndMakeVisible(macros_);
        for (std::size_t i = 0; i < kModes.size(); ++i)
            composition_.addItem(kModes[i], static_cast<int>(i) + 1);
        addAndMakeVisible(composition_);
        composition_.onChange = [this] {
            beforeEdit_();
            session_.setComposition(composition_.getSelectedId() - 1, ChangeOrigin::engineeringUI);
        };
        researchLabel(*this, provenance_,
                      "Engineering baseline: SPIKE-W-DSP-001 | all controls APPLY | inactive "
                      "values retained");
        for (std::size_t i = 0; i < kControls.size(); ++i) {
            const auto& spec = kControls[i];
            researchLabel(*this, labels_[i], spec.label);
            sliders_[i].setRange(spec.minimum, spec.maximum, spec.step);
            sliders_[i].setSliderStyle(juce::Slider::LinearHorizontal);
            sliders_[i].setTextBoxStyle(juce::Slider::TextBoxRight, false, 84, 24);
            sliders_[i].setDoubleClickReturnValue(true, spec.initial);
            sliders_[i].onValueChange = [this, i] {
                beforeEdit_();
                session_.setEngineering(kControls[i].id, sliders_[i].getValue(),
                                        ChangeOrigin::engineeringUI);
            };
            addAndMakeVisible(sliders_[i]);
        }
        for (auto& heading : headings_)
            researchLabel(*this, heading, "");
        refresh();
    }
    void refresh() {
        macros_.refresh();
        const auto& state = session_.draft();
        composition_.setSelectedId(state.engineering.mode + 1, juce::dontSendNotification);
        for (std::size_t i = 0; i < kControls.size(); ++i) {
            const auto& spec = kControls[i];
            const bool active = moduleActive(spec.group, state.engineering.mode);
            sliders_[i].setValue(state.engineering.values[i], juce::dontSendNotification);
            sliders_[i].setAlpha(active ? 1.0f : .5f);
            labels_[i].setAlpha(active ? 1.0f : .5f);
            sliders_[i].setTooltip(
                juce::String(spec.stableId()) + " | APPLY | baseline " +
                juce::String(spec.initial) + " | SPIKE-W-DSP-001 | " +
                (active ? "ACTIVE" : "INACTIVE / retained; only affects enabled compositions") +
                " | origin: " + originName(state.ownership[i].origin));
        }
        for (std::size_t i = 0; i < headings_.size(); ++i) {
            const auto group = static_cast<ControlGroup>(i);
            headings_[i].setText(
                juce::String(moduleName(group)).toUpperCase() +
                    (moduleActive(group, state.engineering.mode) ? " / ACTIVE" : " / INACTIVE"),
                juce::dontSendNotification);
        }
    }
    void resized() override {
        auto area = getLocalBounds().reduced(8);
        macros_.setBounds(area.removeFromTop(200));
        composition_.setBounds(area.removeFromTop(30).removeFromLeft(240));
        provenance_.setBounds(area.removeFromTop(28));
        const int width = area.getWidth() / 4;
        constexpr std::array<std::size_t, 5> offsets{0, 7, 14, 18, 21};
        for (std::size_t group = 0; group < 4; ++group) {
            auto column = area.removeFromLeft(width).reduced(5, 0);
            headings_[group].setBounds(column.removeFromTop(26));
            for (auto i = offsets[group]; i < offsets[group + 1]; ++i) {
                labels_[i].setBounds(column.removeFromTop(21));
                sliders_[i].setBounds(column.removeFromTop(26));
            }
        }
    }

  private:
    ResearchSessionModel& session_;
    std::function<void()> beforeEdit_;
    WaterMacroView macros_;
    juce::ComboBox composition_;
    juce::Label provenance_;
    std::array<juce::Label, 4> headings_;
    std::array<juce::Label, kControls.size()> labels_;
    std::array<juce::Slider, kControls.size()> sliders_;
};

} // namespace frazil::water::preview
