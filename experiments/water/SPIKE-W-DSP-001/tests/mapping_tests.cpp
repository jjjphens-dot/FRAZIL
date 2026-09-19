#include "dsp/BubbleEnsemble.h"
#include "dsp/DropletImpactExciter.h"
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
        check(f.bubbleRateHz >= 0 && f.bubbleRateHz <= 480 && c.motionDepth <= .35);
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
        constexpr double frequencies[]{520, 260, 130}, rates[]{0, 120, 480};
        constexpr double bubbleDecay[]{.02, .07, .245}, dropletDecay[]{.004, .012, .036};
        constexpr double modalDecay[]{.03, .12, .48}, modalInterval[]{1.96, .7, .25};
        check(target.fluid.dropletEventsEnabled == (i == 0 ? 0 : 1));
        check(near(target.resonant.rootHz, frequencies[i]) &&
              near(target.fluid.bubbleRateHz, rates[i]) &&
              near(target.fluid.bubbleDecaySeconds, bubbleDecay[i]) &&
              near(target.fluid.dropletDecaySeconds, dropletDecay[i]) &&
              near(target.resonant.decaySeconds, modalDecay[i]) &&
              near(target.resonant.motionIntervalSeconds, modalInterval[i]));
    }
    // Exercise actual scheduling, not only the endpoint numbers. A transient, gated sine
    // and sustained source must remain unable to create either kind of event at Motion=0.
    for (double rate : {44100., 48000., 96000.}) {
        using namespace frazil::water::research;
        const auto zero = ResearchWaterMacroMapper::map({WaterModel::fluid, .5, 0, .5})->fluid;
        BubbleConfig a;
        DropletConfig b;
        a.maximumEventRateHz = zero.bubbleRateHz;
        b.eventsEnabled = zero.dropletEventsEnabled;
        for (int signal = 0; signal < 3; ++signal) {
            BubbleEnsemble bubble;
            DropletImpactExciter droplet;
            check(bubble.prepare({rate, 42}, a) && droplet.prepare({rate, 42}, b));
            for (int i = 0; i < static_cast<int>(rate); ++i) {
                const float x = signal == 0   ? (i % 1000 == 0 ? 1.f : 0.f)
                                : signal == 1 ? (i % 10000 < 5000 ? .8f * std::sin(i * .1f) : 0.f)
                                              : .8f;
                (void)bubble.process({x, x});
                (void)droplet.process({x, x});
            }
            check(bubble.events() == 0 && droplet.events() == 0);
        }
    }
    check(!ResearchWaterMacroMapper::map(
        {WaterModel::fluid, std::numeric_limits<double>::quiet_NaN(), .5, .5}));
    check(ResearchWaterMacroMapper::map({WaterModel::fluid, -1, 2, .5})->fluid.bubbleMinimumHz ==
          500);
    std::cout << "research mapping failures=" << failures << '\n';
    return failures ? 1 : 0;
}
