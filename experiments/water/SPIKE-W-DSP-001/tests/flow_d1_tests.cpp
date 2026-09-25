#include "dsp/FlowD1.h"

#include <iostream>
#include <limits>
#include <vector>
using namespace frazil::water::research;
int main() {
    int failures = 0;
    const auto check = [&](bool value, const char* name) {
        if (!value && failures++ < 12)
            std::cerr << name << '\n';
    };
    const auto near = [](double a, double b, double e = 1e-12) { return std::abs(a - b) <= e; };
    // Hard-coded historical numeric identities, not derived from enum values.
    const std::array domains{RandomDomain::bubble,
                             RandomDomain::droplet,
                             RandomDomain::flow,
                             RandomDomain::modalMotion,
                             RandomDomain::dropletActivity,
                             RandomDomain::bubbleA1,
                             RandomDomain::dropletB1Identity,
                             RandomDomain::dropletB1Admission,
                             RandomDomain::dropletB1Jitter,
                             RandomDomain::flowD1};
    for (std::size_t i = 0; i < domains.size(); ++i)
        check(static_cast<std::uint64_t>(domains[i]) == i + 1, "domain identity");
    check(near(FlowD1Model::delaySeconds(.015), .000010107816711590296), "independent SI oracle");
    check(near(FlowD1Model::characteristicRateHz({.2, .03, .015}), 20.0 / 3),
          "transport timescale");
    check(FlowD1Model::delaySeconds(.03) > FlowD1Model::delaySeconds(.015), "delay monotonic");
    check(FlowD1Model::characteristicRateHz({.4, .03, .015}) ==
              2 * FlowD1Model::characteristicRateHz({.2, .03, .015}),
          "U ratio");
    check(FlowD1Model::characteristicRateHz({.2, .06, .015}) ==
              .5 * FlowD1Model::characteristicRateHz({.2, .03, .015}),
          "L ratio");
    for (double rate : {44100., 48000., 96000.}) {
        FlowD1 flow;
        check(flow.prepare({rate, 42}), "prepare");
        for (int i = 0; i < 10000; ++i)
            check(flow.process({}).transferred == FlowD1WideFrame{}, "zero history");
        for (FlowD1Config c : {FlowD1Config{0, .03, .015}, FlowD1Config{.2, .03, 0}}) {
            check(flow.prepare({rate, 42}, c), "identity prepare");
            for (int n = 0; n < 1000; ++n) {
                StereoFrame x{float(std::sin(n * .3)), float(std::cos(n * .7))};
                auto y = flow.process(x);
                check(y.transferred[0] == x[0] && y.transferred[1] == x[1] &&
                          y.correction == FlowD1WideFrame{},
                      "exact identity");
            }
        }
        for (double u : {0., std::numeric_limits<double>::denorm_min(), .05, .2, 1.}) {
            for (double length : {.005, .03, .2}) {
                FlowD1Trajectory t, repeat, other;
                FlowD1Config c{u, length, .05};
                check(t.prepare({rate, 42}, c) && repeat.prepare({rate, 42}, c) &&
                          other.prepare({rate, 43}, c),
                      "trajectory prepare");
                double previous = 0;
                bool differs = false;
                for (int n = 0; n < int(rate); ++n) {
                    double s = t.process();
                    check(s >= 0 && s <= .05 && std::abs(s - previous) * rate <= u + 1e-10,
                          "path/speed bounds");
                    check(s == repeat.process(), "same seed");
                    differs |= s != other.process();
                    previous = s;
                }
                if (u >= .05)
                    check(differs, "different seeds");
                t.reset();
                repeat.reset();
                for (int n = 0; n < 100; ++n)
                    check(t.process() == repeat.process(), "trajectory reset");
            }
        }
        FlowD1 silentTrajectory, drivenTrajectory;
        silentTrajectory.prepare({rate, 42}, {.5, .01, .05});
        drivenTrajectory.prepare({rate, 42}, {.5, .01, .05});
        std::vector<float> sourceHistory;
        for (int n = 0; n < 5000; ++n) {
            sourceHistory.push_back(float(std::sin(n * .47)));
            const auto actual = drivenTrajectory.process({sourceHistory.back(), 0});
            (void)silentTrajectory.process({});
            check(drivenTrajectory.pathMeters() == silentTrajectory.pathMeters(),
                  "source-independent trajectory");
            // Independent product-form oracle evaluated on the actual moving path.
            const double d = drivenTrajectory.pathMeters() * rate / 1484.0;
            const int base = std::max(0, int(std::floor(d)) - 1);
            double expected = 0;
            for (int k = 0; k < 4; ++k) {
                double coefficient = 1;
                for (int j = 0; j < 4; ++j)
                    if (j != k)
                        coefficient *= (d - base - j) / double(k - j);
                if (n >= base + k)
                    expected += coefficient * sourceHistory[n - base - k];
            }
            check(near(actual.transferred[0], expected, 1e-12), "moving numerical oracle");
        }
        FlowD1FractionalDelay kernel;
        check(kernel.prepare(rate, .05), "kernel prepare");
        // Cubic sample history is exactly reproduced by a cubic interpolation polynomial.
        for (double d : {0., .25, .75, 1.25}) {
            kernel.reset();
            for (int n = 0; n < 20; ++n) {
                auto y = kernel.process({float(n * n * n), 0}, d);
                if (n >= 4)
                    check(near(y[0], std::pow(n - d, 3), 1e-9) && y[1] == 0,
                          "independent cubic oracle");
            }
        }
        kernel.reset();
        for (int n = 0; n < 10; ++n) {
            const auto y = kernel.process({n == 0 ? 1.f : 0.f, 0}, .5);
            const std::array expected{.3125, .9375, -.3125, .0625};
            check(y[0] == (n < 4 ? expected[n] : 0), "independent half-sample impulse");
        }
        flow.prepare({rate, 42});
        std::vector<FlowD1Result> reference;
        for (int n = 0; n < 20000; ++n) {
            auto y = flow.process({float(std::sin(n * .2)), 0});
            check(y.transferred[1] == 0 && y.correction[1] == 0, "no crossfeed");
            check(near(y.transferred[0] - y.correction[0], float(std::sin(n * .2))),
                  "correction ownership");
            reference.push_back(y);
        }
        for (int block : {1, 7, 32, 64, 128, 256, 257, 512, 1024}) {
            flow.reset();
            for (int start = 0; start < 20000; start += block)
                for (int n = start; n < std::min(start + block, 20000); ++n) {
                    auto y = flow.process({float(std::sin(n * .2)), 0});
                    check(y.transferred == reference[n].transferred &&
                              y.correction == reference[n].correction,
                          "partition/reset exact");
                }
        }
        flow.prepare({rate, 42}, {1, .005, .05});
        for (int n = 0; n < 20000; ++n) {
            float x = (n % 2 ? 1.f : -1.f) * std::numeric_limits<float>::max();
            auto y = flow.process({x, x});
            check(std::isfinite(y.transferred[0]) && std::isfinite(y.correction[0]) &&
                      y.transferred[0] == y.transferred[1],
                  "finite float extrema/linked stereo");
        }
        for (std::size_t n = 0; n < flow.drainSamples() + 1; ++n)
            (void)flow.process({});
        check(flow.process({}).transferred == FlowD1WideFrame{}, "bounded history drain");
        check(!flow.prepare({rate, 42}, {-1, .03, .015}), "invalid config");
        flow.reset();
        check(flow.process({1, 1}).transferred == FlowD1WideFrame{}, "failed prepare inactive");
    }
    for (double rate : {0., 44099., 96001., std::numeric_limits<double>::infinity(),
                        std::numeric_limits<double>::quiet_NaN()}) {
        FlowD1 f;
        check(!f.prepare({rate, 42}), "invalid rate");
        check(f.process({1, 1}).transferred == FlowD1WideFrame{}, "invalid rate safe");
    }
    std::cout << "D1 failures=" << failures << '\n';
    return failures ? 1 : 0;
}
