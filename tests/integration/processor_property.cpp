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
    neutralPrepareProcessReprepareProcessExact,
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

constexpr std::array<ParameterCase, 10> kParameterCases{{
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
constexpr double kCanonicalSampleRateHz = 48000.0;
constexpr int kCanonicalBlockSizeSamples = 128;
constexpr int kStereoChannelCount = 2;
constexpr float kImpulseAmplitude = 0.5f;
constexpr float kExtremeFiniteMagnitude = 1.0e6f;
constexpr std::uint32_t kFixtureSeed = 0x13579bdfU;
constexpr std::uint32_t kLcgMultiplier = 1664525U;
constexpr std::uint32_t kLcgIncrement = 1013904223U;
constexpr int kDeterministicFixtureChannels = kStereoChannelCount;
constexpr std::array<double, 3> kSampleRates{44100.0, kCanonicalSampleRateHz, 96000.0};
constexpr std::array<int, 6> kRepresentativeNominalBlockSizes{
    32, 64, kCanonicalBlockSizeSamples, 256, 512, kRepresentativeNominalBlockUpperBound};
constexpr std::array<int, 2> kChannelCounts{1, 2};
constexpr std::array<int, 5> kShortAndOddCallbackSizes{0, 1, 7, 31,
                                                       kRepresentativeNominalBlockUpperBound};
constexpr std::array<InputCase, 4> kInputCases{InputCase::silence, InputCase::impulse,
                                               InputCase::deterministicNoise,
                                               InputCase::extremeFinite};
constexpr std::array<LifecycleCase, 6> kLifecycleCases{
    LifecycleCase::prepareAndProcess,
    LifecycleCase::prepareProcessReprepareProcess,
    LifecycleCase::neutralPrepareProcessReprepareProcessExact,
    LifecycleCase::repeatedPrepare,
    LifecycleCase::repeatedReleaseThenPrepare,
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
        return "prepare-process-reprepare-process-active-recovery";
    case LifecycleCase::neutralPrepareProcessReprepareProcessExact:
        return "neutral-prepare-process-reprepare-process-exact";
    case LifecycleCase::repeatedPrepare:
        return "repeated-prepare";
    case LifecycleCase::repeatedReleaseThenPrepare:
        return "repeated-release-then-prepare";
    case LifecycleCase::zeroLengthThenProcess:
        return "zero-length";
    }
    return "unknown-lifecycle";
}

bool setParameterValue(TestContext& context, FRAZILAudioProcessor& processor, const char* id,
                       float value, const std::string& caseName) {
    auto* parameter = processor.parameters.getParameter(id);
    expect(context, parameter != nullptr, caseName + ": parameter exists: " + id);
    if (parameter == nullptr)
        return false;

    const auto normalizedValue = processor.parameters.getParameterRange(id).convertTo0to1(value);
    parameter->setValueNotifyingHost(normalizedValue);
    const auto applied =
        std::abs(parameter->getValue() - normalizedValue) <= kParameterValueTolerance;
    expect(context, applied, caseName + ": parameter applied: " + id);
    return applied;
}

bool applyParameters(TestContext& context, FRAZILAudioProcessor& processor,
                     const ParameterCase& values, const std::string& caseName) {
    using namespace frazil::plugin::parameterIds;
    bool allApplied = true;
    allApplied =
        setParameterValue(context, processor, kWaterEnabled, values.waterEnabled, caseName) &&
        allApplied;
    allApplied = setParameterValue(context, processor, kIceEnabled, values.iceEnabled, caseName) &&
                 allApplied;
    allApplied =
        setParameterValue(context, processor, kRoutingMode, values.routingMode, caseName) &&
        allApplied;
    allApplied =
        setParameterValue(context, processor, kParallelBalance, values.parallelBalance, caseName) &&
        allApplied;
    allApplied =
        setParameterValue(context, processor, kWaterAmount, values.waterAmount, caseName) &&
        allApplied;
    allApplied =
        setParameterValue(context, processor, kIceAmount, values.iceAmount, caseName) && allApplied;
    allApplied = setParameterValue(context, processor, kInputGain, values.inputGainDb, caseName) &&
                 allApplied;
    allApplied =
        setParameterValue(context, processor, kGlobalMix, values.globalMix, caseName) && allApplied;
    allApplied =
        setParameterValue(context, processor, kOutputGain, values.outputGainDb, caseName) &&
        allApplied;
    return allApplied;
}

bool applyNeutralDryFixture(TestContext& context, FRAZILAudioProcessor& processor,
                            const std::string& caseName) {
    // ADR-0005 neutral/dry fixture: unity input/output gain and global.mix=0.
    using namespace frazil::plugin::parameterIds;
    bool allApplied = true;
    allApplied = setParameterValue(context, processor, kInputGain, 0.0f, caseName) && allApplied;
    allApplied = setParameterValue(context, processor, kOutputGain, 0.0f, caseName) && allApplied;
    allApplied = setParameterValue(context, processor, kGlobalMix, 0.0f, caseName) && allApplied;
    return allApplied;
}

void fillInput(juce::AudioBuffer<float>& buffer, InputCase input) {
    buffer.clear();
    if (input == InputCase::silence)
        return;

    if (input == InputCase::impulse) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, 0, kImpulseAmplitude);
        return;
    }

    std::uint32_t state = kFixtureSeed;
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        state = state * kLcgMultiplier + kLcgIncrement;
        const auto normalized = static_cast<float>(state) /
                                static_cast<float>(std::numeric_limits<std::uint32_t>::max());
        const auto value =
            input == InputCase::extremeFinite
                ? (sample % 2 == 0 ? kExtremeFiniteMagnitude : -kExtremeFiniteMagnitude)
                : (normalized * 2.0f - 1.0f);
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
    }
}

void expectFinite(TestContext& context, const juce::AudioBuffer<float>& buffer,
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

void expectSilenceOutput(TestContext& context, const juce::AudioBuffer<float>& buffer,
                         const std::string& caseName) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        double sum = 0.0;
        float maximumMagnitude = 0.0f;
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            const auto value = buffer.getSample(channel, sample);
            sum += static_cast<double>(value);
            maximumMagnitude = std::max(maximumMagnitude, std::abs(value));
        }

        const auto measuredDc =
            buffer.getNumSamples() == 0 ? 0.0 : sum / static_cast<double>(buffer.getNumSamples());
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

bool configureBusLayout(TestContext& context, FRAZILAudioProcessor& processor, int channelCount,
                        const std::string& caseName) {
    const auto channelSet =
        channelCount == 1 ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo();
    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add(channelSet);
    layout.outputBuses.add(channelSet);
    const auto accepted = processor.setBusesLayout(layout);
    expect(context, accepted, caseName + ": mono/stereo bus layout accepted");
    return accepted;
}

void runPropertyCase(TestContext& context, double sampleRate, int blockSize, int channelCount,
                     const ParameterCase& parameterValues, InputCase input,
                     LifecycleCase lifecycle) {
    std::ostringstream caseName;
    caseName << "rate=" << sampleRate << ", block=" << blockSize << ", channels=" << channelCount
             << ", parameters="
             << (lifecycle == LifecycleCase::neutralPrepareProcessReprepareProcessExact
                     ? "neutral/dry"
                     : parameterValues.name)
             << ", input=" << inputName(input) << ", lifecycle=" << lifecycleName(lifecycle);
    const auto name = caseName.str();
    ++context.cases;

    FRAZILAudioProcessor processor;
    if (!configureBusLayout(context, processor, channelCount, name))
        return;
    if (!applyParameters(context, processor, parameterValues, name))
        return;
    if (lifecycle == LifecycleCase::neutralPrepareProcessReprepareProcessExact &&
        !applyNeutralDryFixture(context, processor, name))
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
        expect(context,
               buffer.getNumChannels() == originalChannels &&
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
    case LifecycleCase::prepareProcessReprepareProcess:
        processOnce("active initial process");
        processor.releaseResources();
        processor.prepareToPlay(sampleRate, blockSize);
        processOnce("active recovered process");
        break;
    case LifecycleCase::neutralPrepareProcessReprepareProcessExact: {
        const auto firstOutput = processOnce("neutral initial process");
        processor.releaseResources();
        processor.prepareToPlay(sampleRate, blockSize);
        const auto secondOutput = processOnce("neutral recovered process");
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
        expect(context,
               emptyBuffer.getNumChannels() == channelCount && emptyBuffer.getNumSamples() == 0,
               name + ": zero-length buffer dimensions remain valid");
        processOnce("process after zero-length callback");
        break;
    }
    }
}

void runShortAndOddCallbackCases(TestContext& context) {
    const auto& parameters = kParameterCases.front();
    for (const auto actualBlockSize : kShortAndOddCallbackSizes) {
        std::ostringstream caseName;
        caseName << "rate=" << kCanonicalSampleRateHz
                 << ", prepare nominal=" << kRepresentativeNominalBlockUpperBound
                 << ", callback=" << actualBlockSize << ", channels=" << kStereoChannelCount
                 << ", parameters=" << parameters.name
                 << ", input=deterministic-noise, lifecycle=single-prepare";
        const auto name = caseName.str();
        ++context.cases;

        FRAZILAudioProcessor processor;
        if (!configureBusLayout(context, processor, kStereoChannelCount, name))
            continue;
        if (!applyParameters(context, processor, parameters, name))
            continue;
        processor.prepareToPlay(kCanonicalSampleRateHz, kRepresentativeNominalBlockUpperBound);

        juce::AudioBuffer<float> buffer(kStereoChannelCount, actualBlockSize);
        fillInput(buffer, InputCase::deterministicNoise);
        juce::MidiBuffer midi;
        processor.processBlock(buffer, midi);
        expect(context,
               buffer.getNumChannels() == kStereoChannelCount &&
                   buffer.getNumSamples() == actualBlockSize,
               name + ": actual callback dimensions remain valid");
        expectFinite(context, buffer, name);
    }
}

std::vector<float> runDeterministicCase(TestContext& context, int blockSize,
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
    processor.prepareToPlay(kCanonicalSampleRateHz, blockSize);

    juce::AudioBuffer<float> buffer(kDeterministicFixtureChannels, blockSize);
    fillInput(buffer, InputCase::deterministicNoise);
    juce::MidiBuffer midi;
    processor.processBlock(buffer, midi);

    std::vector<float> result(static_cast<std::size_t>(kDeterministicFixtureChannels * blockSize));
    for (int channel = 0; channel < kDeterministicFixtureChannels; ++channel)
        for (int sample = 0; sample < blockSize; ++sample)
            result[static_cast<std::size_t>(channel * blockSize + sample)] =
                buffer.getSample(channel, sample);
    return result;
}

void testM1NeutralDeterministicOutput(TestContext& context) {
    const auto first =
        runDeterministicCase(context, kCanonicalBlockSizeSamples, "first fresh processor");
    const auto second =
        runDeterministicCase(context, kCanonicalBlockSizeSamples, "second fresh processor");
    const auto expectedSize =
        static_cast<std::size_t>(kDeterministicFixtureChannels * kCanonicalBlockSizeSamples);
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
    const auto& defaultParameters = kParameterCases.front();
    for (const auto sampleRate : kSampleRates)
        for (const auto blockSize : kRepresentativeNominalBlockSizes)
            for (const auto channelCount : kChannelCounts)
                runPropertyCase(context, sampleRate, blockSize, channelCount, defaultParameters,
                                InputCase::deterministicNoise, LifecycleCase::prepareAndProcess);

    for (const auto& parameterValues : kParameterCases) {
        runPropertyCase(context, kCanonicalSampleRateHz, kCanonicalBlockSizeSamples,
                        kStereoChannelCount, parameterValues, InputCase::deterministicNoise,
                        LifecycleCase::prepareAndProcess);
        runPropertyCase(context, kCanonicalSampleRateHz, kCanonicalBlockSizeSamples,
                        kStereoChannelCount, parameterValues, InputCase::extremeFinite,
                        LifecycleCase::prepareAndProcess);
    }

    for (const auto input : kInputCases)
        runPropertyCase(context, kCanonicalSampleRateHz, kCanonicalBlockSizeSamples,
                        kStereoChannelCount, defaultParameters, input,
                        LifecycleCase::prepareAndProcess);

    for (const auto lifecycle : kLifecycleCases)
        runPropertyCase(context, kCanonicalSampleRateHz, kCanonicalBlockSizeSamples,
                        kStereoChannelCount, defaultParameters, InputCase::deterministicNoise,
                        lifecycle);

    runShortAndOddCallbackCases(context);
    testM1NeutralDeterministicOutput(context);
    if (context.failures != 0)
        return 1;

    std::cout << "TEST-002 processor property harness passed (" << context.cases
              << " representative cases; nominal matrix plus parameter, input, lifecycle, "
                 "and short/odd callback sub-matrices)\n";
    return 0;
}
