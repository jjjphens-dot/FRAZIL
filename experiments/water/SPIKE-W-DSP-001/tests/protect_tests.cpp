#include "dsp/FluidCandidate.h"
#include "dsp/FluidProtect.h"
#include "dsp/LiquidModalResonator.h"
#include "dsp/ResidualProtect.h"

#include <iostream>
#include <limits>
#include <vector>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok) { failures += ok ? 0 : 1; };
    const FluidResiduals cancellation{{1.0f, -1.0f}, {-1.0f, 1.0f}, {}};
    check(cancellation.sum() == StereoFrame{});
    check(applyFluidProtect(cancellation, .5, FluidProtectTopology::whole) == StereoFrame{});
    check(applyFluidProtect(cancellation, .5, FluidProtectTopology::dropletExempt)[0] == -.5f);
    check(applyFluidProtect(cancellation, .5, FluidProtectTopology::dropletHalf)[0] == -.25f);
    for (auto topology : {FluidProtectTopology::whole, FluidProtectTopology::dropletExempt,
                          FluidProtectTopology::dropletHalf})
        check(applyFluidProtect(cancellation, 1.0, topology) == cancellation.sum());
    for (double rate : {44100.0, 48000.0, 96000.0}) {
        for (double cap : {3.0, 6.0, 9.0, 12.0}) {
            ProtectConfig extremes;
            extremes.capDb = cap;
            extremes.attackSeconds = .00025;
            extremes.releaseSeconds = .2;
            check(protectTargetDb(0.0, 1.0, extremes) == 0.0);
            check(protectTargetDb(100.0, 1.0, extremes) == -cap);
            ResidualProtect bounded;
            check(bounded.prepare(rate, extremes, 1.0));
            double lastTarget{};
            for (int i = 0; i <= 1000; ++i) {
                const double target = protectTargetDb(100.0, i / 1000.0, extremes);
                check(target <= lastTarget && target >= -cap);
                lastTarget = target;
                const double gain = bounded.processSource({i % 2 ? 1.0f : -1.0f, 0.0f});
                check(gain >= std::pow(10.0, -cap / 20.0) - 1e-14 && gain <= 1.0);
                const auto y = ResidualProtect::apply({std::numeric_limits<float>::max(), 0}, gain);
                check(std::isfinite(y[0]) && y[1] == 0.0f);
            }
        }
        for (auto kind : {ProtectScore::difference, ProtectScore::logRatio}) {
            ProtectConfig config;
            config.score = kind;
            if (kind == ProtectScore::difference) {
                config.thresholdLow = .01;
                config.thresholdHigh = .12;
            }
            ResidualProtect protect, off;
            check(protect.prepare(rate, config, 1.0) && off.prepare(rate, config, 0.0));
            FluidCandidate baseline, candidate;
            LiquidModalResonator modalBase, modalCandidate;
            check(baseline.prepare({rate, 42}) && candidate.prepare({rate, 42}));
            check(modalBase.prepare(rate) && modalCandidate.prepare(rate));
            const int offAt = 12000;
            const auto ramp = static_cast<int>(std::ceil(rate * config.offSeconds));
            std::vector<double> gains;
            double previousGain = 1.0, maxStep{}, maxGr{};
            for (int i = 0; i < 30000; ++i) {
                const StereoFrame x{i % 4096 < 700 ? .5f : 0.0f, 0.0f};
                if (i >= offAt)
                    check(protect.setDepth(0.0));
                const double g = protect.processSource(x);
                check(g >= std::pow(10.0, -config.capDb / 20.0) - 1e-14 && g <= 1.0);
                if (i > offAt && i < offAt + ramp)
                    maxStep = std::max(maxStep, std::abs(g - previousGain));
                maxGr = std::max(maxGr, protect.reductionDb());
                const auto e0 = baseline.process(x), e1 = candidate.process(x);
                const auto c0 = modalBase.process(x), c1 = modalCandidate.process(x);
                check(e0 == e1 && c0 == c1); // Full sample stream compares state/RNG continuation.
                check(ResidualProtect::apply(e1, off.processSource(x)) == e0);
                const auto y = ResidualProtect::apply(e1, g), yc = ResidualProtect::apply(c1, g);
                check(std::abs(y[0]) <= std::abs(e0[0]) && y[1] == 0.0f);
                check(std::abs(yc[0]) <= std::abs(c0[0]) && yc[1] == 0.0f);
                if (i >= offAt + ramp - 1)
                    check(g == 1.0 && y == e0 && yc == c0);
                previousGain = g;
                gains.push_back(g);
            }
            check(maxGr > 1.0 && maxStep < .01);
            for (int block : {1, 7, 31, 32, 128, 1024}) {
                check(protect.prepare(rate, config, 1.0));
                for (int start = 0; start < 30000; start += block)
                    for (int i = start; i < std::min(start + block, 30000); ++i) {
                        if (i >= offAt)
                            check(protect.setDepth(0.0));
                        check(protect.processSource({i % 4096 < 700 ? .5f : 0.0f, 0.0f}) ==
                              gains[i]);
                    }
            }
            check(!protect.setDepth(std::numeric_limits<double>::quiet_NaN()));
            check(!protect.setDepth(-1));
            for (int i = 0; i < 10000; ++i) {
                check(protect.setDepth(i % 2 ? .2 : 1.0));
                const auto g = protect.processSource({.3f, -.8f});
                check(std::isfinite(g) && g > 0 && g <= 1);
            }
            protect.reset();
            check(protect.setDepth(0.0) && protect.processSource({}) == 1.0);
            config.releaseSeconds = -1;
            check(!protect.prepare(rate, config) && protect.processSource({1, 1}) == 1.0);
        }
    }
    std::cout << "protect failures=" << failures << '\n';
    return failures ? 1 : 0;
}
