#include "dsp/DropletImpactExciter.h"
#include "preview/ResearchWaterMacroMapper.h"

#include <iostream>
#include <limits>
#include <source_location>
#include <vector>
using namespace frazil::water::research;
int main() {
    int failures{};
    const auto check = [&](bool ok,
                           std::source_location location = std::source_location::current()) {
        if (!ok && failures++ < 10)
            std::cerr << "FAIL activity line " << location.line() << '\n';
    };
    for (double rate : {44100., 48000., 96000.}) {
        // Space impulses beyond detector hysteresis release and voice expiry. Eligibility is
        // intentionally unchanged; activity only thins otherwise-valid onsets.
        const int period = static_cast<int>(rate * .25);
        for (double activity : {0., .25, .5, 1.}) {
            DropletConfig config;
            config.decaySeconds = .002;
            config.transientThreshold = .0001;
            config.eventActivity = activity;
            DropletImpactExciter gated, full, rightOnly;
            check(gated.prepare({rate, 42}, config));
            check(rightOnly.prepare({rate, 42}, config));
            config.eventActivity = 1;
            check(full.prepare({rate, 42}, config));
            RandomSource probability(
                ResearchConfig{rate, 42}.seedFor(RandomDomain::dropletActivity));
            bool accepted{};
            std::uint64_t expected{};
            std::vector<StereoFrame> reference;
            for (int i = 0; i < period * 40 + 3; ++i) {
                const bool onset = i % period == 0;
                const StereoFrame x{onset ? std::numeric_limits<float>::max() : 0.f, 0};
                if (onset) {
                    accepted = activity >= 1 || probability.nextUnipolar() < activity;
                    expected += accepted;
                }
                const auto y = gated.process(x), all = full.process(x);
                const auto right = rightOnly.process({0, x[0]});
                check(right[0] == 0 && right[1] == y[0]);
                check(std::isfinite(y[0]) && y[1] == 0 && gated.events() == expected);
                check(y ==
                      (accepted ? all : StereoFrame{})); // Shared eligible onsets retain family.
                if (i < 12003)
                    reference.push_back(y);
            }
            check((activity == 0) == (gated.events() == 0));
            for (int block : {32, 64, 128, 256, 257, 512, 1024}) {
                gated.reset();
                for (int start = 0; start < 12003; start += block)
                    for (int i = start; i < std::min(start + block, 12003); ++i) {
                        const StereoFrame x{
                            i % period == 0 ? std::numeric_limits<float>::max() : 0.f, 0};
                        check(gated.process(x) == reference[i]);
                    }
            }
            check(!gated.setEventActivity(-.1) && !gated.setEventActivity(1.1));
        }
        DropletImpactExciter tail;
        check(tail.prepare({rate, 42}));
        for (int i = 0; i < 100 && tail.events() == 0; ++i)
            (void)tail.process({1, 0});
        check(tail.events() == 1 && tail.setEventActivity(0));
        bool persists{};
        for (int i = 0; i < 1000; ++i)
            persists |= tail.process({})[0] != 0;
        check(persists && tail.events() == 1);
        DropletConfig invalid;
        invalid.eventActivity = std::numeric_limits<double>::quiet_NaN();
        check(!tail.prepare({rate, 42}, invalid) && tail.process({1, 1}) == StereoFrame{});
    }
    using frazil::water::preview::ResearchWaterMacroMapper;
    check(ResearchWaterMacroMapper::continuousDropletActivity(0) == 0);
    check(ResearchWaterMacroMapper::continuousDropletActivity(.25) == .25);
    check(ResearchWaterMacroMapper::continuousDropletActivity(.5) == 1);
    check(ResearchWaterMacroMapper::continuousDropletActivity(1) == 1);
    check(!ResearchWaterMacroMapper::continuousDropletActivity(
        std::numeric_limits<double>::quiet_NaN()));
    std::cout << "activity failures=" << failures << '\n';
    return failures ? 1 : 0;
}
