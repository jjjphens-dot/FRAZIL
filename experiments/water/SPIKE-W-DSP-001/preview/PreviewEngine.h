#pragma once

#include "PreviewSettings.h"
#include "ProtectDiagnostics.h"
#include "render/ReadConfig.h"

namespace frazil::water::preview {

// Processing owner for the standalone preview, not a production WaterProcessor. prepare may
// allocate/parse JSON and requires an inactive callback. Only the audio owner calls process/reset.
class PreviewEngine final {
  public:
    bool prepare(double sampleRate, const PreviewSettings& settings) {
        ready_ = false;
        readout_ = {};
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
            !protect_.prepare(sampleRate, protectConfig.gain, protectConfig.depth))
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
        fluid_.reset();
        modal_.reset();
        protect_.reset();
        readout_ = {};
    }

    research::StereoFrame residual(const research::StereoFrame& input) noexcept {
        if (!ready_)
            return {};
        const auto gain = protect_.processSource(input);
        if (baseline_)
            return {};
        // Always advance every enabled generator once, even at full attenuation or during OFF.
        auto& frames = readout_;
        if (modalMode_) {
            frames.at(WaterSignal::modal) = modal_.process(input);
            frames.at(WaterSignal::totalPreProtect) = frames.at(WaterSignal::modal);
            frames.at(WaterSignal::postProtect) =
                research::ResidualProtect::apply(frames.at(WaterSignal::modal), gain);
        } else {
            const auto parts = fluid_.processComponents(input);
            frames.at(WaterSignal::bubble) = parts.bubble;
            frames.at(WaterSignal::droplet) = parts.droplet;
            frames.at(WaterSignal::flow) = parts.flow;
            frames.at(WaterSignal::totalPreProtect) = parts.sum();
            frames.at(WaterSignal::postProtect) =
                research::applyFluidProtect(parts, gain, topology_);
        }
        return frames.at(WaterSignal::postProtect);
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
        return ready_ && protect_.setDepth(depth);
    }
    ProtectReadout protectReadout() const noexcept {
        const auto detection = protect_.detection();
        return {detection.fast, detection.slow, detection.difference, detection.logRatioDb,
                protect_.reductionDb()};
    }

    static constexpr std::uint32_t kSeed = 42;

  private:
    // Lifecycle/audio owner only. No UI access or runtime config mutation of these DSP objects.
    research::FluidCandidate fluid_;
    research::LiquidModalResonator modal_;
    research::ResidualProtect protect_;
    research::FluidProtectTopology topology_{research::FluidProtectTopology::whole};
    bool ready_{}, baseline_{}, modalMode_{};
    WaterFrameReadout readout_;
    research::ModalConfig modalConfig_;
    double sampleRate_{48000};
};
} // namespace frazil::water::preview
