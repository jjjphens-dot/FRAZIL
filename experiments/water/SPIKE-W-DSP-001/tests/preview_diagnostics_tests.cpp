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
        return ProtectBlockReadout{data, data};
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
            previous = value.fast;
        }
        if (done && snapshot.blocks == 0)
            break;
        std::this_thread::yield();
    }
    producer.join();
    check(consumed + dropped == total, "every attempted summary is consumed or explicitly dropped");
    std::cout << "Protect diagnostics failures=" << failures << '\n';
    return failures;
}
