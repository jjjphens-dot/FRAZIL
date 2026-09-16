#include "dsp/WaterExcitationFeatures.h"

#include <cmath>
#include <iostream>
#include <limits>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok) { failures += ok ? 0 : 1; };
    for (double rate : {44100.0, 48000.0, 96000.0}) {
        WaterExcitationFeatures detector;
        check(detector.prepare(rate));
        const auto first = detector.process({1.0f, 0.0f});
        check(std::abs(first.fast - (1.0 - std::exp(-1.0 / (0.001 * rate)))) < 1.0e-14);
        check(first.transient > 0.0);
        detector.reset();
        const auto right = detector.process({0.0f, -1.0f});
        check(right.fast == first.fast && right.slow == first.slow);
        for (int i = 1; i < static_cast<int>(rate); ++i)
            (void)detector.process({1.0f, 0.0f});
        const auto steady = detector.process({1.0f, 0.0f});
        const auto release = detector.process({});
        check(std::abs(release.fast - steady.fast * std::exp(-1.0 / (0.03 * rate))) < 1.0e-14);
        for (int i = 0; i < static_cast<int>(rate * 8); ++i)
            (void)detector.process({});
        check(detector.process({}).slow == 0.0);
        check(detector.prepare(rate));
        check(detector.process({1.0f, 0.0f}).fast == first.fast);
        const auto large = detector.process({std::numeric_limits<float>::max(), 0.0f});
        check(std::isfinite(large.fast) && large.fast <= 1.0);
        check(!detector.prepare(rate, {0.0, 0.1, 0.1, 0.1}));
        check(detector.process({1.0f, 1.0f}).fast == 0.0);
    }
    std::cout << "feature failures=" << failures << '\n';
    return failures ? 1 : 0;
}
