#include "preview/PreviewEngine.h"
#include "preview/ResearchSessionModel.h"
#include "preview/SessionCodec.h"

#include <iostream>

int runPreviewProtectTests() {
    using namespace frazil::water;
    using namespace preview;
    int failures{};
    const auto check = [&](bool ok, const char* message) {
        if (!ok) {
            if (failures < 20)
                std::cerr << "FAIL preview Protect: " << message << '\n';
            ++failures;
        }
    };
    ResearchSessionModel session;
    check(session.draft().engineering.protect.depth == 0, "research baseline OFF");
    session.setProtect(ProtectId::depth, .7, ChangeOrigin::engineeringUI);
    check(!session.dirty() && session.applied().engineering.protect.depth == .7,
          "Depth is live without Apply");
    session.setProtectEnabled(false, ChangeOrigin::soundLeadUI);
    check(session.applied().engineering.protect.depth == 0, "Enable OFF uses Depth 0");
    session.setProtectEnabled(true, ChangeOrigin::soundLeadUI);
    check(session.applied().engineering.protect.depth == .7, "Enable restores last nonzero Depth");
    session.setDetector(research::ProtectScore::difference, ChangeOrigin::engineeringUI);
    check(session.draft().engineering.protect.gain.thresholdLow == .01 &&
              session.draft().engineering.protect.gain.thresholdHigh == .12 && session.dirty(),
          "D0 baseline and Apply lifecycle");
    session.setProtect(ProtectId::low, .02, ChangeOrigin::engineeringUI);
    session.setProtect(ProtectId::high, .15, ChangeOrigin::engineeringUI);
    session.setDetector(research::ProtectScore::logRatio, ChangeOrigin::engineeringUI);
    check(session.draft().engineering.protect.gain.thresholdLow == 1 &&
              session.draft().engineering.protect.gain.thresholdHigh == 9,
          "D1 remains dB domain");
    session.setProtect(ProtectId::low, 2, ChangeOrigin::engineeringUI);
    session.setProtect(ProtectId::high, 10, ChangeOrigin::engineeringUI);
    session.setDetector(research::ProtectScore::difference, ChangeOrigin::engineeringUI);
    check(session.draft().engineering.protect.gain.thresholdLow == .02 &&
              session.draft().engineering.protect.gain.thresholdHigh == .15,
          "D0 calibration retained");
    session.setTopology(research::FluidProtectTopology::dropletExempt, ChangeOrigin::engineeringUI);
    session.setModel(WaterModel::resonant, ChangeOrigin::soundLeadUI);
    check(session.draft().engineering.protect.topology == research::FluidProtectTopology::whole &&
              !session.setTopology(research::FluidProtectTopology::dropletHalf,
                                   ChangeOrigin::engineeringUI),
          "C only accepts Whole");
    session.setModel(WaterModel::fluid, ChangeOrigin::soundLeadUI);
    check(session.draft().engineering.protect.topology ==
              research::FluidProtectTopology::dropletExempt,
          "Fluid topology retained across C");
    session.applyValidated();
    session.capture(0);
    ResearchSessionState decoded;
    check(decodeSession(encodeSession(session.applied()).toStdString(), decoded).isEmpty() &&
              decoded.protectMemory.logRatio.low == 2 &&
              decoded.protectMemory.difference.high == .15 &&
              decoded.protectMemory.lastNonzeroDepth == .7,
          "Protect memory roundtrip");
    session.reset();
    session.applyValidated();
    session.restoreValidated(*session.slot(0));
    check(sameProtect(session.applied().engineering.protect, decoded.engineering.protect),
          "A/B restores Protect");
    for (const double rate : {44100.0, 48000.0, 96000.0}) {
        for (const auto score :
             {research::ProtectScore::difference, research::ProtectScore::logRatio}) {
            for (int mode : {0, 1})
                for (int topology = 1; topology <= (mode == 1 ? 1 : 3); ++topology) {
                    PreviewSettings settings;
                    settings.mode = mode;
                    settings.protect.depth = 1;
                    settings.protect.gain.score = score;
                    settings.protect.topology =
                        static_cast<research::FluidProtectTopology>(topology);
                    if (score == research::ProtectScore::difference) {
                        settings.protect.gain.thresholdLow = .01;
                        settings.protect.gain.thresholdHigh = .12;
                    }
                    PreviewEngine engine, off;
                    auto offSettings = settings;
                    offSettings.protect.depth = 0;
                    check(engine.prepare(rate, settings) && off.prepare(rate, offSettings),
                          "prepare integrated/OFF paths");
                    research::ResidualProtect reference;
                    research::FluidCandidate fluid;
                    research::LiquidModalResonator modal;
                    check(reference.prepare(rate, settings.protect.gain, 1) &&
                              fluid.prepare({rate, 42}) && modal.prepare(rate),
                          "prepare original reference");
                    bool attenuated{};
                    const int offAt = 6000, ramp = static_cast<int>(
                                                std::ceil(rate * settings.protect.gain.offSeconds));
                    for (int i = 0; i < 16000; ++i) {
                        const research::StereoFrame input{i % 2048 < 400 ? .6f : 0.0f, 0.0f};
                        if (i == offAt) {
                            check(engine.setProtectDepth(0), "live OFF retarget");
                            reference.setDepth(0);
                        }
                        const auto gain = reference.processSource(input);
                        const auto expected =
                            mode == 1
                                ? research::ResidualProtect::apply(modal.process(input), gain)
                                : research::applyFluidProtect(fluid.processComponents(input), gain,
                                                              settings.protect.topology);
                        const auto actual = engine.residual(input), baseline = off.residual(input);
                        check(actual == expected, "output equals unchanged research DSP");
                        check(std::isfinite(actual[0]) && actual[1] == 0, "finite and isolated");
                        attenuated |= actual != baseline;
                        if (i >= offAt + ramp)
                            check(actual == baseline, "exact OFF and generator/RNG continuation");
                    }
                    check(attenuated, "Depth modifies actual residual");
                    engine.reset();
                    off.reset();
                    for (int i = 0; i < 1000; ++i)
                        check(engine.residual({.3f, 0}) == off.residual({.3f, 0}),
                              "reset retains OFF target");
                }
        }
    }
    PreviewSettings invalid;
    PreviewEngine engine;
    invalid.mode = 1;
    invalid.protect.topology = research::FluidProtectTopology::dropletExempt;
    check(!engine.prepare(48000, invalid), "C rejects unsupported topology");
    invalid = {};
    invalid.protect.gain.epsilon = invalid.protect.gain.floor * 2;
    check(!engine.prepare(48000, invalid), "Protect validation even at depth zero");
    std::cout << "preview Protect failures=" << failures << '\n';
    return failures;
}
