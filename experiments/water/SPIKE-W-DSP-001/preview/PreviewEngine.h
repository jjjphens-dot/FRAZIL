#pragma once

#include "PreviewSettings.h"
#include "render/ReadConfig.h"

namespace frazil::water::preview {

// Processing owner for the standalone preview, not a production WaterProcessor. prepare may
// allocate/parse JSON and requires an inactive callback. Only the audio owner calls process/reset.
class PreviewEngine final {
  public:
    bool prepare(double sampleRate, const PreviewSettings& settings) {
        ready_ = false;
        if (settings.mode < 0 || settings.mode >= static_cast<int>(kModes.size()) ||
            !std::isfinite(sampleRate) || sampleRate < 44100 || sampleRate > 96000)
            return false;
        research::FluidConfig fluidConfig;
        research::ModalConfig modalConfig;
        const auto json = settings.moduleJson().toStdString();
        if (!research::readConfigText(json, fluidConfig, modalConfig))
            return false;
        const std::string_view mode(kModes[static_cast<std::size_t>(settings.mode)]);
        baseline_ = mode == "baseline";
        modalMode_ = mode == "c";
        fluidConfig.bubbleEnabled = !baseline_ && mode.find('a') != std::string_view::npos;
        fluidConfig.dropletEnabled = !baseline_ && mode.find('b') != std::string_view::npos;
        fluidConfig.flowEnabled = !baseline_ && mode.find('d') != std::string_view::npos;
        research::ResearchConfig config;
        config.sampleRateHz = sampleRate;
        config.baseSeed = kSeed;
        ready_ = baseline_ || (modalMode_ ? modal_.prepare(sampleRate, modalConfig)
                                          : fluid_.prepare(config, fluidConfig));
        reset();
        return ready_;
    }

    void reset() noexcept {
        fluid_.reset();
        modal_.reset();
    }

    research::StereoFrame residual(const research::StereoFrame& input) noexcept {
        if (!ready_ || baseline_)
            return {};
        return modalMode_ ? modal_.process(input) : fluid_.process(input);
    }

    static constexpr std::uint32_t kSeed = 42;

  private:
    // Lifecycle/audio owner only. No UI access or runtime config mutation of these DSP objects.
    research::FluidCandidate fluid_;
    research::LiquidModalResonator modal_;
    bool ready_{}, baseline_{}, modalMode_{};
};
} // namespace frazil::water::preview
