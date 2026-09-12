#pragma once

#include "../app/AudioEngine.h"
#include "../app/ParameterMapper.h"
#include "../app/ParameterSnapshot.h"
#include "../app/ProcessSpec.h"
#include "DeveloperDiagnostics.h"
#if FRAZIL_ENABLE_DEVELOPER_UI
#include "DeveloperExperimentState.h"
#include "DeveloperParameterOverride.h"
#endif

#include <JuceHeader.h>
#include <optional>

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

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    frazil::plugin::DeveloperDiagnosticsSnapshot getDeveloperDiagnosticsSnapshot() const noexcept;
#if FRAZIL_ENABLE_DEVELOPER_UI
    frazil::plugin::DeveloperHostParameterSnapshot
    getDeveloperHostParameterSnapshot() const noexcept;
    void setDeveloperHostParameterOverride(
        const frazil::plugin::DeveloperHostParameterSnapshot&) noexcept;
    std::optional<frazil::plugin::DeveloperParameterOverride::ControlToken>
    getDeveloperHostParameterOverrideToken() const noexcept;
    bool trySetDeveloperHostParameterOverrideIfCurrent(
        const frazil::plugin::DeveloperHostParameterSnapshot&,
        frazil::plugin::DeveloperParameterOverride::ControlToken) noexcept;
    void clearDeveloperHostParameterOverride() noexcept;
    bool isDeveloperHostParameterOverrideActive() const noexcept;
    void setDeveloperComparisonMode(frazil::plugin::DeveloperComparisonMode) noexcept;
    frazil::plugin::DeveloperComparisonMode getDeveloperComparisonMode() const noexcept;
#endif

    juce::AudioProcessorValueTreeState parameters;

  private:
    ParameterSourcePointers parameterSources_;
    ParameterMapper parameterMapper_;
    AudioEngine audioEngine;
    frazil::plugin::DeveloperDiagnostics developerDiagnostics_;
#if FRAZIL_ENABLE_DEVELOPER_UI
    frazil::plugin::DeveloperParameterOverride developerParameterOverride_;
    frazil::plugin::DeveloperParameterOverride::AudioReadState developerOverrideAudioReadState_;
    std::atomic<bool> developerDryComparison_{};
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FRAZILAudioProcessor)
};
