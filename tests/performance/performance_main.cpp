#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
// clang-format off: Windows API must precede the JUCE translation-unit headers.
#include <windows.h>
#include <psapi.h>
#include <malloc.h>
// clang-format on
#endif

#include "app/AudioEngine.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <new>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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

void operator delete(void* pointer) noexcept {
    std::free(pointer);
}
void operator delete[](void* pointer) noexcept {
    std::free(pointer);
}
void operator delete(void* pointer, std::size_t) noexcept {
    std::free(pointer);
}
void operator delete[](void* pointer, std::size_t) noexcept {
    std::free(pointer);
}

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

void operator delete(void* pointer, const std::nothrow_t&) noexcept {
    std::free(pointer);
}
void operator delete[](void* pointer, const std::nothrow_t&) noexcept {
    std::free(pointer);
}

#if defined(_WIN32)
void* operator new(std::size_t size, std::align_val_t alignment) {
    if (auto* pointer =
            _aligned_malloc(size == 0 ? 1 : size, static_cast<std::size_t>(alignment))) {
        allocation_observer::record();
        return pointer;
    }
    throw std::bad_alloc{};
}

void* operator new[](std::size_t size, std::align_val_t alignment) {
    if (auto* pointer =
            _aligned_malloc(size == 0 ? 1 : size, static_cast<std::size_t>(alignment))) {
        allocation_observer::record();
        return pointer;
    }
    throw std::bad_alloc{};
}

void operator delete(void* pointer, std::align_val_t) noexcept {
    _aligned_free(pointer);
}
void operator delete[](void* pointer, std::align_val_t) noexcept {
    _aligned_free(pointer);
}
void operator delete(void* pointer, std::size_t, std::align_val_t) noexcept {
    _aligned_free(pointer);
}
void operator delete[](void* pointer, std::size_t, std::align_val_t) noexcept {
    _aligned_free(pointer);
}

void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    try {
        return ::operator new(size, alignment);
    } catch (...) {
        return nullptr;
    }
}

void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    try {
        return ::operator new[](size, alignment);
    } catch (...) {
        return nullptr;
    }
}

void operator delete(void* pointer, std::align_val_t, const std::nothrow_t&) noexcept {
    _aligned_free(pointer);
}
void operator delete[](void* pointer, std::align_val_t, const std::nothrow_t&) noexcept {
    _aligned_free(pointer);
}
#endif

namespace {
constexpr double kReferenceSampleRateHz = 48000.0;
constexpr int kReferenceBlockSizeSamples = 128;
constexpr int kReferenceChannels = 2;
constexpr int kWarmupBlocks = 2000;
constexpr int kMeasuredBlocks = 20000;
constexpr double kReferenceFrequencyHz = 440.0;
constexpr double kTwoPi = 6.283185307179586476925286766559;
constexpr double kReferenceAmplitude = 0.25;
constexpr double kReferenceChannelPhaseRadians = 0.25;
// Parameter-retarget workload alternates between approximately -6 dB and +6 dB.
constexpr float kRetargetLowGainLinear = 0.5011872f;
constexpr float kRetargetHighGainLinear = 1.9952623f;

struct ResourceSnapshot final {
    bool memoryAvailable{};
    std::uint64_t workingSetBytes{};
    std::uint64_t peakWorkingSetBytes{};
    bool cpuAvailable{};
    std::uint64_t cpuTime100Nanoseconds{};
};

struct BenchmarkResult final {
    std::string_view name;
    double meanMicroseconds{};
    double p95Microseconds{};
    double p99Microseconds{};
    double worstMicroseconds{};
    double deadlineMicroseconds{};
    double meanDeadlineUsagePercent{};
    double worstDeadlineUsagePercent{};
    double wallSeconds{};
    double harnessProcessCpuPercent{};
    bool finiteOutput{};
    std::uint64_t allocationCount{};
    ResourceSnapshot beforeResources{};
    ResourceSnapshot afterResources{};
};

struct DenormalResult final {
    bool prepared{};
    std::size_t inputSubnormalSamples{};
    std::size_t outputSubnormalSamples{};
    std::size_t outputNonFiniteSamples{};
};

std::string configuredCommit() {
#if defined(FRAZIL_GIT_COMMIT)
    return FRAZIL_GIT_COMMIT;
#else
    return "unknown";
#endif
}

std::string configuredSourceState() {
#if defined(FRAZIL_GIT_STATE)
    return FRAZIL_GIT_STATE;
#else
    return "unknown";
#endif
}

std::string configuredSourceDirectory() {
#if defined(FRAZIL_SOURCE_DIR)
    return FRAZIL_SOURCE_DIR;
#else
    return "unknown";
#endif
}

std::optional<std::string> runGitCommandInSourceTree(std::string_view sourceDirectory,
                                                     std::string_view command) {
    if (sourceDirectory == "unknown")
        return std::nullopt;

#if defined(_WIN32)
    constexpr std::string_view nullDevice = "NUL";
#else
    constexpr std::string_view nullDevice = "/dev/null";
#endif

    const auto commandWithRedirect = "git -C \"" + std::string(sourceDirectory) + "\" " +
                                     std::string(command) + " 2>" + std::string(nullDevice);
#if defined(_WIN32)
    auto* pipe = _popen(commandWithRedirect.c_str(), "r");
#else
    auto* pipe = popen(commandWithRedirect.c_str(), "r");
#endif
    if (pipe == nullptr)
        return std::nullopt;

    std::string output;
    std::array<char, 128> buffer{};
    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
        output += buffer.data();

#if defined(_WIN32)
    const auto exitCode = _pclose(pipe);
#else
    const auto exitCode = pclose(pipe);
#endif
    if (exitCode != 0)
        return std::nullopt;

    while (!output.empty() && (output.back() == '\r' || output.back() == '\n'))
        output.pop_back();
    return output;
}

struct RuntimeProvenance final {
    std::string commit{"unknown"};
    std::string sourceState{"unknown"};
};

RuntimeProvenance captureRuntimeProvenance(std::string_view sourceDirectory) {
    const auto commit = runGitCommandInSourceTree(sourceDirectory, "rev-parse HEAD");
    const auto status =
        runGitCommandInSourceTree(sourceDirectory, "status --porcelain --untracked-files=normal");
    if (!commit.has_value() || !status.has_value())
        return {};

    return {commit.value(), status->empty() ? "clean" : "dirty"};
}

bool hasFormalProvenance(std::string_view configuredCommitValue,
                         std::string_view configuredSourceStateValue,
                         const RuntimeProvenance& runtime) noexcept {
    return configuredCommitValue != "unknown" && runtime.commit != "unknown" &&
           configuredCommitValue == runtime.commit && configuredSourceStateValue == "clean" &&
           runtime.sourceState == "clean";
}

std::string buildType() {
#if defined(FRAZIL_BUILD_TYPE)
    return FRAZIL_BUILD_TYPE;
#elif defined(_DEBUG)
    return "Debug";
#else
    return "unknown";
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

std::string compilerFlags() {
#if defined(FRAZIL_COMPILER_FLAGS)
    return FRAZIL_COMPILER_FLAGS;
#else
    return "unknown";
#endif
}

#if defined(_WIN32)
std::uint64_t fileTimeTo100Nanoseconds(const FILETIME& fileTime) noexcept {
    ULARGE_INTEGER value{};
    value.LowPart = fileTime.dwLowDateTime;
    value.HighPart = fileTime.dwHighDateTime;
    return value.QuadPart;
}
#endif

ResourceSnapshot captureResources() noexcept {
    ResourceSnapshot snapshot{};

#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX memory{};
    memory.cb = sizeof(memory);
    if (GetProcessMemoryInfo(GetCurrentProcess(),
                             reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory),
                             sizeof(memory)) != FALSE) {
        snapshot.memoryAvailable = true;
        snapshot.workingSetBytes = static_cast<std::uint64_t>(memory.WorkingSetSize);
        snapshot.peakWorkingSetBytes = static_cast<std::uint64_t>(memory.PeakWorkingSetSize);
    }

    FILETIME creationTime{};
    FILETIME exitTime{};
    FILETIME kernelTime{};
    FILETIME userTime{};
    if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime) !=
        FALSE) {
        snapshot.cpuAvailable = true;
        snapshot.cpuTime100Nanoseconds =
            fileTimeTo100Nanoseconds(kernelTime) + fileTimeTo100Nanoseconds(userTime);
    }
#endif

    return snapshot;
}

class ReferenceOscillator final {
  public:
    ReferenceOscillator(double sampleRateHz, double frequencyHz) noexcept
        : phaseIncrement_(kTwoPi * frequencyHz / sampleRateHz) {}

    void fill(juce::AudioBuffer<float>& buffer) noexcept {
        auto* const* channelData = buffer.getArrayOfWritePointers();
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                const auto channelPhase =
                    static_cast<double>(channel) * kReferenceChannelPhaseRadians;
                channelData[channel][sample] =
                    static_cast<float>(kReferenceAmplitude * std::sin(phase_ + channelPhase));
            }

            phase_ += phaseIncrement_;
            if (phase_ >= kTwoPi)
                phase_ -= kTwoPi;
        }
    }

  private:
    double phase_{};
    double phaseIncrement_{};
};

bool hasFiniteOutput(const juce::AudioBuffer<float>& buffer) noexcept {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            if (!std::isfinite(buffer.getSample(channel, sample)))
                return false;
    return true;
}

std::size_t countSubnormalSamples(const juce::AudioBuffer<float>& buffer) noexcept {
    std::size_t count{};
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            if (std::fpclassify(buffer.getSample(channel, sample)) == FP_SUBNORMAL)
                ++count;
    return count;
}

std::size_t countNonFiniteSamples(const juce::AudioBuffer<float>& buffer) noexcept {
    std::size_t count{};
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            if (!std::isfinite(buffer.getSample(channel, sample)))
                ++count;
    return count;
}

void applyScenarioParameters(EngineParameters& parameters, int block, bool retarget) noexcept {
    if (!retarget)
        return;

    const auto firstTarget = (block % 2) == 0;
    parameters.inputGainLinear = firstTarget ? kRetargetLowGainLinear : kRetargetHighGainLinear;
    parameters.globalMix = firstTarget ? 0.0f : 1.0f;
    parameters.outputGainLinear = firstTarget ? kRetargetHighGainLinear : kRetargetLowGainLinear;
}

double percentileSorted(const std::vector<double>& sortedSamples, double fraction) {
    const auto rank = static_cast<std::size_t>(std::ceil(fraction * sortedSamples.size()));
    const auto rankIndex = rank == 0 ? std::size_t{} : rank - 1;
    const auto index = rankIndex < sortedSamples.size() ? rankIndex : sortedSamples.size() - 1;
    return sortedSamples[index];
}

BenchmarkResult measureScenario(std::string_view name, bool retarget) {
    constexpr ProcessSpec spec{kReferenceSampleRateHz, kReferenceBlockSizeSamples,
                               kReferenceChannels};
    AudioEngine engine;
    if (!engine.prepare(spec))
        return BenchmarkResult{name};

    juce::AudioBuffer<float> work(kReferenceChannels, kReferenceBlockSizeSamples);
    ReferenceOscillator reference(kReferenceSampleRateHz, kReferenceFrequencyHz);

    EngineParameters parameters{};
    for (int block = 0; block < kWarmupBlocks; ++block) {
        applyScenarioParameters(parameters, block, retarget);
        reference.fill(work);
        engine.process(work, parameters);
    }

    std::vector<double> callbackMicroseconds;
    callbackMicroseconds.reserve(static_cast<std::size_t>(kMeasuredBlocks));
    const auto resourcesBefore = captureResources();
    const auto wallStart = std::chrono::steady_clock::now();
    allocation_observer::count.store(0, std::memory_order_relaxed);
    allocation_observer::enabled.store(true, std::memory_order_relaxed);
    bool finiteOutput = true;

    // This process-wide window includes reference generation and benchmark bookkeeping around
    // the separately timed AudioEngine::process call; it is not a callback-only CPU metric.
    for (int block = 0; block < kMeasuredBlocks; ++block) {
        applyScenarioParameters(parameters, block, retarget);
        reference.fill(work);
        const auto callbackStart = std::chrono::steady_clock::now();
        engine.process(work, parameters);
        const auto callbackEnd = std::chrono::steady_clock::now();
        callbackMicroseconds.push_back(
            std::chrono::duration<double, std::micro>(callbackEnd - callbackStart).count());
        finiteOutput = finiteOutput && hasFiniteOutput(work);
    }

    allocation_observer::enabled.store(false, std::memory_order_relaxed);
    const auto allocationCount = allocation_observer::count.load(std::memory_order_relaxed);
    const auto wallEnd = std::chrono::steady_clock::now();
    const auto resourcesAfter = captureResources();
    const auto wallSeconds = std::chrono::duration<double>(wallEnd - wallStart).count();
    std::sort(callbackMicroseconds.begin(), callbackMicroseconds.end());
    const auto mean =
        std::accumulate(callbackMicroseconds.begin(), callbackMicroseconds.end(), 0.0) /
        static_cast<double>(callbackMicroseconds.size());
    const auto p95 = percentileSorted(callbackMicroseconds, 0.95);
    const auto p99 = percentileSorted(callbackMicroseconds, 0.99);
    const auto worst = callbackMicroseconds.back();
    const auto deadlineMicroseconds =
        static_cast<double>(kReferenceBlockSizeSamples) / kReferenceSampleRateHz * 1.0e6;

    double harnessProcessCpuPercent = 0.0;
    if (resourcesBefore.cpuAvailable && resourcesAfter.cpuAvailable && wallSeconds > 0.0) {
        const auto cpuSeconds = static_cast<double>(resourcesAfter.cpuTime100Nanoseconds -
                                                    resourcesBefore.cpuTime100Nanoseconds) /
                                1.0e7;
        harnessProcessCpuPercent = cpuSeconds / wallSeconds * 100.0;
    }

    return BenchmarkResult{name,
                           mean,
                           p95,
                           p99,
                           worst,
                           deadlineMicroseconds,
                           mean / deadlineMicroseconds * 100.0,
                           worst / deadlineMicroseconds * 100.0,
                           wallSeconds,
                           harnessProcessCpuPercent,
                           finiteOutput,
                           allocationCount,
                           resourcesBefore,
                           resourcesAfter};
}

DenormalResult runDenormalProbe() {
    constexpr ProcessSpec spec{kReferenceSampleRateHz, kReferenceBlockSizeSamples,
                               kReferenceChannels};
    AudioEngine engine;
    if (!engine.prepare(spec))
        return {};

    juce::AudioBuffer<float> buffer(kReferenceChannels, kReferenceBlockSizeSamples);
    buffer.clear();
    const auto denormalValue = std::numeric_limits<float>::denorm_min();
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, denormalValue);

    const auto inputSubnormalSamples = countSubnormalSamples(buffer);
    EngineParameters parameters{};
    engine.process(buffer, parameters);
    return {true, inputSubnormalSamples, countSubnormalSamples(buffer),
            countNonFiniteSamples(buffer)};
}

void printResourceObservation(const ResourceSnapshot& before, const ResourceSnapshot& after) {
    const auto toMiB = [](std::uint64_t bytes) {
        return static_cast<double>(bytes) / (1024.0 * 1024.0);
    };

    if (before.memoryAvailable && after.memoryAvailable) {
        std::cout << "working_set_before_mib=" << toMiB(before.workingSetBytes) << '\n'
                  << "working_set_after_mib=" << toMiB(after.workingSetBytes) << '\n'
                  << "peak_working_set_after_mib=" << toMiB(after.peakWorkingSetBytes) << '\n';
    } else {
        std::cout << "memory_observation_status=NOT RUN\n";
    }

    std::cout << "cpu_observation_status="
              << (before.cpuAvailable && after.cpuAvailable ? "PASS" : "NOT RUN") << '\n';
}

void printResult(const BenchmarkResult& result) {
    std::cout << "scenario=" << result.name << '\n'
              << "measured_blocks=" << kMeasuredBlocks << '\n'
              << "warmup_blocks=" << kWarmupBlocks << '\n'
              << "mean_callback_us=" << result.meanMicroseconds << '\n'
              << "p95_callback_us=" << result.p95Microseconds << '\n'
              << "p99_callback_us=" << result.p99Microseconds << '\n'
              << "worst_callback_us=" << result.worstMicroseconds << '\n'
              << "callback_deadline_us=" << result.deadlineMicroseconds << '\n'
              << "mean_deadline_usage_percent=" << result.meanDeadlineUsagePercent << '\n'
              << "worst_deadline_usage_percent=" << result.worstDeadlineUsagePercent << '\n'
              << "wall_seconds=" << result.wallSeconds << '\n'
              << "harness_process_cpu_percent=" << result.harnessProcessCpuPercent << '\n'
              << "allocation_observation_measured_callback_operator_new_calls="
              << result.allocationCount << '\n'
              << "finite_output_status=" << (result.finiteOutput ? "PASS" : "FAIL") << '\n';
    printResourceObservation(result.beforeResources, result.afterResources);
    std::cout << "status=" << (result.finiteOutput ? "PASS" : "FAIL") << "\n\n";
}
} // namespace

int main() {
    const auto configuredCommitValue = configuredCommit();
    const auto configuredSourceStateValue = configuredSourceState();
    const auto runtimeProvenance = captureRuntimeProvenance(configuredSourceDirectory());
    const auto formalProvenance =
        hasFormalProvenance(configuredCommitValue, configuredSourceStateValue, runtimeProvenance);

    std::cout << std::fixed << std::setprecision(3) << "FRAZIL PERF-BASE-001\n"
              << "configured_commit=" << configuredCommitValue << '\n'
              << "configured_source_state=" << configuredSourceStateValue << '\n'
              << "runtime_commit=" << runtimeProvenance.commit << '\n'
              << "runtime_source_state=" << runtimeProvenance.sourceState << '\n'
              << "formal_provenance_status=" << (formalProvenance ? "PASS" : "NOT RUN") << '\n'
              << "build_type=" << buildType() << '\n'
              << "compiler=" << compilerDescription() << '\n'
              << "compiler_flags=" << compilerFlags() << '\n'
              << "reference_daw=N/A; headless AudioEngine benchmark\n"
              << "measurement_tool=std::chrono::steady_clock around AudioEngine::process\n"
              << "thread_configuration=one benchmark process thread\n"
              << "instance_configuration=one AudioEngine instance per scenario\n"
              << "statistical_method=arithmetic mean, nearest-rank P95/P99, maximum\n"
              << "sample_rate_hz=" << kReferenceSampleRateHz << '\n'
              << "block_size_samples=" << kReferenceBlockSizeSamples << '\n'
              << "channels=" << kReferenceChannels << '\n';

    const auto steadyState = measureScenario("steady-state", false);
    const auto parameterRetarget = measureScenario("parameter-retarget", true);
    const auto denormal = runDenormalProbe();

    printResult(steadyState);
    printResult(parameterRetarget);
    std::cout << "input_subnormal_samples=" << denormal.inputSubnormalSamples << '\n'
              << "output_subnormal_samples=" << denormal.outputSubnormalSamples << '\n'
              << "output_nonfinite_samples=" << denormal.outputNonFiniteSamples << '\n'
              << "denormal_probe_status=" << (denormal.prepared ? "OBSERVED" : "NOT RUN") << '\n'
              << "denormal_finite_output_status="
              << (denormal.prepared && denormal.outputNonFiniteSamples == 0 ? "PASS" : "FAIL")
              << '\n';

    return steadyState.finiteOutput && parameterRetarget.finiteOutput && denormal.prepared &&
                   denormal.outputNonFiniteSamples == 0
               ? 0
               : 1;
}
