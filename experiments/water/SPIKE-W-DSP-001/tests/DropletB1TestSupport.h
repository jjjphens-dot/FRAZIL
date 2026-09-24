#pragma once
#include "dsp/DropletB1.h"

#include <cmath>
#include <iostream>
#include <numbers>

namespace b1test {
struct Checks final {
    int failures{};
    void operator()(bool ok, const char* why) {
        if (!ok && failures++ < 20)
            std::cerr << why << '\n';
    }
};
inline bool near(double a, double b, double tolerance = 1e-10) {
    return std::abs(a - b) <= tolerance * std::max({1., std::abs(a), std::abs(b)});
}
inline frazil::water::research::DropletB1Event
event(double rate, const frazil::water::research::DropletB1Config& config = {}) {
    using namespace frazil::water::research;
    DropletB1EntrainmentModel model;
    model.prepare({rate, 42});
    DropletB1Event e;
    model.define(e, {17, {1, -.3}, .5, .25, 9}, DropletB1Model::make(config), config, rate, 1);
    return e;
}
inline frazil::water::research::StereoFrame source(std::uint64_t n, double rate) {
    // Abrupt 20 ms musical-energy bursts separated by 230 ms; deterministic and bipolar.
    const double phase = std::fmod(double(n) / rate, .25);
    const float x =
        phase < .02 ? static_cast<float>(.7 * std::cos(2 * std::numbers::pi * 173 * n / rate)) : 0;
    return {x, -.3f * x};
}
} // namespace b1test
