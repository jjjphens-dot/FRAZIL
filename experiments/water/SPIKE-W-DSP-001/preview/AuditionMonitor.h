#pragma once

#include "PreviewSettings.h"
#include "dsp/WaterExcitationFeatures.h"
#include "dsp/primitives/LinearSmoother.h"

namespace frazil::water::preview {
// Audio-owner monitor only: E has already passed Protect. No detector or generator can see this
// gain. Set targets once per block; all three transitions use the existing 10 ms ramp primitive.
class AuditionMonitor final {
  public:
    void prepare(double rate, MonitorMode mode, float trimGain) noexcept {
        carrier_.prepare(rate, .01);
        effect_.prepare(rate, .01);
        output_.prepare(rate, .01);
        carrier_.reset(mode == MonitorMode::residual ? 0.f : 1.f);
        effect_.reset(mode == MonitorMode::dry ? 0.f : trimGain);
        output_.reset(0); // Fade in at device start.
    }
    void setTargets(MonitorMode mode, float outputGain, float trimGain) noexcept {
        carrier_.setTarget(mode == MonitorMode::residual ? 0.f : 1.f);
        effect_.setTarget(mode == MonitorMode::dry ? 0.f : trimGain);
        output_.setTarget(outputGain);
    }
    research::StereoFrame process(const research::StereoFrame& source,
                                  const research::StereoFrame& protectedE) noexcept {
        const float x = carrier_.getNextValue(), e = effect_.getNextValue(),
                    g = output_.getNextValue();
        return {(x * source[0] + e * protectedE[0]) * g, (x * source[1] + e * protectedE[1]) * g};
    }

  private:
    LinearSmoother carrier_, effect_, output_;
};
} // namespace frazil::water::preview
