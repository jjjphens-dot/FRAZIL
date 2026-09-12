#include "PluginProcessor.h"

#include "ParameterLayout.h"
#include "PluginEditor.h"
#include "StateAdapter.h"

#include <algorithm>
#include <cmath>

namespace {
struct BufferMetrics final {
    float peak{};
    float rms{};
    bool finite{true};
};

BufferMetrics measureBuffer(const juce::AudioBuffer<float>& buffer) noexcept {
    const auto channelCount = buffer.getNumChannels();
    const auto sampleCount = buffer.getNumSamples();
    if (channelCount <= 0 || sampleCount <= 0)
        return {};

    double sumSquares = 0.0;
    float peak = 0.0f;
    bool finite = true;
    for (int channel = 0; channel < channelCount; ++channel) {
        const auto* samples = buffer.getReadPointer(channel);
        for (int sample = 0; sample < sampleCount; ++sample) {
            const auto value = samples[sample];
            if (!std::isfinite(value)) {
                finite = false;
                continue;
            }

            peak = std::max(peak, std::abs(value));
            sumSquares += static_cast<double>(value) * static_cast<double>(value);
        }
    }

    const auto sampleTotal = static_cast<double>(channelCount) * sampleCount;
    return {peak, static_cast<float>(std::sqrt(sumSquares / sampleTotal)), finite};
}
} // namespace

FRAZILAudioProcessor::FRAZILAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "FRAZIL", createParameterLayout()) {
    parameterSources_ = {
        parameters.getRawParameterValue(frazil::plugin::parameterIds::waterEnabled),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::iceEnabled),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::routingMode),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::parallelBalance),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::waterAmount),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::iceAmount),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::inputGain),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::globalMix),
        parameters.getRawParameterValue(frazil::plugin::parameterIds::outputGain)};
}

juce::AudioProcessorValueTreeState::ParameterLayout FRAZILAudioProcessor::createParameterLayout() {
    return frazil::plugin::createParameterLayout();
}

void FRAZILAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    audioEngine.prepare(ProcessSpec{sampleRate, samplesPerBlock, getTotalNumOutputChannels()});
#if FRAZIL_ENABLE_DEVELOPER_UI
    developerDiagnostics_.setPrepared(static_cast<float>(sampleRate), samplesPerBlock,
                                      getTotalNumOutputChannels());
#endif
}

void FRAZILAudioProcessor::releaseResources() {
    audioEngine.reset();
#if FRAZIL_ENABLE_DEVELOPER_UI
    developerDiagnostics_.reset();
#endif
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
#if FRAZIL_ENABLE_DEVELOPER_UI
    const auto inputMetrics = measureBuffer(buffer);
#endif
    const auto snapshot = ParameterSnapshot::capture(parameterSources_);
    const auto engineParameters = parameterMapper_.map(snapshot);
    audioEngine.process(buffer, engineParameters);
#if FRAZIL_ENABLE_DEVELOPER_UI
    const auto outputMetrics = measureBuffer(buffer);
    developerDiagnostics_.publish(inputMetrics.peak, outputMetrics.peak, inputMetrics.rms,
                                  outputMetrics.rms, inputMetrics.finite && outputMetrics.finite);
#endif
}

frazil::plugin::DeveloperDiagnosticsSnapshot
FRAZILAudioProcessor::getDeveloperDiagnosticsSnapshot() const noexcept {
    return developerDiagnostics_.snapshot();
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
