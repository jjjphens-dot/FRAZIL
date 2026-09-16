#include "app/AudioEngine.h"
#include "dsp/FlowModulator.h"
#include "dsp/FluidCandidate.h"
#include "dsp/LiquidModalResonator.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string_view>
#include <vector>

using namespace frazil::water::research;

namespace {
constexpr int kBlock = 128;
constexpr int kWarmup = 2000;
constexpr int kMeasured = 20000;
constexpr double kRate = 48000.0;

template <typename Effect> double measure(const char* name, Effect effect, double baselineMean) {
    AudioEngine engine;
    if (!engine.prepare({kRate, kBlock, 2}))
        return -1;
    EngineParameters parameters;
    juce::AudioBuffer<float> source(2, kBlock), block(2, kBlock);
    for (int i = 0; i < kBlock; ++i) {
        source.setSample(0, i, static_cast<float>(0.25 * std::sin(i * 0.13)));
        source.setSample(1, i, static_cast<float>(0.2 * std::cos(i * 0.19)));
    }
    std::vector<double> times(kMeasured);
    double outputSum{};
    for (int b = -kWarmup; b < kMeasured; ++b) {
        block.copyFrom(0, 0, source, 0, 0, kBlock);
        block.copyFrom(1, 0, source, 1, 0, kBlock);
        if ((b + kWarmup) % 64 >= 8)
            block.clear(); // Fixed gated workload exercises repeated Droplet onsets after warmup.
        const auto start = std::chrono::steady_clock::now();
        engine.process(block, parameters);
        effect(block);
        const auto end = std::chrono::steady_clock::now();
        if (b >= 0)
            times[b] = std::chrono::duration<double, std::micro>(end - start).count();
        outputSum += block.getSample(0, b < 0 ? 0 : b % kBlock);
    }
    const double mean = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    std::sort(times.begin(), times.end());
    std::cout << name << ',' << mean << ','
              << times[static_cast<std::size_t>(std::ceil(.95 * kMeasured)) - 1] << ','
              << times[static_cast<std::size_t>(std::ceil(.99 * kMeasured)) - 1] << ','
              << times.back() << ',' << (baselineMean == 0.0 ? 0.0 : mean - baselineMean) << ','
              << outputSum << '\n';
    return mean;
}
} // namespace

int main() {
    std::cout
        << "research_only rate=48000 block=128 stereo warmup=2000 measured=20000; "
           "gated stereo workload; nearest-rank percentiles; wall time, not process CPU percent\n";
    std::cout << "formal_provenance_status=NOT RUN (standalone research timing)\n";
    std::cout << "case,mean_us,p95_us,p99_us,worst_us,delta_mean_us,output_sum\n";
    const double baseline = measure("M1", [](auto&) {}, 0.0);
    LiquidModalResonator modal;
    if (!modal.prepare(kRate))
        return 1;
    measure(
        "M1+C",
        [&](auto& buffer) {
            for (int i = 0; i < kBlock; ++i) {
                const auto y = modal.process({buffer.getSample(0, i), buffer.getSample(1, i)});
                for (int c = 0; c < 2; ++c)
                    buffer.addSample(c, i, y[c]);
            }
        },
        baseline);
    FlowModulator flow;
    if (!flow.prepare({}))
        return 1;
    measure(
        "M1+D",
        [&](auto& buffer) {
            for (int i = 0; i < kBlock; ++i) {
                const auto y = flow.process({buffer.getSample(0, i), buffer.getSample(1, i)});
                for (int c = 0; c < 2; ++c)
                    buffer.addSample(c, i, y[c]);
            }
        },
        baseline);
    for (const auto name : {"A", "B", "AB", "AD", "BD", "ABD"}) {
        const std::string_view mode(name);
        FluidConfig config;
        config.bubbleEnabled = mode.find('A') != std::string_view::npos;
        config.dropletEnabled = mode.find('B') != std::string_view::npos;
        config.flowEnabled = mode.find('D') != std::string_view::npos;
        FluidCandidate fluid;
        if (!fluid.prepare({}, config))
            return 1;
        measure(
            name,
            [&](auto& buffer) {
                for (int i = 0; i < kBlock; ++i) {
                    const auto y = fluid.process({buffer.getSample(0, i), buffer.getSample(1, i)});
                    for (int c = 0; c < 2; ++c)
                        buffer.addSample(c, i, y[c]);
                }
            },
            baseline);
    }
    return 0;
}
