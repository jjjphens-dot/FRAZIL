#include "dsp/BubbleEnsemble.h"
#include "dsp/DropletImpactExciter.h"

#include <iostream>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok) { failures += ok ? 0 : 1; };
    for (double rate : {44100.0, 48000.0, 96000.0}) {
        DropletImpactExciter droplet, repeated;
        BubbleEnsemble bubble, reference;
        WaterExcitationFeatures detector;
        const ResearchConfig config{rate, 42u};
        check(droplet.prepare(config) && repeated.prepare(config));
        check(bubble.prepare(config) && reference.prepare(config) && detector.prepare(rate));
        for (int i = 0; i < 1000; ++i)
            check(droplet.process({}) == StereoFrame{});
        droplet.reset();
        for (int i = 0; i < static_cast<int>(rate * 2.0); ++i) {
            const int period = static_cast<int>(rate * .5);
            const StereoFrame x{i % period < rate * .05 ? 0.8f : 0.0f, 0.0f};
            const auto feature = detector.process(x);
            const auto before = droplet.events();
            const auto y = droplet.process(x);
            check(y == repeated.process(x));
            check(std::isfinite(y[0]) && std::abs(y[0]) <= .15 && y[1] == 0.0f);
            if (droplet.events() != before)
                check(feature.transient > .015 && x[0] != 0.0f);
            check(bubble.process(x) == reference.process(x));
        }
        check(droplet.events() == 4);
        const auto events = droplet.events();
        for (int i = 0; i < static_cast<int>(rate); ++i)
            (void)droplet.process({});
        check(droplet.events() == events && droplet.activeVoices() == 0);
        check(droplet.prepare(config));
        check(droplet.events() == 0 && droplet.process({}) == StereoFrame{});
        for (double frequency : {40.0, rate * .45})
            for (double decay : {.002, .1})
                for (double threshold : {.0001, 1.0}) {
                    DropletConfig edge{frequency, frequency, decay, threshold, .001, .3, 1};
                    check(droplet.prepare(config, edge));
                    for (int i = 0; i < 4097; ++i) {
                        const auto y = droplet.process({i % 256 < 64 ? 1.0f : 0.0f, 0.0f});
                        check(std::isfinite(y[0]) && std::abs(y[0]) <= .300001 && y[1] == 0.0f);
                    }
                }
        DropletConfig invalid;
        invalid.voices = 17;
        check(!droplet.prepare(config, invalid));
    }
    std::cout << "droplet failures=" << failures << '\n';
    return failures ? 1 : 0;
}
