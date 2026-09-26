#include "preview/DraftSummary.h"
#include "preview/PreviewController.h"
#include "preview/PreviewEngine.h"
#include "preview/ResearchPresentation.h"
#include "preview/ResearchViews.h"
#include "preview/SessionCodec.h"

#include <iostream>
#include <vector>

using namespace frazil::water;

int runReworkedCoreTests() {
    int failures{};
    const auto check = [&](bool pass, const char* name) {
        if (!pass) {
            ++failures;
            std::cerr << "FAIL reworked: " << name << '\n';
        }
    };
    preview::ResearchSessionModel session;
    check(session.draft().engineering.core == preview::WaterResearchCore::legacy, "default Legacy");
    session.setProtect(preview::ProtectId::depth, .7, preview::ChangeOrigin::engineeringUI);
    session.applyValidated();
    session.capture(0);
    const auto legacy = session.applied();
    preview::ResearchOperations operations(session);
    operations.action("Core Revision", preview::ChangeOrigin::soundLeadUI, true, true, [&] {
        session.setCore(preview::WaterResearchCore::reworked, preview::ChangeOrigin::soundLeadUI);
    });
    check(session.dirty() && session.dspDirty() && session.sessionDirty(), "revision dirty");
    check(preview::draftSummary(session).contains("Core Revision"), "draft revision visible");
    check(operations.history().size() == 1 &&
              preview::operationHistoryText(operations.history()).contains("Reworked A1/B1/D1"),
          "one complete revision operation");
    session.applyValidated();
    session.capture(1);
    session.restoreValidated(*session.slot(0));
    check(preview::sameOperationValues(session.applied(), legacy), "A restores Legacy values");
    session.restoreValidated(*session.slot(1));
    check(
        session.applied().engineering.core == preview::WaterResearchCore::reworked &&
            session.applied().engineering.values == legacy.engineering.values &&
            preview::sameProtect(session.applied().engineering.protect, legacy.engineering.protect),
        "B restores revision and retained settings");
    // v5 import must reset to Legacy even if the destination was previously Reworked.
    auto imported = session.applied();
    check(
        preview::decodeSession(preview::encodeSession(legacy).toStdString(), imported).isEmpty() &&
            imported.engineering.core == preview::WaterResearchCore::legacy,
        "v5 import remains Legacy");
    session.reset();
    check(session.draft().engineering.core == preview::WaterResearchCore::legacy,
          "reset returns Legacy");

    // Exercise the real selector callback and notification-free refresh without an audio device.
    {
        preview::ResearchSessionModel uiSession;
        preview::ResearchOperations uiOperations(uiSession);
        preview::WaterMacroView view(uiSession, preview::ChangeOrigin::soundLeadUI, uiOperations);
        view.setSize(1060, 320);
        uiSession.onChange = [&] { view.refresh(); };
        juce::ComboBox* selector{};
        for (auto* child : view.getChildren())
            if (child->getTitle() == "Research core revision")
                selector = dynamic_cast<juce::ComboBox*>(child);
        check(selector != nullptr, "core selector exists");
        if (selector) {
            selector->setSelectedId(2, juce::sendNotificationSync);
            check(uiSession.dspDirty() && uiOperations.history().size() == 1,
                  "selector changes draft once");
            int disabled{};
            for (auto* child : view.getChildren())
                if (dynamic_cast<preview::ResearchSlider*>(child) && !child->isEnabled())
                    ++disabled;
            check(disabled == 3, "Reworked Fluid macros disabled");
            // Optional local artifact for layout inspection; never an audio or human acceptance.
            const auto output =
                juce::SystemStats::getEnvironmentVariable("FRAZIL_PREVIEW_QA_PATH", "");
            if (output.isNotEmpty()) {
                auto stream = juce::File(output).createOutputStream();
                if (stream) {
                    stream->setPosition(0);
                    stream->truncate();
                    juce::PNGImageFormat png;
                    check(png.writeImageToStream(
                              view.createComponentSnapshot(view.getLocalBounds()), *stream),
                          "optional UI snapshot");
                } else
                    check(false, "optional UI snapshot file");
            }
            uiSession.setComposition(1, preview::ChangeOrigin::engineeringUI);
            for (auto* child : view.getChildren())
                if (dynamic_cast<preview::ResearchSlider*>(child))
                    check(child->isEnabled(), "C macros remain available");
            selector->setSelectedId(1, juce::sendNotificationSync);
            check(uiSession.draft().engineering.core == preview::WaterResearchCore::legacy,
                  "selector returns Legacy");
        }
        uiSession.onChange = {};
    }

    preview::PreviewController controller;
    preview::PreviewSettings settings;
    settings.core = preview::WaterResearchCore::reworked;
    settings.mode = 4;
    check(controller.validate(settings).contains("D1 requires A1 and/or B1"), "D-only reason");
    preview::PreviewEngine engine;
    check(!engine.prepare(48000, settings) && engine.residual({1, 1}) == research::StereoFrame{},
          "D-only rejects safely");

    // Independent direct chain mirrors the renderer float sum/transfer boundary, without
    // invoking Preview's dispatch. Monitor gain/trim/crossfade are deliberately outside parity.
    constexpr std::array<int, 6> modes{2, 3, 5, 6, 7, 0};
    constexpr std::array<unsigned, 6> flags{1, 2, 3, 5, 6, 7};
    for (const double rate : {44100., 48000., 96000.}) {
        for (std::size_t index = 0; index < modes.size(); ++index) {
            settings.mode = modes[index];
            settings.protect.depth = 1; // Retained but inactive, never coupled into this path.
            settings.protect.topology = research::FluidProtectTopology::dropletExempt;
            // Inactive legacy coupled constraints must not choose/reject the research defaults.
            settings.values[preview::controlIndex(preview::ControlId::bubbleMinFrequency)] = 2000;
            settings.values[preview::controlIndex(preview::ControlId::bubbleMaxFrequency)] = 1000;
            auto a = std::make_unique<research::BubbleA1>();
            auto b = std::make_unique<research::DropletB1>();
            research::FlowD1 d;
            const research::ResearchConfig config{rate, preview::PreviewEngine::kSeed};
            check(engine.prepare(rate, settings) && a->prepare(config) && b->prepare(config) &&
                      d.prepare(config),
                  "typed defaults prepare");
            std::vector<research::StereoFrame> reference;
            bool equal = true, finite = true;
            double energy{};
            const int samples = static_cast<int>(rate);
            for (int i = 0; i < samples; ++i) {
                const research::StereoFrame input{i % 8000 < 1800 ? .45f * std::cos(i * .13f) : 0.f,
                                                  0.f};
                research::FluidResiduals parts;
                if (flags[index] & 1)
                    parts.bubble = a->process(input);
                if (flags[index] & 2)
                    parts.droplet = b->process(input);
                auto expected = parts.sum();
                if (flags[index] & 4) {
                    const auto transfer = d.process(expected);
                    for (std::size_t ch = 0; ch < 2; ++ch)
                        expected[ch] = static_cast<float>(transfer.transferred[ch]);
                }
                const auto actual = engine.residual(input);
                equal &= actual == expected && actual[1] == 0;
                finite &= std::isfinite(actual[0]);
                energy += double(actual[0]) * actual[0];
                reference.push_back(actual);
            }
            check(equal && finite && energy > 0, "exact reference parity / finite / isolation");
            check(engine.protectReadout().reductionDb == 0 && !engine.setProtectDepth(.5),
                  "Protect inactive");
            engine.reset();
            bool repeated = true;
            for (int i = 0; i < samples; ++i)
                repeated &= engine.residual({i % 8000 < 1800 ? .45f * std::cos(i * .13f) : 0.f,
                                             0.f}) == reference[static_cast<std::size_t>(i)];
            check(repeated, "reset exact repeat");
            check(engine.prepare(rate, settings), "restart reprepare");
            bool restarted = true;
            for (int i = 0; i < 4096; ++i)
                restarted &= engine.residual({i % 8000 < 1800 ? .45f * std::cos(i * .13f) : 0.f,
                                              0.f}) == reference[static_cast<std::size_t>(i)];
            check(restarted, "reprepare exact repeat");
        }
        for (int mode : {1, 8}) {
            preview::PreviewSettings old;
            old.mode = mode;
            auto revised = old;
            revised.core = preview::WaterResearchCore::reworked;
            preview::PreviewEngine legacyEngine;
            check(legacyEngine.prepare(rate, old) && engine.prepare(rate, revised),
                  "C/baseline prepare");
            bool equal = true;
            for (int i = 0; i < 4096; ++i) {
                const research::StereoFrame input{i == 0 ? .5f : 0.f, 0.f};
                equal &= legacyEngine.residual(input) == engine.residual(input);
            }
            check(equal, "C/baseline unchanged");
        }
    }
    return failures;
}
