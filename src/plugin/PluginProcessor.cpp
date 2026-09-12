#include "PluginProcessor.h"

#include "ParameterLayout.h"
#include "PluginEditor.h"
#include "StateAdapter.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

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
    auto snapshot = ParameterSnapshot::capture(parameterSources_);
#if FRAZIL_ENABLE_DEVELOPER_UI
    developerParameterOverride_.applyTo(snapshot);
    const auto dryReferenceOnly = developerDryComparison_.load(std::memory_order_acquire);
#else
    constexpr auto dryReferenceOnly = false;
#endif
    const auto engineParameters = parameterMapper_.map(snapshot);
    audioEngine.process(buffer, engineParameters, dryReferenceOnly);
#if FRAZIL_ENABLE_DEVELOPER_UI
    const auto outputMetrics = measureBuffer(buffer);
    developerDiagnostics_.publish(buffer.getNumSamples(), buffer.getNumChannels(),
                                  inputMetrics.peak, outputMetrics.peak, inputMetrics.rms,
                                  outputMetrics.rms, inputMetrics.finite && outputMetrics.finite);
#endif
}

frazil::plugin::DeveloperDiagnosticsSnapshot
FRAZILAudioProcessor::getDeveloperDiagnosticsSnapshot() const noexcept {
    return developerDiagnostics_.snapshot();
}

#if FRAZIL_ENABLE_DEVELOPER_UI
namespace {
frazil::plugin::DeveloperHostParameterSnapshot
toDeveloperHostSnapshot(const ParameterSnapshot& snapshot) noexcept {
    frazil::plugin::DeveloperHostParameterSnapshot result;
    result
        .rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::waterEnabled)] =
        snapshot.waterEnabled ? 1.0f : 0.0f;
    result.rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::iceEnabled)] =
        snapshot.iceEnabled ? 1.0f : 0.0f;
    result
        .rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::routingMode)] =
        static_cast<float>(snapshot.routingModeIndex);
    result.rawValues[static_cast<std::size_t>(
        frazil::plugin::DeveloperHostParameter::parallelBalance)] = snapshot.parallelBalance;
    result
        .rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::waterAmount)] =
        snapshot.waterAmount;
    result.rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::iceAmount)] =
        snapshot.iceAmount;
    result
        .rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::inputGainDb)] =
        snapshot.inputGainDb;
    result.rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::globalMix)] =
        snapshot.globalMix;
    result
        .rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::outputGainDb)] =
        snapshot.outputGainDb;
    return result;
}
} // namespace

frazil::plugin::DeveloperHostParameterSnapshot
FRAZILAudioProcessor::getDeveloperHostParameterSnapshot() const noexcept {
    auto snapshot = ParameterSnapshot::capture(parameterSources_);
    developerParameterOverride_.applyTo(snapshot);
    return toDeveloperHostSnapshot(snapshot);
}

void FRAZILAudioProcessor::setDeveloperHostParameterOverride(
    const frazil::plugin::DeveloperHostParameterSnapshot& snapshot) noexcept {
    developerParameterOverride_.set(snapshot);
}

void FRAZILAudioProcessor::clearDeveloperHostParameterOverride() noexcept {
    developerParameterOverride_.clear();
}

bool FRAZILAudioProcessor::isDeveloperHostParameterOverrideActive() const noexcept {
    return developerParameterOverride_.isActive();
}

void FRAZILAudioProcessor::setDeveloperComparisonMode(
    frazil::plugin::DeveloperComparisonMode mode) noexcept {
    developerDryComparison_.store(mode == frazil::plugin::DeveloperComparisonMode::dry,
                                  std::memory_order_release);
}

frazil::plugin::DeveloperComparisonMode
FRAZILAudioProcessor::getDeveloperComparisonMode() const noexcept {
    return developerDryComparison_.load(std::memory_order_acquire)
               ? frazil::plugin::DeveloperComparisonMode::dry
               : frazil::plugin::DeveloperComparisonMode::processed;
}
#endif

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
#if FRAZIL_ENABLE_DEVELOPER_UI
    clearDeveloperHostParameterOverride();
    developerDryComparison_.store(false, std::memory_order_release);
#endif
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
