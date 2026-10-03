#include "AllocationObserver.h"
#include "dsp/BubbleA1.h"
#include "dsp/DropletB1.h"
#include "dsp/DropletB2.h"

#include <iostream>
#include <memory>
#include <string_view>
#include <vector>
using namespace frazil::water::research;
int main(int argc, char** argv) {
    if (argc > 2 || (argc == 2 && std::string_view(argv[1]) != "--full" &&
                     std::string_view(argv[1]) != "--fast")) {
        std::cerr << "Expected --fast or --full\n";
        return 2;
    }
    const bool full = argc == 1 || std::string_view(argv[1]) == "--full";
    const std::vector<double> rates =
        full ? std::vector<double>{44100., 48000., 96000.} : std::vector<double>{48000.};
    int failures{};
    const auto check = [&](bool ok, const char* message) {
        if (!ok) {
            ++failures;
            std::cerr << "FAIL " << message << '\n';
        }
    };
    for (double rate : rates) {
        auto baseline = std::make_unique<DropletB1>();
        auto candidate = std::make_unique<DropletB2>();
        DropletB2Config c;
        c[B2Parameter::radiusSpread] = 0;
        c[B2Parameter::detectorMode] = 0;
        c[B2Parameter::gamma] = 1;
        c[B2Parameter::detune] = 0;
        check(baseline->prepare({rate, 42}) && candidate->prepare({rate, 42}, c),
              "prepare B1-like B2");
        bool exact = true;
        allocationtest::allocations = 0;
        allocationtest::observing = true;
        for (int n = 0; n < static_cast<int>(rate); ++n) {
            const float x = n % 4000 < 200 ? .5f * std::sin(n * .13f) : 0;
            const StereoFrame in{x, -.5f * x};
            const auto a = baseline->process(in), b = candidate->process(in);
            exact &= a == b;
        }
        allocationtest::observing = false;
        check(exact && candidate->counters().eligible > 0, "B1/B2 ablation sample exact");
        check(allocationtest::allocations == 0, "B2 process allocates zero");
        auto changed = std::make_unique<DropletB2>();
        c[B2Parameter::radiusSpread] = 5;
        c[B2Parameter::gamma] = .5;
        c[B2Parameter::detune] = 1;
        check(changed->prepare({rate, 42}, c), "prepare radius gamma spatial");
        baseline->reset();
        bool timing = true, finite = true, stereo = false;
        for (int n = 0; n < static_cast<int>(rate); ++n) {
            const float x = n % 4000 < 200 ? .5f * std::sin(n * .13f) : 0;
            baseline->process({x, x});
            const auto out = changed->process({x, x});
            timing &= baseline->counters().eligible == changed->counters().eligible &&
                      baseline->counters().admitted == changed->counters().admitted;
            finite &= std::isfinite(out[0]) && std::isfinite(out[1]);
            stereo |= out[0] != out[1];
        }
        check(timing, "radius gamma spatial preserve eligibility/admission");
        check(finite && stereo, "dual mono generates finite stereo difference");
        for (double radius : {.2, .355, 2., 7.})
            for (double spread : {0., 1., 2.5, 5.}) {
                const auto lo = DropletB2RadiusModel::radiusMm(radius, spread, 0),
                           hi = DropletB2RadiusModel::radiusMm(radius, spread, 1);
                check(lo >= .2 && hi <= 7 && lo <= hi, "radius bounds");
                if (spread == 0)
                    check(lo == radius && hi == radius, "radius zero exact");
                for (double t : {0., .01, .1, 1.}) {
                    const double f =
                        std::min(BubblePhysics::minnaertFrequency(radius * .001) *
                                     (1 + .1 * BubblePhysics::damping(radius * .001) * t),
                                 .45 * rate);
                    const double cents = DropletB2SpatialRenderer::boundedCents(f, 1, 2);
                    const auto hz = DropletB2SpatialRenderer::frequencies(f, cents, 1);
                    check(std::abs(hz[1] - hz[0]) <= 2 + 1e-9, "analytic beat cap");
                    check(std::abs(std::sqrt(hz[0] * hz[1]) - f) < 1e-9, "geometric center");
                }
            }
        for (double spacing : {8., 20., 40., 80.})
            for (double pulse : {8., 10., 20., 40., 80.})
                for (double amplitude : {.1, .8}) {
                    c = {};
                    c.values[static_cast<std::size_t>(B1Parameter::spacing)] = spacing;
                    check(changed->prepare({rate, 42}, c), "spacing fixture prepare");
                    std::uint64_t last{}, seen{};
                    bool bounded = true;
                    const int period = static_cast<int>(std::ceil(rate * pulse * .001));
                    for (int n = 0; n < static_cast<int>(rate * .4); ++n) {
                        const float x = n % period < static_cast<int>(rate * .001)
                                            ? static_cast<float>(amplitude)
                                            : 0;
                        changed->process({x, x});
                        if (changed->counters().eligible != seen) {
                            if (seen)
                                bounded &= n - last >= static_cast<std::uint64_t>(
                                                           std::ceil(rate * spacing * .001));
                            seen = changed->counters().eligible;
                            last = n;
                        }
                    }
                    check(bounded && seen > 0, "hybrid synthetic spacing gate");
                }
        c = {};
        check(changed->prepare({rate, 42}, c), "steady prepare");
        for (int n = 0; n < static_cast<int>(rate * .5); ++n)
            changed->process({.25f, .25f});
        check(changed->counters().eligible == 1, "steady level no repeated attack");
        changed->reset();
        for (int n = 0; n < 10000; ++n)
            changed->process({});
        check(changed->counters().eligible == 0, "silence rearm no events");
        // Default FULL candidate, including both independent event RNG domains.
        check(candidate->prepare({rate, 42}), "FULL prepare");
        check(changed->prepare({rate, 42}), "FULL independent instance");
        std::vector<StereoFrame> first;
        first.reserve(static_cast<std::size_t>(rate));
        bool same = true, allFinite = true;
        for (int n = 0; n < static_cast<int>(rate); ++n) {
            const float x = n % 960 < 96 ? .3f * std::sin(.13f * n) : 0;
            allocationtest::observing = true;
            const auto out = candidate->process({x, x});
            const auto other = changed->process({x, x});
            allocationtest::observing = false;
            same &= out == other;
            allFinite &= std::isfinite(out[0]) && std::isfinite(out[1]);
            first.push_back(out);
        }
        check(same && allFinite && allocationtest::allocations == 0,
              "FULL deterministic independent instances and zero allocation");
        candidate->reset();
        for (int n = 0; n < static_cast<int>(rate); ++n) {
            const float x = n % 960 < 96 ? .3f * std::sin(.13f * n) : 0;
            same &= candidate->process({x, x}) == first[n];
        }
        check(same, "FULL reset exact");
        for (int n = 0; n < static_cast<int>(rate * 3); ++n)
            candidate->process({});
        check(candidate->pool().active() == 0, "FULL finite tail drains");
        // Exercise copied pool lifecycle independently for the new stereo voice.
        auto event = changed->lastEligible();
        event.center.physics = DropletB1Model::make(DropletB1Config{});
        event.mappedExcitation = .5;
        event.center.impact.carrier = {.5, .5};
        auto pool = std::make_unique<DropletB2VoicePool>();
        check(pool->prepare(rate, 16), "B2 pool prepare");
        for (int n = 0; n < 32; ++n)
            check(pool->request(event), "B2 pool fill and release");
        check(!pool->request(event), "B2 pool bounded overflow");
        for (int n = 0; n < static_cast<int>(rate * 3); ++n) {
            const auto out = pool->process();
            allFinite &= std::isfinite(out[0]) && std::isfinite(out[1]) && pool->active() <= 16;
        }
        check(allFinite && pool->active() == 0 && pool->counters().started == 32,
              "B2 overload completes bounded replacements");
        auto a = std::make_unique<BubbleA1>(), acmp = std::make_unique<BubbleA1>();
        BubbleA1Config ac;
        ac.depthAmplitudeGamma = .5;
        check(a->prepare({rate, 42}) && acmp->prepare({rate, 42}, ac), "A1 mapping prepare");
        bool identity = true;
        for (int n = 0; n < 10000; ++n) {
            a->process({.2f, .2f});
            acmp->process({.2f, .2f});
            const auto& x = a->lastRequestedEvent();
            const auto& y = acmp->lastRequestedEvent();
            identity &= a->pool().counters().started == acmp->pool().counters().started &&
                        a->pool().active() == acmp->pool().active() &&
                        a->pool().counters().steals == acmp->pool().counters().steals &&
                        a->requested() == acmp->requested() && x.bin == y.bin &&
                        x.riseXi == y.riseXi && x.depthExcitationProxy == y.depthExcitationProxy;
        }
        check(identity, "A1 gamma leaves scheduler RNG radius and rise exact");
    }
    std::cout << "B2 failures=" << failures << '\n';
    return failures ? 1 : 0;
}
