#include "dsp/ProtectDetector.h"

#include <iostream>
#include <limits>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok) { failures += ok ? 0 : 1; };
    for (double rate : {44100.0, 48000.0, 96000.0}) {
        ProtectDetector high, low, right, capped;
        check(high.prepare(rate) && low.prepare(rate) && right.prepare(rate) &&
              capped.prepare(rate));
        for (int i = 0; i < 1000; ++i)
            check(high.process({}).logRatioDb == 0.0);
        high.reset();
        double maximumRatioError{}, maximumDifference{};
        for (int i = 0; i < 12000; ++i) {
            const auto a = high.process({.5f, 0.0f});
            const auto b = low.process({.05f, 0.0f});
            const auto c = right.process({0.0f, -.5f});
            check(a.fast == c.fast && a.slow == c.slow && a.logRatioDb == c.logRatioDb);
            if (b.slow > .001)
                maximumRatioError =
                    std::max(maximumRatioError, std::abs(a.logRatioDb - b.logRatioDb));
            check(std::abs(a.difference - 10.0 * b.difference) < 1.0e-7);
            maximumDifference = std::max(maximumDifference, a.difference);
            const auto saturated = capped.process({std::numeric_limits<float>::max(), 0.0f});
            check(std::isfinite(saturated.logRatioDb) && saturated.fast <= 1.0);
        }
        check(maximumRatioError < .001 && maximumDifference > .1);
        high.reset();
        low.reset();
        const auto impulse = high.process({.5f, 0.0f});
        check(impulse.difference > 0.0); // Causal envelope lag is observable, not zero onset delay.
        for (int i = 0; i < 1000; ++i)
            check(low.process({1.0e-6f, 0.0f}).logRatioDb == 0.0);
        high.reset();
        check(high.process({.5f, 0.0f}).difference == impulse.difference);
        check(!high.prepare(rate, 0.0) && high.process({1.0f, 0.0f}).difference == 0.0);
        check(!high.prepare(rate, .001, .1));
    }
    ProtectDetector detector;
    check(!detector.prepare(std::numeric_limits<double>::quiet_NaN()));
    check(!detector.prepare(0.0));
    std::cout << "protect detector failures=" << failures << '\n';
    return failures ? 1 : 0;
}
