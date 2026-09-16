#include "dsp/ResearchBaseline.h"

#include <charconv>
#include <iostream>
#include <juce_audio_formats/juce_audio_formats.h>
#include <memory>
#include <string_view>
#include <vector>

using namespace frazil::water::research;

namespace {
template <typename Integer> bool parse(std::string_view text, Integer& value) {
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

int render(int argc, char** argv) {
    if (argc != 6) {
        std::cerr
            << "Usage: renderer <input.wav> <output.wav> <baseline|residual> <block> <seed>\n";
        return 2;
    }
    const std::string_view mode(argv[3]);
    int blockSize{};
    ResearchConfig config;
    if ((mode != "baseline" && mode != "residual") || !parse(argv[4], blockSize) || blockSize < 1 ||
        blockSize > 8192 || !parse(argv[5], config.baseSeed))
        return 2;

    const auto input = juce::File::getCurrentWorkingDirectory().getChildFile(argv[1]);
    const auto output = juce::File::getCurrentWorkingDirectory().getChildFile(argv[2]);
    if (input == output || output.exists()) {
        std::cerr << "Output must be a new file, distinct from input.\n";
        return 2;
    }
    auto inputStream = input.createInputStream();
    if (!inputStream)
        return 2;
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(
        format.createReaderFor(inputStream.release(), true));
    if (!reader || reader->numChannels < 1 || reader->numChannels > 2 ||
        reader->lengthInSamples <= 0)
        return 2;
    config.sampleRateHz = reader->sampleRate;
    ResearchBaseline baseline;
    if (!baseline.prepare(config))
        return 2;
    const auto channels = static_cast<int>(reader->numChannels);
    juce::AudioBuffer<float> buffer(channels, blockSize);
    std::vector<float> residual(static_cast<std::size_t>(blockSize));
    std::unique_ptr<juce::OutputStream> stream = output.createOutputStream();
    if (!stream)
        return 1;
    auto writer = format.createWriterFor(stream, juce::AudioFormatWriterOptions{}
                                                     .withSampleRate(reader->sampleRate)
                                                     .withNumChannels(channels)
                                                     .withBitsPerSample(24));
    if (!writer)
        return 1;
    for (juce::int64 start = 0; start < reader->lengthInSamples; start += blockSize) {
        const auto count =
            static_cast<int>(std::min<juce::int64>(blockSize, reader->lengthInSamples - start));
        if (!reader->read(&buffer, 0, count, start, true, true))
            return 1;
        for (int channel = 0; channel < channels; ++channel) {
            auto audio =
                std::span(buffer.getWritePointer(channel), static_cast<std::size_t>(count));
            for (const auto sample : audio)
                if (!std::isfinite(sample) || std::abs(sample) > 1.0f)
                    return 1; // PCM output must not silently clip invalid input.
            if (!baseline.processResidual(audio, std::span(residual).first(audio.size())))
                return 1;
            for (std::size_t i = 0; i < audio.size(); ++i)
                audio[i] = mode == "residual" ? residual[i] : audio[i] + residual[i];
        }
        if (!writer->writeFromAudioSampleBuffer(buffer, 0, count))
            return 1;
    }
    if (!writer->flush())
        return 1;
    std::cout << "LOCAL-WDSP-00 mode=" << mode << " seed=" << config.baseSeed
              << " samples=" << reader->lengthInSamples << " rate=" << config.sampleRateHz
              << " block=" << blockSize << " (no Water algorithm)\n";
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    return render(argc, argv);
}
