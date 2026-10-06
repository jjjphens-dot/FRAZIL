#include "AllocationObserver.h"
#include "dsp/BubbleA1.h"
#include "preview/PreviewEventTrace.h"

#include <iostream>
#include <memory>
#include <string_view>
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
int main(int argc, char** argv) {
    if (argc > 2 || (argc == 2 && std::string_view(argv[1]) != "--full" &&
                     std::string_view(argv[1]) != "--fast" && std::string_view(argv[1]) != "--current")) {
        std::cerr << "Expected --current, --fast or --full\n";
        return 2;
    }
    const bool current = argc == 2 && std::string_view(argv[1]) == "--current";
    const bool full = argc == 1 || std::string_view(argv[1]) == "--full";
    const std::vector<double> rates =
        full ? std::vector<double>{44100., 48000., 96000.} : std::vector<double>{48000.};
    using frazil::water::preview::PreviewEventRecord;
    using frazil::water::preview::PreviewEventTrace;
    {
        auto pool = std::make_unique<BubbleA1VoicePool>();
        BubbleA1Config c;
        BubbleA1Event e;
        e.physics.frequencyHz = 1000;
        e.physics.tauSeconds = 1;
        e.physics.poleRadius = .9;
        e.amplitude = {1.01e-4, 0};
        check(pool->trigger(e) == BubbleA1TriggerResult::notReady, "typed not-ready");
        check(pool->prepare(48000, c, BubbleA1LifecyclePolicy::admissionAwareL1), "L1 prepare");
        check(pool->trigger(e) == BubbleA1TriggerResult::started,
              "L1 deliberately does not cull just-above-floor events");
        (void)pool->process();
        check(pool->counters().lifecycle.completedWithoutNonZero == 1 &&
                  pool->counters().lifecycle.preStartCulled == 0,
              "post-first-decay silent start remains distinct from admission cull");
        pool->reset();
        e.amplitude = {1e-4, 0};
        check(pool->trigger(e) == BubbleA1TriggerResult::preStartCulled, "inclusive floor");
        pool->reset();
        e.amplitude = {.1, -.1};
        (void)pool->trigger(e);
        e.amplitude = {-.1, .1};
        (void)pool->trigger(e);
        (void)pool->process();
        check(pool->process() == StereoFrame{} && pool->counters().lifecycle.firstNonZero == 2,
              "voice-local nonzero survives exact summed cancellation");
        pool->reset();
        check(pool->counters().lifecycle.firstNonZero == 0, "sidecar reset");
    }
    {
        auto pool = std::make_unique<BubbleA1VoicePool>();
        BubbleA1Config c;
        c.voiceCapacity = 128;
        check(pool->prepare(48000, c), "pending cancellation setup");
        BubbleA1Event e;
        e.physics.frequencyHz = 300;
        e.physics.tauSeconds = 1;
        e.physics.poleRadius = std::exp(-1. / 48000);
        e.amplitude = {.5, -.25};
        for (int i = 0; i < 128; ++i)
            (void)pool->trigger(e);
        check(pool->trigger(e) == BubbleA1TriggerResult::pendingReplacement,
              "typed pending replacement");
        auto queue = std::make_unique<PreviewEventTrace>();
        pool->setObserver(queue.get(), PreviewEventTrace::captureA1);
        check(pool->setCapacity(64), "downshift retains releasing voices");
        e.requestId = 130;
        check(pool->trigger(e) == BubbleA1TriggerResult::capacityDropped,
              "downshift rejects new request with typed outcome");
        for (int i = 0; i < 72; ++i)
            (void)pool->process();
        const auto& life = pool->counters().lifecycle;
        check(life.pendingReplacementDropped == 1 && life.causedStealButNeverNonZero == 1 &&
                  life.replacementStarted == 0 && life.completedWithoutNonZero == 0,
              "unstarted pending cancellation is not a completed voice");
        PreviewEventRecord record;
        std::size_t dropped{}, pendingDropped{};
        while (queue->pop(record)) {
            const auto& r = record.a1;
            if (r.kind == BubbleA1ObservationKind::capacityDropped) {
                ++dropped;
                check(r.event.requestId == 130 && r.capacityDrops == 1,
                      "capacity drop closes the exact incoming request");
            }
            if (r.kind == BubbleA1ObservationKind::pendingDropped) {
                ++pendingDropped;
                check(r.capacityDrops == pool->counters().capacityDrops && r.capacityDrops == 2 &&
                          r.lifecycle.pendingReplacementDropped == life.pendingReplacementDropped &&
                          r.lifecycle.causedStealButNeverNonZero == life.causedStealButNeverNonZero,
                      "pending drop snapshot includes current cumulative bookkeeping");
            }
        }
        check(dropped == 1 && pendingDropped == 1 && queue->takeDropped() == 0,
              "one direct outcome and one later pending cancellation");
    }
    // Exact band-edge semantics and non-coalesced deferred-start identity.
    for (std::size_t i = 0; i < 6; ++i) {
        const double edge = 250. * std::pow(2., double(i));
        check(bubbleA1Band(edge) == i + 1 && bubbleA1Band(edge - .01) == i,
              "half-open analysis bands");
    }
    {
        auto pool = std::make_unique<BubbleA1VoicePool>();
        auto queue = std::make_unique<PreviewEventTrace>();
        BubbleA1Config c;
        c.voiceCapacity = 64;
        check(pool->prepare(48000, c), "diagnostic pool prepare");
        pool->setObserver(queue.get(), PreviewEventTrace::captureA1);
        BubbleA1Event e;
        e.physics.frequencyHz = 300;
        e.physics.tauSeconds = 1;
        e.physics.poleRadius = std::exp(-1. / 48000);
        e.amplitude = {.5, -.25};
        e.sourceCarrier = {1, -.5};
        for (std::uint64_t id = 1; id <= 128; ++id) {
            e.requestId = id;
            check(a1TriggerAccepted(pool->trigger(e)), "fill and reserve every release");
        }
        e.requestId = 129;
        check(!a1TriggerAccepted(pool->trigger(e)), "all releasing capacity drop");
        PreviewEventRecord row;
        std::size_t dropped{};
        while (queue->pop(row)) {
            if (row.a1.kind == BubbleA1ObservationKind::capacityDropped) {
                ++dropped;
                check(row.a1.event.requestId == 129 && row.a1.capacityDrops == 1,
                      "all-pending rejection retains incoming identity and cumulative drop");
            }
        }
        check(dropped == 1, "all-pending rejection is directly observable");
        allocationtest::allocations = 0;
        allocationtest::observing = true;
        for (int n = 0; n < 72; ++n)
            (void)pool->process();
        allocationtest::observing = false;
        check(allocationtest::allocations == 0, "deferred trace processing allocates nothing");
        std::size_t starts{};
        while (queue->pop(row)) {
            if (row.a1.kind != BubbleA1ObservationKind::started)
                continue;
            check(row.a1.kind == BubbleA1ObservationKind::started && row.a1.frame == 71 &&
                      row.a1.event.requestId == 65 + starts && row.a1.event.requestFrame == 0 &&
                      row.a1.event.sourceCarrier == e.sourceCarrier,
                  "all 64 same-frame deferred starts retain causal payload");
            ++starts;
        }
        check(starts == 64 && queue->takeDropped() == 0, "no start coalescing");
        check(pool->counters().lifecycle.firstNonZero == 64 &&
                  pool->counters().lifecycle.acceptedAsPendingReplacement == 64 &&
                  pool->counters().lifecycle.replacementStarted == 64 &&
                  pool->counters().lifecycle.replacementFirstNonZero == 0,
              "replacement admission/start/emission are distinct");
        const auto band = pool->bandObservation(1);
        check(band.bandCounters.requested == 129 && band.bandCounters.started == 128 &&
                  band.bandCounters.completed == 64 && band.bandCounters.stolen == 64 &&
                  band.bandCounters.capacityDrops == 1 &&
                  near(band.bandCounters.initialSquaredAmplitudeSum, 20.),
              "lifecycle totals and independent initial squared amplitude oracle");
        // Reset clears pending work and all diagnostic totals.
        for (int n = 0; n < 64; ++n)
            (void)a1TriggerAccepted(pool->trigger(e));
        pool->reset();
        check(pool->counters().requested == 0 && pool->bandObservation(1).bandCounters.started == 0,
              "reset clears cumulative counters");
    }
    for (double rate : rates) {
        auto observed = std::make_unique<BubbleA1>();
        auto reference = std::make_unique<BubbleA1>();
        auto queue = std::make_unique<PreviewEventTrace>();
        check(observed->prepare({rate, 42}) && reference->prepare({rate, 42}), "trace fixtures");
        observed->setObserver(queue.get(), PreviewEventTrace::captureA1);
        allocationtest::allocations = 0;
        allocationtest::observing = true;
        for (int n = 0; n < 12000; ++n) {
            const StereoFrame x{.7f, -.35f};
            check(observed->process(x) == reference->process(x), "trace preserves samples");
            PreviewEventRecord row;
            while (queue->pop(row)) {
                const auto& e = row.a1.event;
                check(e.requestId > 0 && e.requestFrame <= row.a1.frame,
                      "captured request identity and clock");
                check(near(e.amplitude[0],
                           e.sourceCarrier[0] * e.physics.amplitude * e.audibleDepth * .2),
                      "captured amplitude decomposition");
            }
        }
        allocationtest::observing = false;
        check(allocationtest::allocations == 0 && queue->takeDropped() == 0,
              "observed process has no allocations or transport loss");
    }

    // Preserve the numeric identity of EVERY historic stream, including named A1 domain 6.
    const std::array domains{RandomDomain::bubble,
                             RandomDomain::droplet,
                             RandomDomain::flow,
                             RandomDomain::modalMotion,
                             RandomDomain::dropletActivity,
                             RandomDomain::bubbleA1,
                             RandomDomain::dropletB1Identity,
                             RandomDomain::dropletB1Admission,
                             RandomDomain::dropletB1Jitter};
    for (std::size_t i = 0; i < domains.size(); ++i) {
        check(static_cast<std::uint64_t>(domains[i]) == i + 1, "stable domain ID");
        for (RandomSource::Seed seed : {0u, 42u, 20260916u}) {
            ResearchConfig research{48000, seed};
            check(research.seedFor(domains[i]) == RandomSource::deriveInstanceSeed(seed, i + 1),
                  "named domain retains exact historical seed");
        }
    }
    // Independent all-domain bound, not a call to production physics/config metadata.
    // Every unnormalized radius amplitude >=1 => weighted second-moment norm >=1.
    // Rmax/Rmin<=250, alpha<=2.25, carrier<=sqrt(2), proxy/gain<=1.
    // Damping decreases with R; tau<=4/d(.05); absolute floor>=1e-5.
    const double maximumAmplitude = std::sqrt(2.) * std::pow(250., 2.25);
    const double maximumTau = 4 / (.13 / .05 + .0072 / std::pow(.05, 1.5));
    const double lifetimeBound = maximumTau * std::log(maximumAmplitude / 1e-5);
    check(lifetimeBound < 29.942, "independent global audible-envelope lifetime");
    for (double rate : rates) {
        check(lifetimeBound + 2 / rate < BubbleA1Voice::kMaximumLifetimeSeconds,
              "30s guard exceeds global bound plus sample margin");
        BubbleA1Event adversary;
        adversary.physics.frequencyHz = 400;
        adversary.physics.tauSeconds = maximumTau;
        adversary.physics.poleRadius = std::exp(-1 / (maximumTau * rate));
        adversary.amplitude = {maximumAmplitude, -maximumAmplitude};
        BubbleA1Voice bounded;
        bounded.start(adversary, rate, -100);
        while (!bounded.done() && bounded.age < static_cast<unsigned>(30 * rate))
            (void)bounded.process();
        check(bounded.audibleEnvelope() <= 1e-5 && bounded.age < 30 * rate,
              "adversarial envelope retires naturally before guard at every rate");
        for (double minimum : {.2, 1., 10.})
            for (double maximum : {2., 10., 50.})
                for (double gamma : {0., 2., 6.})
                    for (double alpha : {.75, 1.5, 2.25}) {
                        if (current && !((minimum == .2 && maximum == 2. && gamma == 0. && alpha == .75) ||
                                         (minimum == 10. && maximum == 50. && gamma == 6. && alpha == 2.25) ||
                                         (minimum == 1. && maximum == 10. && gamma == 2. && alpha == 1.5)))
                            continue;
                        if (minimum >= maximum)
                            continue;
                        BubbleA1Config corner;
                        corner.radiusMinMm = minimum;
                        corner.radiusMaxMm = maximum;
                        corner.populationGamma = gamma;
                        corner.amplitudeRadiusExponent = alpha;
                        corner.persistenceScale = 4;
                        BubbleA1Model table;
                        check(table.prepare(rate, corner), "guard property table");
                        for (const auto& bin : table.bins())
                            check(bin.amplitude * std::sqrt(2.) <= maximumAmplitude * (1 + 1e-12) &&
                                      bin.tauSeconds <= maximumTau * (1 + 1e-12),
                                  "population normalization respects analytic guard bound");
                    }
    }
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
    for (double rate : rates)
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
        check(a1TriggerAccepted(pool->trigger(event)), "single trigger");
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
        check(a1TriggerAccepted(pool->trigger(event)), "fill pool");
    check(pool->setCapacity(128) && pool->active() == 512, "non-destructive 512-to-128 downshift");
    check(!a1TriggerAccepted(pool->trigger(event)) && pool->counters().capacityDrops == 1,
          "downshift inhibits allocation");
    for (int n = 0; n < 48000; ++n)
        (void)pool->process();
    check(pool->active() == 0, "tails retire after downshift");
    check(pool->setCapacity(64), "lower capacity for stealing test");
    for (int i = 0; i < 64; ++i)
        check(a1TriggerAccepted(pool->trigger(event)), "fill lower ceiling");
    for (int n = 0; n < 100; ++n)
        (void)pool->process();
    check(a1TriggerAccepted(pool->trigger(event)) && pool->counters().steals == 1,
          "bounded releasing replacement");
    const auto beforeStart = pool->counters().started;
    for (int n = 0; n < 71; ++n)
        (void)pool->process();
    check(pool->counters().started == beforeStart, "no hard immediate steal");
    (void)pool->process();
    check(pool->counters().started == beforeStart + 1, "release then replacement");

    for (double rate : rates) {
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
        for (int block : (full ? std::vector<int>{1, 7, 32, 64, 128, 256, 257, 512, 1024}
                               : std::vector<int>{1, 128, 257, 1024})) {
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
    for (double rate : rates) {
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
    if (current) {
        const double rate = 96000.;
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
        a->reset();
        check(a->requested() == 0, "cross-rate reset clears scheduler");
    }
    std::cout << "bubble_a1 failures=" << failures << '\n';
    return failures ? 1 : 0;
}
