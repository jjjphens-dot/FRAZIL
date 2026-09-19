#include "dsp/LiquidModalResonator.h"

#include <iostream>
#include <limits>
#include <vector>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok, const char* name) {
        if (!ok && failures++ < 10)
            std::cerr << "FAIL normalization: " << name << '\n';
    };
    for (double rate : {44100., 48000., 96000.}) {
        for (double root : {130., 260., 520.}) {
            double previousCentroid{-1};
            double previousEnergy{};
            for (double decay : {.03, .12, .48}) {
                LiquidModalResonator modal;
                const ModalConfig config{root, decay, .3};
                check(modal.prepare(rate, config, ModalExcitation::hard, ModalNormalization::c3),
                      "prepare grid");
                const double bound = modal.normalizationReadout().residualBound;
                check(bound > 0 && bound <= 3.9600001, "declared residual ceiling");
                const auto frames = static_cast<int>(rate * std::max(1., 8 * decay));
                std::vector<float> impulse;
                double absolute{}, energy{}, timeEnergy{};
                for (int i = 0; i < frames; ++i) {
                    const auto y = modal.process({i == 0 ? 1.f : 0.f, 0});
                    check(std::isfinite(y[0]) && y[1] == 0, "finite isolated impulse");
                    impulse.push_back(y[0]);
                    absolute += std::abs(static_cast<double>(y[0]));
                    energy += static_cast<double>(y[0]) * y[0];
                    timeEnergy += i / rate * static_cast<double>(y[0]) * y[0];
                }
                const double centroid = timeEnergy / energy;
                check(centroid > previousCentroid, "increased energy-time persistence");
                if (previousEnergy > 0)
                    check(energy > .85 * previousEnergy && energy < 1.15 * previousEnergy,
                          "bank impulse energy does not collapse across mapped Decay");
                previousCentroid = centroid;
                previousEnergy = energy;
                check(absolute <= bound, "sampled induced response below analytical ceiling");
                // Exact historical failure construction, now through the bounded carrier.
                modal.reset();
                StereoFrame last{};
                for (int i = 0; i < frames; ++i) {
                    const float h = impulse[static_cast<std::size_t>(frames - 1 - i)];
                    const float x =
                        h == 0 ? 0 : std::copysign(std::numeric_limits<float>::max(), h);
                    last = modal.process({x, 0});
                    check(std::isfinite(last[0]) && std::abs(last[0]) <= bound && last[1] == 0,
                          "adversarial finite float input remains bounded");
                }
                check(std::abs(last[0] - absolute) < 2e-5 * std::max(1., absolute),
                      "adversarial output agrees with measured induced response");
            }
        }
        for (double root : {40., rate * .45 / 4.17})
            for (double decay : {.002, 1.})
                for (auto excitation : {ModalExcitation::hard, ModalExcitation::softsign,
                                        ModalExcitation::tanh, ModalExcitation::feature}) {
                    LiquidModalResonator modal;
                    check(modal.prepare(rate, {root, decay, .3, .35, .02}, excitation,
                                        ModalNormalization::c3),
                          "raw parameter corners");
                    std::vector<StereoFrame> reference;
                    for (int i = 0; i < 4099; ++i) {
                        const float x = i % 1024 < 512 ? std::numeric_limits<float>::max()
                                                       : -std::numeric_limits<float>::max();
                        const auto y = modal.process({x, -.5f * x});
                        check(std::isfinite(y[0]) && std::abs(y[0]) <= 4 && y[1] == -.5f * y[0],
                              "finite moving stereo extreme");
                        reference.push_back(y);
                    }
                    for (int block : {32, 64, 128, 256, 257, 512, 1024}) {
                        modal.reset();
                        for (int start = 0; start < 4099; start += block)
                            for (int i = start; i < std::min(start + block, 4099); ++i) {
                                const float x = i % 1024 < 512 ? std::numeric_limits<float>::max()
                                                               : -std::numeric_limits<float>::max();
                                check(modal.process({x, -.5f * x}) == reference[i],
                                      "partition/reset identity");
                            }
                    }
                }
        LiquidModalResonator invalid;
        check(!invalid.prepare(rate, {}, ModalExcitation::raw, ModalNormalization::c3),
              "C3 rejects unbounded raw excitation");
        check(invalid.process({1, 1}) == StereoFrame{}, "failed prepare stays silent");
    }
    std::cout << "normalization failures=" << failures << '\n';
    return failures ? 1 : 0;
}
