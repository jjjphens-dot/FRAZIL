#pragma once

#include "WaterExcitationFeatures.h"

#include <string_view>

namespace frazil::water::research {

// EXP-W-RX-001 comparison identities, not product parameters or a selected default.
enum class ModalExcitation { raw, hard, softsign, tanh, feature };

inline constexpr std::array kModalExcitationNames{"raw", "hard", "softsign", "tanh", "feature"};
inline const char* modalExcitationName(ModalExcitation mode) noexcept {
    const auto index = static_cast<std::size_t>(mode);
    return index < kModalExcitationNames.size() ? kModalExcitationNames[index] : "invalid";
}
inline bool parseModalExcitation(std::string_view name, ModalExcitation& mode) noexcept {
    for (std::size_t i = 0; i < kModalExcitationNames.size(); ++i)
        if (name == kModalExcitationNames[i]) {
            mode = static_cast<ModalExcitation>(i);
            return true;
        }
    return false;
}

// Only the modal residual's source carrier is conditioned. A common nonnegative linked gain
// preserves stereo ratios/signs without crossfeed. Bounded candidates have |e[channel]| <= 1
// for every finite float input. Raw is the exact historical control, not a bounded candidate.
class ModalExcitationConditioner final {
  public:
    bool prepare(double rate, ModalExcitation mode = ModalExcitation::raw) noexcept {
        ready_ = false;
        reset();
        if (mode < ModalExcitation::raw || mode > ModalExcitation::feature ||
            !features_.prepare(rate))
            return false;
        mode_ = mode;
        ready_ = true;
        return true;
    }

    void reset() noexcept {
        features_.reset();
    }

    StereoFrame process(const StereoFrame& input) noexcept {
        if (!ready_)
            return {};
        if (mode_ == ModalExcitation::raw)
            return input;
        const double magnitude = std::max(std::abs(static_cast<double>(input[0])),
                                          std::abs(static_cast<double>(input[1])));
        double gain{};
        switch (mode_) {
        case ModalExcitation::hard:
            gain = 1.0 / std::max(1.0, magnitude);
            break;
        case ModalExcitation::softsign:
            gain = 1.0 / (1.0 + magnitude);
            break;
        case ModalExcitation::tanh:
            gain = magnitude == 0 ? 1.0 : std::tanh(magnitude) / magnitude;
            break;
        case ModalExcitation::feature: {
            const auto features = features_.process(input);
            // Candidate constants, not perceptual truth: bounded envelope on a linked carrier.
            // The .001 floor avoids dividing by tiny samples; zero source always gives zero e,
            // even while feature envelopes release. No oscillator or autonomous events exist.
            const double envelope =
                std::min(1.0, 2 * features.fast + features.slow + 2 * features.transient);
            // Fast retains the waveform scale between peaks instead of dividing each sample
            // by its own magnitude (which would flatten a sinusoidal carrier).
            gain = envelope / std::max({.001, magnitude, features.fast});
            break;
        }
        default:
            return {};
        }
        return {static_cast<float>(static_cast<double>(input[0]) * gain),
                static_cast<float>(static_cast<double>(input[1]) * gain)};
    }

  private:
    WaterExcitationFeatures features_;
    ModalExcitation mode_{ModalExcitation::raw};
    bool ready_{};
};
} // namespace frazil::water::research
