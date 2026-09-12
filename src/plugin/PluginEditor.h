#pragma once

#include "PluginProcessor.h"

#include <array>
#include <memory>

#ifndef FRAZIL_ENABLE_DEVELOPER_UI
#define FRAZIL_ENABLE_DEVELOPER_UI 0
#endif

class FRAZILAudioProcessorEditor final : public juce::AudioProcessorEditor
#if FRAZIL_ENABLE_DEVELOPER_UI
    ,
                                         private juce::Timer
#endif
{
  public:
    explicit FRAZILAudioProcessorEditor(FRAZILAudioProcessor&);
    ~FRAZILAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

  private:
#if FRAZIL_ENABLE_DEVELOPER_UI
    struct ABState final {
        std::array<float, 9> values{};
        bool captured{};
    };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void configureLabel(juce::Label&, const juce::String& text, bool heading = false);
    void configureSlider(juce::Slider&);
    void setParameterNormalizedValue(const char* id, float normalizedValue);
    void captureSlot(int slotIndex);
    void applySlot(int slotIndex);
    void resetHostParameters();
    void resetExperimentControls();
    void copyExperimentConfig();
    void exportExperimentConfig();
    juce::String createExperimentConfig() const;
    void setWorkflowStatus(const juce::String&);
    void timerCallback() override;

    FRAZILAudioProcessor& processor_;

    juce::Label titleLabel_;
    juce::Label subtitleLabel_;
    juce::Label hostParametersLabel_;
    juce::Label experimentLabel_;
    juce::Label workflowLabel_;
    juce::Label diagnosticsLabel_;
    juce::Label workflowStatusLabel_;

    juce::ToggleButton waterEnabledButton_;
    juce::ToggleButton iceEnabledButton_;
    juce::ComboBox routingModeBox_;
    std::array<juce::Label, 6> parameterLabels_;
    std::array<juce::Slider, 6> parameterSliders_;
    std::array<std::unique_ptr<SliderAttachment>, 6> parameterAttachments_;
    std::unique_ptr<ButtonAttachment> waterEnabledAttachment_;
    std::unique_ptr<ButtonAttachment> iceEnabledAttachment_;
    std::unique_ptr<ComboBoxAttachment> routingModeAttachment_;

    juce::ComboBox waterModelBox_;
    juce::Slider waterSizeSlider_;
    juce::Slider waterMotionSlider_;
    juce::Label waterModelLabel_;
    juce::Label waterSizeLabel_;
    juce::Label waterMotionLabel_;

    juce::TextButton captureAButton_{"Capture A"};
    juce::TextButton applyAButton_{"Apply A"};
    juce::TextButton captureBButton_{"Capture B"};
    juce::TextButton applyBButton_{"Apply B"};
    juce::TextButton resetHostButton_{"Reset Host"};
    juce::TextButton resetExperimentButton_{"Reset Exp"};
    juce::TextButton copyConfigButton_{"Copy Config"};
    juce::TextButton exportConfigButton_{"Export Config"};
    std::unique_ptr<juce::FileChooser> configFileChooser_;
    std::array<ABState, 2> abStates_;
#else
    FRAZILAudioProcessor& processor_;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FRAZILAudioProcessorEditor)
};
