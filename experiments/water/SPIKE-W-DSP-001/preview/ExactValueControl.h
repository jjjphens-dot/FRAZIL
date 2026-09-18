#pragma once

#include "TimeValue.h"

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

namespace frazil::water::preview {

// Reusable UI-only slider + exact entry. Parsing happens before any slider snapping/clamping,
// so invalid text can never silently replace a session value. No state ownership beyond display.
class ExactValueControl final : public juce::Component {
  public:
    std::function<bool(double)> onEdit;
    ExactValueControl() {
        for (auto* child : std::array<juce::Component*, 4>{&label_, &slider_, &entry_, &error_})
            addAndMakeVisible(child);
        label_.setColour(juce::Label::textColourId, juce::Colour(0xffbed4dc));
        error_.setColour(juce::Label::textColourId, juce::Colour(0xffffa38d));
        slider_.setSliderStyle(juce::Slider::LinearHorizontal);
        slider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider_.setMouseDragSensitivity(350);
        slider_.setSliderSnapsToMousePosition(false);
        slider_.onValueChange = [this] {
            if (!syncing_)
                submit(slider_.getValue());
        };
        entry_.setSelectAllWhenFocused(true);
        entry_.setFont(juce::FontOptions(13));
        entry_.onReturnKey = [this] { commitText(); };
        entry_.onFocusLost = [this] {
            if (textEdited_)
                commitText();
        };
        entry_.onTextChange = [this] {
            if (!syncing_)
                textEdited_ = true;
        };
        entry_.onEscapeKey = [this] {
            textEdited_ = false;
            error_.setText({}, juce::dontSendNotification);
            refreshValue(value_);
        };
    }
    void configure(const juce::String& label, double low, double high, double baseline, bool time,
                   bool integer = false) {
        const juce::ScopedValueSetter<bool> guard(syncing_, true);
        low_ = low;
        high_ = high;
        time_ = time;
        integer_ = integer;
        label_.setText(label, juce::dontSendNotification);
        entry_.setTitle(label);
        slider_.setTitle(label + " slider");
        slider_.setRange(low, high, integer ? 1.0 : 0.0);
        slider_.setDoubleClickReturnValue(true, baseline);
        if (time && low > 0 && high / low > 20)
            slider_.setSkewFactorFromMidPoint(std::sqrt(low * high));
        else if (time && low == 0 && high > 0)
            slider_.setSkewFactorFromMidPoint(high * .1);
        else
            slider_.setSkewFactor(1);
        entry_.setTooltip(
            time ? "Exact ms/s input; no unit means ms. Enter applies, Escape restores."
                 : "Exact numeric input. Enter applies, Escape restores; out-of-range values "
                   "rejected.");
    }
    void setHelp(const juce::String& help) {
        const auto full = help + " | Shift before drag: fine adjustment; double-click: baseline.";
        slider_.setTooltip(full);
        label_.setTooltip(full);
    }
    void refreshValue(double value) {
        const juce::ScopedValueSetter<bool> guard(syncing_, true);
        if (value != value_) {
            textEdited_ = false;
            error_.setText({}, juce::dontSendNotification);
        }
        value_ = value;
        slider_.setValue(value, juce::dontSendNotification);
        if (!textEdited_)
            entry_.setText(display(value), false);
    }
    void discardPendingText() {
        textEdited_ = false;
        error_.setText({}, juce::dontSendNotification);
        refreshValue(value_);
    }
    void resized() override {
        auto area = getLocalBounds();
        label_.setBounds(area.removeFromTop(22));
        error_.setBounds(area.removeFromBottom(18));
        entry_.setBounds(area.removeFromRight(190).reduced(2));
        slider_.setBounds(area);
    }

  private:
    // Keep fine gestures in normalized slider space so skewed time ranges remain usable.
    class FineSlider final : public juce::Slider {
      public:
        void mouseDown(const juce::MouseEvent& event) override {
            fine_ = event.mods.isShiftDown() && event.mods.isLeftButtonDown();
            start_ = valueToProportionOfLength(getValue());
            if (!fine_)
                juce::Slider::mouseDown(event);
            else
                grabKeyboardFocus();
        }
        void mouseDrag(const juce::MouseEvent& event) override {
            if (fine_)
                setValue(proportionOfLengthToValue(juce::jlimit(
                             0.0, 1.0, start_ + event.getDistanceFromDragStartX() / 3500.0)),
                         juce::sendNotificationSync);
            else
                juce::Slider::mouseDrag(event);
        }
        void mouseUp(const juce::MouseEvent& event) override {
            if (!fine_)
                juce::Slider::mouseUp(event);
        }

      private:
        double start_{};
        bool fine_{};
    };
    juce::String display(double value) const {
        if (time_)
            return juce::String(formatTimeValue(value).value_or("invalid"));
        std::array<char, 64> buffer{};
        const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
        return result.ec == std::errc{}
                   ? juce::String::fromUTF8(buffer.data(),
                                            static_cast<int>(result.ptr - buffer.data()))
                   : "invalid";
    }
    void commitText() {
        double candidate = value_;
        bool valid{};
        const auto text = entry_.getText().trim().toStdString();
        if (time_)
            valid = parseTimeValue(text, low_, high_, candidate) == TimeValueError::none;
        else if (!text.empty()) {
            const auto result = std::from_chars(text.data(), text.data() + text.size(), candidate);
            valid = result.ec == std::errc{} && result.ptr == text.data() + text.size() &&
                    std::isfinite(candidate) && candidate >= low_ && candidate <= high_;
        }
        valid = valid && (!integer_ || candidate == std::floor(candidate));
        if (!valid) {
            error_.setText(time_ ? "Invalid time or range" : "Invalid value or range",
                           juce::dontSendNotification);
            return;
        }
        submit(candidate);
    }
    void submit(double candidate) {
        if (onEdit && !onEdit(candidate)) {
            error_.setText("Value rejected", juce::dontSendNotification);
            return;
        }
        textEdited_ = false;
        error_.setText({}, juce::dontSendNotification);
        refreshValue(candidate);
    }
    juce::Label label_, error_;
    FineSlider slider_;
    juce::TextEditor entry_;
    double low_{}, high_{1}, value_{};
    bool time_{}, integer_{}, syncing_{}, textEdited_{};
};
} // namespace frazil::water::preview
