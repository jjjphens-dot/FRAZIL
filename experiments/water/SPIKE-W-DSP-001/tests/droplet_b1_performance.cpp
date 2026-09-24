#include "DropletB1TestSupport.h"

#include <chrono>
#include <memory>
#include <numeric>
#include <vector>
using namespace frazil::water::research;
int main() {
    constexpr int block = 128, warmup = 100, measured = 1000;
    std::cout << "rate,capacity,profile,radius_mm,persistence,mean_us,p95_us,p99_us,worst_us,"
                 "active_mean,active_peak,event_start_mean_us,event_start_p95_us,eligible,started,"
                 "steals,drops,output_sum\n";
    for (double rate : {44100., 48000., 96000.})
        for (std::size_t capacity : {16u, 32u, 64u, 128u, 256u})
            for (double radius : {.2, 7.})
                for (double persistence : {.25, 4.})
                    for (int stress : {0, 1}) {
                        DropletB1Config c;
                        c[B1Parameter::capacity] = double(capacity);
                        c[B1Parameter::radius] = radius;
                        c[B1Parameter::persistence] = persistence;
                        if (stress) {
                            c[B1Parameter::spacing] = 8;
                            c[B1Parameter::onset] = 3;
                            c[B1Parameter::hysteresis] = 1;
                        }
                        auto processor = std::make_unique<DropletB1>();
                        if (!processor->prepare({rate, 42}, c))
                            return 1;
                        std::vector<StereoFrame> inputs((warmup + measured) * block);
                        for (std::size_t n = 0; n < inputs.size(); ++n) {
                            const auto position = std::fmod(n / rate, stress ? .06 : .25);
                            const float x =
                                position < (stress ? .004 : .02)
                                    ? float(.7 * std::cos(2 * std::numbers::pi * 173 * n / rate))
                                    : 0;
                            inputs[n] = {x, -.3f * x};
                        }
                        std::vector<double> times(measured), startTimes(measured);
                        double activeSum{}, sink{};
                        std::size_t activePeak{};
                        for (int b = 0; b < warmup + measured; ++b) {
                            const auto start = std::chrono::steady_clock::now();
                            for (int n = 0; n < block; ++n)
                                sink += processor->process(inputs[b * block + n])[0];
                            const auto end = std::chrono::steady_clock::now();
                            if (b >= warmup) {
                                times[b - warmup] =
                                    std::chrono::duration<double, std::micro>(end - start).count();
                                activeSum += processor->pool().active();
                                activePeak = std::max(activePeak, processor->pool().active());
                            }
                        }
                        // Isolated event-start coefficient setup, measured separately from the
                        // source path.
                        auto e = b1test::event(rate, c);
                        DropletB1BubbleVoice voice;
                        // An opaque call prevents the optimizer from hoisting invariant
                        // coefficient setup outside the timed interval. This barrier belongs
                        // to the offline benchmark only; include its call overhead in the result.
                        void (*volatile startVoice)(DropletB1BubbleVoice&, const DropletB1Event&,
                                                    double) =
                            +[](DropletB1BubbleVoice& v, const DropletB1Event& event, double fs) {
                                v.start(event, fs);
                            };
                        for (auto& duration : startTimes) {
                            const auto start = std::chrono::steady_clock::now();
                            startVoice(voice, e, rate);
                            const auto end = std::chrono::steady_clock::now();
                            duration =
                                std::chrono::duration<double, std::micro>(end - start).count();
                            sink += voice.process()[0];
                        }
                        const double mean =
                            std::accumulate(times.begin(), times.end(), 0.) / measured;
                        const double startMean =
                            std::accumulate(startTimes.begin(), startTimes.end(), 0.) / measured;
                        std::sort(times.begin(), times.end());
                        std::sort(startTimes.begin(), startTimes.end());
                        const auto& v = processor->pool().counters();
                        if (!std::isfinite(sink))
                            return 1;
                        std::cout << rate << ',' << capacity << ','
                                  << (stress ? "stress-onsets" : "normal-onsets") << ',' << radius
                                  << ',' << persistence << ',' << mean << ',' << times[949] << ','
                                  << times[989] << ',' << times.back() << ','
                                  << activeSum / measured << ',' << activePeak << ',' << startMean
                                  << ',' << startTimes[949] << ',' << processor->counters().eligible
                                  << ',' << v.started << ',' << v.steals << ','
                                  << v.droppedByVoiceCapacity << ',' << sink << '\n';
                    }
}
