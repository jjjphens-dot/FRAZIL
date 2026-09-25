#include "dsp/BubbleA1.h"
#include "dsp/DropletB1.h"
#include "dsp/FlowD1.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>
using namespace frazil::water::research;
int main() {
    std::cout << "rate,block,profile,mean_us,p95_us,p99_us,worst_us,deadline_us,output_sum\n";
    constexpr int warmup = 100, measured = 1000;
    for (double rate : {44100., 48000., 96000.})
        for (int block : {32, 128, 1024})
            for (int profile = 0; profile < 4; ++profile) {
                FlowD1 flow;
                flow.prepare({rate, 42}, {.2, .03, .015});
                FlowD1FractionalDelay cubic;
                cubic.prepare(rate, .05);
                auto a = std::make_unique<BubbleA1>();
                auto b = std::make_unique<DropletB1>();
                if (!a->prepare({rate, 42}) || !b->prepare({rate, 42}))
                    return 1;
                std::array<std::array<float, 8>, 2> linear{};
                std::size_t write = 0;
                std::vector<StereoFrame> input((warmup + measured) * block);
                for (std::size_t n = 0; n < input.size(); ++n) {
                    const float x = float(.5 * std::sin(n * .13));
                    input[n] = {x, -.3f * x};
                }
                std::vector<double> times(measured);
                double sink = 0;
                for (int batch = 0; batch < warmup + measured; ++batch) {
                    auto start = std::chrono::steady_clock::now();
                    for (int n = 0; n < block; ++n) {
                        auto x = input[batch * block + n];
                        if (profile == 0) {
                            // Independent numeric baseline, same two channels, fixed .5 sample
                            // delay.
                            for (std::size_t c = 0; c < 2; ++c) {
                                linear[c][write] = x[c];
                                sink += .5 * (double(x[c]) + linear[c][(write + 7) % 8]);
                            }
                            write = (write + 1) % 8;
                        } else if (profile == 1) {
                            auto y = cubic.process(x, .5);
                            sink += y[0] + y[1];
                        } else {
                            if (profile == 3) {
                                auto av = a->process(x), bv = b->process(x);
                                for (std::size_t c = 0; c < 2; ++c)
                                    x[c] = float(double(av[c]) + bv[c]);
                            }
                            auto y = flow.process(x);
                            sink += y.transferred[0] + y.transferred[1];
                        }
                    }
                    auto end = std::chrono::steady_clock::now();
                    if (batch >= warmup)
                        times[batch - warmup] =
                            std::chrono::duration<double, std::micro>(end - start).count();
                }
                if (!std::isfinite(sink))
                    return 1;
                double mean = std::accumulate(times.begin(), times.end(), 0.) / measured;
                std::sort(times.begin(), times.end());
                const std::array names{"linear-kernel", "lagrange3-kernel", "D1", "A1+B1+D1"};
                std::cout << rate << ',' << block << ',' << names[profile] << ',' << mean << ','
                          << times[949] << ',' << times[989] << ',' << times.back() << ','
                          << block / rate * 1e6 << ',' << sink << '\n';
            }
}
