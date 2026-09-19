#include "preview/PreviewEngine.h"
#include "preview/ProtectDiagnostics.h"

#include <atomic>
#include <iostream>
#include <thread>

int runProtectDiagnosticsTests() {
    using namespace frazil::water::preview;
    int failures{};
    const auto check = [&](bool ok, const char* message) {
        if (!ok) {
            if (failures < 20)
                std::cerr << "FAIL Protect diagnostics: " << message << '\n';
            ++failures;
        }
    };
    const auto block = [](double value) {
        const ProtectReadout data{value, value * 2, value * 3, value * 4, value * 5};
        ProtectBlockReadout result{data, data};
        result.water.latest.bubbleEvents = static_cast<std::uint64_t>(value);
        result.water.latest.dropletEvents = static_cast<std::uint64_t>(value * 2);
        WaterFrameReadout frame;
        frame.at(WaterSignal::bubble) = {static_cast<float>(value), 0};
        result.water.include(frame, 2);
        return result;
    };
    ProtectDiagnostics queue;
    check(queue.snapshot().blocks == 0, "empty initial snapshot");
    for (std::size_t i = 0; i < ProtectDiagnostics::kCapacity; ++i)
        check(queue.publish(block(static_cast<double>(i + 1))), "bounded queue accepts capacity");
    check(!queue.publish(block(999)), "full queue drops without overwrite");
    const auto full = queue.snapshot();
    check(full.blocks == ProtectDiagnostics::kCapacity && full.latest.fast == 256 &&
              full.peak.reductionDb == 1280 && full.droppedBlocks == 1,
          "coherent batch and explicit loss");
    const auto empty = queue.snapshot();
    check(empty.blocks == 0 && empty.latest.fast == 256 && empty.peak.fast == 0,
          "no fabricated new peak when no blocks arrive");
    queue.clear();
    const auto cleared = queue.snapshot();
    check(cleared.latest.fast == 0 && cleared.blocks == 0 && cleared.droppedBlocks == 0,
          "detached lifecycle clear");
    constexpr int total = 50000;
    std::atomic<bool> finished{};
    std::jthread producer([&] {
        for (int i = 1; i <= total; ++i)
            queue.publish(block(i));
        finished.store(true, std::memory_order_release);
    });
    std::size_t consumed{};
    double previous{};
    std::uint64_t dropped{};
    for (;;) {
        const bool done = finished.load(std::memory_order_acquire);
        const auto snapshot = queue.snapshot();
        consumed += snapshot.blocks;
        dropped = snapshot.droppedBlocks;
        check(snapshot.blocks <= ProtectDiagnostics::kCapacity, "bounded reader work");
        if (snapshot.blocks) {
            const auto& value = snapshot.latest;
            check(value.fast >= previous && value.slow == value.fast * 2 &&
                      value.difference == value.fast * 3 && value.logRatioDb == value.fast * 4 &&
                      value.reductionDb == value.fast * 5,
                  "real concurrent reader never observes torn payload");
            check(snapshot.peak.slow == snapshot.peak.fast * 2 &&
                      snapshot.peak.reductionDb == snapshot.peak.fast * 5,
                  "aggregate peaks remain coherent");
            check(snapshot.water.latest.bubbleEvents == static_cast<std::uint64_t>(value.fast) &&
                      snapshot.water.latest.dropletEvents ==
                          static_cast<std::uint64_t>(value.fast * 2) &&
                      snapshot.water.levels[1].samples == snapshot.blocks * 2,
                  "Water payload shares coherent bounded transport");
            previous = value.fast;
        }
        if (done && snapshot.blocks == 0)
            break;
        std::this_thread::yield();
    }
    producer.join();
    check(consumed + dropped == total, "every attempted summary is consumed or explicitly dropped");
    WaterDiagnostics first, second;
    WaterFrameReadout frame;
    frame.at(WaterSignal::totalPreProtect) = {1, 0};
    first.include(frame, 1);
    frame.at(WaterSignal::totalPreProtect) = {0, 0};
    for (int i = 0; i < 3; ++i)
        second.include(frame, 1);
    first.merge(second);
    check(first.levels[0].peak == 1 && first.levels[0].rms() == .5 && first.levels[0].samples == 4,
          "RMS sums energy and sample counts, never averages block RMS");
    for (int mode : {0, 1, 4, 8}) {
        PreviewSettings settings;
        settings.mode = mode;
        PreviewEngine engine;
        check(engine.prepare(48000, settings), "diagnostics engine prepare");
        WaterDiagnostics measured;
        for (int i = 0; i < 12003; ++i) {
            const float x = i % 1000 < 500 ? .7f : 0.f;
            const auto e = engine.residual({x, 0});
            const auto& current = engine.waterReadout();
            check(current.signals[5] == e && current.signals[0] == e,
                  "OFF total/pre/post Protect share exact residual");
            measured.include(current, 1);
        }
        measured.latest = engine.waterActivity();
        if (mode == 0)
            check(measured.latest.bubbleEvents > 0 && measured.latest.dropletEvents > 0 &&
                      measured.latest.bubbleActive <= 16 && measured.latest.dropletActive <= 8 &&
                      measured.levels[1].rms() > 0 && measured.levels[2].rms() > 0,
                  "active Fluid events/voices/component levels available");
        if (mode == 1)
            check(measured.latest.modalRootHz == 260 && measured.latest.modalDecaySeconds == .12 &&
                      measured.levels[4].rms() > 0 && measured.latest.bubbleEvents == 0,
                  "Modal diagnostics are explicit and inactive Fluid counters zero");
        if (mode == 4)
            check(measured.latest.bubbleEvents == 0 && measured.latest.flowDelayMs > 0,
                  "ablation exposes delay without fabricated events");
        if (mode == 8)
            check(measured.levels[0].peak == 0 && measured.latest.flowDelayMs == 0,
                  "baseline has zero Water diagnostics");
        engine.reset();
        check(engine.waterReadout().signals[0] == frazil::water::research::StereoFrame{} &&
                  engine.waterActivity().bubbleEvents == 0,
              "reset clears diagnostics");
        engine.residual({1, 0});
        check(!engine.prepare(0, settings) &&
                  engine.waterReadout().signals[0] == frazil::water::research::StereoFrame{},
              "failed prepare cannot retain old diagnostic frames");
    }
    std::cout << "Protect diagnostics failures=" << failures << '\n';
    return failures;
}
