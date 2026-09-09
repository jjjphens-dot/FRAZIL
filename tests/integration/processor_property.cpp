#include "plugin/ParameterLayout.h"
#include "plugin/PluginProcessor.h"

#include <algorithm>
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
    prepareProcessReprepareProcess,
    repeatedPrepare,
    repeatedReleaseThenPrepare,
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

constexpr float kSilenceOutputTolerance = 1.0e-6f;
constexpr float kParameterValueTolerance = 1.0e-5f;
constexpr int kRepresentativeNominalBlockUpperBound = 1024;
constexpr std::array<double, 3> sampleRates{44100.0, 48000.0, 96000.0};
constexpr std::array<int, 6> representativeNominalBlockSizes{32, 64, 128, 256, 512,
                                                              kRepresentativeNominalBlockUpperBound};
constexpr std::array<int, 2> channelCounts{1, 2};
constexpr std::array<int, 5> shortAndOddCallbackSizes{
    0, 1, 7, 31, kRepresentativeNominalBlockUpperBound};
constexpr std::array<InputCase, 4> inputCases{
    InputCase::silence, InputCase::impulse, InputCase::deterministicNoise,
    InputCase::extremeFinite};
constexpr std::array<LifecycleCase, 5> lifecycleCases{
    LifecycleCase::prepareAndProcess, LifecycleCase::prepareProcessReprepareProcess,
    LifecycleCase::repeatedPrepare, LifecycleCase::repeatedReleaseThenPrepare,
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
    case LifecycleCase::prepareProcessReprepareProcess:
        return "prepare-process-reprepare-process";
    case LifecycleCase::repeatedPrepare:
        return "repeated-prepare";
    case LifecycleCase::repeatedReleaseThenPrepare:
        return "repeated-release-then-prepare";
    case LifecycleCase::zeroLengthThenProcess:
        return "zero-length";
    }
    return "unknown-lifecycle";
}

bool setParameterValue(TestContext& context,
                       FRAZILAudioProcessor& processor,
                       const char* id,
                       float value,
                       const std::string& caseName) {
    auto* parameter = processor.parameters.getParameter(id);
    expect(context, parameter != nullptr,
           caseName + ": parameter exists: " + id);
    if (parameter == nullptr)
        return false;

    const auto normalizedValue =
        processor.parameters.getParameterRange(id).convertTo0to1(value);
    parameter->setValueNotifyingHost(normalizedValue);
    const auto applied = std::abs(parameter->getValue() - normalizedValue) <=
                         kParameterValueTolerance;
    expect(context, applied, caseName + ": parameter applied: " + id);
    return applied;
}

bool applyParameters(TestContext& context,
                     FRAZILAudioProcessor& processor,
                     const ParameterCase& values,
                     const std::string& caseName) {
    using namespace frazil::plugin::parameterIds;
    bool allApplied = true;
    allApplied = setParameterValue(context, processor, waterEnabled, values.waterEnabled,
                                   caseName) && allApplied;
    allApplied = setParameterValue(context, processor, iceEnabled, values.iceEnabled, caseName) &&
                 allApplied;
    allApplied = setParameterValue(context, processor, routingMode, values.routingMode,
                                   caseName) && allApplied;
    allApplied = setParameterValue(context, processor, parallelBalance, values.parallelBalance,
                                   caseName) && allApplied;
    allApplied = setParameterValue(context, processor, waterAmount, values.waterAmount, caseName) &&
                 allApplied;
    allApplied = setParameterValue(context, processor, iceAmount, values.iceAmount, caseName) &&
                 allApplied;
    allApplied = setParameterValue(context, processor, inputGain, values.inputGainDb, caseName) &&
                 allApplied;
    allApplied = setParameterValue(context, processor, globalMix, values.globalMix, caseName) &&
                 allApplied;
    allApplied = setParameterValue(context, processor, outputGain, values.outputGainDb, caseName) &&
                 allApplied;
    return allApplied;
}

bool applyNeutralDryFixture(TestContext& context,
                            FRAZILAudioProcessor& processor,
                            const std::string& caseName) {
    // ADR-0005 neutral/dry fixture: unity input/output gain and global.mix=0.
    using namespace frazil::plugin::parameterIds;
    bool allApplied = true;
    allApplied = setParameterValue(context, processor, inputGain, 0.0f, caseName) && allApplied;
    allApplied = setParameterValue(context, processor, outputGain, 0.0f, caseName) && allApplied;
    allApplied = setParameterValue(context, processor, globalMix, 0.0f, caseName) && allApplied;
    return allApplied;
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

std::vector<float> snapshotBuffer(const juce::AudioBuffer<float>& buffer) {
    std::vector<float> snapshot;
    snapshot.reserve(static_cast<std::size_t>(buffer.getNumChannels() * buffer.getNumSamples()));
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            snapshot.push_back(buffer.getSample(channel, sample));
    return snapshot;
}

void expectSilenceOutput(TestContext& context,
                         const juce::AudioBuffer<float>& buffer,
                         const std::string& caseName) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        double sum = 0.0;
        float maximumMagnitude = 0.0f;
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            const auto value = buffer.getSample(channel, sample);
            sum += static_cast<double>(value);
            maximumMagnitude = std::max(maximumMagnitude, std::abs(value));
        }

        const auto measuredDc = buffer.getNumSamples() == 0
                                    ? 0.0
                                    : sum / static_cast<double>(buffer.getNumSamples());
        if (maximumMagnitude > kSilenceOutputTolerance ||
            std::abs(measuredDc) > static_cast<double>(kSilenceOutputTolerance)) {
            std::ostringstream description;
            description << caseName << ": silence output has unexplained signal on channel "
                        << channel << ", measured DC=" << measuredDc
                        << ", max magnitude=" << maximumMagnitude
                        << ", tolerance=" << kSilenceOutputTolerance;
            expect(context, false, description.str());
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
    const auto exactRepeatability = lifecycle == LifecycleCase::prepareProcessReprepareProcess;
    caseName << "rate=" << sampleRate << ", block=" << blockSize
             << ", channels=" << channelCount << ", parameters="
             << (exactRepeatability ? "neutral/dry" : parameterValues.name)
             << ", input=" << inputName(input) << ", lifecycle=" << lifecycleName(lifecycle);
    const auto name = caseName.str();
    ++context.cases;

    FRAZILAudioProcessor processor;
    if (!configureBusLayout(context, processor, channelCount, name))
        return;
    if (!applyParameters(context, processor, parameterValues, name))
        return;
    if (exactRepeatability && !applyNeutralDryFixture(context, processor, name))
        return;
    processor.prepareToPlay(sampleRate, blockSize);

    const auto processOnce = [&](const char* phase) {
        juce::AudioBuffer<float> buffer(channelCount, blockSize);
        fillInput(buffer, input);
        const auto originalChannels = buffer.getNumChannels();
        const auto originalSamples = buffer.getNumSamples();
        juce::MidiBuffer midi;
        processor.processBlock(buffer, midi);

        const auto phaseName = name + ": " + phase;
        expect(context, buffer.getNumChannels() == originalChannels &&
                           buffer.getNumSamples() == originalSamples,
               phaseName + ": buffer dimensions remain valid after processBlock");
        expectFinite(context, buffer, phaseName);
        if (input == InputCase::silence)
            expectSilenceOutput(context, buffer, phaseName);
        return snapshotBuffer(buffer);
    };

    switch (lifecycle) {
    case LifecycleCase::prepareAndProcess:
        processOnce("process");
        break;
    case LifecycleCase::prepareProcessReprepareProcess: {
        const auto firstOutput = processOnce("initial process");
        processor.releaseResources();
        processor.prepareToPlay(sampleRate, blockSize);
        const auto secondOutput = processOnce("reprepared process");
        expect(context, firstOutput == secondOutput,
               name + ": neutral/dry exact output repeats after release and reprepare; "
                       "this is not a production-randomness contract");
        break;
    }
    case LifecycleCase::repeatedPrepare:
        processor.prepareToPlay(sampleRate, blockSize);
        processOnce("process after repeated prepare");
        break;
    case LifecycleCase::repeatedReleaseThenPrepare:
        processor.releaseResources();
        processor.releaseResources();
        processor.prepareToPlay(sampleRate, blockSize);
        processOnce("process after repeated release and prepare");
        break;
    case LifecycleCase::zeroLengthThenProcess: {
        juce::AudioBuffer<float> emptyBuffer(channelCount, 0);
        juce::MidiBuffer midi;
        processor.processBlock(emptyBuffer, midi);
        expect(context, emptyBuffer.getNumChannels() == channelCount &&
                           emptyBuffer.getNumSamples() == 0,
               name + ": zero-length buffer dimensions remain valid");
        processOnce("process after zero-length callback");
        break;
    }
    }
}

void runShortAndOddCallbackCases(TestContext& context) {
    const auto& parameters = parameterCases.front();
    for (const auto actualBlockSize : shortAndOddCallbackSizes) {
        std::ostringstream caseName;
        caseName << "rate=48000, prepare nominal=" << kRepresentativeNominalBlockUpperBound
                 << ", callback=" << actualBlockSize << ", channels=2, parameters="
                 << parameters.name << ", input=deterministic-noise, lifecycle=single-prepare";
        const auto name = caseName.str();
        ++context.cases;

        FRAZILAudioProcessor processor;
        if (!configureBusLayout(context, processor, 2, name))
            continue;
        if (!applyParameters(context, processor, parameters, name))
            continue;
        processor.prepareToPlay(48000.0, kRepresentativeNominalBlockUpperBound);

        juce::AudioBuffer<float> buffer(2, actualBlockSize);
        fillInput(buffer, InputCase::deterministicNoise);
        juce::MidiBuffer midi;
        processor.processBlock(buffer, midi);
        expect(context, buffer.getNumChannels() == 2 &&
                           buffer.getNumSamples() == actualBlockSize,
               name + ": actual callback dimensions remain valid");
        expectFinite(context, buffer, name);
    }
}

std::vector<float> runDeterministicCase(TestContext& context,
                                        int channelCount,
                                        int blockSize,
                                        const char* instanceName) {
    FRAZILAudioProcessor processor;
    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add(juce::AudioChannelSet::stereo());
    layout.outputBuses.add(juce::AudioChannelSet::stereo());
    const auto caseName = std::string("M1 neutral/deterministic path, ") + instanceName;
    if (!processor.setBusesLayout(layout)) {
        expect(context, false, caseName + ": stereo bus layout accepted");
        return {};
    }
    if (!applyNeutralDryFixture(context, processor, caseName))
        return {};
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

void testM1NeutralDeterministicOutput(TestContext& context) {
    const auto first = runDeterministicCase(context, 2, 128, "first fresh processor");
    const auto second = runDeterministicCase(context, 2, 128, "second fresh processor");
    const auto expectedSize = static_cast<std::size_t>(2 * 128);
    expect(context, first.size() == expectedSize && second.size() == expectedSize,
           "M1 neutral/deterministic path produced expected fixture output sizes");
    expect(context, first == second,
           "M1 neutral/deterministic path repeats for fresh processors and deterministic input; "
           "this is not a production-randomness contract");
}
} // namespace

int main() {
    TestContext context;

    // Keep the high-cost processor construction outside the full Cartesian product. The full
    // rate/block/channel matrix validates runtime dimensions; parameter, input, and lifecycle
    // dimensions are then exercised against a canonical 48 kHz/128/stereo configuration.
    const auto& defaultParameters = parameterCases.front();
    for (const auto sampleRate : sampleRates)
    for (const auto blockSize : representativeNominalBlockSizes)
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

    runShortAndOddCallbackCases(context);
    testM1NeutralDeterministicOutput(context);
    if (context.failures != 0)
        return 1;

    std::cout << "TEST-002 processor property harness passed (" << context.cases
              << " representative cases; nominal matrix plus parameter, input, lifecycle, "
                 "and short/odd callback sub-matrices)\n";
    return 0;
}
