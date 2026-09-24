#include "dsp/BubbleA1.h"

#include <iostream>
#include <memory>
#include <vector>

using namespace frazil::water::research;
namespace {
int failures{};
void check(bool ok, const char* message) {
    if (!ok && failures++ < 20)
        std::cerr << message << '\n';
}
bool near(double a, double b, double tolerance = 1e-10) {
    return std::abs(a - b) < tolerance;
}
} // namespace
int main() {
    BubbleA1Model model;
    BubbleA1Config cfg;
    check(model.prepare(48000, cfg), "default model");
    check(near(BubbleA1Model::frequency(.001) * .001, 3.286, .002), "Minnaert SI");
    check(near(BubbleA1Model::damping(.001), 357.6839915321233, 1e-8), "damping SI");
    double moment{}, probability{};
    for (std::size_t i = 0; i < 128; ++i) {
        const auto& b = model.bins()[i];
        moment += b.probability * b.amplitude * b.amplitude;
        probability += b.probability;
        check(b.frequencyHz < .45 * 44100 && b.poleRadius < 1, "supported stable bin");
        check(near(b.tauSeconds * b.dampingPerSecond, 1), "tau reciprocal damping");
        if (i)
            check(b.radiusMeters > model.bins()[i - 1].radiusMeters &&
                      b.cdf > model.bins()[i - 1].cdf &&
                      b.frequencyHz < model.bins()[i - 1].frequencyHz &&
                      near(b.radiusMeters / model.bins()[i - 1].radiusMeters,
                           std::pow(50., 1. / 127)) &&
                      near(b.amplitude / model.bins()[i - 1].amplitude,
                           std::pow(b.radiusMeters / model.bins()[i - 1].radiusMeters, 1.5)),
                  "monotone table");
    }
    check(near(moment, 1) && near(probability, 1), "probability/energy normalization");
    check(model.bins().front().radiusMeters == .0002 && model.bins().back().radiusMeters == .01,
          "exact endpoints");
    for (std::size_t invalid : {0u, 1u, 16u, 32u, 63u, 65u, 127u, 1025u})
        check(!a1Capacity(invalid), "discrete capacity reject");
    cfg.populationGamma = 0;
    check(model.prepare(48000, cfg), "flat prepare");
    for (const auto& b : model.bins())
        check(b.probability == 1. / 128, "flat log-bin probability");
    RandomSource random(42);
    std::array<int, 128> counts{};
    for (int i = 0; i < 128000; ++i)
        ++counts[model.sample(random.nextUnipolar())];
    for (int n : counts)
        check(n > 850 && n < 1150, "fixed-seed flat histogram");
    for (double bad : {-1., 11., std::numeric_limits<double>::quiet_NaN()}) {
        auto c = cfg;
        c.radiusMinMm = bad;
        check(!model.prepare(48000, c), "reject invalid radius");
    }
    cfg = {};
    check(model.prepare(48000, cfg), "restore model");
    counts = {};
    random.reseed(42);
    constexpr int populationDraws = 256000;
    for (int i = 0; i < populationDraws; ++i)
        ++counts[model.sample(random.nextUnipolar())];
    for (std::size_t i = 0; i < 128; ++i) {
        const double expected = model.bins()[i].probability * populationDraws;
        if (expected > 25)
            check(std::abs(counts[i] - expected) < 6 * std::sqrt(expected),
                  "power-law sampling six sigma");
    }
    // Analytic amplitude and integrated chirp, independent of population scheduling.
    BubbleA1Event event;
    event.physics = model.bins()[100];
    event.amplitude = {.3, -.3};
    event.riseXi = .1;
    BubbleA1Voice voice;
    voice.start(event, 48000, -100);
    for (int n = 0; n < 300; ++n) {
        const auto y = voice.process();
        const double t = n / 48000.;
        const double phase = 2 * std::numbers::pi * event.physics.frequencyHz *
                             (t + .5 * event.riseXi * event.physics.dampingPerSecond * t * t);
        const double expected = .3 * std::exp(-t / event.physics.tauSeconds) * std::sin(phase);
        check(near(y[0], expected, 1e-11) && y[0] == -y[1], "analytic integrated chirp");
    }
    for (int n = 0; n < 100000; ++n)
        (void)voice.process();
    check(voice.instantaneousFrequency() <= std::sqrt(2.) * event.physics.frequencyHz + 1e-8,
          "surface rise cap");
    // Independent analytic P0/P1 check through and beyond the cap, at all persistence values.
    for (double persistence : {.25, 1., 4.})
        for (auto policy :
             {BubbleA1RiseModel::physicalDampingP0, BubbleA1RiseModel::effectiveDampingP1}) {
            auto e = event;
            e.physics.tauSeconds = persistence / e.physics.dampingPerSecond;
            e.physics.poleRadius = std::exp(-1 / (48000 * e.physics.tauSeconds));
            e.riseModel = policy;
            voice.start(e, 48000, -100);
            const double sigma = e.riseXi * (policy == BubbleA1RiseModel::effectiveDampingP1
                                                 ? 1 / e.physics.tauSeconds
                                                 : e.physics.dampingPerSecond);
            double phase = 0, previous = 0;
            for (int n = 0; n < 12000; ++n) {
                const auto y = voice.process();
                const double expected =
                    .3 * std::exp(-n / (48000 * e.physics.tauSeconds)) * std::sin(phase);
                check(near(y[0], expected, 2e-10), "P0/P1 integrated rendered damping waveform");
                const double f = std::min(e.physics.frequencyHz * (1 + sigma * (n + .5) / 48000),
                                          std::sqrt(2.) * e.physics.frequencyHz);
                phase += 2 * std::numbers::pi * f / 48000;
                check(f >= previous, "monotone bounded rise");
                previous = f;
            }
            const double risePerLifetime = sigma * e.physics.tauSeconds;
            check(near(risePerLifetime,
                       e.riseXi *
                           (policy == BubbleA1RiseModel::effectiveDampingP1 ? 1 : persistence)),
                  "P1 lifetime invariant; P0 explicit persistence dependent stylization");
        }
    // Shared-frame oracle: each carrier must be collinear with a REAL stereo frame from
    // the window, including quadrature/decorrelated/unequal transient cases and swap ties.
    for (double rate : {44100., 48000., 96000.})
        for (int scenario = 0; scenario < 8; ++scenario) {
            SharedExcitationAnalyzer analyzer, swappedAnalyzer;
            check(analyzer.prepare(rate) && swappedAnalyzer.prepare(rate), "stereo oracle prepare");
            std::vector<std::array<double, 2>> history(
                static_cast<std::size_t>(std::ceil(.002 * rate)));
            auto original = std::make_unique<BubbleA1>(), swapped = std::make_unique<BubbleA1>();
            check(original->prepare({rate, 42}) && swapped->prepare({rate, 42}),
                  "stereo event prepare");
            RandomSource noise(719);
            for (std::size_t n = 0; n < 8192; ++n) {
                const float x = float(.7 * std::sin(n * .071));
                StereoFrame input{x, x};
                if (scenario == 1)
                    input = {x, -x};
                if (scenario == 2)
                    input = {x, 0};
                if (scenario == 3)
                    input = {0, x};
                if (scenario == 4)
                    input = {x, .2f * x};
                if (scenario == 5)
                    input = {x, float(.7 * std::cos(n * .071))};
                if (scenario == 6)
                    input = {x, .5f * (2 * noise.nextUnipolar() - 1)};
                if (scenario == 7)
                    input = {n % 113 == 0 ? .9f : .1f * x, n % 127 == 0 ? -.6f : -.2f * x};
                history[n % history.size()] = {input[0], input[1]};
                (void)analyzer.process(input);
                (void)swappedAnalyzer.process({input[1], input[0]});
                std::array<double, 2> selected{};
                double energy = 0;
                for (const auto& frame : history) {
                    const double value = frame[0] * frame[0] + frame[1] * frame[1];
                    if (value > energy) {
                        energy = value;
                        selected = frame;
                    }
                }
                const double norm = std::sqrt(energy / 2);
                for (bool sourceEnergy : {false, true}) {
                    const auto carrier = analyzer.eventCarrier(sourceEnergy);
                    const auto reverse = swappedAnalyzer.eventCarrier(sourceEnergy);
                    const double level = sourceEnergy ? std::sqrt(analyzer.state().fastPower) : .25;
                    for (std::size_t ch = 0; ch < 2; ++ch) {
                        check(near(carrier[ch], norm <= 1e-12 ? 0 : selected[ch] / norm * level),
                              "one shared-frame direction and linked level");
                        check(carrier[ch] == reverse[1 - ch], "carrier exact swap");
                    }
                }
                const auto y = original->process(input), z = swapped->process({input[1], input[0]});
                check(y[0] == z[1] && y[1] == z[0], "all stereo scenarios shared oscillator");
                check(original->requested() == swapped->requested(),
                      "shared request ID/timing/RNG");
                const auto& a = original->lastRequestedEvent();
                const auto& b = swapped->lastRequestedEvent();
                check(a.bin == b.bin && a.physics.radiusMeters == b.physics.radiusMeters &&
                          a.physics.frequencyHz == b.physics.frequencyHz &&
                          a.physics.dampingPerSecond == b.physics.dampingPerSecond &&
                          a.physics.tauSeconds == b.physics.tauSeconds && a.riseXi == b.riseXi &&
                          a.depthExcitationProxy == b.depthExcitationProxy &&
                          a.amplitude[0] == b.amplitude[1] && a.amplitude[1] == b.amplitude[0],
                      "shared event physical state and excitation proxy");
            }
        }
    // Exact single-event waveform must not depend on capacity or memory slot ordering.
    std::vector<StereoFrame> single;
    for (std::size_t cap : {64u, 128u, 256u, 512u, 1024u}) {
        auto pool = std::make_unique<BubbleA1VoicePool>();
        cfg.voiceCapacity = cap;
        check(pool->prepare(48000, cfg), "pool prepare");
        check(pool->trigger(event), "single trigger");
        for (int n = 0; n < 2000; ++n) {
            const auto y = pool->process();
            if (cap == 64)
                single.push_back(y);
            else
                check(y == single[n], "capacity independent gain");
        }
    }
    auto pool = std::make_unique<BubbleA1VoicePool>();
    cfg.voiceCapacity = 512;
    check(pool->prepare(48000, cfg), "downshift prepare");
    for (int i = 0; i < 512; ++i)
        check(pool->trigger(event), "fill pool");
    check(pool->setCapacity(128) && pool->active() == 512, "non-destructive 512-to-128 downshift");
    check(!pool->trigger(event) && pool->counters().capacityDrops == 1,
          "downshift inhibits allocation");
    for (int n = 0; n < 48000; ++n)
        (void)pool->process();
    check(pool->active() == 0, "tails retire after downshift");
    check(pool->setCapacity(64), "lower capacity for stealing test");
    for (int i = 0; i < 64; ++i)
        check(pool->trigger(event), "fill lower ceiling");
    for (int n = 0; n < 100; ++n)
        (void)pool->process();
    check(pool->trigger(event) && pool->counters().steals == 1, "bounded releasing replacement");
    const auto beforeStart = pool->counters().started;
    for (int n = 0; n < 71; ++n)
        (void)pool->process();
    check(pool->counters().started == beforeStart, "no hard immediate steal");
    (void)pool->process();
    check(pool->counters().started == beforeStart + 1, "release then replacement");

    for (double rate : {44100., 48000., 96000.}) {
        auto a = std::make_unique<BubbleA1>();
        auto swapped = std::make_unique<BubbleA1>();
        cfg = {};
        check(a->prepare({rate, 42}, cfg) && swapped->prepare({rate, 42}, cfg), "A1 prepare");
        for (int n = 0; n < 1000; ++n)
            check(a->process({}) == StereoFrame{}, "initial silence");
        a->reset();
        std::vector<StereoFrame> samples, output;
        for (int n = 0; n < int(rate / 2); ++n)
            samples.push_back({float(.7 * std::sin(n * .023)), float(.3 * std::cos(n * .031))});
        for (const auto& x : samples) {
            const auto y = a->process(x);
            const auto z = swapped->process({x[1], x[0]});
            check(y[0] == z[1] && y[1] == z[0], "channel-swap equivariance");
            output.push_back(y);
        }
        check(a->requested() > 0, "nonempty events");
        for (int block : {1, 7, 32, 64, 128, 256, 257, 512, 1024}) {
            a->reset();
            for (std::size_t start = 0; start < samples.size(); start += block)
                for (std::size_t i = start; i < std::min(samples.size(), start + block); ++i)
                    check(a->process(samples[i]) == output[i], "reset and block partition exact");
        }
        for (int stereo : {0, 1, 2}) {
            a->reset();
            double energy{};
            for (int n = 0; n < int(rate / 2); ++n) {
                const float x = float(.8 * std::sin(n * .031));
                const auto y = a->process({x, stereo == 0 ? 0.f : stereo == 1 ? x : -x});
                check(stereo == 0   ? y[1] == 0
                      : stereo == 1 ? y[1] == y[0]
                                    : y[1] == -y[0],
                      "stereo invariants");
                energy += double(y[0]) * y[0];
            }
            check(energy > 0, "anti-phase does not collapse");
        }
        const auto count = a->requested();
        check(a->setMotion(0), "close Motion");
        for (int n = 0; n < int(rate); ++n)
            (void)a->process({.8f, .8f});
        check(a->requested() == count && a->pool().active() == 0,
              "zero Motion preserves then retires tails");
        SharedExcitationAnalyzer analyzer;
        check(analyzer.prepare(rate), "analyzer");
        for (int n = 0; n < int(rate); ++n)
            (void)analyzer.process({.5f, -.5f});
        check(near(analyzer.state().rms, .5, 1e-10), "linked RMS calibration");
        const auto carrier = analyzer.eventCarrier(true);
        check(near(carrier[0], .5) && carrier[0] == -carrier[1], "window energy stereo carrier");
        for (int n = 0; n < 200; ++n)
            (void)analyzer.process({});
        check(analyzer.state().activity == 0, "empty-window event silence");
        cfg = {};
        cfg.maxEventRateHz = 10000;
        cfg.depthExponent = 1;
        cfg.voiceCapacity = 64;
        check(a->prepare({rate, 42}, cfg), "dense finite prepare");
        for (int n = 0; n < int(rate); ++n) {
            const auto y = a->process({1, 1});
            check(std::isfinite(y[0]) && a->pool().active() <= 64, "dense finite bounded");
        }
        const double expected = rate * -std::expm1(-10000 / rate);
        // Startup slow envelope reduces the first-second total; test stationary second second.
        const auto warm = a->requested();
        for (int n = 0; n < int(rate); ++n)
            (void)a->process({1, 1});
        check(std::abs(double(a->requested() - warm) - expected) < 6 * std::sqrt(expected),
              "occupancy mean six sigma");
        check(a->pool().counters().steals > 0, "overload exercised");
        for (int n = 0; n < 4097; ++n) {
            const auto y =
                a->process({std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()});
            check(std::isfinite(y[0]) && std::isfinite(y[1]), "finite extreme input");
        }
        cfg.radiusMinMm = 10;
        cfg.radiusMaxMm = 2;
        check(!a->prepare({rate, 42}, cfg) && a->process({1, 1}) == StereoFrame{},
              "failed prepare silent");
    }
    for (double rate : {44100., 48000., 96000.}) {
        auto a = std::make_unique<BubbleA1>();
        for (double gamma : {0., 6.})
            for (double alpha : {.75, 2.25})
                for (double persistence : {.25, 4.}) {
                    cfg = {};
                    cfg.radiusMaxMm = 50;
                    cfg.populationGamma = gamma;
                    cfg.amplitudeRadiusExponent = alpha;
                    cfg.persistenceScale = persistence;
                    cfg.riseXi = .2;
                    cfg.riseCutoff = .8;
                    cfg.maxEventRateHz = 10000;
                    cfg.depthExponent = 1;
                    cfg.residualGain = 1;
                    cfg.voiceCapacity = 1024;
                    check(a->prepare({rate, 19}, cfg), "physical parameter corners");
                    for (int n = 0; n < 8193; ++n) {
                        const auto y = a->process({n % 2 ? 1.f : -1.f, .8f});
                        check(std::isfinite(y[0]) && std::isfinite(y[1]), "finite corner output");
                    }
                }
    }
    SharedExcitationAnalyzer window;
    check(window.prepare(48000), "zero crossing window prepare");
    // Settle the 1 ms attack before testing a source zero crossing, not attack startup.
    for (int n = 0; n < 960; ++n)
        (void)window.process({.5f, 0});
    (void)window.process({});
    check(window.eventCarrier(true)[0] > .49 && window.eventCarrier(true)[1] == 0,
          "triggering zero sample does not erase source energy");
    window.reset();
    for (int n = 0; n < 48000; ++n) {
        const float x = static_cast<float>(.8 * std::sin(2 * std::numbers::pi * 100 * n / 48000));
        (void)window.process({x, x});
        if (n > 24000)
            check(std::abs(window.eventCarrier(true)[0]) > .5,
                  "sustained low-frequency excitation remains phase robust");
    }
    check(!mapBubbleA1(.5, -.1, .5) && !mapBubbleA1(2, .5, .5) &&
              !mapBubbleA1(.5, .5, std::numeric_limits<double>::quiet_NaN()),
          "invalid macro rejected");
    const auto low = *mapBubbleA1(0, 0, 0), high = *mapBubbleA1(1, 1, 1);
    check(low.motionFactor == 0 && high.motionFactor == 1 && low.persistenceScale == .25 &&
              high.persistenceScale == 4,
          "macro endpoints");
    check(high.radiusMinMm > low.radiusMinMm && high.radiusMaxMm > low.radiusMaxMm,
          "Size direction");
    std::cout << "bubble_a1 failures=" << failures << '\n';
    return failures ? 1 : 0;
}
