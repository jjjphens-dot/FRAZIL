#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace frazil::water::preview {
// Physical mouse boundaries only. JUCE drag notifications also wrap wheel/key events, so those
// notifications cannot be used for the one-physical-gesture history contract.
class ResearchSlider final : public juce::Slider {
  public:
    std::function<void()> onMouseBegin, onMouseEnd;
    double normalStep{};
    double snapValue(double value, DragMode) override {
        return normalStep > 0 && !fine_
                   ? juce::jlimit(getMinimum(), getMaximum(),
                                  getMinimum() +
                                      std::round((value - getMinimum()) / normalStep) * normalStep)
                   : value;
    }
    void mouseDown(const juce::MouseEvent& event) override {
        if (onMouseBegin)
            onMouseBegin();
        fine_ = event.mods.isShiftDown() && event.mods.isLeftButtonDown();
        start_ = valueToProportionOfLength(getValue());
        if (!fine_)
            juce::Slider::mouseDown(event);
        else if (isShowing())
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
        fine_ = false;
        if (onMouseEnd)
            onMouseEnd();
    }
    void mouseDoubleClick(const juce::MouseEvent& event) override {
        if (onMouseBegin)
            onMouseBegin();
        juce::Slider::mouseDoubleClick(event);
        if (onMouseEnd)
            onMouseEnd();
    }

  private:
    double start_{}; // Normalized mouse-down position; never an audio state.
    bool fine_{};
};
} // namespace frazil::water::preview
