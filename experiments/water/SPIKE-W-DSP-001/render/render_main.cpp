#include "ReadConfig.h"
#include "dsp/BubbleA1.h"
#include "dsp/ResearchBaseline.h"

#include <charconv>
#include <iomanip>
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
    if (argc < 6 || argc > 13) {
        std::cerr << "Usage: renderer input.wav NEW-output.wav mode block seed [config.json|-] "
                     "[tail-seconds] [NEW-protect-trace.csv|-] [raw|hard|softsign|tanh|feature] "
                     "[NEW-excitation.wav|-] [c0|c3] [independent|structured]\n"
                     "Modes: baseline, residual (zero), a, b, d, ab, ad, bd, abd, c; append "
                     "-residual for E only. Offline A1 modes: a1, a1b, a1d, a1bd.\n";
        return 2;
    }
    std::string_view mode(argv[3]);
    bool residualOnly = mode == "residual";
    if (mode.ends_with("-residual")) {
        residualOnly = true;
        mode.remove_suffix(9);
    }
    const bool baselineMode = mode == "baseline" || mode == "residual";
    const bool a1Mode = mode == "a1" || mode == "a1b" || mode == "a1d" || mode == "a1bd";
    if (!baselineMode && mode != "a" && mode != "b" && mode != "d" && mode != "ab" &&
        mode != "ad" && mode != "bd" && mode != "abd" && mode != "c" && !a1Mode)
        return 2;
    ModalExcitation excitationMode{ModalExcitation::raw};
    if (argc >= 10 && (mode != "c" || !parseModalExcitation(argv[9], excitationMode)))
        return 2;
    ModalNormalization normalization{ModalNormalization::c0};
    if (argc >= 12) {
        if (mode != "c" ||
            (std::string_view(argv[11]) != "c0" && std::string_view(argv[11]) != "c3"))
            return 2;
        normalization =
            std::string_view(argv[11]) == "c3" ? ModalNormalization::c3 : ModalNormalization::c0;
    }
    ModalMotionModel motionModel{ModalMotionModel::independent};
    if (argc == 13) {
        if (mode != "c" || (std::string_view(argv[12]) != "independent" &&
                            std::string_view(argv[12]) != "structured"))
            return 2;
        motionModel = std::string_view(argv[12]) == "structured" ? ModalMotionModel::structured
                                                                 : ModalMotionModel::independent;
    }
    const bool captureExcitation = argc >= 11 && std::string_view(argv[10]) != "-";
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
    if (captureExcitation) {
        const auto excitationFile = cwd.getChildFile(argv[10]);
        if (excitationFile == input || excitationFile == output || excitationFile.exists() ||
            (std::string_view(argv[8]) != "-" && excitationFile == cwd.getChildFile(argv[8])))
            return 2;
    }
    FluidConfig fluidConfig;
    ModalConfig modalConfig;
    ProtectRenderConfig protectConfig;
    BubbleA1RenderConfig a1Config;
    if (argc >= 7 && std::string_view(argv[6]) != "-" &&
        !readConfig(cwd.getChildFile(argv[6]), fluidConfig, modalConfig, &protectConfig,
                    &a1Config)) {
        std::cerr << "Invalid research config\n";
        return 2;
    }
    if ((!a1Mode && a1Config.supplied) || (a1Mode && protectConfig.depth != 0))
        return 2; // Explicit model selection, raw A1 sum only in this experiment.
    fluidConfig.bubbleEnabled = !a1Mode && mode.find('a') != std::string_view::npos;
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
    // Fixed DSP storage is allocated once here, outside the processing loop and Windows stack.
    auto a1 = a1Mode ? std::make_unique<BubbleA1>() : nullptr;
    LiquidModalResonator modal;
    // Explicit CLI comparison options override module fields; omission preserves typed config.
    if (argc < 10)
        excitationMode = modalConfig.excitation;
    if (argc < 12)
        normalization = modalConfig.normalization;
    if (argc < 13)
        motionModel = modalConfig.motionModel;
    ResidualProtect protect;
    if (!protect.prepare(config.sampleRateHz, protectConfig.gain, protectConfig.depth))
        return 2;
    const bool prepared = baselineMode  ? baseline.prepare(config)
                          : mode == "c" ? modal.prepare(config, modalConfig, excitationMode,
                                                        normalization, motionModel)
                                        : fluid.prepare(config, fluidConfig);
    if (!prepared || (a1 && !a1->prepare(config, a1Config.bubble, a1Config.analysis)))
        return 2;
    // Optional diagnostic trace is offline-only, outside DSP and timing; refuse any overwrite.
    std::unique_ptr<juce::FileOutputStream> trace;
    if (argc >= 9 && std::string_view(argv[8]) != "-") {
        const auto traceFile = cwd.getChildFile(argv[8]);
        if (traceFile == output || traceFile == input || traceFile.exists() ||
            (captureExcitation && traceFile == cwd.getChildFile(argv[10])))
            return 2;
        trace = traceFile.createOutputStream();
        if (!trace || !trace->writeText("frame,d0,d1_db,gr_db\n", false, false, "\n"))
            return 1;
    }
    const auto channels = static_cast<int>(reader->numChannels);
    const auto totalFrames =
        reader->lengthInSamples + static_cast<juce::int64>(tailSeconds * config.sampleRateHz);
    juce::AudioBuffer<float> buffer(channels, blockSize), excitationBuffer(channels, blockSize);
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
    // Research-only diagnostic: the actual common modal-bank driver, before weight distribution.
    // This never changes the residual, dry carrier, module JSON or Host/session state.
    std::unique_ptr<juce::AudioFormatWriter> excitationWriter;
    if (captureExcitation) {
        const auto excitationFile = cwd.getChildFile(argv[10]);
        if (excitationFile == input || excitationFile == output || excitationFile.exists())
            return 2;
        std::unique_ptr<juce::OutputStream> excitationStream = excitationFile.createOutputStream();
        if (!excitationStream)
            return 1;
        excitationWriter = format.createWriterFor(excitationStream, options);
        if (!excitationWriter)
            return 1;
    }
    double peak{}, squareSum{}, dcSum{}, residualSquareSum{};
    double flowMinimum{}, flowMaximum{}, flowTravel{}, previousFlowDelay{};
    bool observedFlow{};
    double a1RateSum{}, a1ActivitySum{}, a1EnergySum{}, a1ActiveSum{}, a1Peak{}, a1SquareSum{};
    std::size_t a1PeakActive{};
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
            const auto beforeBubble = a1 ? a1->pool().counters().started : fluid.bubbleEvents();
            const auto beforeDroplet = fluid.dropletEvents();
            if (mode == "c")
                effect = ResidualProtect::apply(modal.process(frame), gain);
            else if (!baselineMode) {
                auto components = fluid.processComponents(frame);
                if (a1) {
                    components.bubble = a1->process(frame);
                    a1RateSum += a1->requestedRate();
                    a1ActivitySum += a1->excitation().activity;
                    a1EnergySum += a1->excitation().fastPower;
                    a1ActiveSum += a1->pool().active();
                    a1PeakActive = std::max(a1PeakActive, a1->pool().active());
                    for (float x : components.bubble) {
                        a1Peak = std::max(a1Peak, std::abs(double(x)));
                        a1SquareSum += double(x) * x;
                    }
                }
                effect = applyFluidProtect(components, gain, protectConfig.topology);
            } else if (!baseline.processResidual(frame, effect))
                return 1;
            if (excitationWriter)
                for (int channel = 0; channel < channels; ++channel)
                    excitationBuffer.setSample(channel, sample, modal.excitationFrame()[channel]);
            // Input-window trajectory observation only; a final tail value returns to base delay
            // and cannot describe Motion. This work is outside the realtime/timing harness.
            if (!baselineMode && mode != "c" && fluidConfig.flowEnabled &&
                start + sample < reader->lengthInSamples) {
                const auto delay = fluid.flowDelaySamples();
                if (!observedFlow) {
                    flowMinimum = flowMaximum = delay;
                    observedFlow = true;
                } else {
                    flowMinimum = std::min(flowMinimum, delay);
                    flowMaximum = std::max(flowMaximum, delay);
                    flowTravel += std::abs(delay - previousFlowDelay);
                }
                previousFlowDelay = delay;
            }
            if (trace) {
                const auto detection = protect.detection();
                const auto line = juce::String(start + sample) + "," +
                                  juce::String(detection.difference, 12) + "," +
                                  juce::String(detection.logRatioDb, 12) + "," +
                                  juce::String(protect.reductionDb(), 12) + "\n";
                if (!trace->writeText(line, false, false, "\n"))
                    return 1;
            }
            const auto newBubble =
                (a1 ? a1->pool().counters().started : fluid.bubbleEvents()) - beforeBubble;
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
        if (excitationWriter &&
            !excitationWriter->writeFromAudioSampleBuffer(excitationBuffer, 0, count))
            return 1;
        if (!writer->writeFromAudioSampleBuffer(buffer, 0, count))
            return 1;
    }
    if (excitationWriter && !excitationWriter->flush())
        return 1;
    if (!writer->flush())
        return 1;
    if (trace) {
        trace->flush();
        if (trace->getStatus().failed())
            return 1;
    }
    const double samples = static_cast<double>(totalFrames) * channels;
    std::cout << std::setprecision(17);
    std::cout << "research mode=" << argv[3] << " seed=" << config.baseSeed
              << " frames=" << totalFrames << " rate=" << config.sampleRateHz
              << " block=" << blockSize << " excitation=" << modalExcitationName(excitationMode)
              << " peak=" << peak << " rms=" << std::sqrt(squareSum / samples)
              << " dc=" << dcSum / samples
              << " residual_rms=" << std::sqrt(residualSquareSum / samples)
              << " bubble_events=" << (a1 ? a1->pool().counters().started : fluid.bubbleEvents())
              << " droplet_events=" << fluid.dropletEvents()
              << " flow_final_delay_samples=" << fluid.flowDelaySamples()
              << " flow_input_min_samples=" << flowMinimum
              << " flow_input_max_samples=" << flowMaximum
              << " flow_input_travel_samples=" << flowTravel
              << " bubble_silent_events=" << bubbleSilentEvents
              << " droplet_silent_events=" << dropletSilentEvents
              << " bubble_first_frame=" << bubbleFirstFrame
              << " droplet_first_frame=" << dropletFirstFrame << '\n';
    if (a1) {
        const auto& p = a1->pool();
        const auto& counters = p.counters();
        std::cout << "bubble_model=A1 requested=" << a1->requested()
                  << " accepted=" << counters.accepted << " started=" << counters.started
                  << " capacity_drops=" << counters.capacityDrops << " steals=" << counters.steals
                  << " requested_rate_mean=" << a1RateSum / totalFrames
                  << " accepted_rate=" << counters.accepted * config.sampleRateHz / totalFrames
                  << " active_mean=" << a1ActiveSum / totalFrames << " active_peak=" << a1PeakActive
                  << " capacity=" << p.capacity()
                  << " utilization=" << a1ActiveSum / totalFrames / p.capacity()
                  << " activity_mean=" << a1ActivitySum / totalFrames
                  << " source_energy_mean=" << a1EnergySum / totalFrames
                  << " population_gamma=" << a1Config.bubble.populationGamma << " rising_fraction="
                  << (counters.started ? double(counters.rising) / counters.started : 0)
                  << " a1_peak=" << a1Peak
                  << " a1_rms=" << std::sqrt(a1SquareSum / (2 * totalFrames)) << " radius_hist=";
        for (std::size_t i = 0; i < 128; ++i)
            std::cout << (i ? "," : "") << counters.radiusHistogram[i];
        std::cout << " lifetime_hist=";
        for (std::size_t i = 0; i < 128; ++i)
            std::cout << (i ? "," : "") << counters.lifetimeHistogram[i];
        std::cout << '\n';
    }
    if (mode == "c") {
        const auto& readout = modal.normalizationReadout();
        std::cout << "modal_normalization="
                  << (normalization == ModalNormalization::c3 ? "c3" : "c0") << " modal_motion="
                  << (motionModel == ModalMotionModel::structured ? "structured" : "independent")
                  << " modal_bound=" << readout.residualBound
                  << " modal_energy_scale=" << readout.energyScale
                  << " modal_safety_scale=" << readout.safetyScale << " modal_excitation=";
        for (std::size_t i = 0; i < readout.excitation.size(); ++i)
            std::cout << (i ? "," : "") << readout.excitation[i];
        std::cout << '\n';
    }
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    return render(argc, argv);
}
