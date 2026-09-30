#pragma once
#include "dsp/BubbleA1.h"

#include <fstream>
#include <iomanip>
#include <juce_audio_formats/juce_audio_formats.h>
#include <memory>
#include <vector>

namespace frazil::water::research {
// Offline ablation only: runtime A1 keeps 128 bins. The 128 case is checked sample-exact
// against the real processor; 512 changes only population discretization. No production link.
inline int a1BinningStudy(const juce::File& input, const juce::File& output,
                          const juce::File& events, int binCount, double radiusMin, double gamma) {
    if ((binCount != 128 && binCount != 512) || output.exists() || events.exists())
        return 2;
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(
        format.createReaderFor(input.createInputStream().release(), true));
    if (!reader || reader->numChannels != 2 || reader->lengthInSamples > 120 * reader->sampleRate)
        return 2;
    const double rate = reader->sampleRate;
    BubbleA1Config c;
    c.radiusMinMm = radiusMin;
    c.depthAmplitudeGamma = gamma;
    BubbleA1Model authority;
    if (!authority.prepare(rate, c))
        return 2;
    std::vector<BubbleA1Bin> bins(static_cast<std::size_t>(binCount));
    double sum{}, moment{};
    for (int i = 0; i < binCount; ++i) {
        auto& b = bins[i];
        const double mm = i == binCount - 1
                              ? c.radiusMaxMm
                              : c.radiusMinMm * std::pow(c.radiusMaxMm / c.radiusMinMm,
                                                         double(i) / (binCount - 1));
        b.radiusMeters = mm * .001;
        b.frequencyHz = BubbleA1Model::frequency(b.radiusMeters);
        b.dampingPerSecond = BubbleA1Model::damping(b.radiusMeters);
        b.tauSeconds = c.persistenceScale / b.dampingPerSecond;
        b.poleRadius = std::exp(-1 / (b.tauSeconds * rate));
        b.probability =
            b.frequencyHz <= .45 * rate ? std::pow(mm / c.radiusMinMm, -c.populationGamma) : 0;
        b.amplitude = std::pow(mm / c.radiusMinMm, c.amplitudeRadiusExponent);
        sum += b.probability;
        moment += b.probability * b.amplitude * b.amplitude;
    }
    double cumulative{};
    const double normalization = std::sqrt(moment / sum);
    for (auto& b : bins) {
        b.probability /= sum;
        b.cdf = cumulative += b.probability;
        b.amplitude /= normalization;
    }
    bins.back().cdf = 1;
    auto reference = std::make_unique<BubbleA1>();
    auto pool = std::make_unique<BubbleA1VoicePool>();
    SharedExcitationAnalyzer analyzer;
    const ResearchConfig research{rate, 42};
    if (!reference->prepare(research, c) || !pool->prepare(rate, c) || !analyzer.prepare(rate))
        return 2;
    RandomSource random;
    random.reseed(research.seedFor(RandomDomain::bubbleA1));
    std::ofstream trace(events.getFullPathName().toStdString());
    trace << std::setprecision(17)
          << "frame,bin,radius_mm,frequency_hz,depth,amplitude_l,amplitude_r\n";
    std::unique_ptr<juce::OutputStream> stream = output.createOutputStream();
    auto writer = format.createWriterFor(
        stream, juce::AudioFormatWriterOptions{}
                    .withSampleRate(rate)
                    .withNumChannels(2)
                    .withBitsPerSample(32)
                    .withSampleFormat(juce::AudioFormatWriterOptions::SampleFormat::floatingPoint));
    if (!writer || !trace)
        return 1;
    constexpr int block = 256;
    juce::AudioBuffer<float> buffer(2, block);
    const auto total = reader->lengthInSamples + static_cast<juce::int64>(rate * 3);
    for (juce::int64 frame = 0; frame < total;) {
        const auto count = static_cast<int>(std::min<juce::int64>(block, total - frame));
        buffer.clear();
        const auto sourceCount = static_cast<int>(std::min<juce::int64>(
            count, std::max<juce::int64>(0, reader->lengthInSamples - frame)));
        if (sourceCount && !reader->read(&buffer, 0, sourceCount, frame, true, true))
            return 1;
        for (int n = 0; n < count; ++n, ++frame) {
            const StereoFrame source{buffer.getSample(0, n), buffer.getSample(1, n)};
            const auto feature = analyzer.process(source);
            const double probability =
                -std::expm1(-c.maxEventRateHz * c.motionFactor * feature.activity / rate);
            if (double(random.nextUnipolar()) < probability) {
                const double draw = random.nextUnipolar();
                const auto found =
                    std::upper_bound(bins.begin(), bins.end(), draw,
                                     [](double u, const BubbleA1Bin& bin) { return u < bin.cdf; });
                const auto bin = std::min<std::size_t>(found - bins.begin(), bins.size() - 1);
                BubbleA1Event e;
                e.physics = bins[bin];
                e.bin =
                    bin * 128 / bins.size(); // Existing pool histogram only; never used for sound.
                e.depthExcitationProxy = std::pow(double(random.nextUnipolar()), c.depthExponent);
                e.riseXi = e.depthExcitationProxy > c.riseCutoff ? c.riseXi : 0;
                e.riseModel = c.riseModel;
                e.amplitude = analyzer.eventCarrier(c.sourceEnergyAmplitude);
                e.lifecycleAmplitude = e.amplitude;
                for (auto& a : e.lifecycleAmplitude)
                    a *= e.physics.amplitude * e.depthExcitationProxy * c.residualGain;
                e.separateAmplitudeRole = gamma != 1;
                const double depth =
                    gamma == 1 ? e.depthExcitationProxy : std::pow(e.depthExcitationProxy, gamma);
                for (auto& a : e.amplitude)
                    a *= e.physics.amplitude * depth * c.residualGain;
                pool->trigger(e);
                trace << frame << ',' << bin << ',' << e.physics.radiusMeters * 1000 << ','
                      << e.physics.frequencyHz << ',' << e.depthExcitationProxy << ','
                      << e.amplitude[0] << ',' << e.amplitude[1] << '\n';
            }
            const auto value = pool->process();
            if (binCount == 128 && value != reference->process(source))
                return 3;
            for (int ch = 0; ch < 2; ++ch)
                buffer.setSample(ch, n, value[ch]);
        }
        if (!writer->writeFromAudioSampleBuffer(buffer, 0, count))
            return 1;
    }
    return pool->active() == 0 && trace.good() ? 0 : 4;
}
} // namespace frazil::water::research
