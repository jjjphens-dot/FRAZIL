#include "PluginEditor.h"

FRAZILAudioProcessorEditor::FRAZILAudioProcessorEditor(juce::AudioProcessor& processor)
    : AudioProcessorEditor(processor) {
    setSize(640, 360);
}

void FRAZILAudioProcessorEditor::paint(juce::Graphics& graphics) {
    graphics.fillAll(juce::Colour(0xff101820));
    graphics.setColour(juce::Colours::white);
    graphics.setFont(juce::FontOptions(28.0f));
    graphics.drawFittedText("FRAZIL", getLocalBounds().reduced(32), juce::Justification::centred,
                            1);
    graphics.setFont(juce::FontOptions(14.0f));
    graphics.drawFittedText("M1 parameter and gain skeleton / Water-Ice pending",
                            getLocalBounds().reduced(32).withTop(190), juce::Justification::centred,
                            1);
}

void FRAZILAudioProcessorEditor::resized() {}
