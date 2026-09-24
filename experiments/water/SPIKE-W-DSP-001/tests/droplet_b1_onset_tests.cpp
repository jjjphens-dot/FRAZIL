#include "DropletB1TestSupport.h"

#include <memory>
using namespace frazil::water::research;
int main() {
    b1test::Checks check;
    for (double rate : {44100., 48000., 96000.}) {
        DropletB1Config c;
        DropletB1OnsetDetector detector;
        detector.prepare(rate, c);
        auto onset = detector.process({.04, .001, .2, 1});
        check(onset.eligible && b1test::near(onset.noveltyDb, 10 * std::log10(40.)),
              "relative novelty oracle");
        for (int i = 0; i < int(rate * .1); ++i)
            check(!detector.process({.04, .001, .2, 1}).eligible, "no repeated plateau onset");
        detector.process({.01, .01, .1, 1});
        check(detector.process({.04, .001, .2, 1}).eligible, "hysteresis rearms");
        detector.reset();
        for (int i = 0; i < 1000; ++i)
            check(!detector.process({1e-10, 1e-12, 1e-5, 1}).eligible,
                  "level floor rejects quiet novelty");
        // Highest hysteresis and lowest threshold still recover from silence.
        c[B1Parameter::onset] = 3;
        c[B1Parameter::hysteresis] = 6;
        detector.prepare(rate, c);
        check(detector.process({.04, .001, .2, 1}).eligible, "extreme hysteresis first event");
        for (int i = 0; i < int(rate * .1); ++i)
            detector.process({});
        check(detector.process({.04, .001, .2, 1}).eligible, "silence rearm negative threshold");
        auto processor = std::make_unique<DropletB1>();
        check(processor->prepare({rate, 42}), "prepare");
        std::uint64_t previous{}, eligible{}, due{};
        for (int n = 0; n < int(rate); ++n) {
            const auto input = n < int(rate * .02) ? StereoFrame{.7f, -.21f} : StereoFrame{};
            processor->process(input);
            if (processor->counters().eligible != eligible) {
                eligible = processor->counters().eligible;
                const auto& e = processor->lastEligible();
                due = e.dueSample;
                check(e.impact.sourceSample == std::uint64_t(n),
                      "captured original onset timestamp");
                check(due - e.impact.sourceSample == std::uint64_t(std::ceil(.024 * rate)),
                      "explicit ceil delay");
                check(b1test::near(e.impact.carrier[1] / e.impact.carrier[0], -.3, 1e-7),
                      "captured source direction");
                check(e.impact.sourceExcitation > 0, "captured excitation");
            }
            if (processor->pool().counters().started != previous) {
                check(std::uint64_t(n) == due, "starts on captured due sample despite silence");
                check(input == StereoFrame{}, "causality fixture starts after input ended");
                previous = processor->pool().counters().started;
            }
        }
        check(eligible == 1 && previous == 1, "isolated burst one eligible and one delayed start");
        processor->reset();
        for (int n = 0; n < int(rate); ++n)
            processor->process({.3f, .3f});
        check(processor->counters().eligible == 1, "sustained pad no continuing new events");
        // Direct detector spacing: immediate re-arm is insufficient before refractory expiry.
        c = {};
        c[B1Parameter::spacing] = 8;
        detector.prepare(rate, c);
        std::uint64_t last{}, count{};
        for (std::uint64_t n = 0; n < std::uint64_t(rate); ++n) {
            const auto o =
                detector.process(n % 2 ? SharedExcitation{} : SharedExcitation{.1, .001, .4, 1});
            if (o.eligible) {
                if (count)
                    check(n - last >= std::uint64_t(std::ceil(.008 * rate)), "minimum spacing");
                last = n;
                ++count;
            }
        }
        check(count > 100, "dense eligible fixture exercised");
    }
    return check.failures ? 1 : 0;
}
