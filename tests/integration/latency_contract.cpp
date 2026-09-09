#include "plugin/PluginProcessor.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {
constexpr int kBlockSize = 128;
constexpr float kComparisonTolerance = 1.0e-6f;

void printFailure(const std::string& message) {
    std::cerr << "ARCH-LAT-001 FAIL: " << message << '\n';
}

bool loadInput(const juce::File& inputFile, juce::AudioBuffer<float>& buffer, double& sampleRate) {
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(inputFile));
    if (reader == nullptr || reader->lengthInSamples <= 0 || reader->numChannels != 2 ||
        !std::isfinite(reader->sampleRate) || reader->sampleRate <= 0.0) {
        return false;
    }

    const auto length = static_cast<int>(reader->lengthInSamples);
    buffer.setSize(2, length, false, true, true);
    if (!reader->read(&buffer, 0, length, 0, true, true))
        return false;
    sampleRate = reader->sampleRate;
    return true;
}

int firstPeakSample(const juce::AudioBuffer<float>& buffer) {
    float peak = 0.0f;
    int peakSample = -1;
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            const auto magnitude = std::abs(buffer.getSample(channel, sample));
            if (magnitude > peak) {
                peak = magnitude;
                peakSample = sample;
            }
        }
    }
    return peakSample;
}
} // namespace

int main(int argc, char** argv) {
    if (argc != 3 || std::string(argv[1]) != "--input") {
        printFailure("usage: frazil_latency_contract --input <zero_state_response__impulse.wav>");
        return 2;
    }

    const juce::File inputFile{juce::String(argv[2])};
    juce::AudioBuffer<float> input;
    double sampleRate = 0.0;
    if (!inputFile.existsAsFile() || !loadInput(inputFile, input, sampleRate)) {
        printFailure("cannot load the canonical TESTDATA-001 impulse input");
        return 1;
    }

    FRAZILAudioProcessor processor;
    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add(juce::AudioChannelSet::stereo());
    layout.outputBuses.add(juce::AudioChannelSet::stereo());
    if (!processor.setBusesLayout(layout)) {
        printFailure("stereo bus layout was rejected");
        return 1;
    }
    processor.prepareToPlay(sampleRate, kBlockSize);

    if (processor.getLatencySamples() != 0) {
        printFailure("plugin reported non-zero processing latency");
        return 1;
    }
    if (processor.getTailLengthSeconds() != 0.0) {
        printFailure("M1 pass-through plugin reported an unexpected tail");
        return 1;
    }

    juce::AudioBuffer<float> output(2, input.getNumSamples());
    juce::AudioBuffer<float> block(2, kBlockSize);
    juce::MidiBuffer midi;
    float maximumError = 0.0f;
    for (int position = 0; position < input.getNumSamples(); position += kBlockSize) {
        const auto samplesThisBlock =
            std::min(kBlockSize, input.getNumSamples() - position);
        block.clear();
        for (int channel = 0; channel < 2; ++channel)
            block.copyFrom(channel, 0, input, channel, position, samplesThisBlock);

        processor.processBlock(block, midi);
        for (int channel = 0; channel < 2; ++channel) {
            for (int sample = 0; sample < samplesThisBlock; ++sample) {
                const auto value = block.getSample(channel, sample);
                if (!std::isfinite(value)) {
                    printFailure("impulse render produced NaN/Inf");
                    return 1;
                }
                output.setSample(channel, position + sample, value);
                maximumError = std::max(
                    maximumError, std::abs(value - input.getSample(channel, position + sample)));
            }
        }
    }

    if (maximumError > kComparisonTolerance) {
        printFailure("identity impulse response changed sample alignment; maximum error=" +
                     std::to_string(maximumError));
        return 1;
    }

    const auto inputPeakSample = firstPeakSample(input);
    const auto outputPeakSample = firstPeakSample(output);
    if (inputPeakSample != outputPeakSample) {
        printFailure("impulse peak moved from sample " + std::to_string(inputPeakSample) +
                     " to sample " + std::to_string(outputPeakSample));
        return 1;
    }

    std::cout << "ARCH-LAT-001 latency contract passed: reported=0 samples, tail=0 seconds, "
              << "impulse_peak_sample=" << outputPeakSample << ", maximum_error="
              << maximumError << '\n';
    return 0;
}
