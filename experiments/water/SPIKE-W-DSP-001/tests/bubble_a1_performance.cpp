#include "dsp/BubbleA1.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>
using namespace frazil::water::research;
int main() {
    constexpr int block = 128, warmup = 500, measured = 3000;
    std::cout << "rate,capacity,profile,mean_us,p95_us,p99_us,worst_us,active_mean,active_peak,"
                 "events_per_second,steals,drops,output_sum\n";
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
                if (!a->prepare({rate, 42}, c))
                    return 1;
                std::vector<double> times(measured);
                double sink{}, activeSum{};
                std::size_t peak{};
                std::uint64_t eventStart{}, stealStart{}, dropStart{};
                for (int b = -warmup; b < measured; ++b) {
                    if (b == 0) {
                        eventStart = a->requested();
                        stealStart = a->pool().counters().steals;
                        dropStart = a->pool().counters().capacityDrops;
                    }
                    const auto start = std::chrono::steady_clock::now();
                    for (int n = 0; n < block; ++n) {
                        const auto y = a->process({.7f, -.7f});
                        sink += y[0];
                    }
                    const auto end = std::chrono::steady_clock::now();
                    if (b >= 0) {
                        times[b] = std::chrono::duration<double, std::micro>(end - start).count();
                        activeSum += a->pool().active();
                        peak = std::max(peak, a->pool().active());
                    }
                }
                const double mean = std::accumulate(times.begin(), times.end(), 0.) / measured;
                std::sort(times.begin(), times.end());
                std::cout << rate << ',' << cap << ','
                          << (profile ? "dense-stress" : "physical-reference") << ',' << mean << ','
                          << times[2849] << ',' << times[2969] << ',' << times.back() << ','
                          << activeSum / measured << ',' << peak << ','
                          << (a->requested() - eventStart) * rate / (measured * block) << ','
                          << a->pool().counters().steals - stealStart << ','
                          << a->pool().counters().capacityDrops - dropStart << ',' << sink << '\n';
            }
}
