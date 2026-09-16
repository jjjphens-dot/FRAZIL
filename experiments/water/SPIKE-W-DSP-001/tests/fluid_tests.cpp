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
        // Semantic errors in disabled modules cannot affect the enabled component's stream.
        FluidConfig onlyA;
        onlyA.dropletEnabled = onlyA.flowEnabled = false;
        onlyA.droplet.voices = 0;
        onlyA.flow.depthSeconds = -1.0;
        FluidCandidate isolated;
        BubbleEnsemble bubble;
        check(isolated.prepare(config, onlyA) && bubble.prepare(config));
        for (auto x : input)
            check(isolated.process(x) == bubble.process(x));
        check(isolated.bubbleEvents() > 0 && isolated.dropletEvents() == 0);
        FluidConfig onlyD;
        onlyD.bubbleEnabled = onlyD.dropletEnabled = false;
        onlyD.bubble.voices = std::numeric_limits<std::size_t>::max();
        onlyD.droplet.decaySeconds = -1.0;
        FlowModulator flow;
        check(isolated.prepare(config, onlyD) && flow.prepare(config));
        check(isolated.bubbleEvents() == 0 && isolated.dropletEvents() == 0);
        for (auto x : input)
            check(isolated.process(x) == flow.process(x));
        onlyD.bubbleEnabled = true;
        check(!isolated.prepare(config, onlyD));
        isolated.reset();
        check(isolated.process({1.0f, 0.0f}) == StereoFrame{});
        check(isolated.prepare(config, onlyA));
        bubble.reset();
        for (auto x : input)
            check(isolated.process(x) == bubble.process(x));
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
        check(all.bubbleEvents() == 0 && all.dropletEvents() == 0);
        for (auto x : input)
            check(all.process(x) == StereoFrame{});
        check(all.prepare(config));
        for (int i = 0; i < 10000; ++i)
            check(std::isfinite(all.process({std::numeric_limits<float>::max(), 0.0f})[0]));
    }
    std::cout << "fluid failures=" << failures << '\n';
    return failures ? 1 : 0;
}
