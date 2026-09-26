#include "AllocationObserver.h"
#include "LatencyCandidate.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
using frazil::water::research::latencytest::LatencyCandidate;
constexpr int kMeasuredBlocks = 500;
constexpr std::size_t kMaximumFixtureFrames = 1000000;
struct Frame {
    double delay{};
    LatencyCandidate::Stereo audio{};
};
void require(bool condition, const char* message) {
    if (!condition)
        throw std::runtime_error(message);
}
} // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 5 || (argc == 6 && std::string(argv[5]) == "--quick"),
                "usage: native coefficients input output metrics [--quick]");
        const int repetitions = argc == 6 ? 5 : kMeasuredBlocks;
        const int warmup = argc == 6 ? 3 : 100;
        std::ifstream coefficients(argv[1]);
        LatencyCandidate::Configuration config;
        int firCount{}, sosCount{};
        coefficients >> config.guard >> config.filterLatency >> firCount >> sosCount;
        require(coefficients.good() && firCount >= 0 &&
                    firCount <= static_cast<int>(LatencyCandidate::kMaximumFirTaps) &&
                    sosCount >= 0 &&
                    sosCount <= static_cast<int>(LatencyCandidate::kMaximumSosSections) &&
                    config.guard >= 8 && config.guard <= 64,
                "invalid coefficient header");
        config.fir.resize(static_cast<std::size_t>(firCount));
        config.sos.resize(static_cast<std::size_t>(sosCount));
        config.table.resize((LatencyCandidate::kTableCells + 1) * (2u * config.guard + 1u));
        for (auto& x : config.fir)
            coefficients >> x;
        for (auto& section : config.sos)
            for (auto& x : section)
                coefficients >> x;
        for (auto& x : config.table)
            coefficients >> x;
        require(!coefficients.fail(), "incomplete coefficients");
        LatencyCandidate processor;
        const auto saved = config;
        require(processor.prepare(std::move(config)), "candidate prepare failed");
        std::ifstream input(argv[2]);
        std::vector<Frame> frames;
        Frame frame;
        while (input >> frame.delay >> frame.audio[0] >> frame.audio[1]) {
            require(std::isfinite(frame.delay) && frame.delay >= 0 &&
                        frame.delay <= LatencyCandidate::kMaximumPhysicalDelaySamples &&
                        std::isfinite(frame.audio[0]) && std::isfinite(frame.audio[1]) &&
                        std::abs(frame.audio[0]) <= std::numeric_limits<float>::max() &&
                        std::abs(frame.audio[1]) <= std::numeric_limits<float>::max(),
                    "invalid input frame");
            frames.push_back(frame);
            require(frames.size() <= kMaximumFixtureFrames, "oversized fixture");
        }
        require(input.eof() && !frames.empty(), "invalid input file");
        frames.resize(frames.size() + static_cast<std::size_t>(processor.latency()));
        std::vector<LatencyCandidate::Stereo> reference(frames.size());
        allocationtest::allocations = 0;
        allocationtest::observing = true;
        for (std::size_t i = 0; i < frames.size(); ++i)
            reference[i] = processor.process(frames[i].audio, frames[i].delay);
        allocationtest::observing = false;
        require(allocationtest::allocations == 0, "render allocation");
        for (auto value : reference)
            require(std::isfinite(value[0]) && std::isfinite(value[1]), "nonfinite output");
        std::ofstream output(argv[3]);
        output << std::setprecision(17);
        for (auto value : reference)
            output << value[0] << ',' << value[1] << '\n';
        output.flush();
        require(output.good(), "output write failed");
        std::ofstream metrics(argv[4]);
        metrics << "block,mean_us,p95_us,p99_us,max_us,state_bytes,observed_allocations,reset_"
                   "partition_exact,"
                   "measured_blocks\n";
        double observable = 0;
        for (std::size_t block : {32u, 64u, 128u, 256u, 257u, 512u, 1024u}) {
            bool equal = true;
            allocationtest::allocations = 0;
            allocationtest::observing = true;
            processor.reset();
            for (std::size_t offset = 0; offset < frames.size(); offset += block)
                for (std::size_t i = offset; i < std::min(offset + block, frames.size()); ++i)
                    equal &= processor.process(frames[i].audio, frames[i].delay) == reference[i];
            allocationtest::observing = false;
            require(equal && allocationtest::allocations == 0,
                    "reset/partition/allocation regression");
            double sum = 0, peak = 0;
            std::array<double, kMeasuredBlocks> timings{};
            // Warm and measured loops use the actual source/path fixture. No I/O or
            // clock call occurs inside the measured sample loop. This is candidate-
            // only CPU evidence, not a plugin callback or formal performance budget.
            std::size_t position = frames.size() / 4;
            for (int iteration = -warmup; iteration < repetitions; ++iteration) {
                const auto begin = std::chrono::steady_clock::now();
                for (std::size_t n = 0; n < block; ++n) {
                    const auto& f = frames[position];
                    observable += processor.process(f.audio, f.delay)[0];
                    position = (position + 1) % frames.size();
                }
                const double us = std::chrono::duration<double, std::micro>(
                                      std::chrono::steady_clock::now() - begin)
                                      .count();
                if (iteration >= 0) {
                    timings[static_cast<std::size_t>(iteration)] = us;
                    sum += us;
                    peak = std::max(peak, us);
                }
            }
            std::sort(timings.begin(), timings.begin() + repetitions);
            const auto p95 = static_cast<std::size_t>(std::ceil(.95 * repetitions)) - 1;
            const auto p99 = static_cast<std::size_t>(std::ceil(.99 * repetitions)) - 1;
            metrics << block << ',' << sum / repetitions << ',' << timings[p95] << ','
                    << timings[p99] << ',' << peak << ',' << processor.stateBytes() << ','
                    << allocationtest::allocations << ",true," << repetitions << '\n';
        }
        metrics.flush();
        require(metrics.good() && std::isfinite(observable), "benchmark failed");
        require(processor.prepare(saved), "reprepare failed");
        bool repeat = true;
        for (std::size_t i = 0; i < frames.size(); ++i)
            repeat &= processor.process(frames[i].audio, frames[i].delay) == reference[i];
        require(repeat, "reprepare changed output");
        return 0;
    } catch (const std::exception& error) {
        allocationtest::observing = false;
        std::cerr << error.what() << '\n';
        return 1;
    }
}
