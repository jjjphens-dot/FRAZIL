#include "dsp/LiquidModalResonator.h"

#include <iostream>
#include <limits>
#include <vector>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok, const char* name) {
        if (!ok && failures++ < 8)
            std::cerr << "FAIL excitation: " << name << '\n';
    };
    constexpr std::array modes{ModalExcitation::raw, ModalExcitation::hard,
                               ModalExcitation::softsign, ModalExcitation::tanh,
                               ModalExcitation::feature};
    const float maximum = std::numeric_limits<float>::max();
    for (double rate : {44100., 48000., 96000.}) {
        for (auto mode : modes) {
            ModalExcitationConditioner conditioner;
            check(conditioner.prepare(rate, mode), "prepare");
            for (float x : {0.f, 1e-20f, .01f, .25f, 1.f, 2.f, maximum, -maximum}) {
                const StereoFrame input{x, -.5f * x};
                const auto y = conditioner.process(input);
                check(std::isfinite(y[0]) && std::isfinite(y[1]), "finite extrema");
                check(y[1] == -.5f * y[0], "linked stereo ratio and polarity");
                if (mode != ModalExcitation::raw)
                    check(std::abs(y[0]) <= 1 && std::abs(y[1]) <= 1, "Emax=1");
                if (mode == ModalExcitation::raw ||
                    (mode == ModalExcitation::hard && std::abs(x) <= 1))
                    check(y == input, "raw / hard in-range identity");
            }
            for (int i = 0; i < 2000; ++i)
                check(conditioner.process({}) == StereoFrame{}, "no excitation during release");
            // Engineering waveform regression, after settling: fit one scalar to a 260 Hz sine.
            // This detects instantaneous-magnitude flattening without labelling a sound natural.
            conditioner.reset();
            double xx{}, xe{}, ee{};
            for (int i = 0; i < static_cast<int>(rate); ++i) {
                const float x =
                    .25f * static_cast<float>(std::sin(2 * std::numbers::pi * 260 * i / rate));
                const auto e = conditioner.process({x, -.5f * x});
                if (i > rate * .5) {
                    xx += static_cast<double>(x) * x;
                    xe += static_cast<double>(x) * e[0];
                    ee += static_cast<double>(e[0]) * e[0];
                }
            }
            check(std::sqrt(std::max(0., ee - xe * xe / xx) / ee) < .05,
                  "settled sine scalar-fit shape error below five percent");
            // Mixed sustained/adversarial/impulse/gated input; actual modal output, not just e.
            std::vector<StereoFrame> input(4099), output, drivers;
            for (std::size_t i = 0; i < input.size(); ++i) {
                float value = .4f * static_cast<float>(std::sin(i * .073));
                if (i < 512)
                    value = maximum;
                else if (i < 1024)
                    value = i % 2 ? maximum : -maximum;
                else if (i < 1600)
                    value = i == 1024 ? maximum : 0;
                else if (i % 300 > 180)
                    value = 0;
                input[i] = {value, 0};
            }
            LiquidModalResonator modal;
            const ModalConfig config{40., 1., .3, .35, .02};
            check(modal.prepare(rate, config, mode), "actual bank prepares");
            for (const auto& x : input) {
                const auto y = modal.process(x);
                check(std::isfinite(y[0]) && y[1] == 0, "bank finite and isolated");
                output.push_back(y);
                drivers.push_back(modal.excitationFrame());
            }
            for (int block : {32, 64, 128, 256, 257, 512, 1024}) {
                modal.reset();
                for (std::size_t start = 0; start < input.size(); start += block)
                    for (std::size_t i = start; i < std::min(start + block, input.size()); ++i) {
                        check(modal.process(input[i]) == output[i], "partition/reset identity");
                        check(modal.excitationFrame() == drivers[i], "actual driver repeat");
                    }
            }
            modal.reset();
            for (std::size_t i = 0; i < input.size(); ++i) {
                const auto y = modal.process({0, input[i][0]});
                check(y[0] == 0 && y[1] == output[i][0], "right-only transpose symmetry");
            }
            check(!modal.prepare(0, config, mode) && modal.process({1, 1}) == StereoFrame{} &&
                      modal.excitationFrame() == StereoFrame{},
                  "failed prepare clears bank/driver");
        }
    }
    ModalExcitation parsed{};
    check(parseModalExcitation("feature", parsed) && parsed == ModalExcitation::feature &&
              !parseModalExcitation("unknown", parsed),
          "strict candidate names");
    std::cout << "excitation failures=" << failures << '\n';
    return failures ? 1 : 0;
}
