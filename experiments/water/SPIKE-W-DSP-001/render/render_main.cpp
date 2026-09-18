#include "ReadConfig.h"
#include "dsp/ResearchBaseline.h"

#include <charconv>
#include <iostream>
#include <juce_audio_formats/juce_audio_formats.h>
#include <memory>
#include <string_view>

using namespace frazil::water::research;

namespace {
template <typename Integer> bool parse(std::string_view text, Integer& value) {
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

int render(int argc, char** argv) {
    if (argc < 6 || argc > 9) {
        std::cerr << "Usage: renderer input.wav NEW-output.wav mode block seed [config.json|-] "
                     "[tail-seconds] [NEW-protect-trace.csv]\n"
                     "Modes: baseline, residual (zero), a, b, d, ab, ad, bd, abd, c; append "
                     "-residual for E only.\n";
        return 2;
    }
    std::string_view mode(argv[3]);
    bool residualOnly = mode == "residual";
    if (mode.ends_with("-residual")) {
        residualOnly = true;
        mode.remove_suffix(9);
    }
    const bool baselineMode = mode == "baseline" || mode == "residual";
    if (!baselineMode && mode != "a" && mode != "b" && mode != "d" && mode != "ab" &&
        mode != "ad" && mode != "bd" && mode != "abd" && mode != "c")
        return 2;
    int blockSize{}, tailSeconds{};
    ResearchConfig config;
    if (!parse(argv[4], blockSize) || blockSize < 1 || blockSize > 8192 ||
        !parse(argv[5], config.baseSeed) ||
        (argc >= 8 && (!parse(argv[7], tailSeconds) || tailSeconds < 0 || tailSeconds > 30)))
        return 2;
    const auto cwd = juce::File::getCurrentWorkingDirectory();
    const auto input = cwd.getChildFile(argv[1]);
    const auto output = cwd.getChildFile(argv[2]);
    if (input == output || output.exists()) {
        std::cerr << "Output must be a new file, distinct from input.\n";
        return 2;
    }
    FluidConfig fluidConfig;
    ModalConfig modalConfig;
    ProtectRenderConfig protectConfig;
    if (argc >= 7 && std::string_view(argv[6]) != "-" &&
        !readConfig(cwd.getChildFile(argv[6]), fluidConfig, modalConfig, protectConfig)) {
        std::cerr << "Invalid research config\n";
        return 2;
    }
    fluidConfig.bubbleEnabled = mode.find('a') != std::string_view::npos;
    fluidConfig.dropletEnabled = mode.find('b') != std::string_view::npos;
    fluidConfig.flowEnabled = mode.find('d') != std::string_view::npos;
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
    FluidCandidate fluid;
    LiquidModalResonator modal;
    ResidualProtect protect;
    if (!protect.prepare(config.sampleRateHz, protectConfig.gain, protectConfig.depth))
        return 2;
    const bool prepared = baselineMode  ? baseline.prepare(config)
                          : mode == "c" ? modal.prepare(config, modalConfig)
                                        : fluid.prepare(config, fluidConfig);
    if (!prepared)
        return 2;
    // Optional diagnostic trace is offline-only, outside DSP and timing; refuse any overwrite.
    std::unique_ptr<juce::FileOutputStream> trace;
    if (argc == 9) {
        const auto traceFile = cwd.getChildFile(argv[8]);
        if (traceFile == output || traceFile == input || traceFile.exists())
            return 2;
        trace = traceFile.createOutputStream();
        if (!trace || !trace->writeText("frame,d0,d1_db,gr_db\n", false, false, "\n"))
            return 1;
    }
    const auto channels = static_cast<int>(reader->numChannels);
    const auto totalFrames =
        reader->lengthInSamples + static_cast<juce::int64>(tailSeconds * config.sampleRateHz);
    juce::AudioBuffer<float> buffer(channels, blockSize);
    std::unique_ptr<juce::OutputStream> stream = output.createOutputStream();
    if (!stream)
        return 1;
    auto options = juce::AudioFormatWriterOptions{}
                       .withSampleRate(reader->sampleRate)
                       .withNumChannels(channels);
    // Float candidate WAVs preserve peaks above full scale for analysis; no clipping/makeup.
    options = baselineMode ? options.withBitsPerSample(24)
                           : options.withBitsPerSample(32).withSampleFormat(
                                 juce::AudioFormatWriterOptions::SampleFormat::floatingPoint);
    auto writer = format.createWriterFor(stream, options);
    if (!writer)
        return 1;
    double peak{}, squareSum{}, dcSum{}, residualSquareSum{};
    // Offline observations only: never placed inside the DSP or timed callback harness.
    std::uint64_t bubbleSilentEvents{}, dropletSilentEvents{};
    juce::int64 bubbleFirstFrame{-1}, dropletFirstFrame{-1};
    for (juce::int64 start = 0; start < totalFrames; start += blockSize) {
        const auto count = static_cast<int>(std::min<juce::int64>(blockSize, totalFrames - start));
        buffer.clear();
        const auto fromInput = static_cast<int>(std::min<juce::int64>(
            count, std::max<juce::int64>(0, reader->lengthInSamples - start)));
        if (fromInput > 0 && !reader->read(&buffer, 0, fromInput, start, true, true))
            return 1;
        for (int sample = 0; sample < count; ++sample) {
            StereoFrame frame{buffer.getSample(0, sample),
                              channels == 2 ? buffer.getSample(1, sample) : 0.0f};
            for (float value : frame)
                if (!std::isfinite(value) || std::abs(value) > 1.0f)
                    return 1;
            StereoFrame effect{};
            const double gain = protect.processSource(frame);
            const auto beforeBubble = fluid.bubbleEvents();
            const auto beforeDroplet = fluid.dropletEvents();
            if (mode == "c")
                effect = ResidualProtect::apply(modal.process(frame), gain);
            else if (!baselineMode)
                effect =
                    applyFluidProtect(fluid.processComponents(frame), gain, protectConfig.topology);
            else if (!baseline.processResidual(frame, effect))
                return 1;
            if (trace) {
                const auto detection = protect.detection();
                const auto line = juce::String(start + sample) + "," +
                                  juce::String(detection.difference, 12) + "," +
                                  juce::String(detection.logRatioDb, 12) + "," +
                                  juce::String(protect.reductionDb(), 12) + "\n";
                if (!trace->writeText(line, false, false, "\n"))
                    return 1;
            }
            const auto newBubble = fluid.bubbleEvents() - beforeBubble;
            const auto newDroplet = fluid.dropletEvents() - beforeDroplet;
            if (frame == StereoFrame{}) {
                bubbleSilentEvents += newBubble;
                dropletSilentEvents += newDroplet;
            }
            if (newBubble && bubbleFirstFrame < 0)
                bubbleFirstFrame = start + sample;
            if (newDroplet && dropletFirstFrame < 0)
                dropletFirstFrame = start + sample;
            for (int channel = 0; channel < channels; ++channel) {
                const float value =
                    residualOnly ? effect[channel] : frame[channel] + effect[channel];
                if (!std::isfinite(value))
                    return 1;
                buffer.setSample(channel, sample, value);
                peak = std::max(peak, std::abs(static_cast<double>(value)));
                squareSum += static_cast<double>(value) * value;
                dcSum += value;
                residualSquareSum += static_cast<double>(effect[channel]) * effect[channel];
            }
        }
        if (!writer->writeFromAudioSampleBuffer(buffer, 0, count))
            return 1;
    }
    if (!writer->flush())
        return 1;
    if (trace) {
        trace->flush();
        if (trace->getStatus().failed())
            return 1;
    }
    const double samples = static_cast<double>(totalFrames) * channels;
    std::cout << "research mode=" << argv[3] << " seed=" << config.baseSeed
              << " frames=" << totalFrames << " rate=" << config.sampleRateHz
              << " block=" << blockSize << " peak=" << peak
              << " rms=" << std::sqrt(squareSum / samples) << " dc=" << dcSum / samples
              << " residual_rms=" << std::sqrt(residualSquareSum / samples)
              << " bubble_events=" << fluid.bubbleEvents()
              << " droplet_events=" << fluid.dropletEvents()
              << " bubble_silent_events=" << bubbleSilentEvents
              << " droplet_silent_events=" << dropletSilentEvents
              << " bubble_first_frame=" << bubbleFirstFrame
              << " droplet_first_frame=" << dropletFirstFrame << '\n';
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    return render(argc, argv);
}
