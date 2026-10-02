#include "A1BinningStudy.h"
#include "BubbleA1Descriptor.h"
#include "BubbleA1TraceJson.h"
#include "DropletB1Descriptor.h"
#include "DropletB2Descriptor.h"
#include "FlowD1Descriptor.h"
#include "ReadConfig.h"
#include "dsp/BubbleA1.h"
#include "dsp/DropletB1.h"
#include "dsp/DropletB2.h"
#include "dsp/FlowD1.h"
#include "dsp/ResearchBaseline.h"
#include "preview/PreviewEventTrace.h"

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
    const char* a1TracePath = nullptr;
    if (argc >= 3 && std::string_view(argv[1]) == "--a1-trace") {
        a1TracePath = argv[2];
        argc -= 2;
        argv += 2;
    }
    if (argc == 8 && std::string_view(argv[1]) == "--a1-binning") {
        int bins{};
        double minimum{}, gamma{};
        if (!parse(argv[5], bins) || !parse(argv[6], minimum) || !parse(argv[7], gamma))
            return 2;
        const auto cwd = juce::File::getCurrentWorkingDirectory();
        return a1BinningStudy(cwd.getChildFile(argv[2]), cwd.getChildFile(argv[3]),
                              cwd.getChildFile(argv[4]), bins, minimum, gamma);
    }
    if (argc == 2 && std::string_view(argv[1]) == "--describe-bubble-a1-v3") {
        std::cout << juce::JSON::toString(bubbleA1V3Descriptor()).toStdString() << '\n';
        return 0;
    }
    if (argc == 6 && std::string_view(argv[1]) == "--d1-path") {
        int rate{}, frames{};
        ResearchConfig c;
        if (!parse(argv[2], rate) || !parse(argv[3], frames) || frames < 1 ||
            frames > 150 * 96000 || !parse(argv[4], c.baseSeed))
            return 2;
        c.sampleRateHz = rate;
        FlowD1Trajectory path;
        if (!path.prepare(c, {}))
            return 2;
        const auto file = juce::File::getCurrentWorkingDirectory().getChildFile(argv[5]);
        if (file.exists())
            return 2;
        const auto stream = file.createOutputStream();
        if (!stream)
            return 1;
        for (int i = 0; i < frames; ++i)
            if (!stream->writeDouble(path.process()))
                return 1;
        stream->flush();
        return stream->getStatus().wasOk() ? 0 : 1;
    }
    if (argc == 2 && std::string_view(argv[1]) == "--describe-bubble-a1") {
        std::cout << juce::JSON::toString(bubbleA1Descriptor()).toStdString() << '\n';
        return 0;
    }
    if (argc == 2 && std::string_view(argv[1]) == "--describe-droplet-b2") {
        std::cout << juce::JSON::toString(dropletB2Descriptor()).toStdString() << '\n';
        return 0;
    }
    if (argc == 2 && std::string_view(argv[1]) == "--describe-droplet-b1") {
        std::cout << juce::JSON::toString(dropletB1Descriptor()).toStdString() << '\n';
        return 0;
    }
    if (argc == 2 && std::string_view(argv[1]) == "--describe-flow-d1") {
        std::cout << juce::JSON::toString(flowD1Descriptor()).toStdString() << '\n';
        return 0;
    }
    if (argc < 6 || argc > 13) {
        std::cerr << "Usage: renderer [--a1-trace NEW-events.jsonl] input.wav NEW-output.wav mode "
                     "block seed [config.json|-] "
                     "[tail-seconds] [NEW-protect-trace.csv|-] [raw|hard|softsign|tanh|feature] "
                     "[NEW-excitation.wav|-] [c0|c3] [independent|structured]\n"
                     "Modes: baseline, residual (zero), a, b, d, ab, ad, bd, abd, c; append "
                     "-residual for E only. Offline A1 modes: a1, a1b, a1d, a1bd. "
                     "B1 modes: b1, a1b1, a1b1d. Descriptors: --describe-bubble-a1, "
                     "--describe-droplet-b1, --describe-flow-d1. D1 modes: a1d1, b1d1, a1b1d1.\n";
        return 2;
    }
    std::string_view mode(argv[3]);
    bool residualOnly = mode == "residual";
    if (mode.ends_with("-residual")) {
        residualOnly = true;
        mode.remove_suffix(9);
    }
    const bool baselineMode = mode == "baseline" || mode == "residual";
    const bool b2Mode = mode == "b2" || mode == "a1b2" || mode == "b2d1" || mode == "a1b2d1";
    const bool d1Mode =
        mode == "b2d1" || mode == "a1b2d1" || mode == "a1d1" || mode == "b1d1" || mode == "a1b1d1";
    const bool b1Mode =
        mode == "b1" || mode == "a1b1" || mode == "a1b1d" || mode == "b1d1" || mode == "a1b1d1";
    const bool a1Mode = mode == "a1b2" || mode == "a1b2d1" || mode == "a1" || mode == "a1b" ||
                        mode == "a1d" || mode == "a1bd" || mode == "a1b1" || mode == "a1b1d" ||
                        mode == "a1d1" || mode == "a1b1d1";
    if (!baselineMode && mode != "a" && mode != "b" && mode != "d" && mode != "ab" &&
        mode != "ad" && mode != "bd" && mode != "abd" && mode != "c" && !a1Mode && !b1Mode &&
        !b2Mode)
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
    DropletB1RenderConfig b1Config;
    DropletB2RenderConfig b2Config;
    FlowD1RenderConfig d1Config;
    if (argc >= 7 && std::string_view(argv[6]) != "-" &&
        !readConfig(cwd.getChildFile(argv[6]), fluidConfig, modalConfig, &protectConfig, &a1Config,
                    &b1Config, d1Mode ? &d1Config : nullptr, &b2Config)) {
        std::cerr << "Invalid research config\n";
        return 2;
    }
    if ((!b2Mode && b2Config.supplied) || (!a1Mode && a1Config.supplied) ||
        (!b1Mode && b1Config.supplied) ||
        ((a1Mode || b1Mode || b2Mode) && protectConfig.depth != 0))
        return 2; // Explicit research model selection; Protect coupling remains deferred.
    fluidConfig.bubbleEnabled = !a1Mode && mode.find('a') != std::string_view::npos;
    fluidConfig.dropletEnabled = !b1Mode && !b2Mode && mode.find('b') != std::string_view::npos;
    fluidConfig.flowEnabled = !d1Mode && mode.find('d') != std::string_view::npos;
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
    auto b1 = b1Mode ? std::make_unique<DropletB1>() : nullptr;
    auto b2 = b2Mode ? std::make_unique<DropletB2>() : nullptr;
    FlowD1 d1;
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
    if (!prepared || (a1 && !a1->prepare(config, a1Config.bubble, a1Config.analysis)) ||
        (b1 && !b1->prepare(config, b1Config.droplet)) ||
        (b2 && !b2->prepare(config, b2Config.droplet)) ||
        (d1Mode && !d1.prepare(config, d1Config.flow)))
        return 2;
    // Optional diagnostic trace is offline-only, outside DSP and timing; refuse any overwrite.
    std::unique_ptr<juce::FileOutputStream> trace;
    if (argc >= 9 && std::string_view(argv[8]) != "-") {
        const auto traceFile = cwd.getChildFile(argv[8]);
        if (traceFile == output || traceFile == input || traceFile.exists() ||
            (captureExcitation && traceFile == cwd.getChildFile(argv[10])))
            return 2;
        trace = traceFile.createOutputStream();
        if (!trace || (!b2Mode && !trace->writeText("frame,d0,d1_db,gr_db\n", false, false, "\n")))
            return 1;
    }
    // Optional A1 observations use the SAME fixed transport as Preview; file writes happen
    // only after process() returns. Per-sample drain prevents event coalescing offline.
    std::unique_ptr<juce::FileOutputStream> a1Trace;
    std::unique_ptr<frazil::water::preview::PreviewEventTrace> a1Queue;
    if (a1TracePath) {
        const auto file = cwd.getChildFile(a1TracePath);
        if (!a1 || file.exists() || file == input || file == output ||
            (argc >= 9 && file == cwd.getChildFile(argv[8])) ||
            (captureExcitation && file == cwd.getChildFile(argv[10])))
            return 2;
        a1Queue = std::make_unique<frazil::water::preview::PreviewEventTrace>();
        a1Trace = file.createOutputStream();
        if (!a1Trace)
            return 1;
        a1->setObserver(a1Queue.get(), frazil::water::preview::PreviewEventTrace::captureA1);
    }
    const auto drainA1 = [&]() {
        frazil::water::preview::PreviewEventRecord record;
        while (a1Queue->pop(record))
            if (!a1Trace->writeText(
                    juce::JSON::toString(bubbleA1TraceJson(record.a1, config.sampleRateHz), true,
                                         17) +
                        "\n",
                    false, false, "\n"))
                return false;
        return a1Queue->takeDropped() == 0;
    };
    const auto channels = b2Mode ? 2 : static_cast<int>(reader->numChannels);
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
    double d1MinPath{}, d1MaxPath{}, d1Speed{}, d1Previous{}, d1CorrectionSquares{};
    double peak{}, squareSum{}, dcSum{}, residualSquareSum{};
    double flowMinimum{}, flowMaximum{}, flowTravel{}, previousFlowDelay{};
    bool observedFlow{};
    double a1RateSum{}, a1ActivitySum{}, a1EnergySum{}, a1ActiveSum{}, a1Peak{}, a1SquareSum{};
    std::size_t a1PeakActive{};
    double b1ActiveSum{};
    std::size_t b1PeakActive{};
    // Offline observations only: never placed inside the DSP or timed callback harness.
    std::uint64_t bubbleSilentEvents{}, dropletSilentEvents{};
    juce::int64 bubbleFirstFrame{-1}, dropletFirstFrame{-1}, a1RequestedFrame{-1};
    std::uint64_t a1SourceWindowActive{};
    juce::int64 b1FirstEligible{-1}, b1SourceOnset{-1}, b1Due{-1};
    std::uint64_t b1StartedEligibleId{};
    for (juce::int64 start = 0; start < totalFrames; start += blockSize) {
        const auto count = static_cast<int>(std::min<juce::int64>(blockSize, totalFrames - start));
        buffer.clear();
        const auto fromInput = static_cast<int>(std::min<juce::int64>(
            count, std::max<juce::int64>(0, reader->lengthInSamples - start)));
        if (fromInput > 0 && !reader->read(&buffer, 0, fromInput, start, true, true))
            return 1;
        for (int sample = 0; sample < count; ++sample) {
            StereoFrame frame{buffer.getSample(0, sample),
                              b2Mode && reader->numChannels == 1
                                  ? buffer.getSample(0, sample)
                                  : (channels == 2 ? buffer.getSample(1, sample) : 0.0f)};
            for (float value : frame)
                if (!std::isfinite(value) || std::abs(value) > 1.0f)
                    return 1;
            StereoFrame effect{};
            const double gain = protect.processSource(frame);
            const auto beforeB2Eligible = b2 ? b2->counters().eligible : 0;
            const auto beforeB2Admitted = b2 ? b2->counters().admitted : 0;
            const auto beforeB2Started = b2 ? b2->pool().counters().started : 0;
            const auto beforeA1Requested = a1 ? a1->requested() : 0;
            const auto beforeBubble = a1 ? a1->pool().counters().started : fluid.bubbleEvents();
            const auto beforeDroplet = b1 ? b1->pool().counters().started : fluid.dropletEvents();
            if (mode == "c")
                effect = ResidualProtect::apply(modal.process(frame), gain);
            else if (!baselineMode) {
                auto components = fluid.processComponents(frame);
                if (a1) {
                    components.bubble = a1->process(frame);
                    if (a1Trace && !drainA1())
                        return 1;
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
                if (b1) {
                    components.droplet = b1->process(frame);
                    b1ActiveSum += b1->pool().active();
                    b1PeakActive = std::max(b1PeakActive, b1->pool().active());
                }
                if (b2)
                    components.droplet = b2->process(frame);
                if (d1Mode) {
                    const auto transferred = d1.process(components.sum());
                    const double path = d1.pathMeters();
                    d1MinPath = std::min(d1MinPath, path);
                    d1MaxPath = std::max(d1MaxPath, path);
                    d1Speed = std::max(d1Speed, std::abs(path - d1Previous) * config.sampleRateHz);
                    d1Previous = path;
                    for (std::size_t ch = 0; ch < 2; ++ch) {
                        const double value = transferred.transferred[ch];
                        if (!std::isfinite(value) ||
                            std::abs(value) > std::numeric_limits<float>::max())
                            return 1;
                        effect[ch] = static_cast<float>(value);
                        d1CorrectionSquares +=
                            transferred.correction[ch] * transferred.correction[ch];
                    }
                } else {
                    effect = applyFluidProtect(components, gain, protectConfig.topology);
                }
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
            if (trace && b2) {
                const auto writeEvent = [&](const DropletB2Event& e, bool eligible, bool started) {
                    auto* row = new juce::DynamicObject;
                    row->setProperty("frame", start + sample);
                    row->setProperty("eligible", eligible);
                    row->setProperty("admitted",
                                     started || b2->counters().admitted != beforeB2Admitted);
                    row->setProperty("started", started);
                    row->setProperty("eligibleId", static_cast<juce::int64>(e.center.eligibleId));
                    row->setProperty("noveltyDb", e.center.impact.onsetStrength);
                    row->setProperty("positiveSlope", e.positiveSlope);
                    row->setProperty("sourceExcitation", e.sourceExcitation);
                    row->setProperty("mappedExcitation", e.mappedExcitation);
                    row->setProperty("radiusMm", 1000 * e.center.physics.equivalentRadiusMeters);
                    const auto frequency = e.center.physics.frequencyHz;
                    const auto maxFrequency =
                        e.center.riseXi > 0
                            ? std::min(std::sqrt(2.) * frequency, .45 * config.sampleRateHz)
                            : frequency;
                    const auto cents = DropletB2SpatialRenderer::boundedCents(
                        maxFrequency, e.detuneCents, e.maximumBeatHz);
                    const auto pair =
                        DropletB2SpatialRenderer::frequencies(frequency, cents, e.polarity);
                    row->setProperty("centerFrequencyHz", frequency);
                    row->setProperty("detuneLeftCents", -e.polarity * cents);
                    row->setProperty("detuneRightCents", e.polarity * cents);
                    row->setProperty("renderFrequencyL", pair[0]);
                    row->setProperty("renderFrequencyR", pair[1]);
                    const auto amplitude =
                        e.center.physics.renderAmplitudeScale * e.mappedExcitation * e.center.gain;
                    row->setProperty("renderAmplitudeL", amplitude * e.center.impact.carrier[0]);
                    row->setProperty("renderAmplitudeR", amplitude * e.center.impact.carrier[1]);
                    return trace->writeText(juce::JSON::toString(juce::var(row), true, 17) + "\n",
                                            false, false, "\n");
                };
                if (b2->counters().eligible != beforeB2Eligible &&
                    !writeEvent(b2->lastEligible(), true, false))
                    return 1;
                if (b2->pool().counters().started != beforeB2Started &&
                    !writeEvent(b2->pool().lastStarted(), false, true))
                    return 1;
            }
            if (trace && !b2) {
                const auto detection = protect.detection();
                const auto line = juce::String(start + sample) + "," +
                                  juce::String(detection.difference, 12) + "," +
                                  juce::String(detection.logRatioDb, 12) + "," +
                                  juce::String(protect.reductionDb(), 12) + "\n";
                if (!trace->writeText(line, false, false, "\n"))
                    return 1;
            }
            if (a1) {
                a1SourceWindowActive += a1->excitation().activity > 0 ? 1u : 0u;
                if (a1->requested() > beforeA1Requested && a1RequestedFrame < 0)
                    a1RequestedFrame = start + sample;
            }
            const auto newBubble =
                (a1 ? a1->pool().counters().started : fluid.bubbleEvents()) - beforeBubble;
            const auto newDroplet =
                (b1 ? b1->pool().counters().started : fluid.dropletEvents()) - beforeDroplet;
            if (frame == StereoFrame{}) {
                bubbleSilentEvents += newBubble;
                dropletSilentEvents += newDroplet;
            }
            if (newBubble && bubbleFirstFrame < 0)
                bubbleFirstFrame = start + sample;
            if (b1 && b1FirstEligible < 0 && b1->counters().eligible)
                b1FirstEligible = static_cast<juce::int64>(b1->lastEligible().impact.sourceSample);
            if (newDroplet && dropletFirstFrame < 0) {
                dropletFirstFrame = start + sample;
                if (b1) {
                    // Pair captured source/due with the first observed initialization frame.
                    // A zero current input frame says nothing about this event's causality.
                    const auto& event = b1->pool().lastStarted();
                    b1SourceOnset = static_cast<juce::int64>(event.impact.sourceSample);
                    b1Due = static_cast<juce::int64>(event.dueSample);
                    b1StartedEligibleId = event.eligibleId;
                }
            }
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
    if (a1Trace) {
        for (std::size_t band = 0; band < 7; ++band)
            frazil::water::preview::PreviewEventTrace::captureA1(a1Queue.get(),
                                                                 a1->pool().bandObservation(band));
        if (!drainA1())
            return 1;
        a1Trace->flush();
        if (a1Trace->getStatus().failed())
            return 1;
    }
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
              << " droplet_events=" << (b1 ? b1->pool().counters().started : fluid.dropletEvents())
              << " flow_final_delay_samples=" << fluid.flowDelaySamples()
              << " flow_input_min_samples=" << flowMinimum
              << " flow_input_max_samples=" << flowMaximum
              << " flow_input_travel_samples=" << flowTravel
              << " bubble_silent_events=" << bubbleSilentEvents
              << " droplet_silent_events=" << dropletSilentEvents
              << " bubble_first_frame=" << bubbleFirstFrame
              << " droplet_first_frame=" << dropletFirstFrame << '\n';
    if (d1Mode) {
        std::cout << "flow_model=D1 d1_min_path_m=" << d1MinPath << " d1_max_path_m=" << d1MaxPath
                  << " d1_max_speed_mps=" << d1Speed
                  << " d1_correction_rms=" << std::sqrt(d1CorrectionSquares / (2 * totalFrames))
                  << " d1_drain_samples=" << d1.drainSamples() << '\n';
    }
    if (a1) {
        const auto& p = a1->pool();
        const auto& counters = p.counters();
        // Frame fields are FIRST request/start; window-active counts eligible frames.
        // Legacy silent-events counts zero CURRENT frames, not unexcited windows.
        std::cout << "a1_requested_frame=" << a1RequestedFrame
                  << " a1_started_frame=" << bubbleFirstFrame
                  << " a1_start_on_zero_current_frame=" << bubbleSilentEvents
                  << " a1_source_window_active=" << a1SourceWindowActive << '\n';
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
    if (b2)
        std::cout << "b2_eligible=" << b2->counters().eligible
                  << " b2_admitted=" << b2->counters().admitted
                  << " b2_started=" << b2->pool().counters().started << '\n';
    if (b1) {
        const auto& c = b1->counters();
        const auto& v = b1->pool().counters();
        const auto& p = b1->physics();
        const auto& last = b1->pool().lastStarted();
        // Existing droplet_* and physicalAmplitudeScale outputs are deprecated B1 aliases.
        std::cout << "b1_first_eligible_frame=" << b1FirstEligible
                  << " b1_first_source_onset_frame=" << b1SourceOnset
                  << " b1_first_due_frame=" << b1Due
                  << " b1_first_started_frame=" << dropletFirstFrame
                  << " b1_first_started_eligible_id=" << b1StartedEligibleId
                  << " b1_start_on_zero_current_frame=" << dropletSilentEvents << '\n';
        std::cout << "droplet_model=B1 eligible=" << c.eligible << " admitted=" << c.admitted
                  << " queued=" << c.queued << " started=" << v.started
                  << " rejectedByAdmission=" << c.rejectedByAdmission
                  << " droppedByPendingCapacity=" << c.droppedByPendingCapacity
                  << " droppedByVoiceCapacity=" << v.droppedByVoiceCapacity
                  << " steals=" << v.steals << " completed=" << v.completed
                  << " pending_peak=" << c.pendingPeak
                  << " active_mean=" << b1ActiveSum / totalFrames << " active_peak=" << b1PeakActive
                  << " frequency_hz=" << p.frequencyHz
                  << " damping_per_second=" << p.dampingPerSecond
                  << " relativeFormationAmplitudeScale=" << p.relativeFormationAmplitudeScale
                  << " physicalAmplitudeScale=" << p.relativeFormationAmplitudeScale
                  << " renderAmplitudeScale=" << p.renderAmplitudeScale
                  << " lastStartedSourceExcitation=" << last.impact.sourceExcitation
                  << " lastStartedRenderAmplitude="
                  << last.physics.renderAmplitudeScale * last.impact.sourceExcitation * last.gain
                  << '\n';
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
