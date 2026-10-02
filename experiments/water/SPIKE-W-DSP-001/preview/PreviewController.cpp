#include "PreviewController.h"

#include "AuditionMonitor.h"
#include "MonitorOverRange.h"
#include "PreviewEngine.h"
#include "PreviewMonitorResampler.h"
#include "PreviewSessionLogger.h"
#include "ResearchCoreConfigCodec.h"
#include "render/BubbleA1TraceJson.h"

#include <algorithm>
#include <atomic>
#include <cmath>

namespace frazil::water::preview {
class PreviewController::Impl final : public juce::AudioIODeviceCallback, private juce::Timer {
  public:
    Impl() {
        startTimer(100);
    }
    ~Impl() override {
        stopTimer();
        stop();
        timerCallback();
    }

    void stop() {
        // removeAudioCallback waits for any in-flight callback on this non-realtime caller.
        // Source replacement and prepare are safe only after it returns.
        device.removeAudioCallback(this);
        callbackAttached = false;
        isPlaying.store(false);
        protectMetrics.clear();
        timerCallback();
        engine.traceA1Bands(eventTrace);
        timerCallback();
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* audioDevice) override {
        const auto rate = audioDevice->getCurrentSampleRate();
        deviceRate.store(rate);
        mismatch.store(!resampler.prepare(sourceRate, rate));
        engine.reset();
        frame = 0;
        position.store(0);
        ended.store(false);
        audition.prepareDiagnostics(sourceRate, monitor.load(), auditionGain.load(),
                                    diagnosticSelection.load());
        metrics.setPrepared(static_cast<float>(rate), audioDevice->getCurrentBufferSizeSamples(),
                            2);
    }

    void audioDeviceStopped() override {
        isPlaying.store(false);
        metrics.reset();
    }

    void audioDeviceIOCallbackWithContext(const float* const*, int, float* const* outputs,
                                          int outputChannels, int count,
                                          const juce::AudioIODeviceCallbackContext&) override {
        juce::ScopedNoDenormals noDenormals;
        for (int channel = 0; channel < outputChannels; ++channel)
            if (outputs[channel] != nullptr)
                juce::FloatVectorOperations::clear(outputs[channel], count);
        if (mismatch.load() || ended.load() || source.getNumSamples() == 0)
            return;
        const auto mode = monitor.load();
        audition.setDiagnosticTargets(mode, outputGain.load(), auditionGain.load(),
                                      diagnosticSelection.load());
        engine.setProtectDepth(protectDepth.load(std::memory_order_relaxed));
        constexpr int channels = 2;
        int dspFrames{};
        float inputPeak{}, outputPeak{};
        double inputSquares{}, outputSquares{};
        bool finite = true;
        ProtectBlockReadout protectBlock;
        const auto endFrame = static_cast<std::uint64_t>(source.getNumSamples()) +
                              static_cast<std::uint64_t>(30.0 * sourceRate);
        resampler.render(
            outputs, outputChannels, count, [&](research::StereoFrame& output) noexcept {
                if (frame >= endFrame)
                    return false;
                research::StereoFrame input{};
                if (frame < static_cast<std::uint64_t>(source.getNumSamples())) {
                    const auto n = static_cast<int>(frame);
                    input =
                        canonicalStereo(source.getSample(0, n),
                                        source.getSample(source.getNumChannels() == 1 ? 0 : 1, n),
                                        source.getNumChannels());
                }
                // DSP always advances during Dry monitoring. Replaying, not toggling Dry, resets
                // seed.
                const auto residual = engine.residual(input);
                engine.traceEvents(eventTrace, frame);
                protectBlock.water.include(engine.waterReadout(), channels);
                protectBlock.latest = engine.protectReadout();
                protectBlock.peak.includePeak(protectBlock.latest);
                output = audition.processDiagnostics(input, residual, engine.diagnosticFrames());
                for (int channel = 0; channel < channels; ++channel) {
                    const auto i = static_cast<std::size_t>(channel);
                    finite = finite && std::isfinite(output[i]);
                    inputPeak = std::max(inputPeak, std::abs(input[i]));

                    inputSquares += static_cast<double>(input[i]) * input[i];
                }
                ++frame;
                ++dspFrames;
                return true;
            });
        ended.store(resampler.finished());
        for (int c = 0; c < std::min(2, outputChannels); ++c)
            if (outputs[c])
                for (int n = 0; n < count; ++n) {
                    const float value = outputs[c][n];
                    finite = finite && std::isfinite(value);
                    outputPeak = std::max(outputPeak, std::abs(value));
                    outputSquares += static_cast<double>(value) * value;
                }
        position.store(frame);
        protectBlock.water.latest = engine.waterActivity();
        protectMetrics.publish(protectBlock);
        const auto denominator = static_cast<double>(std::max(1, channels * count));
        overRange.observe(outputPeak);
        metrics.publish(
            count, channels, inputPeak, outputPeak,
            static_cast<float>(std::sqrt(inputSquares / std::max(1, channels * dspFrames))),
            static_cast<float>(std::sqrt(outputSquares / denominator)), finite);
    }

    juce::var rateFields() const {
        auto* fields = new juce::DynamicObject;
        const auto rate = deviceRate.load();
        fields->setProperty("sourceRate", sourceRate);
        fields->setProperty("dspRate", preparedRate);
        fields->setProperty("deviceRate", rate);
        fields->setProperty("resamplingActive", rate > 0 && std::abs(rate - preparedRate) > .5);
        fields->setProperty("resampleRatio", rate > 0 ? preparedRate / rate : 0);
        fields->setProperty("originalSourceChannels", source.getNumChannels());
        fields->setProperty("dspChannels", 2);
        return juce::var(fields);
    }
    juce::String failure(const char* stage, const juce::String& message) {
        auto fields = rateFields();
        fields.getDynamicObject()->setProperty("stage", stage);
        fields.getDynamicObject()->setProperty("message", message);
        logger.write("error", fields);
        return message;
    }
    void timerCallback() override {
        PreviewEventRecord r;
        // Bound each message-thread drain even if a busy producer keeps refilling it.
        for (std::size_t n = 0; n < PreviewEventTrace::kCapacity && eventTrace.pop(r); ++n) {
            if (r.module == 1) {
                logger.write("dsp_event", research::bubbleA1TraceJson(r.a1, preparedRate));
                continue;
            }
            auto* fields = new juce::DynamicObject;
            fields->setProperty("module", r.module == 1 ? "A1" : "B2");
            fields->setProperty("frame", static_cast<juce::int64>(r.frame));
            fields->setProperty("eligibleId", static_cast<juce::int64>(r.eligibleId));
            fields->setProperty("bin", r.bin);
            fields->setProperty("requested", r.requested);
            fields->setProperty("eligible", r.eligible);
            fields->setProperty("admitted", r.admitted);
            fields->setProperty("started", r.started);
            fields->setProperty("startCount", static_cast<juce::int64>(r.startCount));
            fields->setProperty("noveltyDb", r.noveltyDb);
            fields->setProperty("positiveSlope", r.positiveSlope);
            fields->setProperty("sourceExcitation", r.sourceExcitation);
            fields->setProperty("mappedExcitation", r.mappedExcitation);
            fields->setProperty("radiusMm", r.radiusMm);
            fields->setProperty("centerFrequencyHz", r.centerFrequencyHz);
            fields->setProperty("depthExcitationProxy", r.depthExcitationProxy);
            fields->setProperty("detuneLeftCents", r.detuneLeftCents);
            fields->setProperty("detuneRightCents", r.detuneRightCents);
            fields->setProperty("renderFrequencyL", r.renderFrequencyL);
            fields->setProperty("renderFrequencyR", r.renderFrequencyR);
            fields->setProperty("renderAmplitudeL", r.renderAmplitudeL);
            fields->setProperty("renderAmplitudeR", r.renderAmplitudeR);
            fields->setProperty("pathMeters", r.pathMeters);
            logger.write("dsp_event", juce::var(fields));
        }
        if (const auto dropped = eventTrace.takeDropped())
            logger.write("trace_overflow", static_cast<juce::int64>(dropped));
    }
    PreviewEventTrace eventTrace;
    PreviewSessionLogger logger;
    PreviewMonitorResampler resampler;
    std::atomic<double> deviceRate{};
    juce::AudioDeviceManager device;
    // Replaced only with callback detached; read-only while playing. Bounded to 120 seconds.
    juce::AudioBuffer<float> source;
    double sourceRate{};
    juce::String sourceName;
    PreviewEngine engine;
    plugin::DeveloperDiagnostics metrics;
    ProtectDiagnostics protectMetrics;
    // Audio owner: source cursor in frames, and 10 ms monitor-only crossfade/gain state.
    std::uint64_t frame{};
    AuditionMonitor audition;
    MonitorOverRange overRange;
    std::atomic<std::uint64_t> position{};
    std::atomic<bool> isPlaying{}, ended{}, mismatch{};
    std::atomic<DiagnosticSignal> diagnosticSelection{DiagnosticSignal::none};
    std::atomic<MonitorMode> monitor{MonitorMode::processed};
    std::atomic<float> outputGain{0.12589254f}, auditionGain{7.94328235f};
    // Sole UI->audio Protect transport: validated normalized target, sampled at block boundary.
    std::atomic<double> protectDepth{};
    bool callbackAttached{}, prepared{}; // Message-thread lifecycle only.
    double preparedRate{};
    static_assert(std::atomic<bool>::is_always_lock_free);
    static_assert(std::atomic<double>::is_always_lock_free);
    static_assert(std::atomic<MonitorMode>::is_always_lock_free);
    static_assert(std::atomic<DiagnosticSignal>::is_always_lock_free);
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
    static_assert(std::atomic<float>::is_always_lock_free);
};

PreviewController::PreviewController() : impl_(std::make_unique<Impl>()) {}
PreviewController::~PreviewController() = default;

juce::String PreviewController::load(const juce::File& wav) {
    stop();
    impl_->prepared = false;
    juce::WavAudioFormat format;
    auto stream = wav.createInputStream();
    if (!stream)
        return impl_->failure("source_open", "Cannot open WAV.");
    std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(stream.release(), true));
    if (!reader || reader->numChannels < 1 || reader->numChannels > 2 ||
        reader->sampleRate < 44100 || reader->sampleRate > 96000 || reader->lengthInSamples <= 0 ||
        reader->lengthInSamples > 120.0 * reader->sampleRate)
        return impl_->failure("source_metadata",
                              "Use a mono/stereo WAV, 44.1-96 kHz, at most 120 seconds.");
    juce::AudioBuffer<float> loaded(static_cast<int>(reader->numChannels),
                                    static_cast<int>(reader->lengthInSamples));
    if (!reader->read(&loaded, 0, loaded.getNumSamples(), 0, true, true))
        return impl_->failure("source_decode", "WAV decode failed.");
    for (int channel = 0; channel < loaded.getNumChannels(); ++channel)
        for (int sample = 0; sample < loaded.getNumSamples(); ++sample) {
            const auto value = loaded.getSample(channel, sample);
            if (!std::isfinite(value) || std::abs(value) > 1.0f)
                return impl_->failure(
                    "source_samples",
                    "Source must be finite and within full scale (same as research renderer).");
        }
    impl_->source = std::move(loaded);
    impl_->sourceRate = reader->sampleRate;
    impl_->sourceName = wav.getFileName();
    impl_->position.store(0);
    auto fields = impl_->rateFields();
    fields.getDynamicObject()->setProperty("basename", impl_->sourceName);
    fields.getDynamicObject()->setProperty("sampleCount", impl_->source.getNumSamples());
    fields.getDynamicObject()->setProperty("durationSeconds",
                                           impl_->source.getNumSamples() / impl_->sourceRate);
    impl_->logger.write("source_load", fields);
    return {};
}

juce::String PreviewController::validate(const PreviewSettings& settings) const {
    return validate(settings, impl_->sourceRate > 0 ? impl_->sourceRate : 48000);
}
juce::String PreviewController::validate(const PreviewSettings& settings, double rate) const {
    if (!std::isfinite(rate) || rate < 44100 || rate > 96000)
        return "Source: invalid research sample rate.";
    PreviewEngine candidate;
    if (candidate.prepare(rate, settings))
        return {};
    if (settings.mode < 0 || settings.mode >= static_cast<int>(kModes.size()))
        return "Composition: unknown mode.";
    if (settings.reworkedFluid() && settings.mode == 4)
        return "D1 requires A1 and/or B1 emission. Use AD, BD or ABD.";
    if (settings.reworkedFluid())
        return "Reworked core: invalid raw config; check canonical ranges/choices and A1 "
               "radiusMinMm < radiusMaxMm.";
    research::FluidConfig fluid;
    research::ModalConfig modal;
    ProtectSettings protect;
    if (!research::readConfigText(settings.moduleJson().toStdString(), fluid, modal, &protect))
        return "Module config: nonnumeric, nonfinite or invalid representation.";
    if (settings.mode == 1 && protect.topology != research::FluidProtectTopology::whole)
        return "PROTECT: Resonant C supports Whole topology only.";
    // Reuse the authoritative prepare validators to identify the failed module, not parallel
    // numerical rules. These diagnostic-only probes run on the UI thread after prepare failed.
    research::ResidualProtect protectProbe;
    if (!protectProbe.prepare(rate, protect.gain, protect.depth))
        return "PROTECT: check Low < High (D0 amplitude / D1 dB), 0 < Epsilon <= Floor, and "
               "timing/curve ranges.";
    if (settings.mode == 1)
        return "MODAL: root * 4.17 must be <= 0.45 * sample rate; check decay/gain ranges.";
    const std::string_view mode(kModes[static_cast<std::size_t>(settings.mode)]);
    const research::ResearchConfig config{rate, PreviewEngine::kSeed};
    if (mode.find('a') != std::string_view::npos) {
        research::BubbleEnsemble probe;
        if (!probe.prepare(config, fluid.bubble))
            return "BUBBLE: minimum frequency must be <= maximum; check rate, decay, threshold, "
                   "gain and voices.";
    }
    if (mode.find('b') != std::string_view::npos) {
        research::DropletImpactExciter probe;
        if (!probe.prepare(config, fluid.droplet))
            return "DROPLET: minimum frequency must be <= maximum; check decay, refractory, "
                   "threshold, gain and voices.";
    }
    if (mode.find('d') != std::string_view::npos) {
        research::FlowModulator probe;
        if (!probe.prepare(config, fluid.flow))
            return "FLOW: baseDelay - depth must be >= 1/sampleRate; baseDelay + depth <= 0.02 s; "
                   "check target interval/gain.";
    }
    return "Invalid research configuration; keep the previous applied state.";
}

juce::String PreviewController::play(const PreviewSettings& settings) {
    stop();
    if (impl_->source.getNumSamples() == 0)
        return impl_->failure("play", "Load a WAV first.");
    const auto error = prepareStopped(settings);
    return error.isEmpty() ? startPrepared() : error;
}
juce::String PreviewController::prepareStopped(const PreviewSettings& settings) {
    if (impl_->callbackAttached)
        return impl_->failure("prepare",
                              "Stop playback before preparing a research configuration.");
    impl_->prepared = false;
    impl_->preparedRate = impl_->sourceRate > 0 ? impl_->sourceRate : 48000;
    if (!impl_->engine.prepare(impl_->preparedRate, settings))
        return impl_->failure("prepare", validate(settings));
    impl_->engine.setEventTrace(&impl_->eventTrace);
    impl_->protectDepth.store(settings.protect.depth, std::memory_order_relaxed);
    auto* fields = new juce::DynamicObject;
    fields->setProperty("config", juce::JSON::parse(encodeResearchConfig(settings)));
    fields->setProperty("seed", static_cast<int>(PreviewEngine::kSeed));
    fields->setProperty("core", static_cast<int>(settings.core));
    fields->setProperty("composition", settings.mode);
    fields->setProperty("d1Backend", "Historical Lagrange3; NUMERICAL / HUMAN ACCEPTANCE PENDING");
    impl_->logger.write("apply", juce::var(fields));
    impl_->prepared = true;
    return {};
}
juce::String PreviewController::startPrepared() {
    if (!impl_->prepared || impl_->callbackAttached || impl_->source.getNumSamples() == 0 ||
        impl_->preparedRate != impl_->sourceRate)
        return impl_->failure("play",
                              "Load a source and prepare the stopped preview before starting.");
    if (impl_->device.getCurrentAudioDevice() == nullptr) {
        const auto error = impl_->device.initialiseWithDefaultDevices(0, 2);
        if (error.isNotEmpty())
            return impl_->failure("device_setup", error);
    }
    auto* audioDevice = impl_->device.getCurrentAudioDevice();
    if (audioDevice == nullptr || audioDevice->getActiveOutputChannels().countNumberOfSetBits() < 2)
        return impl_->failure("device_setup", "Default output must provide at least two channels.");
    const double deviceRate = audioDevice->getCurrentSampleRate();
    if (!std::isfinite(deviceRate) || deviceRate < 8000 || deviceRate > 384000)
        return impl_->failure("device_rate",
                              "Unsupported monitor device rate (8-384 kHz required).");
    impl_->deviceRate.store(deviceRate);
    auto fields = impl_->rateFields();
    fields.getDynamicObject()->setProperty("deviceName", audioDevice->getName());
    fields.getDynamicObject()->setProperty("deviceType", audioDevice->getTypeName());
    fields.getDynamicObject()->setProperty(
        "outputChannels", audioDevice->getActiveOutputChannels().countNumberOfSetBits());
    fields.getDynamicObject()->setProperty(
        "monitorAntiAlias", deviceRate < impl_->preparedRate - .5
                                ? "129-tap Blackman FIR, cutoff 0.45*deviceRate; monitor only"
                                : "off");
    fields.getDynamicObject()->setProperty("requestedDeviceRate",
                                           "current/default (no rate request)");
    impl_->logger.write("device_setup", fields);
    impl_->isPlaying.store(true);
    impl_->device.addAudioCallback(impl_.get());
    impl_->callbackAttached = true;
    impl_->logger.write("restart", impl_->rateFields());
    impl_->logger.write("play_start", impl_->rateFields());
    return {};
}

void PreviewController::stop() {
    const bool wasPlaying = impl_->isPlaying.load();
    impl_->stop();
    if (wasPlaying)
        impl_->logger.write("play_stop", impl_->rateFields());
}
void PreviewController::setAuditionTrim(double decibels) noexcept {
    if (std::isfinite(decibels) && decibels >= 0 && decibels <= 36)
        impl_->auditionGain.store(static_cast<float>(std::pow(10.0, decibels / 20.0)),
                                  std::memory_order_relaxed);
}

void PreviewController::setProtectDepth(double depth) noexcept {
    if (std::isfinite(depth) && depth >= 0 && depth <= 1)
        impl_->protectDepth.store(depth, std::memory_order_relaxed);
}
void PreviewController::setExcitationAudition(bool enabled) noexcept {
    setDiagnosticSignal(enabled ? DiagnosticSignal::modalDriver : DiagnosticSignal::none);
}
void PreviewController::setDiagnosticSignal(DiagnosticSignal selection) noexcept {
    if (selection >= DiagnosticSignal::count)
        selection = DiagnosticSignal::none;
    impl_->diagnosticSelection.store(selection, std::memory_order_relaxed);
}

void PreviewController::setMonitor(MonitorMode mode, float outputGainDb) noexcept {
    if (mode != MonitorMode::dry && mode != MonitorMode::processed && mode != MonitorMode::residual)
        mode = MonitorMode::processed;
    if (!std::isfinite(outputGainDb))
        outputGainDb = -18.0f;
    impl_->monitor.store(mode);
    impl_->outputGain.store(
        juce::Decibels::decibelsToGain(juce::jlimit(-60.0f, 0.0f, outputGainDb)));
}
bool PreviewController::consumeMonitorOverRange() noexcept {
    return impl_->overRange.consume();
}
plugin::DeveloperDiagnosticsSnapshot PreviewController::diagnostics() const noexcept {
    return impl_->metrics.snapshot();
}
ProtectDiagnosticsSnapshot PreviewController::protectDiagnostics() noexcept {
    return impl_->protectMetrics.snapshot();
}
void PreviewController::logEvent(const juce::String& event) {
    impl_->logger.write(event, impl_->rateFields());
}
juce::String PreviewController::rateDescription() const {
    const auto rate = impl_->deviceRate.load();
    return "SOURCE " + juce::String(impl_->sourceRate, 0) + " Hz | DSP " +
           juce::String(impl_->preparedRate, 0) + " Hz | DEVICE " + juce::String(rate, 0) +
           " Hz | MONITOR SRC " +
           (rate > 0 && std::abs(rate - impl_->preparedRate) > .5 ? "ON" : "OFF");
}
juce::String PreviewController::logStatus() const {
    return impl_->logger.status();
}
juce::String PreviewController::sourceDescription() const {
    if (impl_->sourceName.isEmpty())
        return "No source loaded. Load a WAV; output uses the default system device.";
    return impl_->sourceName + " | " + juce::String(impl_->sourceRate, 0) + " Hz | " +
           juce::String(impl_->source.getNumChannels()) + " ch | " +
           juce::String(impl_->source.getNumSamples() / impl_->sourceRate, 2) + " s";
}
SourceMetadata PreviewController::sourceMetadata() const {
    return {impl_->sourceName, impl_->sourceRate, impl_->source.getNumChannels(),
            impl_->source.getNumSamples()};
}
double PreviewController::positionSeconds() const noexcept {
    return impl_->sourceRate > 0 ? impl_->position.load() / impl_->sourceRate : 0;
}
bool PreviewController::playing() const noexcept {
    return impl_->isPlaying.load();
}
bool PreviewController::finished() const noexcept {
    return impl_->ended.load();
}
bool PreviewController::deviceRateMismatch() const noexcept {
    return impl_->mismatch.load();
}
double PreviewController::sampleRate() const noexcept {
    return impl_->sourceRate;
}
} // namespace frazil::water::preview
