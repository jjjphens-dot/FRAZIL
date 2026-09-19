#include "app/AudioEngine.h"
#include "dsp/FlowModulator.h"
#include "dsp/FluidCandidate.h"
#include "dsp/FluidProtect.h"
#include "dsp/LiquidModalResonator.h"
#include "preview/ResearchWaterMacroMapper.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
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

int main(int argc, char** argv) {
    const bool activityStudy = argc == 2 && std::string_view(argv[1]) == "--activity-study";
    const bool motionStudy = argc == 2 && std::string_view(argv[1]) == "--motion-study";
    const bool normalized = argc == 2 && std::string_view(argv[1]) == "--normalization-study";
    const bool excitationStudy = argc == 2 && std::string_view(argv[1]) == "--excitation-study";
    if (argc > 1 && !excitationStudy && !normalized && !motionStudy && !activityStudy)
        return 2;
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
    if (activityStudy) {
        using namespace frazil::water::preview;
        for (double motion : {0., .25, .5, 1.})
            for (double decay : {0., .5, 1.}) {
                WaterExperimentState state;
                state.motion = motion;
                state.decay = decay;
                const auto targets = ResearchWaterMacroMapper::map(state)->fluid;
                DropletConfig config;
                config.decaySeconds = targets.dropletDecaySeconds;
                config.transientThreshold = targets.dropletThreshold;
                config.refractorySeconds = targets.dropletRefractorySeconds;
                config.eventsEnabled = targets.dropletEventsEnabled;
                config.eventActivity = *ResearchWaterMacroMapper::continuousDropletActivity(motion);
                DropletImpactExciter droplet;
                if (!droplet.prepare({kRate, 42}, config))
                    return 1;
                const auto label = "B_activity_motion_" + std::to_string(motion) + "_decay_" +
                                   std::to_string(decay);
                measure(
                    label.c_str(),
                    [&](auto& buffer) {
                        for (int i = 0; i < kBlock; ++i) {
                            const auto y =
                                droplet.process({buffer.getSample(0, i), buffer.getSample(1, i)});
                            for (int channel = 0; channel < 2; ++channel)
                                buffer.addSample(channel, i, y[channel]);
                        }
                    },
                    baseline);
            }
        return 0;
    }
    if (excitationStudy || normalized || motionStudy) {
        for (const auto name : {"raw", "hard", "softsign", "tanh", "feature"}) {
            ModalExcitation excitation;
            if (!parseModalExcitation(name, excitation))
                return 2;
            if ((normalized || motionStudy) && excitation == ModalExcitation::raw)
                continue;
            for (double motion : {0., .5, 1.})
                for (double decay : {.03, .12, .48}) {
                    const ModalConfig config{260, decay, .3, .35 * motion,
                                             .7 * std::pow(2.8, 1 - 2 * motion)};
                    LiquidModalResonator candidate;
                    if (!candidate.prepare(kRate, config, excitation,
                                           (normalized || motionStudy) ? ModalNormalization::c3
                                                                       : ModalNormalization::c0,
                                           motionStudy ? ModalMotionModel::structured
                                                       : ModalMotionModel::independent))
                        return 1;
                    const auto label = std::string(motionStudy  ? "RM1_C3_"
                                                   : normalized ? "C3_"
                                                                : "C0_") +
                                       name + "_motion_" + std::to_string(motion) + "_decay_" +
                                       std::to_string(decay);
                    measure(
                        label.c_str(),
                        [&](auto& buffer) {
                            for (int i = 0; i < kBlock; ++i) {
                                const auto y = candidate.process(
                                    {buffer.getSample(0, i), buffer.getSample(1, i)});
                                for (int channel = 0; channel < 2; ++channel)
                                    buffer.addSample(channel, i, y[channel]);
                            }
                        },
                        baseline);
                }
        }
        return 0;
    }
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
    for (double depth : {0., .175, .35}) {
        LiquidModalResonator moving;
        if (!moving.prepare(kRate, {260, .12, .3, depth, .25}))
            return 1;
        const auto name = "C_motion_" + std::to_string(depth);
        measure(
            name.c_str(),
            [&](auto& buffer) {
                for (int i = 0; i < kBlock; ++i) {
                    const auto y = moving.process({buffer.getSample(0, i), buffer.getSample(1, i)});
                    for (int c = 0; c < 2; ++c)
                        buffer.addSample(c, i, y[c]);
                }
            },
            baseline);
    }
    // PROTECT-EXP-001: same-run engine baselines, including detector/envelope cost at OFF.
    // No trace/file I/O in measure; existing gated workload, warmup and percentiles apply.
    for (bool resonant : {false, true}) {
        FluidCandidate fluid;
        LiquidModalResonator resonator;
        if (!fluid.prepare({}) || !resonator.prepare(kRate))
            return 1;
        const auto render = [&](auto& buffer, ResidualProtect* protect,
                                FluidProtectTopology topology) {
            for (int i = 0; i < kBlock; ++i) {
                const StereoFrame x{buffer.getSample(0, i), buffer.getSample(1, i)};
                const double g = protect ? protect->processSource(x) : 1.0;
                const auto e = resonant
                                   ? ResidualProtect::apply(resonator.process(x), g)
                                   : applyFluidProtect(fluid.processComponents(x), g, topology);
                for (int c = 0; c < 2; ++c)
                    buffer.addSample(c, i, e[c]);
            }
        };
        const double reference = measure(
            resonant ? "protect_C_reference" : "protect_F_reference",
            [&](auto& buffer) { render(buffer, nullptr, FluidProtectTopology::whole); }, 0.0);
        for (auto score : {ProtectScore::difference, ProtectScore::logRatio})
            for (double depth : {0.0, .5, 1.0}) {
                fluid.reset();
                resonator.reset();
                ProtectConfig config;
                config.score = score;
                if (score == ProtectScore::difference) {
                    config.thresholdLow = .01;
                    config.thresholdHigh = .12;
                }
                ResidualProtect protect;
                if (!protect.prepare(kRate, config, depth))
                    return 1;
                const std::string name = std::string(resonant ? "protect_C_" : "protect_F_") +
                                         (score == ProtectScore::difference ? "D0_" : "D1_") +
                                         std::to_string(depth);
                measure(
                    name.c_str(),
                    [&](auto& buffer) { render(buffer, &protect, FluidProtectTopology::whole); },
                    reference);
            }
    }
    return 0;
}
