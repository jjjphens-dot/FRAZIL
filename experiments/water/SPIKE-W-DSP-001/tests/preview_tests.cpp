#include "preview/PreviewController.h"
#include "preview/PreviewEngine.h"

#include <chrono>
#include <iostream>
#include <limits>

using namespace frazil::water;

int runTimeValueTests();
int runDescriptorTests();
int runSessionTests();
int runSessionCodecTests();
int runPreviewProtectTests();
int runProtectDiagnosticsTests();
int runWorkflowTests();
int runOperationTests();
int runAuditionTests();
int runAuditionWorkflowTests();
int runReworkedCoreTests();

int main() {
    juce::ScopedJuceInitialiser_GUI gui;
    int failures = runTimeValueTests() + runDescriptorTests() + runSessionTests() +
                   runSessionCodecTests() + runPreviewProtectTests() +
                   runProtectDiagnosticsTests() + runWorkflowTests() + runOperationTests() +
                   runAuditionTests() + runAuditionWorkflowTests() + runReworkedCoreTests();
    const auto check = [&](bool result, const char* name) {
        if (!result) {
            std::cerr << "FAIL " << name << '\n';
            ++failures;
        }
    };
    preview::PreviewSettings settings;
    const auto file = juce::File::getSpecialLocation(juce::File::tempDirectory)
                          .getNonexistentChildFile("frazil-preview-config", ".json");
    check(file.replaceWithText(settings.moduleJson()), "export module JSON");
    research::FluidConfig fluidConfig;
    research::ModalConfig modalConfig;
    research::ProtectRenderConfig protectConfig;
    check(research::readConfig(file, fluidConfig, modalConfig, protectConfig),
          "renderer consumes preview export");
    check(fluidConfig.bubble.maximumEventRateHz == 120 && fluidConfig.droplet.voices == 8 &&
              fluidConfig.flow.depthSeconds == .001 && modalConfig.rootFrequencyHz == 260,
          "units and defaults round trip");
    file.deleteFile();
    // Integration boundary: importing Protect must never silently discard its settings.
    constexpr std::string_view protectJson =
        R"({"protect":{"depth":0.6,"detector":0,"topology":2,"thresholdHigh":0.12}})";
    check(research::readConfigText(protectJson, fluidConfig, modalConfig, &protectConfig) &&
              protectConfig.depth == .6 &&
              protectConfig.gain.score == research::ProtectScore::difference &&
              protectConfig.gain.thresholdHigh == .12,
          "in-memory parser retains Protect config");
    check(!research::readConfigText(protectJson, fluidConfig, modalConfig),
          "unconnected preview rejects Protect config");
    check(!research::readConfigText(R"({"protect":{"depth":0,"depth":1}})", fluidConfig,
                                    modalConfig, &protectConfig),
          "in-memory parser rejects duplicate Protect fields");
    preview::PreviewController controller;
    check(controller.validate(settings).isEmpty(), "default application config valid");
    check(controller.play(settings).isNotEmpty(), "play without source fails before device start");

    constexpr std::array<unsigned, 9> flags{7, 0, 1, 2, 4, 3, 5, 6, 0};
    for (int mode = 0; mode < static_cast<int>(flags.size()); ++mode) {
        for (const double rate : {44100.0, 48000.0, 96000.0}) {
            settings.mode = mode;
            preview::PreviewEngine actual, repeat;
            check(actual.prepare(rate, settings) && repeat.prepare(rate, settings),
                  "prepare modes/rates");
            research::ResearchConfig researchConfig;
            researchConfig.sampleRateHz = rate;
            researchConfig.baseSeed = preview::PreviewEngine::kSeed;
            fluidConfig.bubbleEnabled = (flags[mode] & 1) != 0;
            fluidConfig.dropletEnabled = (flags[mode] & 2) != 0;
            fluidConfig.flowEnabled = (flags[mode] & 4) != 0;
            research::FluidCandidate referenceFluid;
            research::LiquidModalResonator referenceModal;
            if (mode == 1)
                check(referenceModal.prepare(rate, modalConfig), "reference modal");
            else
                check(referenceFluid.prepare(researchConfig, fluidConfig), "reference fluid");
            double energy{};
            for (int i = 0; i < 12000; ++i) {
                const float sample = i % 1500 < 800 ? .4f * std::sin(i * .1f) : 0.0f;
                const research::StereoFrame input{sample, 0.0f};
                const auto e = actual.residual(input);
                const auto& frames = actual.diagnosticFrames().signals;
                for (std::size_t c = 0; c < 2; ++c) {
                    const float sum =
                        static_cast<float>(static_cast<double>(frames[1][c]) + frames[2][c] +
                                           frames[3][c] + frames[4][c]);
                    check(std::abs(sum - e[c]) < 1e-7,
                          "diagnostic solos are actual pre-Protect components");
                    if (input == research::StereoFrame{})
                        check(frames[5][c] == 0 && frames[6][c] == 0 && frames[7][c] == 0,
                              "zero input has no actual new driver impulses");
                }

                check(actual.excitationFrame() == (mode == 1 ? input : research::StereoFrame{}),
                      "actual modal driver transport / non-modal silence");
                const auto expected =
                    mode == 1 ? referenceModal.process(input) : referenceFluid.process(input);
                check(e == expected, "UI composition matches unchanged research DSP");
                check(e == repeat.residual(input), "fixed-seed repeat");
                check(e[1] == 0, "stereo audio isolation");
                check(std::isfinite(input[0] + e[0]), "finite x+E");
                energy += static_cast<double>(e[0]) * e[0];
            }
            check(mode == 8 ? energy == 0 : energy > 0,
                  "audible path differs from dry except baseline");
            actual.reset();
            repeat.prepare(rate, settings);
            for (int i = 0; i < 1000; ++i)
                check(actual.residual({.25f, -.125f}) == repeat.residual({.25f, -.125f}),
                      "reset reproduces freshly prepared DSP");
        }
    }
    // Explicit module/session C options reach the same bank as the offline typed path.
    for (const auto carrier :
         {research::ModalExcitation::hard, research::ModalExcitation::feature}) {
        preview::PreviewSettings candidate;
        candidate.mode = 1;
        candidate.values[preview::controlIndex(preview::ControlId::modalExcitation)] =
            static_cast<double>(carrier);
        candidate.values[preview::controlIndex(preview::ControlId::modalNormalization)] = 1;
        candidate.values[preview::controlIndex(preview::ControlId::modalMotionModel)] = 1;
        candidate.values[preview::controlIndex(preview::ControlId::modalMotionDepth)] = .35;
        preview::PreviewEngine engine;
        research::LiquidModalResonator bank;
        research::ModalConfig config;
        config.motionDepth = .35;
        check(engine.prepare(48000, candidate) &&
                  bank.prepare(48000, config, carrier, research::ModalNormalization::c3,
                               research::ModalMotionModel::structured),
              "preview prepares explicit bounded C3 candidate");
        for (int i = 0; i < 12003; ++i) {
            const research::StereoFrame input{.2f * std::sin(i * .13f), 0};
            check(engine.residual(input) == bank.process(input),
                  "preview/offline C3 sample identity");
            check(engine.diagnosticFrames().signals[static_cast<std::size_t>(
                      preview::DiagnosticSignal::modalDriver)] == bank.excitationFrame(),
                  "actual candidate driver transported without recomputation");
        }
        candidate.values[preview::controlIndex(preview::ControlId::modalExcitation)] = 0;
        check(controller.validate(candidate).isNotEmpty(), "UI rejects raw/C3 before playback");
    }
    settings = {};
    settings.mode = 1;
    preview::PreviewEngine first, changed;
    first.prepare(48000, settings);
    settings.values[18] = 900;
    check(changed.prepare(48000, settings), "engineering frequency is applied");
    bool different{};
    for (int i = 0; i < 2000; ++i) {
        const research::StereoFrame impulse{i == 0 ? .5f : 0.0f, 0.0f};
        different |= first.residual(impulse) != changed.residual(impulse);
    }
    check(different, "changing GUI config changes residual samples");
    settings.values[18] = 9000;
    check(!changed.prepare(48000, settings), "invalid active modal rejected");
    check(changed.residual({1, 1}) == research::StereoFrame{},
          "failed prepare does not run stale DSP");
    settings = {};
    settings.values[14] = .001;
    settings.values[15] = .005;
    check(controller.validate(settings).isNotEmpty(), "coupled Flow ranges rejected");
    settings = {};
    settings.mode = -1;
    check(!changed.prepare(48000, settings), "invalid composition rejected");
    settings = {};
    settings.values[0] = std::numeric_limits<double>::quiet_NaN();
    check(!changed.prepare(48000, settings), "nonfinite engineering field rejected");

    // Same fixed workload, preliminary engineering timing only; no formal CPU budget claim.
    settings = {};
    first.prepare(48000, settings);
    double sink{};
    const auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < 256000; ++i)
        sink += first.residual({.1f, -.1f})[0];
    const auto elapsed =
        std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - begin).count();
    std::cout << "preview ABD mean us per 128 frames=" << elapsed / 2000.0
              << " observation=" << sink << '\n';
    std::cout << "preview tests failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
