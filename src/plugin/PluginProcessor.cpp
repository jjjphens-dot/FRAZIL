#include "PluginProcessor.h"

#include "../app/ProcessSpec.h"
#include "ParameterLayout.h"
#include "PluginEditor.h"
#include "StateAdapter.h"

FRAZILAudioProcessor::FRAZILAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, "FRAZIL", frazil::plugin::createParameterLayout()) {
    parameterSources_ = {
        parameters_.getRawParameterValue(frazil::plugin::parameterIds::kWaterEnabled),
        parameters_.getRawParameterValue(frazil::plugin::parameterIds::kIceEnabled),
        parameters_.getRawParameterValue(frazil::plugin::parameterIds::kRoutingMode),
        parameters_.getRawParameterValue(frazil::plugin::parameterIds::kParallelBalance),
        parameters_.getRawParameterValue(frazil::plugin::parameterIds::kWaterAmount),
        parameters_.getRawParameterValue(frazil::plugin::parameterIds::kIceAmount),
        parameters_.getRawParameterValue(frazil::plugin::parameterIds::kInputGain),
        parameters_.getRawParameterValue(frazil::plugin::parameterIds::kGlobalMix),
        parameters_.getRawParameterValue(frazil::plugin::parameterIds::kOutputGain)};
}

juce::RangedAudioParameter* FRAZILAudioProcessor::parameter(const char* id) noexcept {
    return parameters_.getParameter(id);
}

const std::atomic<float>* FRAZILAudioProcessor::rawParameterValue(const char* id) const noexcept {
    return parameters_.getRawParameterValue(id);
}

juce::NormalisableRange<float> FRAZILAudioProcessor::parameterRange(const char* id) const noexcept {
    return parameters_.getParameterRange(id);
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
    const auto state = frazil::plugin::HostStateAdapter::serialize(parameters_);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destinationData);
}

void FRAZILAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
        const auto state = juce::ValueTree::fromXml(*xml);
        frazil::plugin::HostStateAdapter::restore(parameters_, state);
    } else {
        frazil::plugin::HostStateAdapter::restore(parameters_, juce::ValueTree{});
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new FRAZILAudioProcessor();
}
