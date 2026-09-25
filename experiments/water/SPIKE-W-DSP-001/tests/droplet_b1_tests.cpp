#include "DropletB1TestSupport.h"
#include "dsp/BubbleA1.h"

#include <memory>
#include <vector>
using namespace frazil::water::research;
int main() {
    b1test::Checks check;
    // Direct module API: storage invariants cannot depend on public config validation.
    {
        auto pool = std::make_unique<DropletB1VoicePool>();
        const auto event = b1test::event(48000, {});
        check(!pool->request(event) && pool->process() == StereoFrame{}, "unprepared pool safe");
        for (double rate :
             {32000., 44100., 48000., 96000., 192000., std::numeric_limits<double>::quiet_NaN(),
              std::numeric_limits<double>::infinity()})
            for (std::size_t capacity :
                 {std::size_t{0}, std::size_t{1}, std::size_t{16}, std::size_t{32},
                  std::size_t{256}, std::size_t{257}, std::numeric_limits<std::size_t>::max()}) {
                check(pool->prepare(48000, 1) && pool->request(event), "seed stale voice");
                pool->process();
                const bool valid = std::isfinite(rate) && rate >= 44100 && rate <= 96000 &&
                                   capacity >= 1 && capacity <= 256;
                check(pool->prepare(rate, capacity) == valid, "direct prepare domain");
                check(pool->active() == 0 && pool->counters().started == 0 &&
                          pool->lastStarted().eligibleId == 0,
                      "prepare clears captured state");
                if (!valid) {
                    check(!pool->request(event), "invalid prepare rejects request");
                    for (int n = 0; n < 300; ++n)
                        check(pool->process() == StereoFrame{}, "invalid prepare silent finite");
                    pool->reset();
                    check(!pool->request(event), "reset cannot enable failed preparation");
                } else {
                    check(pool->request(event), "valid internal capacity usable");
                    const auto y = pool->process();
                    check(std::isfinite(y[0]) && std::isfinite(y[1]), "valid pool finite");
                }
            }
        DropletB1Config publicConfig;
        publicConfig[B1Parameter::capacity] = 1;
        check(!publicConfig.valid(), "internal capacity1 is not a public choice");
    }
    for (double rate : {44100., 48000., 96000.}) {
        const std::size_t frames = static_cast<std::size_t>(rate * 1.1);
        std::vector<StereoFrame> reference(frames);
        auto a = std::make_unique<DropletB1>(), b = std::make_unique<DropletB1>();
        check(a->prepare({rate, 42}) && b->prepare({rate, 42}), "prepare");
        for (std::size_t n = 0; n < frames; ++n)
            reference[n] = a->process(b1test::source(n, rate));
        check(a->counters().eligible > 2, "fixture onsets exercised");
        for (std::size_t block : {1u, 7u, 32u, 64u, 128u, 256u, 257u, 512u, 1024u}) {
            b->reset();
            for (std::size_t start = 0; start < frames; start += block)
                for (std::size_t n = start; n < std::min(frames, start + block); ++n)
                    check(b->process(b1test::source(n, rate)) == reference[n],
                          "partition/reset exact");
        }
        // Admission, radius and persistence never alter eligible timing or identity rank.
        for (double probability : {0., .25, .5, 1.})
            for (double radius : {.2, 7.})
                for (double persistence : {.25, 4.}) {
                    DropletB1Config c;
                    c[B1Parameter::admission] = probability;
                    c[B1Parameter::radius] = radius;
                    c[B1Parameter::persistence] = persistence;
                    a->reset();
                    check(b->prepare({rate, 42}, c), "variant prepare");
                    RandomSource identity(
                        ResearchConfig{rate, 42}.seedFor(RandomDomain::dropletB1Identity));
                    RandomSource admission(
                        ResearchConfig{rate, 42}.seedFor(RandomDomain::dropletB1Admission));
                    std::uint64_t before{}, admitted{};
                    for (std::size_t n = 0; n < frames; ++n) {
                        a->process(b1test::source(n, rate));
                        const auto y = b->process(b1test::source(n, rate));
                        check(std::isfinite(y[0]) && std::isfinite(y[1]), "finite matrix");
                        check(a->counters().eligible == b->counters().eligible,
                              "Size Decay admission timing isolation");
                        if (b->counters().eligible != before) {
                            ++before;
                            const auto& e = b->lastEligible();
                            check(e.randomRank == identity.nextUInt() &&
                                      e.randomRank == a->lastEligible().randomRank,
                                  "identity consumed before admission");
                            const double draw = admission.nextUnipolar();
                            admitted += draw < probability;
                            check(e.admissionDraw == draw && b->counters().admitted == admitted,
                                  "independent admission oracle");
                            check(e.dueSample == a->lastEligible().dueSample, "due time invariant");
                        }
                    }
                    check(b->counters().eligible ==
                              b->counters().admitted + b->counters().rejectedByAdmission,
                          "admission counter conservation");
                }
        for (int stereo = 0; stereo < 9; ++stereo) {
            a->prepare({rate, 42});
            b->prepare({rate, 42});
            for (std::size_t n = 0; n < frames; ++n) {
                auto x = b1test::source(n, rate);
                const float left = x[0];
                if (stereo == 0)
                    x = {left, left};
                if (stereo == 1)
                    x = {left, 0};
                if (stereo == 2)
                    x = {0, left};
                if (stereo == 3)
                    x = {left, -left};
                if (stereo == 4)
                    x = {left, float(.7 * std::sin(2 * std::numbers::pi * 173 * n / rate))};
                if (stereo == 5)
                    x = {left, .01f * left};
                if (stereo == 6)
                    x = {.01f * left, left};
                if (stereo == 7)
                    x = {n % 500 == 0 ? .9f : left, .001f * left};
                if (stereo == 8)
                    x = {.001f * left, n % 500 == 0 ? .9f : left};
                const auto y = a->process(x), z = b->process({x[1], x[0]});
                check(y[0] == z[1] && y[1] == z[0], "stereo swap and shared event identity");
                if (stereo == 0)
                    check(y[0] == y[1], "dual mono");
                if (stereo == 1)
                    check(y[1] == 0, "left isolated");
                if (stereo == 2)
                    check(y[0] == 0, "right isolated");
                if (stereo == 3)
                    check(y[0] == -y[1], "anti phase");
            }
        }
        // Admission retarget preserves a queued event and its tail, without restarting RNG.
        a->prepare({rate, 42});
        while (a->counters().queued == 0)
            a->process({.7f, .7f});
        const auto captured = a->lastEligible();
        check(a->setEntrainmentProbability(0), "retarget p0");
        for (int n = 0; n < int(rate * .3); ++n)
            a->process({});
        check(a->pool().counters().started == 1 &&
                  a->pool().lastStarted().randomRank == captured.randomRank,
              "pending survives retarget");
        check(!a->setEntrainmentProbability(-1), "invalid retarget rejects");
        a->reset();
        for (int n = 0; n < 1000; ++n)
            check(a->process({}) == StereoFrame{}, "silence after tail reset");
        a->prepare({rate, 42});
        for (int n = 0; n < 1000; ++n) {
            const auto y =
                a->process({std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()});
            check(std::isfinite(y[0]), "finite float extremes");
        }
        // Fixed resource pool: capacity does not scale isolated output; explicit overload.
        DropletB1Config c;
        c[B1Parameter::radius] = 7;
        c[B1Parameter::persistence] = 4;
        const auto e = b1test::event(rate, c);
        std::vector<StereoFrame> isolated(300);
        auto pool = std::make_unique<DropletB1VoicePool>();
        for (std::size_t cap : {16u, 32u, 64u, 128u, 256u}) {
            pool->prepare(rate, cap);
            pool->request(e);
            for (int n = 0; n < 300; ++n) {
                const auto y = pool->process();
                if (cap == 16)
                    isolated[n] = y;
                else
                    check(y == isolated[n], "capacity independent gain");
            }
            pool->reset();
            for (std::size_t n = 0; n < cap * 2; ++n)
                check(pool->request(e), "fill then bounded releases");
            check(!pool->request(e) && pool->counters().droppedByVoiceCapacity == 1,
                  "all releasing overflow drop");
            check(pool->counters().steals == cap, "steal count");
            for (int n = 0; n < int(rate * 2.1); ++n) {
                auto y = pool->process();
                check(std::isfinite(y[0]) && pool->active() <= cap, "stress lifetime bound");
            }
            check(pool->active() == 0 && pool->counters().started == cap * 2 &&
                      pool->counters().completed == cap * 2,
                  "retire all natural/stolen");
        }
        // Captured onset -> due -> free-slot initialization, after the transient ended.
        c[B1Parameter::delay] = 40;
        a->prepare({rate, 42}, c);
        std::uint64_t firstSource{}, due{};
        bool capturedSource{}, observedStart{};
        for (std::uint64_t n = 0; n < static_cast<std::uint64_t>(rate * .1); ++n) {
            const StereoFrame x = n < static_cast<std::uint64_t>(rate * .002)
                                      ? StereoFrame{.9f, -.3f}
                                      : StereoFrame{};
            a->process(x);
            if (!capturedSource && a->counters().eligible) {
                capturedSource = true;
                firstSource = a->lastEligible().impact.sourceSample;
                due = a->lastEligible().dueSample;
                check(firstSource == n &&
                          due == n + static_cast<std::uint64_t>(std::ceil(rate * .04)),
                      "captured delay independently predicted");
            }
            if (!observedStart && a->pool().counters().started) {
                observedStart = true;
                const auto& started = a->pool().lastStarted();
                check(x == StereoFrame{} && n == due && firstSource < due &&
                          started.impact.sourceSample == firstSource && started.dueSample == due &&
                          started.impact.sourceExcitation > 0,
                      "delayed zero-current-frame start retains source causality");
            }
        }
        check(capturedSource && observedStart, "delayed fixture exercised");
        DropletB1PendingQueue queue;
        // Independent release oracle: one victim, zero outgoing contribution at the
        // endpoint, then the fully captured replacement's first sample (no future input).
        pool->prepare(rate, 1);
        pool->request(e);
        DropletB1BubbleVoice outgoing, incoming;
        outgoing.start(e, rate);
        incoming.start(e, rate);
        pool->process();
        outgoing.process();
        auto replacement = e;
        replacement.eligibleId = 77;
        replacement.impact.sourceSample = 0;
        replacement.dueSample = 1; // Request on pool timeline sample1, after sample0 above.
        check(pool->request(replacement), "release request");
        const auto release = static_cast<int>(std::ceil(rate * e.releaseMs * .001));
        for (int n = 0; n < release; ++n) {
            const auto expected = outgoing.process();
            const auto actual = pool->process();
            check(pool->counters().started == (n == release - 1 ? 2u : 1u),
                  "replacement initializes only on final release frame");
            for (int ch = 0; ch < 2; ++ch)
                check(actual[ch] ==
                          static_cast<float>(expected[ch] * (double(release - n - 1) / release)),
                      "release ramp and zero endpoint oracle");
        }
        // Initialization occurs on the final release frame; emission starts next frame.
        const auto initializationFrame = replacement.dueSample + release - 1;
        check(pool->lastStarted().impact.sourceSample == 0 && pool->lastStarted().dueSample == 1 &&
                  initializationFrame >= replacement.dueSample &&
                  initializationFrame - replacement.dueSample < static_cast<std::uint64_t>(release),
              "captured stealing source/due and bounded initialization delay");
        const auto expected = incoming.process();
        const auto actual = pool->process();
        check(actual[0] == static_cast<float>(expected[0]) && pool->lastStarted().eligibleId == 77,
              "captured replacement begins next sample");
        check(!a->prepare({32000, 42}) && a->process({.7f, .7f}) == StereoFrame{},
              "unsupported prepare disables processing");
        check(a->prepare({rate, 42}) && a->counters().eligible == 0 &&
                  a->pool().counters().started == 0,
              "reprepare resets lifetime state");
        for (std::size_t i = 0; i < queue.kCapacity; ++i)
            check(queue.push(e), "pending fill");
        check(!queue.push(e) && !queue.popDue(e.dueSample - 1), "pending bound / due time");
        for (std::size_t i = 0; i < queue.kCapacity; ++i)
            check(queue.popDue(e.dueSample).has_value(), "pending drain");
        check(!queue.popDue(e.dueSample), "pending empty");
    }
    for (std::size_t i = 0; i < kB1Parameters.size(); ++i) {
        DropletB1Config c;
        c.values[i] = std::numeric_limits<double>::quiet_NaN();
        check(!c.valid(), "all fields reject nonfinite");
        c.values[i] = kB1Parameters[i].maximum + 1;
        check(!c.valid(), "all fields reject high");
        c.values[i] = kB1Parameters[i].minimum - 1;
        check(!c.valid(), "all fields reject low");
    }
    return check.failures ? 1 : 0;
}
