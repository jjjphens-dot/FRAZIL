#include "plugin/ParameterLayout.h"
#include "plugin/PluginProcessor.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {
struct TestContext final {
    int failures{};
    std::size_t cases{};
};

void expect(TestContext& context, bool condition, const std::string& description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++context.failures;
    }
}

enum class InputCase : std::uint8_t { silence, impulse, deterministicNoise, extremeFinite };

enum class LifecycleCase : std::uint8_t {
    prepareAndProcess,
    prepareResetAndProcess,
    repeatedPrepare,
    repeatedReset,
    zeroLengthThenProcess
};

struct ParameterCase final {
    const char* name;
    float waterEnabled;
    float iceEnabled;
    float routingMode;
    float parallelBalance;
    float waterAmount;
    float iceAmount;
    float inputGainDb;
    float globalMix;
    float outputGainDb;
};

constexpr std::array<ParameterCase, 10> parameterCases{{
    {"default", 1.0f, 1.0f, 0.0f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {"minimum", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -24.0f, 0.0f, -24.0f},
    {"maximum", 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 24.0f, 1.0f, 24.0f},
    {"intermediate", 0.0f, 1.0f, 1.0f, 0.25f, 0.35f, 0.8f, -6.0f, 0.37f, 3.0f},
    {"routing-parallel", 1.0f, 1.0f, 0.0f, 0.2f, 0.4f, 0.6f, 0.0f, 0.5f, 0.0f},
    {"routing-water-into-ice", 1.0f, 1.0f, 1.0f, 0.5f, 0.4f, 0.6f, 0.0f, 0.5f, 0.0f},
    {"routing-ice-into-water", 1.0f, 1.0f, 2.0f, 0.5f, 0.4f, 0.6f, 0.0f, 0.5f, 0.0f},
    {"water-disabled", 0.0f, 1.0f, 0.0f, 0.5f, 0.9f, 0.6f, 0.0f, 1.0f, 0.0f},
    {"ice-disabled", 1.0f, 0.0f, 0.0f, 0.5f, 0.4f, 0.9f, 0.0f, 1.0f, 0.0f},
    {"both-disabled", 0.0f, 0.0f, 0.0f, 0.5f, 0.4f, 0.9f, 0.0f, 1.0f, 0.0f},
}};

constexpr std::array<double, 3> sampleRates{44100.0, 48000.0, 96000.0};
constexpr std::array<int, 6> blockSizes{32, 64, 128, 256, 512, 1024};
constexpr std::array<int, 2> channelCounts{1, 2};
constexpr std::array<InputCase, 4> inputCases{
    InputCase::silence, InputCase::impulse, InputCase::deterministicNoise,
    InputCase::extremeFinite};
constexpr std::array<LifecycleCase, 5> lifecycleCases{
    LifecycleCase::prepareAndProcess, LifecycleCase::prepareResetAndProcess,
    LifecycleCase::repeatedPrepare, LifecycleCase::repeatedReset,
    LifecycleCase::zeroLengthThenProcess};

const char* inputName(InputCase input) noexcept {
    switch (input) {
    case InputCase::silence:
        return "silence";
    case InputCase::impulse:
        return "impulse";
    case InputCase::deterministicNoise:
        return "deterministic-noise";
    case InputCase::extremeFinite:
        return "extreme-finite";
    }
    return "unknown-input";
}

const char* lifecycleName(LifecycleCase lifecycle) noexcept {
    switch (lifecycle) {
    case LifecycleCase::prepareAndProcess:
        return "prepare-process";
    case LifecycleCase::prepareResetAndProcess:
        return "prepare-reset-process";
    case LifecycleCase::repeatedPrepare:
        return "repeated-prepare";
    case LifecycleCase::repeatedReset:
        return "repeated-reset";
    case LifecycleCase::zeroLengthThenProcess:
        return "zero-length";
    }
    return "unknown-lifecycle";
}

void setParameterValue(TestContext& context,
                       FRAZILAudioProcessor& processor,
                       const char* id,
                       float value,
                       const std::string& caseName) {
    auto* parameter = processor.parameters.getParameter(id);
    expect(context, parameter != nullptr,
           caseName + ": parameter exists: " + id);
    if (parameter == nullptr)
        return;

    parameter->setValueNotifyingHost(
        processor.parameters.getParameterRange(id).convertTo0to1(value));
}

void applyParameters(TestContext& context,
                     FRAZILAudioProcessor& processor,
                     const ParameterCase& values,
                     const std::string& caseName) {
    using namespace frazil::plugin::parameterIds;
    setParameterValue(context, processor, waterEnabled, values.waterEnabled, caseName);
    setParameterValue(context, processor, iceEnabled, values.iceEnabled, caseName);
    setParameterValue(context, processor, routingMode, values.routingMode, caseName);
    setParameterValue(context, processor, parallelBalance, values.parallelBalance, caseName);
    setParameterValue(context, processor, waterAmount, values.waterAmount, caseName);
    setParameterValue(context, processor, iceAmount, values.iceAmount, caseName);
    setParameterValue(context, processor, inputGain, values.inputGainDb, caseName);
    setParameterValue(context, processor, globalMix, values.globalMix, caseName);
    setParameterValue(context, processor, outputGain, values.outputGainDb, caseName);
}

void fillInput(juce::AudioBuffer<float>& buffer, InputCase input) {
    buffer.clear();
    if (input == InputCase::silence)
        return;

    if (input == InputCase::impulse) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, 0, 0.5f);
        return;
    }

    std::uint32_t state = 0x13579bdfU;
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        state = state * 1664525U + 1013904223U;
        const auto normalized = static_cast<float>(state) /
                                static_cast<float>(std::numeric_limits<std::uint32_t>::max());
        const auto value = input == InputCase::extremeFinite
                               ? (sample % 2 == 0 ? 1.0e6f : -1.0e6f)
                               : (normalized * 2.0f - 1.0f);
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
    }
}

void expectFinite(TestContext& context,
                  const juce::AudioBuffer<float>& buffer,
                  const std::string& caseName) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const auto* samples = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (!std::isfinite(samples[sample])) {
                expect(context, false,
                       caseName + ": output contains NaN/Inf at channel " +
                           std::to_string(channel) + ", sample " + std::to_string(sample));
                return;
            }
        }
    }
}

bool configureBusLayout(TestContext& context,
                        FRAZILAudioProcessor& processor,
                        int channelCount,
                        const std::string& caseName) {
    const auto channelSet = channelCount == 1 ? juce::AudioChannelSet::mono()
                                               : juce::AudioChannelSet::stereo();
    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add(channelSet);
    layout.outputBuses.add(channelSet);
    const auto accepted = processor.setBusesLayout(layout);
    expect(context, accepted, caseName + ": mono/stereo bus layout accepted");
    return accepted;
}

void runPropertyCase(TestContext& context,
                     double sampleRate,
                     int blockSize,
                     int channelCount,
                     const ParameterCase& parameterValues,
                     InputCase input,
                     LifecycleCase lifecycle) {
    std::ostringstream caseName;
    caseName << "rate=" << sampleRate << ", block=" << blockSize
             << ", channels=" << channelCount << ", parameters=" << parameterValues.name
             << ", input=" << inputName(input) << ", lifecycle=" << lifecycleName(lifecycle);
    const auto name = caseName.str();
    ++context.cases;

    FRAZILAudioProcessor processor;
    if (!configureBusLayout(context, processor, channelCount, name))
        return;
    applyParameters(context, processor, parameterValues, name);
    processor.prepareToPlay(sampleRate, blockSize);

    if (lifecycle == LifecycleCase::repeatedPrepare)
        processor.prepareToPlay(sampleRate, blockSize);
    else if (lifecycle == LifecycleCase::prepareResetAndProcess) {
        processor.releaseResources();
    } else if (lifecycle == LifecycleCase::repeatedReset) {
        processor.releaseResources();
        processor.releaseResources();
        processor.prepareToPlay(sampleRate, blockSize);
    } else if (lifecycle == LifecycleCase::zeroLengthThenProcess) {
        juce::AudioBuffer<float> emptyBuffer(channelCount, 0);
        juce::MidiBuffer midi;
        processor.processBlock(emptyBuffer, midi);
        expect(context, emptyBuffer.getNumChannels() == channelCount &&
                           emptyBuffer.getNumSamples() == 0,
               name + ": zero-length buffer dimensions remain valid");
    }

    juce::AudioBuffer<float> buffer(channelCount, blockSize);
    fillInput(buffer, input);
    const auto originalChannels = buffer.getNumChannels();
    const auto originalSamples = buffer.getNumSamples();
    juce::MidiBuffer midi;
    processor.processBlock(buffer, midi);

    expect(context, buffer.getNumChannels() == originalChannels &&
                       buffer.getNumSamples() == originalSamples,
           name + ": buffer dimensions remain valid after processBlock");
    expectFinite(context, buffer, name);
}

std::vector<float> runDeterministicCase(int channelCount, int blockSize) {
    FRAZILAudioProcessor processor;
    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add(juce::AudioChannelSet::stereo());
    layout.outputBuses.add(juce::AudioChannelSet::stereo());
    processor.setBusesLayout(layout);
    processor.prepareToPlay(48000.0, blockSize);

    juce::AudioBuffer<float> buffer(channelCount, blockSize);
    fillInput(buffer, InputCase::deterministicNoise);
    juce::MidiBuffer midi;
    processor.processBlock(buffer, midi);

    std::vector<float> result(static_cast<std::size_t>(channelCount * blockSize));
    for (int channel = 0; channel < channelCount; ++channel)
        for (int sample = 0; sample < blockSize; ++sample)
            result[static_cast<std::size_t>(channel * blockSize + sample)] =
                buffer.getSample(channel, sample);
    return result;
}

void testDeterministicOutput(TestContext& context) {
    const auto first = runDeterministicCase(2, 128);
    const auto second = runDeterministicCase(2, 128);
    expect(context, first == second,
           "fresh processors produce deterministic output for deterministic input");
}
} // namespace

int main() {
    TestContext context;

    // Keep the high-cost processor construction outside the full Cartesian product. The full
    // rate/block/channel matrix validates runtime dimensions; parameter, input, and lifecycle
    // dimensions are then exercised against a canonical 48 kHz/128/stereo configuration.
    const auto& defaultParameters = parameterCases.front();
    for (const auto sampleRate : sampleRates)
        for (const auto blockSize : blockSizes)
            for (const auto channelCount : channelCounts)
                runPropertyCase(context, sampleRate, blockSize, channelCount, defaultParameters,
                                InputCase::deterministicNoise,
                                LifecycleCase::prepareAndProcess);

    for (const auto& parameterValues : parameterCases) {
        runPropertyCase(context, 48000.0, 128, 2, parameterValues,
                        InputCase::deterministicNoise, LifecycleCase::prepareAndProcess);
        runPropertyCase(context, 48000.0, 128, 2, parameterValues,
                        InputCase::extremeFinite, LifecycleCase::prepareAndProcess);
    }

    for (const auto input : inputCases)
        runPropertyCase(context, 48000.0, 128, 2, defaultParameters, input,
                        LifecycleCase::prepareAndProcess);

    for (const auto lifecycle : lifecycleCases)
        runPropertyCase(context, 48000.0, 128, 2, defaultParameters,
                        InputCase::deterministicNoise, lifecycle);

    testDeterministicOutput(context);
    if (context.failures != 0)
        return 1;

    std::cout << "TEST-002 processor property harness passed (" << context.cases
              << " representative cases; full rate/block/channel matrix plus parameter, input, "
                 "and lifecycle sub-matrices)\n";
    return 0;
}
