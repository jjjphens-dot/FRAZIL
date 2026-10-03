#include "dsp/DropletB2.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <numbers>
using namespace frazil::water::research;
int main() {
    int failures{};
    const auto check = [&](bool ok, const char* message) {
        if (!ok) {
            ++failures;
            std::cerr << message << '\n';
        }
    };
    for (double rate : {44100., 48000., 96000.}) {
        auto changed = std::make_unique<DropletB2>();
        check(changed->prepare({rate, 42}), "performance prepare");
        bool allFinite = true;
        // A sustained sinusoid/noise is a diagnostic: false-onset count is reported,
        // not turned into a subjective threshold. Measure source-linked callback cost.
        for (int fixture = 0; fixture < 2; ++fixture) {
            changed->reset();
            std::uint32_t noise = 123;
            double peakUs = 0, sumUs = 0;
            int blocks = 0;
            for (int base = 0; base < static_cast<int>(rate); base += 256) {
                const auto start = std::chrono::steady_clock::now();
                for (int j = 0; j < 256; ++j) {
                    noise = noise * 1664525u + 1013904223u;
                    const float x =
                        fixture == 0 ? .25f * static_cast<float>(std::sin(2 * std::numbers::pi *
                                                                          440 * (base + j) / rate))
                                     : .25f * (static_cast<float>(noise >> 8) / 8388608.f - 1.f);
                    const auto out = changed->process({x, x});
                    allFinite &= std::isfinite(out[0]) && std::isfinite(out[1]);
                }
                const double us = std::chrono::duration<double, std::micro>(
                                      std::chrono::steady_clock::now() - start)
                                      .count();
                peakUs = std::max(peakUs, us);
                sumUs += us;
                ++blocks;
            }
            check(allFinite, "B2 steady sine/noise finite");
            std::cout << "B2 diagnostic rate=" << rate << " fixture=" << fixture
                      << " eligible=" << changed->counters().eligible
                      << " mean_callback_us=" << sumUs / blocks << " max_callback_us=" << peakUs
                      << " callback_budget_us=" << 256e6 / rate << '\n';
        }
    }
    return failures ? 1 : 0;
}
