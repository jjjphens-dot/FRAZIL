#include "dsp/BubbleA1.h"
#include "preview/PreviewEventTrace.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>
using namespace frazil::water::research;
int main(int argc, char** argv) {
    bool trace = false, l1 = false, observerOnly = false, workloadOnly = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--trace" && !trace)
            trace = true;
        else if (std::string_view(argv[i]) == "--l1" && !l1)
            l1 = true;
        else if (std::string_view(argv[i]) == "--observer-only" && !observerOnly)
            observerOnly = true;
        else if (std::string_view(argv[i]) == "--workload" && !workloadOnly)
            workloadOnly = true;
        else
            return 2;
    }
    if (trace && observerOnly)
        return 2;
    constexpr int block = 128, warmup = 500, measured = 3000;
    std::cout << "rate,capacity,profile,mean_us,p95_us,p99_us,worst_us,active_mean,active_peak,"
                 "events_per_second,steals,drops,output_sum,trace_drops,policy,starts_per_second,"
                 "firstNonZero_per_second,preStartCull_per_second,causedStealButNeverNonZero,"
                 "observer_records,measurement_kind,voice_bytes,sidecar_bytes,pool_bytes,"
                 "voice_sample_visits,first_nonzero_taken,completion_taken,requests_per_callback,"
                 "sidecar_logical_touches\n";
    for (double rate : {44100., 48000., 96000.})
        for (std::size_t cap : {64u, 128u, 256u, 512u, 1024u})
            for (int profile : {0, 1}) {
                auto a = std::make_unique<BubbleA1>();
                BubbleA1Config c;
                c.voiceCapacity = cap;
                if (profile) {
                    c.radiusMinMm = 10;
                    c.radiusMaxMm = 50;
                    c.persistenceScale = 4;
                    c.maxEventRateHz = 10000;
                    c.depthExponent = 1;
                }
                if (!a->prepare({rate, 42}, c, {},
                                l1 ? BubbleA1LifecyclePolicy::admissionAwareL1
                                   : BubbleA1LifecyclePolicy::historicalL0))
                    return 1;
                auto queue = std::make_unique<frazil::water::preview::PreviewEventTrace>();
                std::uint64_t observedRecords{};
                // Separate payload/callback cost from queue producer cost, without changing DSP.
                if (observerOnly)
                    a->setObserver(&observedRecords,
                                   [](void* context, const BubbleA1Observation&) noexcept {
                                       ++*static_cast<std::uint64_t*>(context);
                                   });
                if (trace)
                    a->setObserver(queue.get(),
                                   frazil::water::preview::PreviewEventTrace::captureA1);
                std::uint64_t traceDrops{};
                std::vector<double> times(measured);
                double sink{}, activeSum{};
                std::size_t peak{};
                std::uint64_t eventStart{}, stealStart{}, dropStart{};
                std::uint64_t startsAtWarmup{};
                BubbleA1LifecycleCounters lifecycleAtWarmup{};
                std::uint64_t voiceVisits{}, completionsAtWarmup{};
                for (int b = -warmup; b < measured; ++b) {
                    if (b == 0) {
                        eventStart = a->requested();
                        stealStart = a->pool().counters().steals;
                        dropStart = a->pool().counters().capacityDrops;
                        startsAtWarmup = a->pool().counters().started;
                        lifecycleAtWarmup = a->pool().counters().lifecycle;
                        completionsAtWarmup = a->pool().counters().completed;
                    }
                    const auto start = std::chrono::steady_clock::now();
                    if (workloadOnly) {
                        // Separate audit loop: no per-sample counting branch in timed runs.
                        for (int n = 0; n < block; ++n) {
                            const auto before = a->pool().active();
                            const auto startsBefore = a->pool().counters().started;
                            const auto y = a->process({.7f, -.7f});
                            if (b >= 0)
                                voiceVisits += before + a->pool().counters().started - startsBefore;
                            sink += y[0];
                        }
                    } else {
                        for (int n = 0; n < block; ++n) {
                            const auto y = a->process({.7f, -.7f});
                            sink += y[0];
                        }
                    }
                    const auto end = std::chrono::steady_clock::now();
                    // Transport writes are timed; consumer/file serialization is not DSP.
                    frazil::water::preview::PreviewEventRecord record;
                    while (queue->pop(record)) {
                    }
                    traceDrops += queue->takeDropped();
                    if (b >= 0) {
                        times[b] = std::chrono::duration<double, std::micro>(end - start).count();
                        activeSum += a->pool().active();
                        peak = std::max(peak, a->pool().active());
                    }
                }
                const double mean = std::accumulate(times.begin(), times.end(), 0.) / measured;
                std::sort(times.begin(), times.end());
                const auto& counts = a->pool().counters();
                const double perSecond = rate / (measured * block);
                const auto& life = counts.lifecycle;
                // Deferred replacement starts reuse a slot already visited in that sample.
                if (workloadOnly)
                    voiceVisits -= life.replacementStarted - lifecycleAtWarmup.replacementStarted;
                std::cout << rate << ',' << cap << ','
                          << (profile ? "dense-stress" : "physical-reference") << ',';
                if (workloadOnly)
                    std::cout << "N/A,N/A,N/A,N/A,";
                else
                    std::cout << mean << ',' << times[2849] << ',' << times[2969] << ','
                              << times.back() << ',';
                std::cout << activeSum / measured << ',' << peak << ','
                          << (a->requested() - eventStart) * rate / (measured * block) << ','
                          << a->pool().counters().steals - stealStart << ','
                          << a->pool().counters().capacityDrops - dropStart << ',' << sink << ','
                          << traceDrops << ',' << (l1 ? "L1" : "L0") << ','
                          << (counts.started - startsAtWarmup) * perSecond << ','
                          << (counts.lifecycle.firstNonZero - lifecycleAtWarmup.firstNonZero) *
                                 perSecond
                          << ','
                          << (counts.lifecycle.preStartCulled - lifecycleAtWarmup.preStartCulled) *
                                 perSecond
                          << ','
                          << counts.lifecycle.causedStealButNeverNonZero -
                                 lifecycleAtWarmup.causedStealButNeverNonZero
                          << ',' << observedRecords << ','
                          << (workloadOnly ? "workload-only-not-timing" : "timing") << ','
                          << sizeof(BubbleA1Voice) << ',' << sizeof(BubbleA1VoiceObservationState)
                          << ',' << sizeof(BubbleA1VoicePool) << ',' << voiceVisits << ','
                          << life.firstNonZero - lifecycleAtWarmup.firstNonZero << ','
                          << counts.completed - completionsAtWarmup << ','
                          << double(a->requested() - eventStart) / measured << ','
                          << (workloadOnly ? voiceVisits + counts.started - startsAtWarmup : 0)
                          << '\n';
            }
}
