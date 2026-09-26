#pragma once

#include "DiagnosticMonitor.h"
#include "PreviewSettings.h"
#include "ProtectDiagnostics.h"
#include "dsp/BubbleA1.h"
#include "dsp/DropletB1.h"
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
        if (reworked_) {
            // Match renderer composition and typed defaults. Large fixed voice pools live off
            // the Windows stack, allocated only with the callback detached during prepare.
            bubbleEnabled_ = fluidConfig.bubbleEnabled;
            dropletEnabled_ = fluidConfig.dropletEnabled;
            flowEnabled_ = fluidConfig.flowEnabled;
            if (!a1_)
                a1_ = std::make_unique<research::BubbleA1>();
            if (!b1_)
                b1_ = std::make_unique<research::DropletB1>();
            ready_ = (!bubbleEnabled_ || a1_->prepare(config)) &&
                     (!dropletEnabled_ || b1_->prepare(config)) &&
                     (!flowEnabled_ || d1_.prepare(config));
        } else
            ready_ = baseline_ || (modalMode_ ? modal_.prepare(config, modalConfig)
                                              : fluid_.prepare(config, fluidConfig));
        reset();
        return ready_;
    }

    void reset() noexcept {
        fluid_.reset();
        modal_.reset();
        protect_.reset();
        if (a1_)
            a1_->reset();
        if (b1_)
            b1_->reset();
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
            activity.bubbleEvents = bubbleEnabled_ ? a1_->pool().counters().started : 0;
            activity.bubbleActive = bubbleEnabled_ ? a1_->pool().active() : 0;
            activity.bubbleSteals = bubbleEnabled_ ? a1_->pool().counters().steals : 0;
            activity.dropletEvents = dropletEnabled_ ? b1_->pool().counters().started : 0;
            activity.dropletActive = dropletEnabled_ ? b1_->pool().active() : 0;
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

    static constexpr std::uint32_t kSeed = 42;

  private:
    research::StereoFrame reworkedResidual(const research::StereoFrame& input) noexcept {
        research::FluidResiduals parts;
        if (bubbleEnabled_) {
            parts.bubble = a1_->process(input);
            diagnostic_.at(DiagnosticSignal::bubbleDriver) = a1_->excitationFrame();
        }
        if (dropletEnabled_)
            parts.droplet = b1_->process(input);
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
