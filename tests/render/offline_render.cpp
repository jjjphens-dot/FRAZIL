#include "app/AudioEngine.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace {
constexpr int kDefaultBlockSize = 128;
constexpr int kMaximumBlockSize = 8192;
constexpr std::uint32_t kDefaultSeed = 20260908;
constexpr float kMinimumGainDb = -24.0f;
constexpr float kMaximumGainDb = 24.0f;

struct RenderOptions final {
    std::optional<juce::File> input;
    std::optional<juce::File> output;
    int blockSize{kDefaultBlockSize};
    std::uint32_t seed{kDefaultSeed};
    bool waterEnabled{true};
    bool iceEnabled{true};
    RoutingMode routing{RoutingMode::parallel};
    float parallelBalance{0.5f};
    float waterAmount{1.0f};
    float iceAmount{1.0f};
    float inputGainDb{};
    float globalMix{1.0f};
    float outputGainDb{};
};

enum class ParseResult {
    success,
    failure,
    help,
};

void printUsage(const char* executable) {
    std::cout << "Usage: " << executable
              << " --input <wav> --output <wav> [options]\n"
                 "Options:\n"
                 "  --block-size <samples>    Offline processing block size (default: 128)\n"
                 "  --seed <integer>          Deterministic render seed metadata\n"
                 "  --water-enabled <0|1>     Water stage enable value\n"
                 "  --ice-enabled <0|1>       Ice stage enable value\n"
                 "  --routing <mode>          parallel, water-into-ice, or ice-into-water\n"
                 "  --parallel-balance <0..1> Relative parallel routing balance\n"
                 "  --water-amount <0..1>      Water stage amount\n"
                 "  --ice-amount <0..1>        Ice stage amount\n"
                 "  --input-gain-db <dB>      Input gain in the M1 range [-24, 24]\n"
                 "  --global-mix <0..1>       Global dry/wet mix\n"
                 "  --output-gain-db <dB>     Output gain in the M1 range [-24, 24]\n"
                 "  --help                    Show this help\n";
}

template <typename Integer>
std::optional<Integer> parseInteger(std::string_view text) {
    Integer value{};
    const auto* begin = text.data();
    const auto* end = begin + text.size();
    const auto result = std::from_chars(begin, end, value);
    if (result.ec != std::errc{} || result.ptr != end)
        return std::nullopt;
    return value;
}

std::optional<float> parseFloat(std::string_view text) {
    std::string value(text);
    char* end{};
    const auto parsed = std::strtof(value.c_str(), &end);
    if (end != value.c_str() + value.size() || !std::isfinite(parsed))
        return std::nullopt;
    return parsed;
}

std::optional<bool> parseBoolean(std::string_view text) {
    if (text == "0")
        return false;
    if (text == "1")
        return true;
    return std::nullopt;
}

std::optional<RoutingMode> parseRoutingMode(std::string_view text) {
    if (text == "parallel")
        return RoutingMode::parallel;
    if (text == "water-into-ice")
        return RoutingMode::waterIntoIce;
    if (text == "ice-into-water")
        return RoutingMode::iceIntoWater;
    return std::nullopt;
}

std::optional<std::string_view> nextArgument(int& index, int argc, char** argv) {
    if (++index >= argc)
        return std::nullopt;
    return std::string_view(argv[index]);
}

template <typename Value, typename Parser>
std::optional<Value> parseNextArgument(int& index, int argc, char** argv, Parser parser) {
    const auto argument = nextArgument(index, argc, argv);
    if (!argument.has_value())
        return std::nullopt;
    return parser(*argument);
}

juce::File toFile(std::string_view path) {
    return juce::File(juce::String::fromUTF8(path.data(), static_cast<int>(path.size())));
}

ParseResult parseArguments(int argc, char** argv, RenderOptions& options) {
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument(argv[index]);
        if (argument == "--help") {
            printUsage(argv[0]);
            return ParseResult::help;
        }

        const auto value = [&]() { return nextArgument(index, argc, argv); };
        if (argument == "--input") {
            const auto path = value();
            if (!path.has_value()) {
                std::cerr << "Missing value for --input\n";
                return ParseResult::failure;
            }
            options.input = toFile(*path);
        } else if (argument == "--output") {
            const auto path = value();
            if (!path.has_value()) {
                std::cerr << "Missing value for --output\n";
                return ParseResult::failure;
            }
            options.output = toFile(*path);
        } else if (argument == "--block-size") {
            const auto parsed = parseNextArgument<int>(index, argc, argv, parseInteger<int>);
            if (!parsed.has_value() || *parsed < 1 || *parsed > kMaximumBlockSize) {
                std::cerr << "--block-size must be between 1 and " << kMaximumBlockSize << "\n";
                return ParseResult::failure;
            }
            options.blockSize = *parsed;
        } else if (argument == "--seed") {
            const auto parsed = parseNextArgument<std::uint32_t>(
                index, argc, argv, parseInteger<std::uint32_t>);
            if (!parsed.has_value()) {
                std::cerr << "--seed must be an unsigned integer\n";
                return ParseResult::failure;
            }
            options.seed = *parsed;
        } else if (argument == "--water-enabled") {
            const auto parsed = parseNextArgument<bool>(index, argc, argv, parseBoolean);
            if (!parsed.has_value()) {
                std::cerr << "--water-enabled must be 0 or 1\n";
                return ParseResult::failure;
            }
            options.waterEnabled = *parsed;
        } else if (argument == "--ice-enabled") {
            const auto parsed = parseNextArgument<bool>(index, argc, argv, parseBoolean);
            if (!parsed.has_value()) {
                std::cerr << "--ice-enabled must be 0 or 1\n";
                return ParseResult::failure;
            }
            options.iceEnabled = *parsed;
        } else if (argument == "--routing") {
            const auto parsed = parseNextArgument<RoutingMode>(index, argc, argv, parseRoutingMode);
            if (!parsed.has_value()) {
                std::cerr << "--routing must be parallel, water-into-ice, or ice-into-water\n";
                return ParseResult::failure;
            }
            options.routing = *parsed;
        } else if (argument == "--parallel-balance") {
            const auto parsed = parseNextArgument<float>(index, argc, argv, parseFloat);
            if (!parsed.has_value() || *parsed < 0.0f || *parsed > 1.0f) {
                std::cerr << "--parallel-balance must be between 0 and 1\n";
                return ParseResult::failure;
            }
            options.parallelBalance = *parsed;
        } else if (argument == "--water-amount") {
            const auto parsed = parseNextArgument<float>(index, argc, argv, parseFloat);
            if (!parsed.has_value() || *parsed < 0.0f || *parsed > 1.0f) {
                std::cerr << "--water-amount must be between 0 and 1\n";
                return ParseResult::failure;
            }
            options.waterAmount = *parsed;
        } else if (argument == "--ice-amount") {
            const auto parsed = parseNextArgument<float>(index, argc, argv, parseFloat);
            if (!parsed.has_value() || *parsed < 0.0f || *parsed > 1.0f) {
                std::cerr << "--ice-amount must be between 0 and 1\n";
                return ParseResult::failure;
            }
            options.iceAmount = *parsed;
        } else if (argument == "--input-gain-db") {
            const auto parsed = parseNextArgument<float>(index, argc, argv, parseFloat);
            if (!parsed.has_value() || *parsed < kMinimumGainDb || *parsed > kMaximumGainDb) {
                std::cerr << "--input-gain-db must be between -24 and 24\n";
                return ParseResult::failure;
            }
            options.inputGainDb = *parsed;
        } else if (argument == "--global-mix") {
            const auto parsed = parseNextArgument<float>(index, argc, argv, parseFloat);
            if (!parsed.has_value() || *parsed < 0.0f || *parsed > 1.0f) {
                std::cerr << "--global-mix must be between 0 and 1\n";
                return ParseResult::failure;
            }
            options.globalMix = *parsed;
        } else if (argument == "--output-gain-db") {
            const auto parsed = parseNextArgument<float>(index, argc, argv, parseFloat);
            if (!parsed.has_value() || *parsed < kMinimumGainDb || *parsed > kMaximumGainDb) {
                std::cerr << "--output-gain-db must be between -24 and 24\n";
                return ParseResult::failure;
            }
            options.outputGainDb = *parsed;
        } else {
            std::cerr << "Unknown argument: " << argument << "\n";
            return ParseResult::failure;
        }
    }

    if (!options.input.has_value() || !options.output.has_value()) {
        printUsage(argv[0]);
        return ParseResult::failure;
    }
    return ParseResult::success;
}

float decibelsToLinear(float decibels) noexcept {
    return std::pow(10.0f, decibels / 20.0f);
}

bool isFinite(const juce::AudioBuffer<float>& buffer, int numSamples) noexcept {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const auto* samples = buffer.getReadPointer(channel);
        for (int sample = 0; sample < numSamples; ++sample) {
            if (!std::isfinite(samples[sample]))
                return false;
        }
    }
    return true;
}

int runRender(const RenderOptions& options) {
    const auto inputFile = *options.input;
    const auto outputFile = *options.output;
    if (!inputFile.existsAsFile()) {
        std::cerr << "Input WAV does not exist: " << inputFile.getFullPathName() << "\n";
        return 1;
    }
    if (inputFile.getFullPathName() == outputFile.getFullPathName()) {
        std::cerr << "Input and output WAV paths must differ\n";
        return 1;
    }
    if (outputFile.getParentDirectory().createDirectory().failed()) {
        std::cerr << "Cannot create output directory: "
                  << outputFile.getParentDirectory().getFullPathName() << "\n";
        return 1;
    }

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(inputFile));
    if (reader == nullptr || reader->lengthInSamples <= 0 || reader->numChannels == 0
        || !std::isfinite(reader->sampleRate) || reader->sampleRate <= 0.0) {
        std::cerr << "Input is not a valid non-empty audio file: "
                  << inputFile.getFullPathName() << "\n";
        return 1;
    }

    const auto numChannels = static_cast<int>(reader->numChannels);
    const auto spec = ProcessSpec{reader->sampleRate, options.blockSize, numChannels};
    AudioEngine engine;
    if (!engine.prepare(spec)) {
        std::cerr << "AudioEngine rejected the input ProcessSpec\n";
        return 1;
    }

    const auto inputGainLinear = decibelsToLinear(options.inputGainDb);
    const auto outputGainLinear = decibelsToLinear(options.outputGainDb);
    EngineParameters parameters;
    parameters.waterEnabled = options.waterEnabled;
    parameters.iceEnabled = options.iceEnabled;
    parameters.routing = options.routing;
    parameters.parallelBalance = options.parallelBalance;
    parameters.waterStageAmount = options.waterAmount;
    parameters.iceStageAmount = options.iceAmount;
    parameters.inputGainLinear = inputGainLinear;
    parameters.globalMix = options.globalMix;
    parameters.outputGainLinear = outputGainLinear;

    auto outputStream = std::unique_ptr<juce::OutputStream>(outputFile.createOutputStream());
    if (outputStream == nullptr) {
        std::cerr << "Cannot open output WAV: " << outputFile.getFullPathName() << "\n";
        return 1;
    }
    juce::WavAudioFormat wavFormat;
    const auto writerOptions = juce::AudioFormatWriterOptions{}
                                   .withSampleRate(reader->sampleRate)
                                   .withNumChannels(numChannels)
                                   .withBitsPerSample(32)
                                   .withSampleFormat(
                                       juce::AudioFormatWriterOptions::SampleFormat::floatingPoint);
    auto writer = wavFormat.createWriterFor(outputStream, writerOptions);
    if (writer == nullptr) {
        std::cerr << "Cannot create output WAV writer\n";
        return 1;
    }

    juce::AudioBuffer<float> block(numChannels, options.blockSize);
    for (juce::int64 position = 0; position < reader->lengthInSamples;
         position += options.blockSize) {
        const auto samplesRemaining = reader->lengthInSamples - position;
        const auto samplesThisBlock = static_cast<int>(
            std::min<juce::int64>(samplesRemaining, static_cast<juce::int64>(options.blockSize)));
        block.clear();
        if (!reader->read(&block, 0, samplesThisBlock, position, true, true)) {
            std::cerr << "Failed to read input WAV at sample " << position << "\n";
            return 1;
        }
        engine.process(block, parameters);
        if (!isFinite(block, samplesThisBlock)) {
            std::cerr << "Offline render produced a non-finite sample at block " << position
                      << "\n";
            return 1;
        }
        if (!writer->writeFromAudioSampleBuffer(block, 0, samplesThisBlock)) {
            std::cerr << "Failed to write output WAV at sample " << position << "\n";
            return 1;
        }
    }
    if (!writer->flush()) {
        std::cerr << "Failed to flush output WAV\n";
        return 1;
    }

    std::cout << "Rendered " << reader->lengthInSamples << " samples at " << reader->sampleRate
              << " Hz with block size " << options.blockSize << " and seed " << options.seed
              << "\n";
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    RenderOptions options;
    const auto parseResult = parseArguments(argc, argv, options);
    if (parseResult == ParseResult::help)
        return 0;
    if (parseResult == ParseResult::failure)
        return 2;
    return runRender(options);
}
