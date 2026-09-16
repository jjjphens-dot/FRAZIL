#include "dsp/BubbleEnsemble.h"

#include <iostream>
#include <vector>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok) { failures += ok ? 0 : 1; };
    for (double rate : {44100.0, 48000.0, 96000.0}) {
        BubbleEnsemble bubble;
        ResearchConfig config{rate, 42u};
        check(bubble.prepare(config));
        for (int i = 0; i < 10000; ++i)
            check(bubble.process({}) == StereoFrame{});
        check(bubble.events() == 0);
        std::vector<StereoFrame> reference;
        for (int i = 0; i < static_cast<int>(rate); ++i) {
            const auto y = bubble.process({0.8f, 0.0f});
            check(std::isfinite(y[0]) && std::abs(y[0]) <= .2 && y[1] == 0.0f);
            check(bubble.activeVoices() <= 16);
            reference.push_back(y);
        }
        const auto events = bubble.events();
        check(events > 0);
        bubble.reset();
        for (const auto& value : reference)
            check(bubble.process({0.8f, 0.0f}) == value);
        check(bubble.events() == events);
        for (int i = 0; i < static_cast<int>(rate * 2); ++i)
            (void)bubble.process({});
        check(bubble.events() == events && bubble.activeVoices() == 0);
        check(bubble.process({}) == StereoFrame{});
        BubbleConfig dense;
        dense.voices = 1;
        dense.maximumEventRateHz = 2000.0;
        check(bubble.prepare(config, dense));
        for (int i = 0; i < 10000; ++i)
            (void)bubble.process({1.0f, 0.0f});
        check(bubble.steals() > 0 && bubble.activeVoices() == 1);
        for (double frequency : {40.0, rate * .45})
            for (double decay : {.002, .5})
                for (std::size_t voices : {1u, 16u}) {
                    BubbleConfig edge{frequency, frequency, decay, 2000.0, 0.0, .3, voices};
                    check(bubble.prepare(config, edge));
                    for (int i = 0; i < 4097; ++i) {
                        const auto y = bubble.process({i % 2 ? 1.0f : -1.0f, 0.0f});
                        check(std::isfinite(y[0]) && std::abs(y[0]) <= .300001 && y[1] == 0.0f);
                        check(bubble.activeVoices() <= voices);
                    }
                }
        dense.maximumFrequencyHz = rate;
        check(!bubble.prepare(config, dense));
        check(bubble.process({1.0f, 1.0f}) == StereoFrame{});
    }
    std::cout << "bubble failures=" << failures << '\n';
    return failures ? 1 : 0;
}
