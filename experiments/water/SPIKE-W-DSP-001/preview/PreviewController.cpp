#include "PreviewController.h"

#include "PreviewEngine.h"
#include "dsp/primitives/LinearSmoother.h"

#include <algorithm>
#include <atomic>
#include <cmath>

namespace frazil::water::preview {
class PreviewController::Impl final : public juce::AudioIODeviceCallback {
  public:
    ~Impl() override {
        stop();
    }

    void stop() {
        // removeAudioCallback waits for any in-flight callback on this non-realtime caller.
        // Source replacement and prepare are safe only after it returns.
        device.removeAudioCallback(this);
        isPlaying.store(false);
        protectMetrics.clear();
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* audioDevice) override {
        const auto rate = audioDevice->getCurrentSampleRate();
        mismatch.store(std::abs(rate - sourceRate) > .5);
        engine.reset();
        frame = 0;
        position.store(0);
        ended.store(false);
        carrier.prepare(rate, .01);
        effect.prepare(rate, .01);
        gain.prepare(rate, .01);
        const auto mode = monitor.load();
        carrier.reset(mode == MonitorMode::residual ? 0.0f : 1.0f);
        effect.reset(mode == MonitorMode::dry ? 0.0f : 1.0f);
        gain.reset(0.0f); // Monitoring fade-in; does not alter the research residual/state.
        metrics.setPrepared(static_cast<float>(rate), audioDevice->getCurrentBufferSizeSamples(),
                            source.getNumChannels());
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
        carrier.setTarget(mode == MonitorMode::residual ? 0.0f : 1.0f);
        engine.setProtectDepth(protectDepth.load(std::memory_order_relaxed));
        effect.setTarget(mode == MonitorMode::dry ? 0.0f : 1.0f);
        gain.setTarget(outputGain.load());
        const int channels = source.getNumChannels();
        float inputPeak{}, outputPeak{};
        double inputSquares{}, outputSquares{};
        bool finite = true;
        ProtectBlockReadout protectBlock;
        const auto endFrame = static_cast<std::uint64_t>(source.getNumSamples()) +
                              static_cast<std::uint64_t>(30.0 * sourceRate);
        for (int sample = 0; sample < count; ++sample) {
            if (frame >= endFrame) {
                ended.store(true);
                break;
            }
            research::StereoFrame input{};
            if (frame < static_cast<std::uint64_t>(source.getNumSamples()))
                for (int channel = 0; channel < channels; ++channel)
                    input[static_cast<std::size_t>(channel)] =
                        source.getSample(channel, static_cast<int>(frame));
            // DSP always advances during Dry monitoring. Replaying, not toggling Dry, resets seed.
            const auto residual = engine.residual(input);
            protectBlock.latest = engine.protectReadout();
            protectBlock.peak.includePeak(protectBlock.latest);
            const auto xWeight = carrier.getNextValue();
            const auto eWeight = effect.getNextValue();
            const auto level = gain.getNextValue();
            research::StereoFrame output{};
            for (int channel = 0; channel < channels; ++channel) {
                const auto i = static_cast<std::size_t>(channel);
                output[i] = (xWeight * input[i] + eWeight * residual[i]) * level;
                finite = finite && std::isfinite(output[i]);
                inputPeak = std::max(inputPeak, std::abs(input[i]));
                outputPeak = std::max(outputPeak, std::abs(output[i]));
                inputSquares += static_cast<double>(input[i]) * input[i];
                outputSquares += static_cast<double>(output[i]) * output[i];
            }
            for (int channel = 0; channel < std::min(2, outputChannels); ++channel)
                if (outputs[channel] != nullptr)
                    outputs[channel][sample] = output[channels == 1 ? 0 : channel];
            ++frame;
        }
        position.store(frame);
        protectMetrics.publish(protectBlock);
        const auto denominator = static_cast<double>(std::max(1, channels * count));
        metrics.publish(count, channels, inputPeak, outputPeak,
                        static_cast<float>(std::sqrt(inputSquares / denominator)),
                        static_cast<float>(std::sqrt(outputSquares / denominator)), finite);
    }

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
    LinearSmoother carrier, effect, gain;
    std::atomic<std::uint64_t> position{};
    std::atomic<bool> isPlaying{}, ended{}, mismatch{};
    std::atomic<MonitorMode> monitor{MonitorMode::processed};
    std::atomic<float> outputGain{0.25118864f};
    // Sole UI->audio Protect transport: validated normalized target, sampled at block boundary.
    std::atomic<double> protectDepth{};
    static_assert(std::atomic<double>::is_always_lock_free);
    static_assert(std::atomic<MonitorMode>::is_always_lock_free);
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
    static_assert(std::atomic<float>::is_always_lock_free);
};

PreviewController::PreviewController() : impl_(std::make_unique<Impl>()) {}
PreviewController::~PreviewController() = default;

juce::String PreviewController::load(const juce::File& wav) {
    stop();
    juce::WavAudioFormat format;
    auto stream = wav.createInputStream();
    if (!stream)
        return "Cannot open WAV.";
    std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(stream.release(), true));
    if (!reader || reader->numChannels < 1 || reader->numChannels > 2 ||
        reader->sampleRate < 44100 || reader->sampleRate > 96000 || reader->lengthInSamples <= 0 ||
        reader->lengthInSamples > 120.0 * reader->sampleRate)
        return "Use a mono/stereo WAV, 44.1-96 kHz, at most 120 seconds.";
    juce::AudioBuffer<float> loaded(static_cast<int>(reader->numChannels),
                                    static_cast<int>(reader->lengthInSamples));
    if (!reader->read(&loaded, 0, loaded.getNumSamples(), 0, true, true))
        return "WAV decode failed.";
    for (int channel = 0; channel < loaded.getNumChannels(); ++channel)
        for (int sample = 0; sample < loaded.getNumSamples(); ++sample) {
            const auto value = loaded.getSample(channel, sample);
            if (!std::isfinite(value) || std::abs(value) > 1.0f)
                return "Source must be finite and within full scale (same as research renderer).";
        }
    impl_->source = std::move(loaded);
    impl_->sourceRate = reader->sampleRate;
    impl_->sourceName = wav.getFileName();
    impl_->position.store(0);
    return {};
}

juce::String PreviewController::validate(const PreviewSettings& settings) const {
    PreviewEngine candidate;
    const auto rate = impl_->sourceRate > 0 ? impl_->sourceRate : 48000;
    if (candidate.prepare(rate, settings))
        return {};
    if (settings.mode < 0 || settings.mode >= static_cast<int>(kModes.size()))
        return "Composition: unknown mode.";
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
        return "Load a WAV first.";
    if (!impl_->engine.prepare(impl_->sourceRate, settings))
        return validate(settings);
    impl_->protectDepth.store(settings.protect.depth, std::memory_order_relaxed);
    if (impl_->device.getCurrentAudioDevice() == nullptr) {
        const auto error = impl_->device.initialiseWithDefaultDevices(0, 2);
        if (error.isNotEmpty())
            return error;
    }
    auto setup = impl_->device.getAudioDeviceSetup();
    setup.sampleRate = impl_->sourceRate;
    const auto error = impl_->device.setAudioDeviceSetup(setup, true);
    if (error.isNotEmpty())
        return "Audio device: " + error;
    auto* audioDevice = impl_->device.getCurrentAudioDevice();
    if (audioDevice == nullptr ||
        audioDevice->getActiveOutputChannels().countNumberOfSetBits() < 2 ||
        std::abs(audioDevice->getCurrentSampleRate() - impl_->sourceRate) > .5)
        return "Default output must support stereo at the WAV sample rate; no resampling is used.";
    impl_->isPlaying.store(true);
    impl_->device.addAudioCallback(impl_.get());
    return {};
}

void PreviewController::stop() {
    impl_->stop();
}
void PreviewController::setProtectDepth(double depth) noexcept {
    if (std::isfinite(depth) && depth >= 0 && depth <= 1)
        impl_->protectDepth.store(depth, std::memory_order_relaxed);
}
void PreviewController::setMonitor(MonitorMode mode, float outputGainDb) noexcept {
    if (mode != MonitorMode::dry && mode != MonitorMode::processed && mode != MonitorMode::residual)
        mode = MonitorMode::processed;
    if (!std::isfinite(outputGainDb))
        outputGainDb = -12.0f;
    impl_->monitor.store(mode);
    impl_->outputGain.store(
        juce::Decibels::decibelsToGain(juce::jlimit(-60.0f, 0.0f, outputGainDb)));
}
plugin::DeveloperDiagnosticsSnapshot PreviewController::diagnostics() const noexcept {
    return impl_->metrics.snapshot();
}
ProtectDiagnosticsSnapshot PreviewController::protectDiagnostics() noexcept {
    return impl_->protectMetrics.snapshot();
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
