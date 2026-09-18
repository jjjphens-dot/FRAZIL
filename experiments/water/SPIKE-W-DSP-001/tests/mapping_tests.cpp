#include "preview/ResearchWaterMacroMapper.h"

#include <iostream>
#include <limits>

int main() {
    using namespace frazil::water::preview;
    int failures{};
    const auto check = [&](bool ok) { failures += !ok; };
    const auto near = [](double a, double b) { return std::abs(a - b) < 1e-10; };
    for (int i = 0; i <= 1000; ++i) {
        const double v = i / 1000.0;
        const auto result = ResearchWaterMacroMapper::map({WaterModel::fluid, v, v, v});
        check(result.has_value());
        const auto& f = result->fluid;
        const auto& c = result->resonant;
        check(f.bubbleMinimumHz >= 125 && f.bubbleMaximumHz <= 5600 && f.dropletMinimumHz >= 300 &&
              f.dropletMaximumHz <= 9000);
        check(f.bubbleRateHz >= 30 && f.bubbleRateHz <= 480 && c.motionDepth <= .35);
        for (double rate : {44100., 48000., 96000.})
            check(.004 - f.flowDepthSeconds >= 1 / rate && .004 + f.flowDepthSeconds <= .020);
        if (i > 0) {
            const auto previous =
                *ResearchWaterMacroMapper::map({WaterModel::fluid, v - .001, v - .001, v - .001});
            check(f.bubbleMinimumHz < previous.fluid.bubbleMinimumHz &&
                  f.bubbleRateHz > previous.fluid.bubbleRateHz &&
                  f.dropletThreshold < previous.fluid.dropletThreshold &&
                  f.flowIntervalSeconds < previous.fluid.flowIntervalSeconds &&
                  c.decaySeconds > previous.resonant.decaySeconds);
        }
    }
    for (int i = 0; i < 3; ++i) {
        const double v = i * .5;
        const auto target = *ResearchWaterMacroMapper::map({WaterModel::resonant, v, v, v});
        constexpr double frequencies[]{520, 260, 130}, rates[]{30, 120, 480};
        constexpr double bubbleDecay[]{.02, .07, .245}, dropletDecay[]{.004, .012, .036};
        constexpr double modalDecay[]{.03, .12, .48}, modalInterval[]{1.96, .7, .25};
        check(near(target.resonant.rootHz, frequencies[i]) &&
              near(target.fluid.bubbleRateHz, rates[i]) &&
              near(target.fluid.bubbleDecaySeconds, bubbleDecay[i]) &&
              near(target.fluid.dropletDecaySeconds, dropletDecay[i]) &&
              near(target.resonant.decaySeconds, modalDecay[i]) &&
              near(target.resonant.motionIntervalSeconds, modalInterval[i]));
    }
    check(!ResearchWaterMacroMapper::map(
        {WaterModel::fluid, std::numeric_limits<double>::quiet_NaN(), .5, .5}));
    check(ResearchWaterMacroMapper::map({WaterModel::fluid, -1, 2, .5})->fluid.bubbleMinimumHz ==
          500);
    std::cout << "research mapping failures=" << failures << '\n';
    return failures ? 1 : 0;
}
