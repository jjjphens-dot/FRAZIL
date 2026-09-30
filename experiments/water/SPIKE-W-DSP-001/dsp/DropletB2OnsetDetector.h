#pragma once
#include "DropletB2Config.h"
#include "SharedExcitationAnalyzer.h"

#include <cstdint>

namespace frazil::water::research {
struct DropletB2Onset final {
    bool eligible{};
    double noveltyDb{}, positiveSlope{};
};
// ENGINEERING source analysis: linked relative energy, not detection of physical drops.
class DropletB2OnsetDetector final {
  public:
    void prepare(double rate, const DropletB2Config& config) noexcept {
        const auto c = config.baseline();
        hybrid_ = config[B2Parameter::detectorMode] == 1;
        slopeThreshold_ = config[B2Parameter::slope];
        samplesPerMs_ = rate * .001;
        threshold_ = c[B1Parameter::onset];
        rearm_ = threshold_ - c[B1Parameter::hysteresis];
        floor_ = std::pow(10., c[B1Parameter::floor] / 10);
        spacing_ = static_cast<std::uint64_t>(std::ceil(rate * c[B1Parameter::spacing] * .001));
        reset();
    }
    void reset() noexcept {
        remaining_ = 0;
        previousLogPower_ = 10 * std::log10(1e-20);
        armed_ = true;
    }
    DropletB2Onset process(const SharedExcitation& source) noexcept {
        constexpr double kPowerEpsilon = 1e-20;
        const double novelty = 10 * std::log10((source.fastPower + kPowerEpsilon) /
                                               (source.slowPower + kPowerEpsilon));
        const double logPower = 10 * std::log10(source.fastPower + kPowerEpsilon);
        const double slope = std::max(0., (logPower - previousLogPower_) * samplesPerMs_);
        previousLogPower_ = logPower;
        if (remaining_ != 0)
            --remaining_;
        // Silence must re-arm even for hysteresis >= onset threshold (negative rearm dB).
        const bool present = source.rms * source.rms >= floor_ && source.fastPower >= floor_;
        if (!present || (novelty <= rearm_ && (!hybrid_ || slope < slopeThreshold_ * .5)))
            armed_ = true;
        const bool eligible = present && armed_ && remaining_ == 0 &&
                              (novelty >= threshold_ || (hybrid_ && slope >= slopeThreshold_));
        if (eligible) {
            armed_ = false;
            remaining_ = spacing_;
        }
        return {eligible, novelty, slope};
    }

  private:
    double previousLogPower_{}, samplesPerMs_{}, slopeThreshold_{}; // Audio-owned dB and dB/ms.
    bool hybrid_{};
    double threshold_{}, rearm_{}, floor_{}; // Prepared power/dB decision bounds.
    std::uint64_t spacing_{}, remaining_{};  // Audio-owner refractory samples; reset clears.
    bool armed_{true}; // Hysteresis latch, consumed even when admission later rejects.
};
} // namespace frazil::water::research
