#pragma once

#include "../app/AudioEngine.h"
#include "../app/ParameterMapper.h"
#include "../app/ParameterSnapshot.h"

#include <JuceHeader.h>

class FRAZILAudioProcessor final : public juce::AudioProcessor {
  public:
    FRAZILAudioProcessor();
    ~FRAZILAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destinationData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Narrow setup/message-thread access for editor and integration boundaries. The APVTS and its
    // state tree remain private; audio processing uses the cached atomics below.
    juce::RangedAudioParameter* parameter(const char* id) noexcept;
    const std::atomic<float>* rawParameterValue(const char* id) const noexcept;
    juce::NormalisableRange<float> parameterRange(const char* id) const noexcept;

  private:
    juce::AudioProcessorValueTreeState parameters_;
    ParameterSourcePointers parameterSources_;
    ParameterMapper parameterMapper_;
    AudioEngine audioEngine_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FRAZILAudioProcessor)
};
