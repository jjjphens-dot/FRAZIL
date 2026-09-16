#include "dsp/ResearchBaseline.h"

#include <array>
#include <iostream>
#include <limits>

using namespace frazil::water::research;

int main() {
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        if (!passed) {
            std::cerr << message << '\n';
            ++failures;
        }
    };
    ResearchBaseline baseline;
    std::array<float, 1027> input{};
    std::array<float, 1027> output{};
    RandomSource fixture(17u);
    for (auto& sample : input)
        sample = 2.0f * fixture.nextUnipolar() - 1.0f;
    check(!baseline.processResidual(input, output), "unprepared processing accepted");
    for (double rate : {44100.0, 48000.0, 96000.0}) {
        ResearchConfig config{rate, 42u};
        check(baseline.prepare(config), "valid prepare rejected");
        for (std::size_t block : {1u, 7u, 32u, 64u, 128u, 256u, 512u, 1024u}) {
            output.fill(99.0f);
            baseline.reset();
            check(baseline.processResidual({}, {}), "empty callback rejected");
            for (std::size_t start = 0; start < input.size(); start += block) {
                const auto count = std::min(block, input.size() - start);
                check(baseline.processResidual(std::span(input).subspan(start, count),
                                               std::span(output).subspan(start, count)),
                      "valid callback rejected");
            }
            for (std::size_t i = 0; i < input.size(); ++i)
                check(output[i] == 0.0f && input[i] + output[i] == input[i],
                      "nonzero residual or duplicated carrier");
        }
        RandomSource bubble(config.seedFor(RandomDomain::bubble));
        RandomSource reference(config.seedFor(RandomDomain::bubble));
        RandomSource droplet(config.seedFor(RandomDomain::droplet));
        check(config.seedFor(RandomDomain::bubble) != config.seedFor(RandomDomain::droplet) &&
                  config.seedFor(RandomDomain::bubble) != config.seedFor(RandomDomain::flow),
              "seed domains coincide for fixture");
        for (int i = 0; i < 1024; ++i) {
            (void)droplet.nextUInt();
            check(bubble.nextUInt() == reference.nextUInt(), "unrelated stream perturbed bubble");
        }
    }
    check(!baseline.prepare({std::numeric_limits<double>::quiet_NaN(), 0u}), "NaN rate accepted");
    check(!baseline.processResidual(input, output), "failed prepare retained validity");
    check(!baseline.prepare({0.0, 0u}), "zero rate accepted");
    check(baseline.prepare({}), "reprepare failed");
    check(!baseline.processResidual(input, std::span(output).first(1)), "mismatch accepted");
    std::cout << "baseline failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
