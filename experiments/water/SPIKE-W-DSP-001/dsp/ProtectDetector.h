#pragma once

#include "WaterExcitationFeatures.h"

#include <algorithm>
#include <cmath>

namespace frazil::water::research {

struct ProtectDetection final {
    double difference{}; // D0: normalized linear amplitude.
    double logRatioDb{}; // D1: positive dB ratio, zero below the slow-envelope floor.
    double fast{}, slow{};
};

// PROTECT-EXP-001: owns its follower independently from the Water generators. Reuses their
// linked/capped preprocessing without changing their excitation, scheduling or random streams.
class ProtectDetector final {
  public:
    bool prepare(double rate, double floor = 1.0e-4, double epsilon = 1.0e-8) noexcept {
        ready_ = false;
        reset();
        if (!std::isfinite(floor) || floor < 1.0e-8 || floor > 0.1 || !std::isfinite(epsilon) ||
            epsilon <= 0.0 || epsilon > floor || !features_.prepare(rate))
            return false;
        floor_ = floor;
        epsilon_ = epsilon;
        ready_ = true;
        return true;
    }

    void reset() noexcept {
        features_.reset();
    }

    ProtectDetection process(const StereoFrame& source) noexcept {
        if (!ready_)
            return {};
        const auto f = features_.process(source);
        const double ratio =
            f.slow >= floor_
                ? std::max(0.0, 20.0 * std::log10((f.fast + epsilon_) / (f.slow + epsilon_)))
                : 0.0;
        return {f.transient, ratio, f.fast, f.slow};
    }

  private:
    // One processing owner; this follower's history is independent of generator excitation.
    WaterExcitationFeatures features_; // Linked amplitude envelopes; reset clears follower history.
    // Linear-amplitude floor suppresses unstable quiet ratios; epsilon regularizes the division.
    // prepare validates/caches both; reset retains them while clearing only follower history.
    double floor_{}, epsilon_{};
    bool ready_{}; // Successful prepare enables processing; reset retains the prepared
                   // configuration.
};
} // namespace frazil::water::research
