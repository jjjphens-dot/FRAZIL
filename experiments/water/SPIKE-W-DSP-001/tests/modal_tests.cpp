#include "dsp/LiquidModalResonator.h"

#include <iostream>
#include <limits>
#include <vector>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok) { failures += ok ? 0 : 1; };
    for (double rate : {44100.0, 48000.0, 96000.0}) {
        LiquidModalResonator modal;
        check(modal.prepare(rate));
        check(modal.process({1.0f, 0.0f}) == StereoFrame{});
        double earlyEnergy{}, lateEnergy{};
        for (int i = 0; i < static_cast<int>(rate * 3); ++i) {
            const auto y = modal.process({});
            check(std::isfinite(y[0]) && y[1] == 0.0f);
            if (i < rate * 0.1)
                earlyEnergy += y[0] * y[0];
            if (i > rate * 2.0)
                lateEnergy += y[0] * y[0];
        }
        check(earlyEnergy > 0.0 && lateEnergy < earlyEnergy * 1.0e-10);
        modal.reset();
        std::vector<StereoFrame> reference;
        for (int i = 0; i < 1027; ++i)
            reference.push_back(modal.process({static_cast<float>(std::sin(i * .17)), 0.0f}));
        modal.reset();
        for (int i = 0; i < 1027; ++i)
            check(reference[i] == modal.process({static_cast<float>(std::sin(i * .17)), 0.0f}));
        for (const auto config :
             {ModalConfig{40.0, 1.0, 0.3}, ModalConfig{rate * .45 / 4.17, .002, .3}}) {
            check(modal.prepare(rate, config));
            for (int i = 0; i < 10000; ++i) {
                const auto y = modal.process({std::numeric_limits<float>::max(), 0.0f});
                check(std::isfinite(y[0]) && y[1] == 0.0f);
            }
        }
        check(!modal.prepare(rate, {rate, .1, .1}));
        check(modal.process({1.0f, 1.0f}) == StereoFrame{});
        // Independent historical recurrence verifies depth-zero compatibility, including arithmetic
        // ordering; a second instance of the new class would not establish this property.
        std::array<detail::DampedResonator, 6> historical;
        std::array<detail::ResonatorCoefficients, 6> coefficients;
        constexpr std::array ratios{1., 1.41, 1.93, 2.57, 3.31, 4.17};
        for (std::size_t i = 0; i < ratios.size(); ++i)
            coefficients[i] = detail::makeResonator(rate, 260 * ratios[i], .12);
        check(modal.prepare(rate));
        for (int i = 0; i < 12000; ++i) {
            const float x = .4f * static_cast<float>(std::sin(i * .17));
            double sum{};
            for (std::size_t j = 0; j < ratios.size(); ++j)
                sum += historical[j].process(x, coefficients[j]);
            check(modal.process({x, 0})[0] == static_cast<float>((.18 / 6) * sum));
        }
        for (double depth : {0., .175, .35}) {
            const ModalConfig moving{260, .12, .3, depth, .02};
            LiquidModalResonator repeat, otherSeed;
            check(modal.prepare({rate, 42}, moving) && repeat.prepare({rate, 42}, moving) &&
                  otherSeed.prepare({rate, 43}, moving));
            std::vector<StereoFrame> output;
            bool seedDifference{};
            for (int i = 0; i < 16003; ++i) {
                const float x =
                    .7f * static_cast<float>(std::sin(i * .073) + .2 * std::sin(i * .313));
                const auto y = modal.process({x, 0});
                const auto other = otherSeed.process({x, 0});
                seedDifference |= y != other;
                check(y == repeat.process({x, 0}) && std::isfinite(y[0]) && y[1] == 0);
                double total{};
                for (auto weight : modal.excitationWeights()) {
                    total += weight;
                    check(weight > 0 && weight <= 1.35 / .65);
                }
                check(std::abs(total - 6) < 1e-12);
                output.push_back(y);
            }
            check(seedDifference == (depth > 0));
            for (int block : {1, 7, 128, 1024}) {
                modal.reset();
                for (int start = 0; start < 16003; start += block)
                    for (int i = start; i < std::min(start + block, 16003); ++i) {
                        const float x =
                            .7f * static_cast<float>(std::sin(i * .073) + .2 * std::sin(i * .313));
                        check(modal.process({x, 0}) == output[i]);
                    }
            }
            check(modal.prepare({rate, 42}, moving));
            for (int i = 0; i < 12000; ++i) {
                const auto y = modal.process({std::numeric_limits<float>::max(), 0});
                check(std::isfinite(y[0]) && y[1] == 0);
            }
        }
        for (const auto bad :
             {ModalConfig{260, .12, .3, -.1, .7}, ModalConfig{260, .12, .3, .351, .7},
              ModalConfig{260, .12, .3, .2, 0}, ModalConfig{260, .12, .3, .2, 11},
              ModalConfig{260, .12, .3, std::numeric_limits<double>::quiet_NaN(), .7}}) {
            check(!modal.prepare(rate, bad));
            check(modal.process({1, 1}) == StereoFrame{});
        }
    }
    std::cout << "modal failures=" << failures << '\n';
    return failures ? 1 : 0;
}
