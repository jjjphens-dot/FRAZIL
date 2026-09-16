#pragma once

#include <cmath>
#include <numbers>

namespace frazil::water::research::detail {

struct ResonatorCoefficients final {
    double real{};
    double imaginary{};
    double excitation{};
};

// Complex-pole realization avoids the near-DC state amplification of a direct-form all-pole
// recurrence. Call only during prepare with 40 <= f <= .45*fs, .002 <= decay <= 1 seconds.
inline ResonatorCoefficients makeResonator(double rate, double frequency, double decay) noexcept {
    const double radius = std::exp(-1.0 / (decay * rate));
    const double angle = 2.0 * std::numbers::pi * frequency / rate;
    return {radius * std::cos(angle), radius * std::sin(angle), 1.0 - radius};
}

// Single-owner Cartesian oscillator state. reset() clears both quadratures. A real excitation
// has no instantaneous imaginary feedthrough. No hidden carrier term exists in this residual.
class DampedResonator final {
  public:
    void reset() noexcept {
        real_ = imaginary_ = 0.0;
    }

    double process(double input, const ResonatorCoefficients& coefficient) noexcept {
        const double nextReal = coefficient.real * real_ - coefficient.imaginary * imaginary_ +
                                coefficient.excitation * input;
        imaginary_ = coefficient.imaginary * real_ + coefficient.real * imaginary_;
        real_ = nextReal;
        if (std::abs(real_) + std::abs(imaginary_) < 1.0e-25)
            reset();
        return imaginary_;
    }

  private:
    // Double state provides headroom for all finite float inputs; zeroed below the tail floor.
    double real_{};
    double imaginary_{};
};
} // namespace frazil::water::research::detail
