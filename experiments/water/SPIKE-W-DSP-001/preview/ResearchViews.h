#pragma once

#include "ExactValueControl.h"
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
                session_.setMacro(i == 0   ? MacroId::size
                                  : i == 1 ? MacroId::motion
                                           : MacroId::decay,
                                  knobs_[i].getValue(), origin_);
            };
            addAndMakeVisible(knob);
            researchLabel(*this, names_[i],
                          i == 0   ? "Size / UNMAPPED"
                          : i == 1 ? "Motion / UNMAPPED"
                                   : "Decay / UNMAPPED");
        }
        knobs_[2].setTooltip("DOC-W-DECAY-001 provisional experiment baseline 0.5. Response "
                             "persistence; UNMAPPED, not a product default.");
        researchLabel(*this, mapping_, "");
        researchLabel(
            *this, notice_,
            "Experiment state only. No Size / Motion / Decay DSP mapping has been accepted.");
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
        knobs_[2].setValue(state.water.decay, juce::dontSendNotification);
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
            const int width = area.getWidth() / static_cast<int>(knobs_.size() - i);
            auto cell = area.removeFromLeft(width).reduced(12, 0);
            names_[i].setBounds(cell.removeFromTop(24));
            knobs_[i].setBounds(cell);
        }
    }

  private:
    ResearchSessionModel& session_; // Borrowed message-thread owner outlives the view.
    ChangeOrigin origin_;
    std::function<void()> beforeEdit_;
    juce::ComboBox model_;
    std::array<juce::Slider, 3> knobs_;
    std::array<juce::Label, 3> names_;
    juce::Label mapping_, notice_;
    juce::TextButton returnMapped_{"Return Model to Mapped"};
};

class EngineeringView final : public juce::Component {
  public:
    std::function<void()> onLayoutChange;
    EngineeringView(ResearchSessionModel& session, std::function<void()> beforeEdit)
        : session_(session), beforeEdit_(std::move(beforeEdit)),
          macros_(session, ChangeOrigin::engineeringUI, beforeEdit_) {
        addAndMakeVisible(macros_);
        for (std::size_t i = 0; i < kModes.size(); ++i)
            composition_.addItem(kModes[i], static_cast<int>(i) + 1);
        composition_.setTitle("Engineering composition");
        addAndMakeVisible(composition_);
        composition_.onChange = [this] {
            beforeEdit_();
            session_.setComposition(composition_.getSelectedId() - 1, ChangeOrigin::engineeringUI);
        };
        researchLabel(*this, provenance_,
                      "All raw controls APPLY | SPIKE-W-DSP-001 research baseline | inactive "
                      "values retained");
        for (std::size_t i = 0; i < kControls.size(); ++i) {
            const auto& spec = kControls[i];
            controls_[i].configure(juce::String(spec.label).replace("(s)", "(ms / s)"),
                                   spec.minimum, spec.maximum, spec.initial,
                                   spec.displayPolicy == DisplayPolicy::adaptiveTime,
                                   spec.valueType == ControlValueType::integer, spec.step);
            controls_[i].onEdit = [this, i](double value) {
                beforeEdit_();
                return session_.setEngineering(kControls[i].id, value, ChangeOrigin::engineeringUI);
            };
            addAndMakeVisible(controls_[i]);
        }
        for (std::size_t i = 0; i < headings_.size(); ++i) {
            addAndMakeVisible(headings_[i]);
            headings_[i].onClick = [this, i] {
                expanded_[i] = !expanded_[i];
                refresh();
                if (onLayoutChange)
                    onLayoutChange();
            };
        }
        refresh();
    }
    int preferredHeight() const noexcept {
        int height = 274;
        for (std::size_t i = 0; i < headings_.size(); ++i)
            height += 36 + (expanded_[i] ? rows(i) * 72 : 0);
        return height;
    }
    void discardPendingText() {
        for (auto& control : controls_)
            control.discardPendingText();
    }
    void refresh() {
        macros_.refresh();
        const auto& state = session_.draft();
        composition_.setSelectedId(state.engineering.mode + 1, juce::dontSendNotification);
        provenance_.setText(
            juce::String(state.customEngineering ? "CUSTOM engineering" : "Research baseline") +
                " | APPLY | SPIKE-W-DSP-001 | inactive values retained",
            juce::dontSendNotification);
        for (std::size_t i = 0; i < kControls.size(); ++i) {
            const auto& spec = kControls[i];
            const auto group = static_cast<std::size_t>(spec.group);
            const bool active = moduleActive(spec.group, state.engineering.mode);
            controls_[i].refreshValue(state.engineering.values[i]);
            controls_[i].setAlpha(active ? 1.0f : .55f);
            controls_[i].setVisible(expanded_[group]);
            const auto baseline =
                spec.displayPolicy == DisplayPolicy::adaptiveTime
                    ? juce::String(formatTimeValue(spec.initial).value_or("invalid"))
                    : juce::String(spec.initial);
            controls_[i].setHelp(juce::String(spec.stableId()) + " | APPLY | baseline " + baseline +
                                 " | SPIKE-W-DSP-001 | " +
                                 (active ? "ACTIVE" : "INACTIVE / retained") +
                                 " | origin: " + originName(state.ownership[i].origin));
        }
        constexpr std::array<const char*, 4> names{"A / BUBBLE", "B / DROPLET", "D / FLOW",
                                                   "C / MODAL"};
        for (std::size_t i = 0; i < headings_.size(); ++i)
            headings_[i].setButtonText(
                juce::String(expanded_[i] ? "[-] " : "[+] ") + names[i] +
                (moduleActive(static_cast<ControlGroup>(i), state.engineering.mode)
                     ? " / ACTIVE"
                     : " / INACTIVE - retained") +
                " / APPLY");
        resized();
    }
    void resized() override {
        auto area = getLocalBounds().reduced(8);
        macros_.setBounds(area.removeFromTop(200));
        composition_.setBounds(area.removeFromTop(30).removeFromLeft(240));
        provenance_.setBounds(area.removeFromTop(28));
        for (std::size_t group = 0; group < headings_.size(); ++group) {
            headings_[group].setBounds(area.removeFromTop(36).reduced(2));
            if (!expanded_[group])
                continue;
            for (int row = 0; row < rows(group); ++row) {
                auto line = area.removeFromTop(72);
                const int width = line.getWidth() / 2;
                for (int column = 0; column < 2; ++column) {
                    const auto index = offsets_[group] + static_cast<std::size_t>(row * 2 + column);
                    if (index < offsets_[group + 1])
                        controls_[index].setBounds(line.removeFromLeft(width).reduced(8, 2));
                }
            }
        }
    }

  private:
    static constexpr std::array<std::size_t, 5> offsets_{0, 7, 14, 18, 21};
    static int rows(std::size_t group) noexcept {
        return static_cast<int>((offsets_[group + 1] - offsets_[group] + 1) / 2);
    }
    ResearchSessionModel& session_;
    std::function<void()> beforeEdit_;
    WaterMacroView macros_;
    juce::ComboBox composition_;
    juce::Label provenance_;
    std::array<juce::TextButton, 4> headings_;
    std::array<bool, 4> expanded_{true, false, false, false};
    std::array<ExactValueControl, kControls.size()> controls_;
};
} // namespace frazil::water::preview
