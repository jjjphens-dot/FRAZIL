#pragma once

#include "ExactValueControl.h"
#include "ResearchOperationHistory.h"

#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace frazil::water::preview {
// Generic message-thread cards; no typed DSP fields or processing objects. The session and
// operation coordinator are borrowed and must outlive this view and its child callbacks.
class ResearchCoreTuningView final : public juce::Component {
  public:
    std::function<void()> onLayoutChange;
    ResearchCoreTuningView(ResearchSessionModel& session, ResearchOperations& operations)
        : session_(session), operations_(operations) {
        title_.setText("REWORKED CORE TUNING | RAW RESEARCH | APPLY ONLY",
                       juce::dontSendNotification);
        addAndMakeVisible(title_);
        addAndMakeVisible(resetAll_);
        addAndMakeVisible(revision_);
        revision_.addItem("B1 Fixed-Radius Baseline", 1);
        revision_.addItem("B2 Variable-Radius Candidate", 2);
        revision_.onChange = [this] {
            operations_.action("Droplet revision", ChangeOrigin::engineeringUI, true, false,
                               [this] {
                                   session_.setRemediation(
                                       revision_.getSelectedId() == 2,
                                       session_.draft().engineering.tuning.depthAmplitudeGamma);
                               });
        };
        addAndMakeVisible(gamma_);
        gamma_.configure("A1 v3 depth amplitude gamma (PRODUCT_MAPPING)", .5, 1, 1, false);
        gamma_.onEdit = [this](double value) {
            operations_.edit("A1 depthAmplitudeGamma", ChangeOrigin::engineeringUI, true);
            session_.setRemediation(session_.draft().engineering.tuning.useB2, value);
            return true;
        };
        gamma_.onGestureEnd = [this] { operations_.finish(); };
        resetAll_.onClick = [this] {
            operations_.action("Reset Reworked Defaults", ChangeOrigin::reset, true, false, [this] {
                session_.resetReworkedDefaults();
                discardPendingText();
            });
        };
        for (std::size_t m = 0; m < kResearchModules.size(); ++m) {
            const auto module = kResearchModules[m];
            auto& card = cards_[m];
            addAndMakeVisible(card.heading);
            addAndMakeVisible(card.advanced);
            addAndMakeVisible(card.reset);
            card.reset.setButtonText(juce::String("Reset ") + Adapter::identifier(module));
            card.reset.onClick = [this, module] {
                operations_.action((std::string("Reset ") + Adapter::identifier(module)).c_str(),
                                   ChangeOrigin::reset, true, false, [this, module] {
                                       session_.resetReworkedModule(module);
                                       discardPendingText();
                                   });
            };
            card.heading.onClick = [this, m] {
                cards_[m].expanded = !cards_[m].expanded;
                layoutChanged();
            };
            card.advanced.onClick = [this, m] {
                cards_[m].showAdvanced = !cards_[m].showAdvanced;
                layoutChanged();
            };
            for (std::size_t i = 0; i < Adapter::parameterCount(module); ++i) {
                auto row = std::make_unique<Row>();
                const auto spec = Adapter::parameter(module, i);
                const auto key = juce::String(Adapter::key(module, i));
                const auto label =
                    juce::String(spec.name.data()) + " (" + juce::String(spec.unit.data()) + ")";
                const auto help = key + " | " + juce::String(spec.classification.data()) +
                                  " | unit: " + juce::String(spec.unit.data()) + " | APPLY ONLY";
                if (spec.choices.empty()) {
                    row->number.configure(label, spec.minimum, spec.maximum, spec.initial, false);
                    row->number.setTitle(key);
                    row->number.setHelp(help);
                    row->number.onEdit = [this, module, i](double value) {
                        operations_.edit(Adapter::key(module, i), ChangeOrigin::engineeringUI,
                                         true);
                        return session_.setReworkedParameter(module, i, value,
                                                             ChangeOrigin::engineeringUI);
                    };
                    row->number.onGestureBegin = [this, module, i] {
                        operations_.edit(Adapter::key(module, i), ChangeOrigin::engineeringUI, true,
                                         false, true);
                    };
                    row->number.onGestureEnd = [this] { operations_.finish(); };
                    addAndMakeVisible(row->number);
                } else {
                    row->label.setText(label, juce::dontSendNotification);
                    row->label.setTooltip(help);
                    row->choice.setTitle(key);
                    row->choice.setTooltip(help);
                    for (std::size_t c = 0; c < spec.choices.size(); ++c)
                        row->choice.addItem(juce::String(spec.choices[c]), static_cast<int>(c) + 1);
                    row->choice.onChange = [this, module, i, control = &row->choice] {
                        const auto choices = Adapter::parameter(module, i).choices;
                        const int selected = control->getSelectedId() - 1;
                        if (selected < 0 || static_cast<std::size_t>(selected) >= choices.size())
                            return;
                        const auto value = choices[static_cast<std::size_t>(selected)];
                        operations_.action(Adapter::key(module, i).c_str(),
                                           ChangeOrigin::engineeringUI, true, false,
                                           [this, module, i, value] {
                                               session_.setReworkedParameter(
                                                   module, i, value, ChangeOrigin::engineeringUI);
                                           });
                    };
                    addAndMakeVisible(row->label);
                    addAndMakeVisible(row->choice);
                }
                card.rows.push_back(std::move(row));
            }
        }
        refresh();
    }
    int preferredHeight() const {
        if (!session_.draft().engineering.reworkedFluid())
            return 0;
        int height = 108;
        for (std::size_t m = 0; m < cards_.size(); ++m) {
            height += 36;
            if (!cards_[m].expanded)
                continue;
            const auto [primary, advanced] = counts(m);
            height += ((primary + 1) / 2) * 72;
            if (advanced)
                height += 30 + (cards_[m].showAdvanced ? ((advanced + 1) / 2) * 72 : 0);
        }
        return height;
    }
    void discardPendingText() {
        gamma_.discardPendingText();
        for (auto& card : cards_)
            for (auto& row : card.rows)
                row->number.discardPendingText();
    }
    void refresh() {
        const auto& settings = session_.draft().engineering;
        setVisible(settings.reworkedFluid());
        revision_.setSelectedId(settings.tuning.useB2 ? 2 : 1, juce::dontSendNotification);
        gamma_.refreshValue(settings.tuning.depthAmplitudeGamma);
        for (std::size_t m = 0; m < cards_.size(); ++m) {
            auto& card = cards_[m];
            const auto module = kResearchModules[m];
            const bool active =
                moduleActive(m == 3 ? ControlGroup::droplet : static_cast<ControlGroup>(m),
                             settings.mode) &&
                (m != 1 || !settings.tuning.useB2) && (m != 3 || settings.tuning.useB2);
            card.heading.setButtonText(juce::String(card.expanded ? "[-] " : "[+] ") +
                                       Adapter::displayName(module) +
                                       (active ? " / ACTIVE" : " / INACTIVE - retained"));
            card.advanced.setButtonText(card.showAdvanced ? "[-] Advanced" : "[+] Advanced");
            card.advanced.setVisible(card.expanded && counts(m).second > 0);
            for (std::size_t i = 0; i < card.rows.size(); ++i) {
                auto& row = *card.rows[i];
                const auto spec = Adapter::parameter(module, i);
                const double value = Adapter::getValue(settings.tuning, module, i);
                const bool visible =
                    card.expanded && (Adapter::primary(module, i) || card.showAdvanced);
                row.number.setVisible(visible && spec.choices.empty());
                row.label.setVisible(visible && !spec.choices.empty());
                row.choice.setVisible(visible && !spec.choices.empty());
                row.number.setAlpha(active ? 1.f : .55f);
                row.choice.setAlpha(active ? 1.f : .55f);
                row.number.refreshValue(value);
                for (std::size_t c = 0; c < spec.choices.size(); ++c)
                    if (value == spec.choices[c])
                        row.choice.setSelectedId(static_cast<int>(c) + 1,
                                                 juce::dontSendNotification);
            }
        }
        resized();
    }
    void resized() override {
        auto area = getLocalBounds();
        auto top = area.removeFromTop(36);
        resetAll_.setBounds(top.removeFromRight(220).reduced(2));
        title_.setBounds(top);
        auto remediation = area.removeFromTop(72);
        revision_.setBounds(remediation.removeFromLeft(area.getWidth() / 2).reduced(8, 16));
        gamma_.setBounds(remediation.reduced(8, 2));
        for (std::size_t m = 0; m < cards_.size(); ++m) {
            auto& card = cards_[m];
            auto heading = area.removeFromTop(36);
            card.reset.setBounds(heading.removeFromRight(140).reduced(2));
            card.heading.setBounds(heading.reduced(2));
            if (!card.expanded)
                continue;
            for (bool primary : {true, false}) {
                if (!primary) {
                    if (!counts(m).second)
                        continue;
                    card.advanced.setBounds(area.removeFromTop(30).reduced(4, 1));
                    if (!card.showAdvanced)
                        continue;
                }
                int column = 0;
                juce::Rectangle<int> line;
                const int width = area.getWidth() / 2;
                for (std::size_t i = 0; i < card.rows.size(); ++i) {
                    if (Adapter::primary(kResearchModules[m], i) != primary)
                        continue;
                    if (column++ % 2 == 0)
                        line = area.removeFromTop(72);
                    auto cell = line.removeFromLeft(width).reduced(8, 2);
                    auto& row = *card.rows[i];
                    row.number.setBounds(cell);
                    row.label.setBounds(cell.removeFromTop(24));
                    row.choice.setBounds(cell.removeFromTop(26));
                }
            }
        }
    }

  private:
    using Adapter = ResearchCoreParameterAdapter;
    struct Row {
        ExactValueControl number;
        juce::Label label;
        juce::ComboBox choice;
    };
    struct Card {
        juce::TextButton heading, advanced, reset;
        bool expanded{true}, showAdvanced{}; // Presentation only, never DSP/session state.
        std::vector<std::unique_ptr<Row>> rows;
    };
    std::pair<int, int> counts(std::size_t m) const {
        int primary{}, advanced{};
        for (std::size_t i = 0; i < cards_[m].rows.size(); ++i)
            (Adapter::primary(kResearchModules[m], i) ? primary : advanced)++;
        return {primary, advanced};
    }
    void layoutChanged() {
        refresh();
        if (onLayoutChange)
            onLayoutChange();
    }
    ResearchSessionModel& session_;
    ResearchOperations& operations_;
    std::array<Card, kResearchModules.size()> cards_;
    juce::Label title_;
    juce::ComboBox revision_;
    ExactValueControl gamma_;
    juce::TextButton resetAll_{"Reset Reworked Defaults"};
};
} // namespace frazil::water::preview
