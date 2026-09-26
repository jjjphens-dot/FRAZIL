#pragma once

#include "ExactValueControl.h"
#include "ResearchOperationHistory.h"
#include "ResearchPresentation.h"
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
                   ResearchOperations& operations)
        : session_(session), origin_(origin), operations_(operations) {
        core_.addItem("Core: Legacy A0/B0/D0", 1);
        core_.addItem("Core: Reworked A1/B1/D1", 2);
        core_.setTitle("Research core revision");
        core_.setTooltip("Runtime A/B only. Session v5 cannot save this revision. C is unchanged.");
        addAndMakeVisible(core_);
        core_.onChange = [this] {
            const auto core = core_.getSelectedId() == 2 ? WaterResearchCore::reworked
                                                         : WaterResearchCore::legacy;
            operations_.action("Core Revision", origin_, true, true,
                               [this, core] { session_.setCore(core, origin_); });
        };
        model_.addItem("Fluid", 1);
        model_.addItem("Resonant", 2);
        addAndMakeVisible(model_);
        model_.onChange = [this] {
            const auto value =
                model_.getSelectedId() == 1 ? WaterModel::fluid : WaterModel::resonant;
            operations_.action("Model", origin_, true, true,
                               [this, value] { session_.setModel(value, origin_); });
        };
        for (std::size_t i = 0; i < knobs_.size(); ++i) {
            auto& knob = knobs_[i];
            knob.setRange(0, 1, 0);
            knob.normalStep = .001;
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 22);
            knob.setDoubleClickReturnValue(true, .5);
            knob.setTooltip("Research mapping v0.2; not product frozen. Legacy sessions require "
                            "explicit adoption.");
            knob.onValueChange = [this, i] {
                const auto value = knobs_[i].getValue();
                const auto& water = session_.draft().water;
                const double previous = i == 0 ? water.size : i == 1 ? water.motion : water.decay;
                if (value == previous)
                    return;
                operations_.edit(macroKey(i), origin_, true, true);
                session_.setMacro(i == 0   ? MacroId::size
                                  : i == 1 ? MacroId::motion
                                           : MacroId::decay,
                                  value, origin_);
            };
            knob.onMouseBegin = [this, i] {
                operations_.edit(macroKey(i), origin_, true, true, true);
            };
            knob.onMouseEnd = [this] { operations_.finish(); };
            addAndMakeVisible(knob);
            researchLabel(*this, names_[i], "");
            addAndMakeVisible(targets_[i]);
            targets_[i].setJustificationType(juce::Justification::topLeft);
            targets_[i].setMinimumHorizontalScale(1.0f);
            targets_[i].setFont(juce::Font(juce::FontOptions(14)));
            targets_[i].setColour(juce::Label::textColourId, juce::Colour(0xffbed4dc));
            targets_[i].setTitle(juce::String(macroKey(i)) + " engineering targets");
            addAndMakeVisible(returnMacro_[i]);
            returnMacro_[i].setButtonText(juce::String("Return ") + macroKey(i) + " to Mapped");
            returnMacro_[i].onClick = [this, i] {
                operations_.action(
                    (std::string("Return ") + macroKey(i) + " to Mapped").c_str(), origin_, true,
                    true, [this, i] { session_.returnMacroToMapped(static_cast<MacroId>(i)); });
            };
        }
        researchLabel(*this, mapping_, "");
        researchLabel(*this, notice_,
                      "RESEARCH MAPPING v0.2 | NOT PRODUCT FROZEN | Protect independent");
        addAndMakeVisible(returnMapped_);
        returnMapped_.onClick = [this] {
            operations_.action("Return Model to Mapped", origin_, true, true,
                               [this] { session_.returnModelToMapped(); });
        };
        addAndMakeVisible(returnAll_);
        returnAll_.onClick = [this] {
            operations_.action("Return All to Mapped / Adopt", origin_, true, true,
                               [this] { session_.returnAllToMapped(); });
        };
        refresh();
    }
    void refresh() {
        const auto& state = session_.draft();
        const bool reworked = state.engineering.reworkedFluid();
        core_.setSelectedId(state.engineering.core == WaterResearchCore::reworked ? 2 : 1,
                            juce::dontSendNotification);
        notice_.setText(reworked ? "Typed defaults | Protect coupling deferred for A1/B1/D1."
                                 : "RESEARCH MAPPING v0.2 | C unchanged by core revision",
                        juce::dontSendNotification);
        returnAll_.setEnabled(!reworked);
        for (std::size_t i = 0; i < knobs_.size(); ++i) {
            knobs_[i].setEnabled(!reworked);
            returnMacro_[i].setEnabled(!reworked);
        }
        model_.setSelectedId(state.water.model == WaterModel::fluid ? 1 : 2,
                             juce::dontSendNotification);
        knobs_[0].setValue(state.water.size, juce::dontSendNotification);
        knobs_[1].setValue(state.water.motion, juce::dontSendNotification);
        knobs_[2].setValue(state.water.decay, juce::dontSendNotification);
        mapping_.setText(state.modelMapping == MappingStatus::mapped ? "Model: MAPPED"
                                                                     : "Model: CUSTOM composition",
                         juce::dontSendNotification);
        returnMapped_.setEnabled(state.modelMapping == MappingStatus::custom);
        for (std::size_t i = 0; i < names_.size(); ++i)
            names_[i].setText(juce::String(macroKey(i)) +
                                  (reworked ? " / NOT MAPPED"
                                   : state.macroMappings[i] == MappingStatus::mapped
                                       ? " / RESEARCH_MAPPED"
                                       : " / CUSTOM"),
                              juce::dontSendNotification);
        for (std::size_t i = 0; i < targets_.size(); ++i)
            targets_[i].setText(reworked ? "NOT MAPPED TO REWORKED CORE"
                                         : macroTargetsText(state, static_cast<MacroId>(i)),
                                juce::dontSendNotification);
        returnAll_.setButtonText(state.mappingRevision != ResearchWaterMacroMapper::revision.data()
                                     ? "Adopt Research Mapping v0.2"
                                     : "Return All to Mapped");
    }
    void resized() override {
        auto area = getLocalBounds().reduced(8);
        notice_.setBounds(area.removeFromBottom(28));
        auto left = area.removeFromLeft(250);
        core_.setBounds(left.removeFromTop(32));
        model_.setBounds(left.removeFromTop(32));
        mapping_.setBounds(left.removeFromTop(32));
        returnMapped_.setBounds(left.removeFromTop(30));
        returnAll_.setBounds(left.removeFromTop(32));
        for (std::size_t i = 0; i < knobs_.size(); ++i) {
            const int width = area.getWidth() / static_cast<int>(knobs_.size() - i);
            auto cell = area.removeFromLeft(width).reduced(12, 0);
            names_[i].setBounds(cell.removeFromTop(36));
            returnMacro_[i].setBounds(cell.removeFromBottom(30));
            targets_[i].setBounds(cell.removeFromBottom(66));
            knobs_[i].setBounds(cell);
        }
    }

  private:
    static const char* macroKey(std::size_t i) {
        return i == 0 ? "Size" : i == 1 ? "Motion" : "Decay";
    }
    ResearchSessionModel& session_; // Borrowed message-thread owner outlives the view.
    ChangeOrigin origin_;
    ResearchOperations& operations_;
    juce::ComboBox model_, core_;
    std::array<ResearchSlider, 3> knobs_;
    std::array<juce::Label, 3> names_;
    std::array<juce::Label, 3> targets_;
    std::array<juce::TextButton, 3> returnMacro_;
    juce::Label mapping_, notice_;
    juce::TextButton returnMapped_{"Return Model to Mapped"};
    juce::TextButton returnAll_{"Return All to Mapped"};
};

class EngineeringView final : public juce::Component {
  public:
    std::function<void()> onLayoutChange;
    EngineeringView(ResearchSessionModel& session, ResearchOperations& operations)
        : session_(session), operations_(operations),
          macros_(session, ChangeOrigin::engineeringUI, operations) {
        addAndMakeVisible(macros_);
        for (std::size_t i = 0; i < kModes.size(); ++i)
            composition_.addItem(kModes[i], static_cast<int>(i) + 1);
        composition_.setTitle("Engineering composition");
        addAndMakeVisible(composition_);
        composition_.onChange = [this] {
            const auto value = composition_.getSelectedId() - 1;
            operations_.action(
                "Composition", ChangeOrigin::engineeringUI, true, false,
                [this, value] { session_.setComposition(value, ChangeOrigin::engineeringUI); });
        };
        researchLabel(*this, provenance_,
                      "All raw controls APPLY | SPIKE-W-DSP-001 research baseline | inactive "
                      "values retained");
        addAndMakeVisible(calibration_);
        calibration_.onClick = [this] {
            operations_.action("Restore Listening Calibration", ChangeOrigin::reset, true, false,
                               [this] { session_.restoreListeningCalibration(); });
        };
        for (std::size_t i = 0; i < kControls.size(); ++i) {
            const auto& spec = kControls[i];
            controls_[i].configure(juce::String(spec.label).replace("(s)", "(ms / s)"),
                                   spec.minimum, spec.maximum, spec.initial,
                                   spec.displayPolicy == DisplayPolicy::adaptiveTime,
                                   spec.valueType == ControlValueType::integer, spec.step);
            controls_[i].onEdit = [this, i](double value) {
                operations_.edit(kControls[i].stableId(), ChangeOrigin::engineeringUI, true);
                return session_.setEngineering(kControls[i].id, value, ChangeOrigin::engineeringUI);
            };
            controls_[i].onGestureBegin = [this, i] {
                operations_.edit(kControls[i].stableId(), ChangeOrigin::engineeringUI, true, false,
                                 true);
            };
            controls_[i].onGestureEnd = [this] { operations_.finish(); };
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
        int height = 394;
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
        const bool reworked = state.engineering.core == WaterResearchCore::reworked;
        calibration_.setEnabled(!reworked || state.engineering.mode == 1);
        composition_.setSelectedId(state.engineering.mode + 1, juce::dontSendNotification);
        provenance_.setText(
            state.engineering.reworkedFluid()
                ? "A1/B1/D1 typed defaults | Legacy listening calibration retained / INACTIVE"
                : juce::String(ResearchListeningCalibration::revision) +
                      (state.listeningCalibration == MappingStatus::mapped ? " / MAPPED"
                                                                           : " / CUSTOM") +
                      " | A .26 / B .24 / D .06 / C .30 | independent of macros",
            juce::dontSendNotification);
        for (std::size_t i = 0; i < kControls.size(); ++i) {
            const auto& spec = kControls[i];
            const auto group = static_cast<std::size_t>(spec.group);
            const bool legacyOnly = reworked && spec.group != ControlGroup::modal;
            const bool active = !legacyOnly && moduleActive(spec.group, state.engineering.mode);
            controls_[i].setEnabled(!legacyOnly);
            controls_[i].refreshValue(state.engineering.values[i]);
            controls_[i].setAlpha(active ? 1.0f : .55f);
            controls_[i].setVisible(expanded_[group]);
            const auto baseline =
                spec.displayPolicy == DisplayPolicy::adaptiveTime
                    ? juce::String(formatTimeValue(spec.initial).value_or("invalid"))
                    : juce::String(spec.initial);
            const auto owner = macroOwner(spec.id);
            const auto ownerText =
                owner ? (*owner == MacroId::size     ? "Size"
                         : *owner == MacroId::motion ? "Motion"
                                                     : "Decay")
                      : (ResearchListeningCalibration::owns(spec.id) ? "Listening Calibration"
                                                                     : "Engineering only");
            controls_[i].setHelp(juce::String("Owner: ") + ownerText + " | " +
                                 juce::String(spec.stableId()) + " | APPLY | baseline " + baseline +
                                 " | SPIKE-W-DSP-001 | " +
                                 (legacyOnly ? "LEGACY CORE ONLY"
                                  : active   ? "ACTIVE"
                                             : "INACTIVE / retained") +
                                 " | origin: " + originName(state.ownership[i].origin));
        }
        constexpr std::array<const char*, 4> names{"A / BUBBLE", "B / DROPLET", "D / FLOW",
                                                   "C / MODAL"};
        for (std::size_t i = 0; i < headings_.size(); ++i)
            headings_[i].setButtonText(
                juce::String(expanded_[i] ? "[-] " : "[+] ") + names[i] +
                (reworked && i != static_cast<std::size_t>(ControlGroup::modal)
                     ? " / LEGACY CORE ONLY"
                 : moduleActive(static_cast<ControlGroup>(i), state.engineering.mode)
                     ? " / ACTIVE"
                     : " / INACTIVE - retained") +
                " / APPLY");
        resized();
    }
    void resized() override {
        auto area = getLocalBounds().reduced(8);
        macros_.setBounds(area.removeFromTop(320));
        auto commands = area.removeFromTop(30);
        composition_.setBounds(commands.removeFromLeft(240));
        calibration_.setBounds(commands.removeFromLeft(245).reduced(3, 0));
        provenance_.setBounds(area.removeFromTop(28));
        for (std::size_t group = 0; group < headings_.size(); ++group) {
            headings_[group].setBounds(area.removeFromTop(36).reduced(2));
            if (!expanded_[group])
                continue;
            juce::Rectangle<int> line;
            const int width = area.getWidth() / 2;
            int column = 0;
            // Descriptor order is append-only for session compatibility, not grouped in memory.
            for (std::size_t index = 0; index < kControls.size(); ++index) {
                if (static_cast<std::size_t>(kControls[index].group) != group)
                    continue;
                if (column++ % 2 == 0)
                    line = area.removeFromTop(72);
                controls_[index].setBounds(line.removeFromLeft(width).reduced(8, 2));
            }
        }
    }

  private:
    static int rows(std::size_t group) noexcept {
        int count = 0;
        for (const auto& control : kControls)
            if (static_cast<std::size_t>(control.group) == group)
                ++count;
        return (count + 1) / 2;
    }
    ResearchSessionModel& session_;
    ResearchOperations& operations_;
    WaterMacroView macros_;
    juce::ComboBox composition_;
    juce::TextButton calibration_{"Restore Listening Calibration"};
    juce::Label provenance_;
    std::array<juce::TextButton, 4> headings_;
    std::array<bool, 4> expanded_{true, false, false, false};
    std::array<ExactValueControl, kControls.size()> controls_;
};
} // namespace frazil::water::preview
