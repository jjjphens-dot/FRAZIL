#pragma once
#include "DropletB1Config.h"
#include "SharedExcitationAnalyzer.h"

#include <cstdint>

namespace frazil::water::research {
struct DropletB1Onset final {
    bool eligible{};
    double noveltyDb{};
};
// ENGINEERING source analysis: linked relative energy, not detection of physical drops.
class DropletB1OnsetDetector final {
  public:
    void prepare(double rate, const DropletB1Config& c) noexcept {
        threshold_ = c[B1Parameter::onset];
        rearm_ = threshold_ - c[B1Parameter::hysteresis];
        floor_ = std::pow(10., c[B1Parameter::floor] / 10);
        spacing_ = static_cast<std::uint64_t>(std::ceil(rate * c[B1Parameter::spacing] * .001));
        reset();
    }
    void reset() noexcept {
        remaining_ = 0;
        armed_ = true;
    }
    DropletB1Onset process(const SharedExcitation& source) noexcept {
        constexpr double kPowerEpsilon = 1e-20;
        const double novelty = 10 * std::log10((source.fastPower + kPowerEpsilon) /
                                               (source.slowPower + kPowerEpsilon));
        if (remaining_ != 0)
            --remaining_;
        // Silence must re-arm even for hysteresis >= onset threshold (negative rearm dB).
        const bool present = source.rms * source.rms >= floor_ && source.fastPower >= floor_;
        if (!present || novelty <= rearm_)
            armed_ = true;
        const bool eligible = present && armed_ && remaining_ == 0 && novelty >= threshold_;
        if (eligible) {
            armed_ = false;
            remaining_ = spacing_;
        }
        return {eligible, novelty};
    }

  private:
    double threshold_{}, rearm_{}, floor_{}; // Prepared power/dB decision bounds.
    std::uint64_t spacing_{}, remaining_{};  // Audio-owner refractory samples; reset clears.
    bool armed_{true}; // Hysteresis latch, consumed even when admission later rejects.
};
} // namespace frazil::water::research
