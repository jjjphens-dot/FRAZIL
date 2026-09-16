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
    }
    std::cout << "modal failures=" << failures << '\n';
    return failures ? 1 : 0;
}
