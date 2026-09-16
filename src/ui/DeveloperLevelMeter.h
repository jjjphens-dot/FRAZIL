#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace frazil::ui {

// Message-thread, value-only display of aggregate latest-block linear amplitudes.
// RMS is the fill and Peak the marker; neither is a hold or a true-peak measurement.
class DeveloperLevelMeter final : public juce::Component {
  public:
    explicit DeveloperLevelMeter(const juce::String& name);
    void setLevels(float peakAmplitude, float rmsAmplitude);
    void paint(juce::Graphics&) override;

    // Shared by the bar and its dBFS ruler. Only geometry clamps to [-60, 0] dBFS.
    static constexpr float kDisplayFloorDb = -60.0f;
    static float positionForDecibels(float decibels) noexcept;

  private:
    juce::String name_;
    // Raw values from the most recent UI observation, with no smoothing/history.
    float peakAmplitude_{};
    float rmsAmplitude_{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeveloperLevelMeter)
};

} // namespace frazil::ui
