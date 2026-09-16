#include "dsp/FluidCandidate.h"
#include "dsp/LiquidModalResonator.h"

#include <iostream>
#include <limits>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok) { failures += ok ? 0 : 1; };
    for (double rate : {44100.0, 48000.0, 96000.0}) {
        const ResearchConfig config{rate, 42u};
        FluidCandidate all, withoutB;
        FluidConfig ablation;
        ablation.dropletEnabled = false;
        check(all.prepare(config) && withoutB.prepare(config, ablation));
        std::vector<StereoFrame> input(32771), reference;
        RandomSource noise(19u);
        for (auto& x : input)
            x = {.8f * (noise.nextUnipolar() * 2.0f - 1.0f), 0.0f};
        double energy{};
        for (auto x : input) {
            const auto parts = all.processComponents(x);
            const auto other = withoutB.processComponents(x);
            check(parts.bubble == other.bubble && parts.flow == other.flow &&
                  other.droplet == StereoFrame{});
            const auto y = parts.sum();
            check(std::isfinite(y[0]) && y[1] == 0.0f && std::abs(y[0]) <= .55);
            energy += y[0] * y[0];
            reference.push_back(y);
        }
        check(energy > 0.0 && all.bubbleEvents() > 0 && all.dropletEvents() > 0);
        for (std::size_t block : {1u, 7u, 32u, 64u, 128u, 256u, 512u, 1024u}) {
            all.reset();
            for (std::size_t start = 0; start < input.size(); start += block)
                for (std::size_t i = start; i < std::min(start + block, input.size()); ++i)
                    check(all.process(input[i]) == reference[i]);
        }
        check(all.prepare(config));
        for (std::size_t i = 0; i < input.size(); ++i)
            check(all.process(input[i]) == reference[i]);
        ablation.bubbleEnabled = ablation.flowEnabled = false;
        check(all.prepare(config, ablation));
        for (auto x : input)
            check(all.process(x) == StereoFrame{});
        check(all.prepare(config));
        for (int i = 0; i < 10000; ++i)
            check(std::isfinite(all.process({std::numeric_limits<float>::max(), 0.0f})[0]));
    }
    std::cout << "fluid failures=" << failures << '\n';
    return failures ? 1 : 0;
}
