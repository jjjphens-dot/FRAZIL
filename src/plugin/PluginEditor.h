#pragma once

#include "PluginProcessor.h"

class FRAZILAudioProcessorEditor final : public juce::AudioProcessorEditor {
  public:
    explicit FRAZILAudioProcessorEditor(FRAZILAudioProcessor&);
    ~FRAZILAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FRAZILAudioProcessorEditor)
};
