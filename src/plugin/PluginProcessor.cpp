#include "PluginProcessor.h"

#include "../app/ProcessSpec.h"
#include "ParameterLayout.h"
#include "PluginEditor.h"
#include "StateAdapter.h"

FRAZILAudioProcessor::FRAZILAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "FRAZIL", createParameterLayout()) {
    parameterSources_ = {
        parameters.getRawParameterValue(frazil::plugin::parameterIds::kWaterEnabled),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::kIceEnabled),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::kRoutingMode),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::kParallelBalance),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::kWaterAmount),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::kIceAmount),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::kInputGain),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::kGlobalMix),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::kOutputGain)};
}

juce::AudioProcessorValueTreeState::ParameterLayout FRAZILAudioProcessor::createParameterLayout() {
    return frazil::plugin::createParameterLayout();
}

void FRAZILAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    const auto prepared =
        audioEngine_.prepare(ProcessSpec{sampleRate, samplesPerBlock, getTotalNumOutputChannels()});
    jassert(prepared);
}

void FRAZILAudioProcessor::releaseResources() {
    audioEngine_.reset();
}

bool FRAZILAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();

    if (input != output)
        return false;

    return input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo();
}

void FRAZILAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    const auto snapshot = ParameterSnapshot::capture(parameterSources_);
    const auto engineParameters = parameterMapper_.map(snapshot);
    audioEngine_.process(buffer, engineParameters);
}

juce::AudioProcessorEditor* FRAZILAudioProcessor::createEditor() {
    return new FRAZILAudioProcessorEditor(*this);
}

bool FRAZILAudioProcessor::hasEditor() const {
    return true;
}

const juce::String FRAZILAudioProcessor::getName() const {
    return "FRAZIL";
}

bool FRAZILAudioProcessor::acceptsMidi() const {
    return false;
}

bool FRAZILAudioProcessor::producesMidi() const {
    return false;
}

bool FRAZILAudioProcessor::isMidiEffect() const {
    return false;
}

double FRAZILAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int FRAZILAudioProcessor::getNumPrograms() {
    return 1;
}

int FRAZILAudioProcessor::getCurrentProgram() {
    return 0;
}

void FRAZILAudioProcessor::setCurrentProgram(int) {}

const juce::String FRAZILAudioProcessor::getProgramName(int) {
    return {};
}

void FRAZILAudioProcessor::changeProgramName(int, const juce::String&) {}

void FRAZILAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData) {
    const auto state = frazil::plugin::HostStateAdapter::serialize(parameters);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destinationData);
}

void FRAZILAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
        const auto state = juce::ValueTree::fromXml(*xml);
        frazil::plugin::HostStateAdapter::restore(parameters, state);
    } else {
        frazil::plugin::HostStateAdapter::restore(parameters, juce::ValueTree{});
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new FRAZILAudioProcessor();
}
