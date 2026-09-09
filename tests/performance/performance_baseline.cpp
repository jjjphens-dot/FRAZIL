#include "plugin/PluginProcessor.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <new>
#include <numeric>
#include <string>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <malloc.h>
#include <windows.h>
#include <psapi.h>
#endif

namespace allocation_observer {
std::atomic<bool> enabled{false};
std::atomic<std::uint64_t> count{0};

void record() noexcept {
    if (enabled.load(std::memory_order_relaxed))
        count.fetch_add(1, std::memory_order_relaxed);
}
} // namespace allocation_observer

void* operator new(std::size_t size) {
    if (auto* pointer = std::malloc(size == 0 ? 1 : size)) {
        allocation_observer::record();
        return pointer;
    }
    throw std::bad_alloc{};
}

void* operator new[](std::size_t size) {
    if (auto* pointer = std::malloc(size == 0 ? 1 : size)) {
        allocation_observer::record();
        return pointer;
    }
    throw std::bad_alloc{};
}

void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete[](void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { std::free(pointer); }

void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
    try {
        return ::operator new(size);
    } catch (...) {
        return nullptr;
    }
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
    try {
        return ::operator new[](size);
    } catch (...) {
        return nullptr;
    }
}

void operator delete(void* pointer, const std::nothrow_t&) noexcept { std::free(pointer); }
void operator delete[](void* pointer, const std::nothrow_t&) noexcept { std::free(pointer); }

#if defined(_WIN32)
void* operator new(std::size_t size, std::align_val_t alignment) {
    if (auto* pointer = _aligned_malloc(size == 0 ? 1 : size,
                                        static_cast<std::size_t>(alignment))) {
        allocation_observer::record();
        return pointer;
    }
    throw std::bad_alloc{};
}

void* operator new[](std::size_t size, std::align_val_t alignment) {
    if (auto* pointer = _aligned_malloc(size == 0 ? 1 : size,
                                        static_cast<std::size_t>(alignment))) {
        allocation_observer::record();
        return pointer;
    }
    throw std::bad_alloc{};
}

void operator delete(void* pointer, std::align_val_t) noexcept { _aligned_free(pointer); }
void operator delete[](void* pointer, std::align_val_t) noexcept { _aligned_free(pointer); }
void operator delete(void* pointer, std::size_t, std::align_val_t) noexcept {
    _aligned_free(pointer);
}
void operator delete[](void* pointer, std::size_t, std::align_val_t) noexcept {
    _aligned_free(pointer);
}

void* operator new(std::size_t size,
                   std::align_val_t alignment,
                   const std::nothrow_t&) noexcept {
    try {
        return ::operator new(size, alignment);
    } catch (...) {
        return nullptr;
    }
}

void* operator new[](std::size_t size,
                     std::align_val_t alignment,
                     const std::nothrow_t&) noexcept {
    try {
        return ::operator new[](size, alignment);
    } catch (...) {
        return nullptr;
    }
}

void operator delete(void* pointer,
                     std::align_val_t,
                     const std::nothrow_t&) noexcept {
    _aligned_free(pointer);
}
void operator delete[](void* pointer,
                       std::align_val_t,
                       const std::nothrow_t&) noexcept {
    _aligned_free(pointer);
}
#endif

namespace {
constexpr int kSampleRate = 48000;
constexpr int kBlockSize = 128;
constexpr int kChannels = 2;
constexpr int kWarmupCallbacks = 256;
constexpr int kMeasuredCallbacks = 2000;

struct ProcessMemory final {
    std::uint64_t workingSetBytes{};
    std::uint64_t peakWorkingSetBytes{};
    bool available{};
};

ProcessMemory readProcessMemory() noexcept {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS counters{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) != 0)
        return {static_cast<std::uint64_t>(counters.WorkingSetSize),
                static_cast<std::uint64_t>(counters.PeakWorkingSetSize), true};
#endif
    return {};
}

std::string escapeJson(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const auto character : value) {
        if (character == '\\' || character == '"')
            escaped.push_back('\\');
        escaped.push_back(character);
    }
    return escaped;
}

std::string buildType() {
#if defined(FRAZIL_BUILD_TYPE)
    return FRAZIL_BUILD_TYPE;
#elif defined(_DEBUG)
    return "Debug";
#else
    return "Unknown";
#endif
}

std::string compilerDescription() {
#if defined(_MSC_VER)
    return "MSVC _MSC_VER=" + std::to_string(_MSC_VER);
#elif defined(__clang__)
    return "Clang " + std::string(__clang_version__);
#elif defined(__GNUC__)
    return "GCC " + std::string(__VERSION__);
#else
    return "unknown compiler";
#endif
}

void fillDeterministicInput(juce::AudioBuffer<float>& buffer, std::uint32_t& state) {
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        state = state * 1664525U + 1013904223U;
        const auto normalized = static_cast<float>(state) /
                                static_cast<float>(std::numeric_limits<std::uint32_t>::max());
        const auto value = normalized * 2.0f - 1.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
    }
}

std::size_t percentileIndex(std::size_t sampleCount, double percentile) {
    const auto rank = static_cast<std::size_t>(std::ceil(percentile * sampleCount));
    return std::max<std::size_t>(1, rank) - 1;
}

bool parseOutputPath(int argc, char** argv, std::filesystem::path& outputPath) {
    if (argc == 1)
        return true;
    if (argc == 3 && std::string(argv[1]) == "--output") {
        outputPath = argv[2];
        return true;
    }
    std::cerr << "Usage: frazil_performance_baseline [--output <report.json>]\n";
    return false;
}

bool writeReport(const std::filesystem::path& outputPath,
                 const std::vector<std::uint64_t>& samples,
                 std::uint64_t allocationCount,
                 const ProcessMemory& memoryBefore,
                 const ProcessMemory& memoryAfter) {
    std::error_code error;
    if (!outputPath.parent_path().empty())
        std::filesystem::create_directories(outputPath.parent_path(), error);
    if (error) {
        std::cerr << "Cannot create performance report directory: " << error.message() << '\n';
        return false;
    }

    const auto sorted = [&] {
        auto copy = samples;
        std::sort(copy.begin(), copy.end());
        return copy;
    }();
    const auto sum = std::accumulate(samples.begin(), samples.end(), std::uint64_t{0});
    const auto mean = static_cast<double>(sum) / static_cast<double>(samples.size());
    const auto p95 = sorted[percentileIndex(sorted.size(), 0.95)];
    const auto p99 = sorted[percentileIndex(sorted.size(), 0.99)];
    const auto worst = sorted.back();
    const auto deadline = 1.0e9 * static_cast<double>(kBlockSize) / kSampleRate;

    std::ofstream report(outputPath);
    if (!report)
        return false;
    report << std::fixed << std::setprecision(3);
    report << "{\n"
           << "  \"workItem\": \"PERF-BASE-001\",\n"
           << "  \"configuredCommit\": \"" << escapeJson(FRAZIL_GIT_COMMIT)
           << "\",\n"
           << "  \"commitFieldMeaning\": \"Git HEAD captured during CMake configure; "
              "run cmake --fresh before formal baseline evidence.\",\n"
           << "  \"referenceMachine\": {\n"
           << "    \"cpuVendor\": \"" << escapeJson(juce::SystemStats::getCpuVendor().toStdString())
           << "\",\n"
           << "    \"cpuModel\": \"" << escapeJson(juce::SystemStats::getCpuModel().toStdString())
           << "\",\n"
           << "    \"logicalCpuCount\": " << juce::SystemStats::getNumCpus() << ",\n"
           << "    \"ramMiB\": " << juce::SystemStats::getMemorySizeInMegabytes() << ",\n"
           << "    \"os\": \""
           << escapeJson(juce::SystemStats::getOperatingSystemName().toStdString()) << "\",\n"
           << "    \"compiler\": \"" << escapeJson(compilerDescription()) << "\",\n"
           << "    \"buildType\": \"" << escapeJson(buildType()) << "\",\n"
           << "    \"juceVersion\": \""
           << escapeJson(juce::SystemStats::getJUCEVersion().toStdString()) << "\"\n"
           << "  },\n"
           << "  \"workload\": {\n"
           << "    \"sampleRate\": " << kSampleRate << ",\n"
           << "    \"blockSize\": " << kBlockSize << ",\n"
           << "    \"channels\": " << kChannels << ",\n"
           << "    \"warmupCallbacks\": " << kWarmupCallbacks << ",\n"
           << "    \"measuredCallbacks\": " << kMeasuredCallbacks << ",\n"
           << "    \"input\": \"deterministic LCG noise, fixed seed 0x2468ace1\"\n"
           << "  },\n"
           << "  \"timing\": {\n"
           << "    \"unit\": \"nanoseconds\",\n"
           << "    \"meanCallback\": " << mean << ",\n"
           << "    \"p95Callback\": " << p95 << ",\n"
           << "    \"p99Callback\": " << p99 << ",\n"
           << "    \"worstCallback\": " << worst << ",\n"
           << "    \"deadline\": " << deadline << ",\n"
           << "    \"meanDeadlineUtilizationPercent\": " << mean / deadline * 100.0 << ",\n"
           << "    \"worstDeadlineUtilizationPercent\": " << worst / deadline * 100.0 << "\n"
           << "  },\n"
           << "  \"memory\": {\n"
           << "    \"measurement\": \"Windows process working set\",\n"
           << "    \"beforeBytes\": " << memoryBefore.workingSetBytes << ",\n"
           << "    \"afterBytes\": " << memoryAfter.workingSetBytes << ",\n"
           << "    \"peakBytes\": " << memoryAfter.peakWorkingSetBytes << ",\n"
           << "    \"available\": " << (memoryAfter.available ? "true" : "false") << "\n"
           << "  },\n"
           << "  \"allocationObservation\": {\n"
           << "    \"mechanism\": \"test executable global operator new observer\",\n"
           << "    \"operatorNewCallsDuringMeasuredCallbacks\": " << allocationCount << "\n"
           << "  },\n"
           << "  \"interpretation\": \"Baseline only; no formal CPU percentage threshold is defined.\"\n"
           << "}\n";
    return report.good();
}
} // namespace

int main(int argc, char** argv) {
    std::filesystem::path outputPath = "perf_base_001.json";
    if (!parseOutputPath(argc, argv, outputPath))
        return 2;

    FRAZILAudioProcessor processor;
    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add(juce::AudioChannelSet::stereo());
    layout.outputBuses.add(juce::AudioChannelSet::stereo());
    if (!processor.setBusesLayout(layout)) {
        std::cerr << "PERF-BASE-001 FAIL: stereo bus layout was rejected\n";
        return 1;
    }
    processor.prepareToPlay(kSampleRate, kBlockSize);

    juce::AudioBuffer<float> buffer(kChannels, kBlockSize);
    juce::MidiBuffer midi;
    std::uint32_t inputState = 0x2468ace1U;
    for (int iteration = 0; iteration < kWarmupCallbacks; ++iteration) {
        fillDeterministicInput(buffer, inputState);
        processor.processBlock(buffer, midi);
    }

    const auto memoryBefore = readProcessMemory();
    std::vector<std::uint64_t> callbackTimes;
    callbackTimes.reserve(kMeasuredCallbacks);
    allocation_observer::count.store(0, std::memory_order_relaxed);
    allocation_observer::enabled.store(true, std::memory_order_relaxed);
    for (int iteration = 0; iteration < kMeasuredCallbacks; ++iteration) {
        fillDeterministicInput(buffer, inputState);
        const auto start = std::chrono::steady_clock::now();
        processor.processBlock(buffer, midi);
        const auto finish = std::chrono::steady_clock::now();
        callbackTimes.push_back(static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count()));
    }
    allocation_observer::enabled.store(false, std::memory_order_relaxed);
    const auto memoryAfter = readProcessMemory();
    const auto allocationCount = allocation_observer::count.load(std::memory_order_relaxed);

    if (!writeReport(outputPath, callbackTimes, allocationCount, memoryBefore, memoryAfter)) {
        std::cerr << "PERF-BASE-001 FAIL: cannot write report " << outputPath.string() << '\n';
        return 1;
    }

    const auto sorted = [&] {
        auto copy = callbackTimes;
        std::sort(copy.begin(), copy.end());
        return copy;
    }();
    const auto sum = std::accumulate(callbackTimes.begin(), callbackTimes.end(), std::uint64_t{0});
    const auto mean = static_cast<double>(sum) / static_cast<double>(callbackTimes.size());
    const auto p95 = sorted[percentileIndex(sorted.size(), 0.95)];
    const auto p99 = sorted[percentileIndex(sorted.size(), 0.99)];
    const auto worst = sorted.back();
    std::cout << std::fixed << std::setprecision(3)
              << "PERF-BASE-001 baseline recorded: mean_ns=" << mean << ", p95_ns=" << p95
              << ", p99_ns=" << p99 << ", worst_ns=" << worst
              << ", deadline_ns=" << (1.0e9 * kBlockSize / kSampleRate)
              << ", operator_new_calls=" << allocationCount << ", configured_commit="
              << FRAZIL_GIT_COMMIT << ", report="
              << outputPath.string() << '\n';
    return allocationCount == 0 ? 0 : 1;
}
