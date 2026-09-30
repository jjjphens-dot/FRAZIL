#pragma once

#include "DiagnosticMonitor.h"
#include "PreviewEventTrace.h"
#include "PreviewSettings.h"
#include "ProtectDiagnostics.h"
#include "ResearchCoreParameterAdapter.h"
#include "dsp/BubbleA1.h"
#include "dsp/DropletB1.h"
#include "dsp/DropletB2.h"
#include "dsp/FlowD1.h"
#include "render/ReadConfig.h"

#include <memory>

namespace frazil::water::preview {

// Processing owner for the standalone preview, not a production WaterProcessor. prepare may
// allocate/parse JSON and requires an inactive callback. Only the audio owner calls process/reset.
class PreviewEngine final {
  public:
    bool prepare(double sampleRate, const PreviewSettings& settings) {
        ready_ = false;
        readout_ = {};
        diagnostic_ = {};
        reworked_ = settings.reworkedFluid();
        if ((settings.core != WaterResearchCore::legacy &&
             settings.core != WaterResearchCore::reworked) ||
            (reworked_ && settings.mode == 4))
            return false;
        if (settings.mode < 0 || settings.mode >= static_cast<int>(kModes.size()) ||
            !std::isfinite(sampleRate) || sampleRate < 44100 || sampleRate > 96000)
            return false;
        if (reworked_)
            return prepareReworked(sampleRate, settings);
        research::FluidConfig fluidConfig;
        research::ModalConfig modalConfig;
        research::ProtectRenderConfig protectConfig;
        const auto json = settings.moduleJson().toStdString();
        if (!research::readConfigText(json, fluidConfig, modalConfig, &protectConfig))
            return false;
        const std::string_view mode(kModes[static_cast<std::size_t>(settings.mode)]);
        baseline_ = mode == "baseline";
        modalMode_ = mode == "c";
        if ((modalMode_ && protectConfig.topology != research::FluidProtectTopology::whole) ||
            (!reworked_ && !protect_.prepare(sampleRate, protectConfig.gain, protectConfig.depth)))
            return false;
        topology_ = protectConfig.topology;
        sampleRate_ = sampleRate;
        modalConfig_ = modalConfig;
        fluidConfig.bubbleEnabled = !baseline_ && mode.find('a') != std::string_view::npos;
        fluidConfig.dropletEnabled = !baseline_ && mode.find('b') != std::string_view::npos;
        fluidConfig.flowEnabled = !baseline_ && mode.find('d') != std::string_view::npos;
        research::ResearchConfig config;
        config.sampleRateHz = sampleRate;
        config.baseSeed = kSeed;
        ready_ = baseline_ || (modalMode_ ? modal_.prepare(config, modalConfig)
                                          : fluid_.prepare(config, fluidConfig));
        reset();
        return ready_;
    }

    void reset() noexcept {
        tracedA_ = tracedAStarts_ = tracedB_ = tracedBAdmitted_ = tracedBStarts_ = 0;
        fluid_.reset();
        modal_.reset();
        protect_.reset();
        if (a1_)
            a1_->reset();
        if (b1_)
            b1_->reset();
        if (b2_)
            b2_->reset();
        d1_.reset();
        readout_ = {};
        diagnostic_ = {};
    }

    research::StereoFrame residual(const research::StereoFrame& input) noexcept {
        diagnostic_ = {};
        if (!ready_)
            return {};
        if (reworked_)
            return reworkedResidual(input);
        const auto gain = protect_.processSource(input);
        if (baseline_)
            return {};
        // Always advance every enabled generator once, even at full attenuation or during OFF.
        auto& frames = readout_;
        if (modalMode_) {
            frames.at(WaterSignal::modal) = modal_.process(input);
            diagnostic_.at(DiagnosticSignal::modal) = frames.at(WaterSignal::modal);
            diagnostic_.at(DiagnosticSignal::modalDriver) = modal_.excitationFrame();
            frames.at(WaterSignal::totalPreProtect) = frames.at(WaterSignal::modal);
            frames.at(WaterSignal::postProtect) =
                research::ResidualProtect::apply(frames.at(WaterSignal::modal), gain);
        } else {
            const auto parts = fluid_.processComponents(input);
            diagnostic_.at(DiagnosticSignal::bubble) = parts.bubble;
            diagnostic_.at(DiagnosticSignal::droplet) = parts.droplet;
            diagnostic_.at(DiagnosticSignal::flow) = parts.flow;
            diagnostic_.at(DiagnosticSignal::bubbleDriver) = fluid_.bubbleExcitation();
            diagnostic_.at(DiagnosticSignal::dropletDriver) = fluid_.dropletExcitation();
            frames.at(WaterSignal::bubble) = parts.bubble;
            frames.at(WaterSignal::droplet) = parts.droplet;
            frames.at(WaterSignal::flow) = parts.flow;
            frames.at(WaterSignal::totalPreProtect) = parts.sum();
            frames.at(WaterSignal::postProtect) =
                research::applyFluidProtect(parts, gain, topology_);
        }
        return frames.at(WaterSignal::postProtect);
    }
    // Actual common modal driver, pre-weight and pre-Protect; zero for non-C compositions.
    research::StereoFrame excitationFrame() const noexcept {
        return ready_ && modalMode_ ? modal_.excitationFrame() : research::StereoFrame{};
    }
    const DiagnosticFrames& diagnosticFrames() const noexcept {
        return diagnostic_;
    }
    const WaterFrameReadout& waterReadout() const noexcept {
        return readout_;
    }
    WaterActivity waterActivity() const noexcept {
        if (!ready_ || baseline_)
            return {};
        WaterActivity activity;
        if (modalMode_) {
            activity.modalRootHz = modalConfig_.rootFrequencyHz;
            activity.modalDecaySeconds = modalConfig_.decaySeconds;
            activity.modalMotionDepth = modalConfig_.motionDepth;
            activity.modalMotionIntervalSeconds = modalConfig_.motionIntervalSeconds;
        } else if (reworked_) {
            activity.reworked = true;
            activity.bubbleRequested = bubbleEnabled_ ? a1_->requested() : 0;
            activity.bubbleRequestedRate = bubbleEnabled_ ? a1_->requestedRate() : 0;
            activity.dropletEligible =
                dropletEnabled_ ? (useB2_ ? b2_->counters().eligible : b1_->counters().eligible)
                                : 0;
            activity.dropletAdmitted =
                dropletEnabled_ ? (useB2_ ? b2_->counters().admitted : b1_->counters().admitted)
                                : 0;
            activity.flowPathMeters = flowEnabled_ ? d1_.pathMeters() : 0;
            activity.bubbleEvents = bubbleEnabled_ ? a1_->pool().counters().started : 0;
            activity.bubbleActive = bubbleEnabled_ ? a1_->pool().active() : 0;
            activity.bubbleSteals = bubbleEnabled_ ? a1_->pool().counters().steals : 0;
            activity.dropletEvents = dropletEnabled_ ? (useB2_ ? b2_->pool().counters().started
                                                               : b1_->pool().counters().started)
                                                     : 0;
            activity.dropletActive =
                dropletEnabled_ ? (useB2_ ? b2_->pool().active() : b1_->pool().active()) : 0;
            activity.flowDelayMs =
                flowEnabled_ ? 1000 * research::FlowD1Model::delaySeconds(d1_.pathMeters()) : 0;
        } else {
            activity.bubbleEvents = fluid_.bubbleEvents();
            activity.dropletEvents = fluid_.dropletEvents();
            activity.bubbleSteals = fluid_.bubbleSteals();
            activity.bubbleActive = fluid_.bubbleActive();
            activity.dropletActive = fluid_.dropletActive();
            activity.flowDelayMs = 1000 * fluid_.flowDelaySamples() / sampleRate_;
        }
        return activity;
    }

    // Audio-owner command, called only at callback/sample boundaries by PreviewController.
    bool setProtectDepth(double depth) noexcept {
        return ready_ && !reworked_ && protect_.setDepth(depth);
    }
    ProtectReadout protectReadout() const noexcept {
        if (reworked_)
            return {};
        const auto detection = protect_.detection();
        return {detection.fast, detection.slow, detection.difference, detection.logRatioDb,
                protect_.reductionDb()};
    }

    void traceEvents(PreviewEventTrace& trace, std::uint64_t frame) noexcept {
        if (!ready_ || !reworked_)
            return;
        if (bubbleEnabled_) {
            const auto make = [&](const research::BubbleA1Event& e, bool requested,
                                  std::uint64_t starts) {
                PreviewEventRecord r;
                r.module = 1;
                r.frame = frame;
                // Request sequence only applies to request records; starts may be deferred.
                r.eligibleId = requested ? a1_->requested() : 0;
                r.requested = requested;
                r.started = starts != 0;
                r.startCount = starts;
                r.bin = static_cast<int>(e.bin);
                r.radiusMm = e.physics.radiusMeters * 1000;
                r.centerFrequencyHz = e.physics.frequencyHz;
                r.depthExcitationProxy = e.depthExcitationProxy;
                r.renderAmplitudeL = e.amplitude[0];
                r.renderAmplitudeR = e.amplitude[1];
                r.pathMeters = flowEnabled_ ? d1_.pathMeters() : 0;
                trace.push(r);
            };
            if (a1_->requested() != tracedA_)
                make(a1_->lastRequestedEvent(), true, 0);
            const auto starts = a1_->pool().counters().started;
            if (starts != tracedAStarts_)
                make(a1_->pool().lastStarted(), false, starts - tracedAStarts_);
            tracedA_ = a1_->requested();
            tracedAStarts_ = starts;
        }
        if (dropletEnabled_ && useB2_) {
            const auto make = [&](const research::DropletB2Event& e, bool eligible, bool started) {
                PreviewEventRecord r;
                r.module = 2;
                r.frame = frame;
                r.eligibleId = e.center.eligibleId;
                r.eligible = eligible;
                r.started = started;
                r.startCount = started ? b2_->pool().counters().started - tracedBStarts_ : 0;
                r.admitted = started || b2_->counters().admitted != tracedBAdmitted_;
                r.noveltyDb = e.center.impact.onsetStrength;
                r.positiveSlope = e.positiveSlope;
                r.sourceExcitation = e.sourceExcitation;
                r.mappedExcitation = e.mappedExcitation;
                r.radiusMm = e.center.physics.equivalentRadiusMeters * 1000;
                r.centerFrequencyHz = e.center.physics.frequencyHz;
                const double maximum =
                    e.center.riseXi > 0
                        ? std::min(std::sqrt(2.) * r.centerFrequencyHz, .45 * sampleRate_)
                        : r.centerFrequencyHz;
                const double cents = research::DropletB2SpatialRenderer::boundedCents(
                    maximum, e.detuneCents, e.maximumBeatHz);
                r.detuneLeftCents = -e.polarity * cents;
                r.detuneRightCents = e.polarity * cents;
                const auto f = research::DropletB2SpatialRenderer::frequencies(r.centerFrequencyHz,
                                                                               cents, e.polarity);
                r.renderFrequencyL = f[0];
                r.renderFrequencyR = f[1];
                const double amplitude =
                    e.center.physics.renderAmplitudeScale * e.mappedExcitation * e.center.gain;
                r.renderAmplitudeL = amplitude * e.center.impact.carrier[0];
                r.renderAmplitudeR = amplitude * e.center.impact.carrier[1];
                r.pathMeters = flowEnabled_ ? d1_.pathMeters() : 0;
                trace.push(r);
            };
            if (b2_->counters().eligible != tracedB_)
                make(b2_->lastEligible(), true, false);
            if (b2_->pool().counters().started != tracedBStarts_)
                make(b2_->pool().lastStarted(), false, true);
            tracedB_ = b2_->counters().eligible;
            tracedBAdmitted_ = b2_->counters().admitted;
            tracedBStarts_ = b2_->pool().counters().started;
        }
    }
    static constexpr std::uint32_t kSeed = 42;

  private:
    bool prepareReworked(double sampleRate, const PreviewSettings& settings) {
        using Adapter = ResearchCoreParameterAdapter;
        if (!Adapter::validate(settings.tuning))
            return false;
        baseline_ = modalMode_ = false;
        useB2_ = settings.tuning.useB2;
        if (!b2_)
            b2_ = std::make_unique<research::DropletB2>();
        sampleRate_ = sampleRate;
        const std::string_view mode(kModes[static_cast<std::size_t>(settings.mode)]);
        bubbleEnabled_ = mode.find('a') != std::string_view::npos;
        dropletEnabled_ = mode.find('b') != std::string_view::npos;
        flowEnabled_ = mode.find('d') != std::string_view::npos;
        // Allocate large fixed pools only with the callback detached. No Legacy JSON is read.
        if (!a1_)
            a1_ = std::make_unique<research::BubbleA1>();
        if (!b1_)
            b1_ = std::make_unique<research::DropletB1>();
        const research::ResearchConfig config{sampleRate, kSeed};
        ready_ =
            (!bubbleEnabled_ ||
             a1_->prepare(config, Adapter::makeBubbleA1Config(settings.tuning),
                          Adapter::makeSharedExcitationConfig(settings.tuning))) &&
            (!dropletEnabled_ ||
             (useB2_ ? b2_->prepare(config, research::DropletB2Config{settings.tuning.dropletB2})
                     : b1_->prepare(config, Adapter::makeDropletB1Config(settings.tuning)))) &&
            (!flowEnabled_ || d1_.prepare(config, Adapter::makeFlowD1Config(settings.tuning)));
        reset();
        return ready_;
    }
    research::StereoFrame reworkedResidual(const research::StereoFrame& input) noexcept {
        research::FluidResiduals parts;
        if (bubbleEnabled_) {
            parts.bubble = a1_->process(input);
            diagnostic_.at(DiagnosticSignal::bubbleDriver) = a1_->excitationFrame();
        }
        if (dropletEnabled_)
            parts.droplet = useB2_ ? b2_->process(input) : b1_->process(input);
        auto effect = parts.sum(); // Same float summation boundary as render_main.cpp.
        if (flowEnabled_) {
            const auto transferred = d1_.process(effect);
            for (std::size_t ch = 0; ch < effect.size(); ++ch)
                effect[ch] = static_cast<float>(transferred.transferred[ch]);
        }
        diagnostic_.at(DiagnosticSignal::bubble) = parts.bubble;
        diagnostic_.at(DiagnosticSignal::droplet) = parts.droplet;
        // D1 is the transferred emission, never an additive D0 carrier residual.
        diagnostic_.at(DiagnosticSignal::flow) = flowEnabled_ ? effect : research::StereoFrame{};
        readout_.at(WaterSignal::bubble) = parts.bubble;
        readout_.at(WaterSignal::droplet) = parts.droplet;
        readout_.at(WaterSignal::flow) = diagnostic_.at(DiagnosticSignal::flow);
        readout_.at(WaterSignal::totalPreProtect) = effect;
        readout_.at(WaterSignal::postProtect) = effect;
        return effect;
    }
    std::uint64_t tracedA_{}, tracedAStarts_{}, tracedB_{}, tracedBAdmitted_{}, tracedBStarts_{};
    bool useB2_{};
    std::unique_ptr<research::DropletB2> b2_;
    std::unique_ptr<research::BubbleA1> a1_;
    std::unique_ptr<research::DropletB1> b1_;
    research::FlowD1 d1_;
    bool reworked_{}, bubbleEnabled_{}, dropletEnabled_{}, flowEnabled_{};
    // Lifecycle/audio owner only. No UI access or runtime config mutation of these DSP objects.
    research::FluidCandidate fluid_;
    research::LiquidModalResonator modal_;
    research::ResidualProtect protect_;
    research::FluidProtectTopology topology_{research::FluidProtectTopology::whole};
    bool ready_{}, baseline_{}, modalMode_{};
    WaterFrameReadout readout_;
    DiagnosticFrames diagnostic_;
    research::ModalConfig modalConfig_;
    double sampleRate_{48000};
};
} // namespace frazil::water::preview
