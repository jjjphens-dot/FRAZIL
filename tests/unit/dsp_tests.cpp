#include "dsp/DryWetMixer.h"
#include "dsp/primitives/LinearSmoother.h"
#include "dsp/primitives/RandomSource.h"
#include "test_support.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
void testLinearSmootherReachesTarget(TestContext& context) {
    LinearSmoother smoother;
    smoother.prepare(1000.0, 0.010);
    smoother.reset(0.0f);
    smoother.setTarget(1.0f);

    for (int sample = 0; sample < 9; ++sample)
        expect(context, smoother.getNextValue() < 1.0f, "smoother ramps before its endpoint");

    expectNear(context, smoother.getNextValue(), 1.0f, 1.0e-6f,
               "smoother reaches target at ramp end");
    expect(context, smoother.getRemainingSamples() == 0,
           "smoother has no remaining samples at endpoint");
}

void testLinearSmootherIsBlockSizeStable(TestContext& context) {
    constexpr std::array<int, 6> blockSizes{16, 32, 64, 128, 256, 512};
    constexpr std::array<double, 3> sampleRates{44100.0, 48000.0, 96000.0};

    for (const auto sampleRate : sampleRates) {
        for (const auto blockSize : blockSizes) {
            LinearSmoother smoother;
            smoother.prepare(sampleRate, 0.010);
            smoother.reset(0.0f);
            smoother.setTarget(1.0f);

            const auto rampSamples = static_cast<int>(std::lround(sampleRate * 0.010));
            int processedSamples = 0;
            while (processedSamples < rampSamples) {
                const auto samplesThisBlock = std::min(blockSize, rampSamples - processedSamples);
                const auto remainingBeforeRepeatedTarget = smoother.getRemainingSamples();
                smoother.setTarget(1.0f);
                expect(context, smoother.getRemainingSamples() == remainingBeforeRepeatedTarget,
                       "repeated block target does not restart an in-flight ramp");

                for (int sample = 0; sample < samplesThisBlock; ++sample) {
                    const auto value = smoother.getNextValue();
                    ++processedSamples;
                    expect(context, std::isfinite(value), "block-ramped smoother output is finite");
                    if (processedSamples < rampSamples)
                        expect(context, value < 1.0f,
                               "block-ramped smoother reaches target only at end");
                    else
                        expectNear(context, value, 1.0f, 1.0e-6f,
                                   "block-ramped smoother reaches target at sample duration");
                }
            }

            expect(context, smoother.getRemainingSamples() == 0,
                   "block-ramped smoother has no remaining samples at endpoint");
        }
    }
}

void testLinearSmootherRetargetsFromCurrentValue(TestContext& context) {
    constexpr int rampSamples = 480;
    LinearSmoother smoother;
    smoother.prepare(48000.0, 0.010);
    smoother.reset(0.0f);
    smoother.setTarget(1.0f);

    for (int sample = 0; sample < 100; ++sample)
        static_cast<void>(smoother.getNextValue());

    const auto currentBeforeRetarget = smoother.getCurrentValue();
    smoother.setTarget(0.25f);
    expectNear(context, smoother.getCurrentValue(), currentBeforeRetarget, 1.0e-6f,
               "retarget keeps the current value as its new ramp origin");
    expect(context, smoother.getRemainingSamples() == rampSamples,
           "retarget establishes a fresh full ramp");

    for (int sample = 0; sample < rampSamples; ++sample) {
        const auto value = smoother.getNextValue();
        expect(context, std::isfinite(value), "retargeted smoother output is finite");
        if (sample == rampSamples - 1)
            expectNear(context, value, 0.25f, 1.0e-6f,
                       "retargeted smoother reaches its new target");
    }
    expect(context, smoother.getRemainingSamples() == 0,
           "retargeted smoother has no remaining samples at endpoint");
}

void testDryWetMixerEndpointsAndMonotonicity(TestContext& context) {
    expectNear(context, DryWetMixer::mix(0.25f, 0.75f, 0.0f), 0.25f, 1.0e-6f,
               "dry/wet mix zero is the dry endpoint");
    expectNear(context, DryWetMixer::mix(0.25f, 0.75f, 1.0f), 0.75f, 1.0e-6f,
               "dry/wet mix one is the wet endpoint");
    expect(context, DryWetMixer::mix(0.0f, 1.0f, 0.25f) < DryWetMixer::mix(0.0f, 1.0f, 0.75f),
           "dry/wet mix is monotonic between endpoints");
}

void testRandomSourceIsDeterministicAndInstanceLocal(TestContext& context) {
    RandomSource first{42u};
    RandomSource second{42u};
    for (int index = 0; index < 8; ++index)
        expect(context, first.nextUInt() == second.nextUInt(),
               "fixed seed produces a repeatable sequence");

    first.reseed(42u);
    second.reseed(42u);
    expect(context, first.nextUInt() == second.nextUInt(),
           "reseed restores the deterministic sequence");
    expect(context,
           RandomSource::deriveInstanceSeed(42u, 0) != RandomSource::deriveInstanceSeed(42u, 1),
           "instance seed derivation decorrelates instances");

    const auto value = first.nextUnipolar();
    expect(context, value >= 0.0f && value < 1.0f,
           "random unipolar value stays in the documented range");
}

} // namespace

void runDspTests(TestContext& context) {
    testLinearSmootherReachesTarget(context);
    testLinearSmootherIsBlockSizeStable(context);
    testLinearSmootherRetargetsFromCurrentValue(context);
    testDryWetMixerEndpointsAndMonotonicity(context);
    testRandomSourceIsDeterministicAndInstanceLocal(context);
}
