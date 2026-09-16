#include "dsp/FlowModulator.h"

#include <iostream>
#include <limits>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok) { failures += ok ? 0 : 1; };
    for (double rate : {44100.0, 48000.0, 96000.0}) {
        FlowModulator flow;
        const ResearchConfig config{rate, 42u};
        check(flow.prepare(config));
        for (int i = 0; i < 1000; ++i)
            check(flow.process({}) == StereoFrame{});
        flow.reset();
        std::vector<StereoFrame> reference;
        for (int i = 0; i < static_cast<int>(rate); ++i) {
            const auto y = flow.process({static_cast<float>(std::sin(i * .7)), 0.0f});
            check(std::isfinite(y[0]) && std::abs(y[0]) <= .2 && y[1] == 0.0f);
            check(flow.lastDelaySamples() >= .003 * rate && flow.lastDelaySamples() <= .005 * rate);
            reference.push_back(y);
        }
        check(flow.prepare(config));
        for (std::size_t i = 0; i < reference.size(); ++i)
            check(flow.process({static_cast<float>(std::sin(i * .7)), 0.0f}) == reference[i]);
        for (int i = 0; i < static_cast<int>(rate * .03); ++i)
            (void)flow.process({});
        check(flow.process({}) == StereoFrame{});
        check(flow.prepare(config, {.01, .009, .02, .15}));
        for (int i = 0; i < 10000; ++i) {
            const float x =
                i % 2 ? std::numeric_limits<float>::max() : -std::numeric_limits<float>::max();
            check(std::isfinite(flow.process({x, 0.0f})[0]));
        }
        check(!flow.prepare(config, {.001, .002, .1, .1}));
        check(flow.process({1.0f, 1.0f}) == StereoFrame{});
    }
    std::cout << "flow failures=" << failures << '\n';
    return failures ? 1 : 0;
}
