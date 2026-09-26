#include "preview/DraftSummary.h"
#include "preview/PreviewEngine.h"
#include "preview/ResearchAuditionWorkflow.h"
#include "preview/ResearchCoreConfigCodec.h"
#include "preview/ResearchCoreTuningView.h"
#include "preview/ResearchPresentation.h"
#include "preview/WaterDiagnosticsText.h"

#include <iostream>
#include <limits>
#include <memory>
#include <vector>

using namespace frazil::water;
namespace {
using Adapter = preview::ResearchCoreParameterAdapter;
using Module = preview::ResearchModule;
std::size_t indexOf(Module module, std::string_view name) {
    for (std::size_t i = 0; i < Adapter::parameterCount(module); ++i)
        if (Adapter::parameter(module, i).name == name)
            return i;
    return Adapter::parameterCount(module);
}
research::StereoFrame sourceFrame(int i) {
    return {i % 8000 < 1800 ? .45f * std::cos(i * .13f) : 0.f, 0.f};
}
} // namespace

int runReworkedParameterTests() {
    int failures{};
    const auto check = [&](bool result, const char* name) {
        if (!result) {
            ++failures;
            std::cerr << "FAIL tuning: " << name << '\n';
        }
    };
    const preview::ResearchCoreTuningState defaults;
    const auto metadata = [&](Module module, const auto& specs) {
        check(Adapter::parameterCount(module) == specs.size(), "canonical count");
        int primary{};
        for (std::size_t i = 0; i < specs.size(); ++i) {
            const auto view = Adapter::parameter(module, i);
            const auto& spec = specs[i];
            check(view.name == spec.name && view.unit == spec.unit &&
                      view.classification == spec.classification &&
                      &view.minimum == &spec.minimum && &view.maximum == &spec.maximum &&
                      &view.initial == &spec.initial &&
                      view.choices.data() == spec.choices.data() &&
                      view.choices.size() == spec.choiceCount,
                  "metadata borrows exact canonical authority");
            check(Adapter::getValue(defaults, module, i) == spec.initial, "canonical default");
            auto state = defaults;
            for (double bad :
                 {spec.minimum - 1, spec.maximum + 1, std::numeric_limits<double>::quiet_NaN(),
                  std::numeric_limits<double>::infinity()})
                check(!Adapter::setValue(state, module, i, bad) && state == defaults,
                      "invalid scalar atomic reject");
            if (spec.choiceCount) {
                for (std::size_t c = 0; c < spec.choiceCount; ++c)
                    check(Adapter::setValue(state, module, i, spec.choices[c]),
                          "all canonical choices writable");
                check(!Adapter::setValue(state, module, i, spec.minimum + .12345),
                      "nonchoice reject");
            } else
                check(Adapter::setValue(state, module, i, spec.maximum), "valid scalar");
            Adapter::resetModule(state, module);
            check(state == defaults, "module reset canonical");
            primary += Adapter::primary(module, i);
        }
        check(primary == (module == Module::flow ? 3 : 7), "primary coverage");
    };
    metadata(Module::bubble, research::kA1Parameters);
    metadata(Module::droplet, research::kB1Parameters);
    metadata(Module::flow, research::kD1Parameters);
    auto invalid = defaults;
    check(!Adapter::setValue(invalid, Module::bubble, 999, 0), "invalid index");
    Adapter::setValue(invalid, Module::bubble, indexOf(Module::bubble, "radiusMinMm"), 10);
    check(!Adapter::validate(invalid), "equal radius coupled reject");
    Adapter::resetAll(invalid);
    check(invalid == defaults && Adapter::validate(invalid), "reset all");

    // Complete non-default input goes through the independent renderer parser. Preview's builders
    // are deliberately not used by the reference chain, including shared A1 analysis fields.
    preview::PreviewSettings settings;
    settings.core = preview::WaterResearchCore::reworked;
    for (auto module : preview::kResearchModules)
        for (std::size_t i = 0; i < Adapter::parameterCount(module); ++i) {
            const auto spec = Adapter::parameter(module, i);
            const double value = spec.choices.empty()
                                     ? spec.minimum + .37 * (spec.maximum - spec.minimum)
                                 : spec.choices.front() == spec.initial ? spec.choices.back()
                                                                        : spec.choices.front();
            check(Adapter::setValue(settings.tuning, module, i, value), "nondefault setup");
        }
    check(Adapter::validate(settings.tuning), "nondefault valid");
    const auto complete = preview::encodeResearchConfig(settings);
    research::FluidConfig unusedFluid;
    research::ModalConfig unusedModal;
    research::BubbleA1RenderConfig aConfig;
    research::DropletB1RenderConfig bConfig;
    research::FlowD1RenderConfig dConfig;
    check(research::readConfigText(complete.toStdString(), unusedFluid, unusedModal, nullptr,
                                   &aConfig, &bConfig, &dConfig),
          "renderer accepts export");
    auto imported = defaults;
    check(preview::decodeReworkedConfig(complete.toStdString(), imported, imported).isEmpty() &&
              imported == settings.tuning,
          "full renderer config exact round trip");
    for (const auto* bad : {"{}", "{\"unknown\":{}}", "{\"bubbleA1\":{\"version\":3}}",
                            "{\"bubbleA1\":{\"version\":2,\"unknown\":1}}",
                            "{\"bubbleA1\":{\"version\":2,\"radiusMinMm\":10,\"radiusMaxMm\":2}}",
                            "{\"bubbleA1\":{\"version\":2,\"motionFactor\":-1}}",
                            "{\"dropletB1\":{\"version\":1,\"riseXi\":0.025}}",
                            "{\"flowD1\":{\"version\":1,\"velocityScaleMps\":2}}",
                            "{\"flowD1\":{\"version\":1,\"velocityScaleMps\":NaN}}",
                            "{\"flowD1\":{\"version\":1,\"velocityScaleMps\":1e999}}",
                            "{\"flowD1\":{\"version\":1,\"version\":1}}",
                            "{\"flowD1\":{\"version\":1},\"bubble\":{}}"}) {
        auto candidate = settings.tuning;
        check(preview::decodeReworkedConfig(bad, candidate, candidate).isNotEmpty() &&
                  candidate == settings.tuning,
              "strict failure preserves entire draft");
    }
    auto partial = settings.tuning;
    check(preview::decodeReworkedConfig("{\"dropletB1\":{\"version\":1}}", partial, partial)
                  .isEmpty() &&
              partial.bubble == settings.tuning.bubble && partial.flow == settings.tuning.flow &&
              partial.droplet == defaults.droplet,
          "omitted fields default, omitted modules retain");

    preview::ResearchSessionModel session;
    preview::ResearchOperations operations(session);
    const auto origin = preview::ChangeOrigin::engineeringUI;
    const auto radius = indexOf(Module::bubble, "radiusMinMm");
    session.setCore(preview::WaterResearchCore::reworked, origin);
    session.applyValidated();
    int stops{};
    operations.onPrepareBegin = [&] { ++stops; };
    operations.begin("A1.radiusMinMm", origin, true, false, true, 0);
    for (int i = 0; i < 100; ++i) {
        operations.begin("A1.radiusMinMm", origin, true, false, false, i);
        session.setReworkedParameter(Module::bubble, radius, .3 + i * .001, origin);
    }
    operations.finish();
    check(stops == 1 && operations.history().size() == 1 && session.dirty() && session.dspDirty() &&
              session.sessionDirty(),
          "one drag one operation and all dirty flags");
    check(preview::draftSummary(session).contains("A1.radiusMinMm") &&
              preview::operationHistoryText(operations.history()).contains("A1.radiusMinMm"),
          "named draft/history delta");
    session.applyValidated();
    session.capture(0);
    session.setCore(preview::WaterResearchCore::legacy, origin);
    session.importReworkedTuning(settings.tuning);
    session.applyValidated();
    session.capture(1);
    session.restoreValidated(*session.slot(0));
    check(session.applied().engineering.core == preview::WaterResearchCore::reworked &&
              Adapter::getValue(session.applied().engineering.tuning, Module::bubble, radius) ==
                  .399,
          "A restores core and tuning");
    session.restoreValidated(*session.slot(1));
    check(session.applied().engineering.core == preview::WaterResearchCore::legacy &&
              session.applied().engineering.tuning == settings.tuning,
          "B retains inactive tuning and core");
    session.setCore(preview::WaterResearchCore::reworked, origin);
    for (auto module : preview::kResearchModules)
        session.resetReworkedModule(module);
    check(session.draft().engineering.tuning == defaults &&
              session.applied().engineering.tuning == settings.tuning,
          "module resets draft only");
    session.importReworkedTuning(settings.tuning);
    session.resetReworkedDefaults();
    check(session.draft().engineering.tuning == defaults, "all reset");
    auto beforeApply = session.applied();
    session.setReworkedParameter(Module::bubble, radius, 10, origin);
    preview::ResearchAuditionWorkflow workflow(session);
    preview::PreviewEngine engine;
    workflow.stop = [] {};
    workflow.status = [](const juce::String&) {};
    workflow.prepare = [&](const preview::PreviewSettings& value) {
        return engine.prepare(48000, value) ? juce::String{} : juce::String("invalid");
    };
    check(!workflow.apply() && preview::sameOperationValues(session.applied(), beforeApply),
          "invalid Apply preserves applied state");
    session.resetReworkedDefaults();
    const auto historyStart = operations.history().size();
    operations.begin("A1.radiusMinMm", origin, true, false, false, 1000);
    session.setReworkedParameter(Module::bubble, radius, .5, origin);
    operations.tick(1249);
    check(operations.history().size() == historyStart, "debounce waits 250ms");
    operations.tick(1250);
    check(operations.history().size() == historyStart + 1, "debounce ends at 250ms");
    for (int i = 0; i < 60; ++i)
        operations.action("A1.radiusMinMm", origin, true, false, [&] {
            session.setReworkedParameter(Module::bubble, radius, .6 + i * .01, origin);
        });
    check(operations.history().size() == 50, "history bounded 50");

    constexpr std::array modes{2, 3, 5, 6, 7, 0};
    constexpr std::array<unsigned, 6> flags{1, 2, 3, 5, 6, 7};
    for (const double rate : {44100., 48000., 96000.}) {
        for (std::size_t m = 0; m < modes.size(); ++m) {
            settings.mode = modes[m];
            const auto json = preview::encodeResearchConfig(settings);
            const auto root = juce::JSON::parse(json);
            for (std::size_t module = 0; module < preview::kResearchModules.size(); ++module)
                check(root.hasProperty(Adapter::configKey(preview::kResearchModules[module])) ==
                          bool(flags[m] & (1u << module)),
                      "export active modules only");
            auto a = std::make_unique<research::BubbleA1>();
            auto b = std::make_unique<research::DropletB1>();
            research::FlowD1 d;
            const research::ResearchConfig config{rate, preview::PreviewEngine::kSeed};
            check(engine.prepare(rate, settings) &&
                      a->prepare(config, aConfig.bubble, aConfig.analysis) &&
                      b->prepare(config, bConfig.droplet) && d.prepare(config, dConfig.flow),
                  "typed nondefault prepare");
            preview::PreviewEngine baseline;
            auto defaultSettings = settings;
            defaultSettings.tuning = {};
            check(baseline.prepare(rate, defaultSettings), "default comparison prepares");
            std::vector<research::StereoFrame> reference;
            bool equal = true, changed = false, finite = true;
            for (int i = 0; i < static_cast<int>(rate); ++i) {
                const auto input = sourceFrame(i);
                research::FluidResiduals parts;
                if (flags[m] & 1)
                    parts.bubble = a->process(input);
                if (flags[m] & 2)
                    parts.droplet = b->process(input);
                auto expected = parts.sum();
                if (flags[m] & 4) {
                    const auto transfer = d.process(expected);
                    for (std::size_t ch = 0; ch < 2; ++ch)
                        expected[ch] = static_cast<float>(transfer.transferred[ch]);
                }
                const auto actual = engine.residual(input);
                equal &= actual == expected && actual[1] == 0;
                finite &= std::isfinite(actual[0]) && std::isfinite(actual[1]);
                changed |= actual != baseline.residual(input);
                reference.push_back(actual);
            }
            check(equal && finite && changed,
                  "six-mode nondefault exact parity/finite/changes sound samples");
            const auto activity = engine.waterActivity();
            check(activity.bubbleRequested == ((flags[m] & 1) ? a->requested() : 0) &&
                      activity.bubbleRequestedRate == ((flags[m] & 1) ? a->requestedRate() : 0) &&
                      activity.dropletEligible == ((flags[m] & 2) ? b->counters().eligible : 0) &&
                      activity.dropletAdmitted == ((flags[m] & 2) ? b->counters().admitted : 0) &&
                      activity.flowPathMeters == ((flags[m] & 4) ? d.pathMeters() : 0),
                  "actual bounded diagnostics");
            preview::ProtectDiagnostics transport;
            preview::ProtectBlockReadout block;
            block.water.latest = activity;
            transport.publish(block);
            const auto snapshot = transport.snapshot();
            check(snapshot.water.latest.flowPathMeters == activity.flowPathMeters &&
                      preview::waterDiagnosticsText(snapshot).contains("A1 requested"),
                  "queue and presentation");
            for (bool reprepare : {false, true}) {
                if (reprepare)
                    check(engine.prepare(rate, settings), "reprepare");
                else
                    engine.reset();
                bool repeat = true;
                for (int i = 0; i < static_cast<int>(rate); ++i)
                    repeat &=
                        engine.residual(sourceFrame(i)) == reference[static_cast<std::size_t>(i)];
                check(repeat, "reset/reprepare sample exact");
            }
        }
    }
    // Raw tuning cannot alter the C/baseline or Legacy processing paths, even if corrupted.
    for (int mode : {0, 1, 8}) {
        preview::PreviewSettings old, changed;
        old.mode = changed.mode = mode;
        if (mode != 0)
            old.core = changed.core = preview::WaterResearchCore::reworked;
        changed.tuning.bubble.fill(std::numeric_limits<double>::quiet_NaN());
        preview::PreviewEngine reference;
        check(reference.prepare(48000, old) && engine.prepare(48000, changed),
              "inactive tuning not read");
        bool same = true;
        for (int i = 0; i < 8192; ++i)
            same &= reference.residual(sourceFrame(i)) == engine.residual(sourceFrame(i));
        check(same, "C baseline Legacy unchanged");
    }
    // Actual widgets: exact entry, canonical choices, folding, reset and notification-free refresh.
    {
        preview::ResearchSessionModel ui;
        preview::ResearchOperations uiOps(ui);
        ui.setCore(preview::WaterResearchCore::reworked, origin);
        preview::ResearchCoreTuningView view(ui, uiOps);
        ui.onChange = [&] { view.refresh(); };
        view.onLayoutChange = [&] { view.setSize(1040, view.preferredHeight()); };
        view.setSize(1040, view.preferredHeight());
        int exact{}, choices{}, visible{};
        for (auto* child : view.getChildren()) {
            if (dynamic_cast<preview::ExactValueControl*>(child)) {
                ++exact;
                visible += child->isVisible();
            }
            if (auto* choice = dynamic_cast<juce::ComboBox*>(child))
                ++choices;
        }
        check(exact == 33 && choices == 7 && visible == 17,
              "generic coverage, Primary only initially");
        for (auto* child : view.getChildren())
            if (auto* button = dynamic_cast<juce::TextButton*>(child);
                button && button->getButtonText() == "[+] Advanced")
                button->onClick();
        for (auto* child : view.getChildren()) {
            if (auto* choice = dynamic_cast<juce::ComboBox*>(child);
                choice && child->getTitle() == "B1.riseXi") {
                check(choice->getNumItems() == 3, "canonical B1 rise choices");
                choice->setSelectedId(2, juce::sendNotificationSync);
                check(Adapter::namedValue(ui.draft().engineering.tuning, Module::droplet,
                                          "riseXi") == .05,
                      "real choice callback changes draft");
            }
            if (auto* control = dynamic_cast<preview::ExactValueControl*>(child);
                control && child->getTitle() == "A1.radiusMinMm") {
                for (auto* editor : control->getChildren())
                    if (auto* entry = dynamic_cast<juce::TextEditor*>(editor)) {
                        entry->setText("0.6", false);
                        entry->onReturnKey();
                        check(Adapter::namedValue(ui.draft().engineering.tuning, Module::bubble,
                                                  "radiusMinMm") == .6,
                              "real exact entry callback");
                        entry->setText("nan", false);
                        entry->onReturnKey();
                        check(Adapter::namedValue(ui.draft().engineering.tuning, Module::bubble,
                                                  "radiusMinMm") == .6,
                              "invalid entry retains value");
                    }
            }
        }
        const auto revision = ui.revision();
        view.refresh();
        view.refresh();
        check(ui.revision() == revision, "refresh never writes session");
        const auto output = juce::SystemStats::getEnvironmentVariable("FRAZIL_TUNING_QA_PATH", {});
        if (output.isNotEmpty()) {
            juce::Image canvas(juce::Image::RGB, view.getWidth(), view.getHeight(), true,
                               juce::SoftwareImageType());
            {
                juce::Graphics g(canvas);
                g.fillAll(juce::Colour(0xff182b36));
                view.paintEntireComponent(g, true);
            }
            auto exported = ui.draft().engineering;
            exported.mode = 0;
            juce::File(output).withFileExtension("json").replaceWithText(
                preview::encodeResearchConfig(exported));
            juce::File file(output);
            auto stream = file.createOutputStream();
            if (stream) {
                stream->setPosition(0);
                stream->truncate();
                juce::PNGImageFormat png;
                png.writeImageToStream(canvas, *stream);
            }
        }
        for (auto* child : view.getChildren())
            if (auto* button = dynamic_cast<juce::TextButton*>(child);
                button && button->getButtonText() == "Reset Reworked Defaults")
                button->onClick();
        check(ui.draft().engineering.tuning == defaults, "real reset button");
        ui.setCore(preview::WaterResearchCore::legacy, origin);
        check(!view.isVisible() && view.preferredHeight() == 0, "Legacy hides raw tuning");
        ui.onChange = {};
    }
    return failures;
}
