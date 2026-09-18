#include "preview/ResearchSessionModel.h"

#include <iostream>
#include <limits>

int runSessionTests() {
    using namespace frazil::water::preview;
    int failures{};
    const auto check = [&](bool ok, const char* text) {
        if (!ok) {
            std::cerr << "FAIL session: " << text << '\n';
            ++failures;
        }
    };
    ResearchSessionModel session;
    int notifications{};
    // Two observers render the same const state; they do not own mutable parameter copies.
    double soundSize{}, engineeringSize{};
    session.onChange = [&] {
        ++notifications;
        soundSize = session.draft().water.size;
        engineeringSize = session.draft().water.size;
    };
    check(!session.dirty() && session.revision() == 0, "baseline applied");
    const auto baselineConfig = session.draft().engineering.moduleJson();
    session.setMacro(MacroId::size, .8, ChangeOrigin::soundLeadUI);
    check(soundSize == .8 && engineeringSize == .8 && notifications == 1 && session.revision() == 1,
          "one edit one publication two observers");
    check(session.draft().engineering.moduleJson() != baselineConfig && session.dspDirty() &&
              session.draft().macroMappings[0] == MappingStatus::mapped,
          "research macro updates owned DSP targets");
    session.setMacro(MacroId::size, .8, ChangeOrigin::engineeringUI);
    check(notifications == 1, "identical write cannot feed back");
    session.setMacro(MacroId::size, .2, ChangeOrigin::engineeringUI);
    check(soundSize == .2 && engineeringSize == .2 &&
              session.draft().lastChange.origin == ChangeOrigin::engineeringUI,
          "reverse view sync and origin");
    session.setModel(WaterModel::resonant, ChangeOrigin::soundLeadUI);
    check(session.draft().engineering.mode == 1 &&
              session.draft().modelMapping == MappingStatus::mapped,
          "Resonant maps C");
    session.setComposition(0, ChangeOrigin::engineeringUI);
    check(session.draft().water.model == WaterModel::fluid &&
              session.draft().modelMapping == MappingStatus::mapped,
          "ABD maps Fluid");
    session.setComposition(2, ChangeOrigin::engineeringUI);
    check(session.draft().modelMapping == MappingStatus::custom,
          "ablation marks custom composition");
    session.returnModelToMapped();
    check(session.draft().engineering.mode == 0 &&
              session.draft().lastChange.origin == ChangeOrigin::mapper,
          "return model to justified mapping");
    session.setComposition(1, ChangeOrigin::engineeringUI);
    const auto beforeMacros = session.draft().water;
    session.setEngineering(ControlId::bubbleDecay, .11, ChangeOrigin::engineeringUI);
    check(!moduleActive(ControlGroup::bubble, 1) && moduleActive(ControlGroup::modal, 1) &&
              session.draft().engineering.values[controlIndex(ControlId::bubbleDecay)] == .11,
          "inactive edit retained");
    check(session.draft().water == beforeMacros && session.draft().customEngineering,
          "no reverse macro mapping");
    check(session.draft().ownership[controlIndex(ControlId::bubbleDecay)].revision ==
              session.revision(),
          "target edit provenance");
    check(session.applied().engineering.values[controlIndex(ControlId::bubbleDecay)] == .07 &&
              session.dirty(),
          "draft does not mutate applied");
    const auto revision = session.revision();
    check(!session.setEngineering(ControlId::bubbleVoices, 2.5, ChangeOrigin::engineeringUI) &&
              !session.setEngineering(ControlId::count, 1, ChangeOrigin::engineeringUI) &&
              !session.setMacro(MacroId::motion, std::numeric_limits<double>::quiet_NaN(),
                                ChangeOrigin::soundLeadUI) &&
              !session.setComposition(9, ChangeOrigin::engineeringUI) &&
              session.revision() == revision,
          "invalid commands atomic");
    session.applyValidated();
    check(!session.dirty() && session.applied().water == session.draft().water,
          "apply captures one complete value state");
    session.setMonitor(MonitorMode::residual, -18);
    check(!session.dirty() && session.applied().monitor == MonitorMode::residual,
          "monitor live outside draft");
    check(session.sessionDirty() && !session.dspDirty(), "monitor changes session only");
    session.applyValidated();
    check(!session.sessionDirty(), "apply checkpoints session context");
    session.capture(0);
    const auto captured = *session.slot(0);
    session.reset();
    session.applyValidated();
    const auto beforeRecall = notifications;
    session.restoreValidated(captured);
    check(notifications == beforeRecall + 1 && !session.dirty() &&
              session.applied().water == captured.water &&
              session.applied().engineering.moduleJson() == captured.engineering.moduleJson() &&
              session.applied().monitorGainDb == -18,
          "A/B complete and single-publication recall");
    check(session.draft().lastChange.origin == ChangeOrigin::sessionLoad,
          "recall records new session origin");
    session.reset();
    session.applyValidated();
    check(session.applied().engineering.moduleJson() == baselineConfig &&
              session.applied().water.size == .5,
          "baseline reset");
    for (int mode = 0; mode < 9; ++mode) {
        constexpr std::array<unsigned, 9> flags{7, 0, 1, 2, 4, 3, 5, 6, 0};
        check(moduleActive(ControlGroup::bubble, mode) == bool(flags[mode] & 1) &&
                  moduleActive(ControlGroup::droplet, mode) == bool(flags[mode] & 2) &&
                  moduleActive(ControlGroup::flow, mode) == bool(flags[mode] & 4),
              "active module truth table");
    }
    session.reset();
    session.setModel(WaterModel::resonant, ChangeOrigin::engineeringUI);
    session.applyValidated();
    session.setModel(WaterModel::fluid, ChangeOrigin::engineeringUI);
    session.setTopology(frazil::water::research::FluidProtectTopology::dropletExempt,
                        ChangeOrigin::engineeringUI);
    session.setModel(WaterModel::resonant, ChangeOrigin::engineeringUI);
    check(session.draft().engineering.moduleJson() == session.applied().engineering.moduleJson() &&
              session.unappliedChanges() == 1 && session.dirty(),
          "retained Fluid topology is dirty even when active C config is unchanged");
    check(session.sessionDirty() && !session.dspDirty(), "retained topology is context only");
    session.applyValidated();
    session.setProtect(ProtectId::depth, .8, ChangeOrigin::soundLeadUI);
    session.setProtect(ProtectId::depth, 0, ChangeOrigin::soundLeadUI);
    session.reset();
    check(session.dirty() && session.unappliedChanges() >= 2,
          "reset counts retained enable-depth and topology changes");
    session.reset();
    session.applyValidated();
    session.setEngineering(ControlId::bubbleMinFrequency, 1234, ChangeOrigin::engineeringUI);
    session.setEngineering(ControlId::dropletDecay, .025, ChangeOrigin::engineeringUI);
    session.setEngineering(ControlId::flowGain, .02, ChangeOrigin::engineeringUI);
    check(session.draft().listeningCalibration == MappingStatus::custom,
          "gain edits mark listening calibration CUSTOM");
    check(session.draft().macroMappings ==
              std::array{MappingStatus::custom, MappingStatus::mapped, MappingStatus::custom},
          "raw target edit marks only owning macro CUSTOM");
    session.returnMacroToMapped(MacroId::size);
    check(session.draft().engineering.values[controlIndex(ControlId::bubbleMinFrequency)] == 250 &&
              session.draft().engineering.values[controlIndex(ControlId::dropletDecay)] == .025 &&
              session.draft().engineering.values[controlIndex(ControlId::flowGain)] == .02 &&
              session.draft().macroMappings[2] == MappingStatus::custom,
          "Return Size preserves other CUSTOM destinations and unowned gain");
    session.returnAllToMapped();
    check(session.draft().engineering.values[controlIndex(ControlId::dropletDecay)] == .012 &&
              session.draft().engineering.values[controlIndex(ControlId::flowGain)] == .02,
          "Return All owns macro targets only");
    for (const auto macro : {MacroId::size, MacroId::motion, MacroId::decay}) {
        const auto before = session.draft().engineering;
        session.setMacro(macro, .8, ChangeOrigin::soundLeadUI);
        for (const auto& control : kControls)
            if (macroOwner(control.id) != macro)
                check(session.draft().engineering.values[controlIndex(control.id)] ==
                          before.values[controlIndex(control.id)],
                      "macro orthogonality preserves every unowned destination");
        check(sameProtect(before.protect, session.draft().engineering.protect),
              "macros never alter Protect");
    }
    auto legacy = session.draft();
    legacy.mappingRevision = "legacy-unmapped";
    legacy.macroMappings.fill(MappingStatus::custom);
    session.restoreValidated(legacy);
    session.setMacro(MacroId::size, .1, ChangeOrigin::soundLeadUI);
    check(session.draft().engineering.values == legacy.engineering.values && !session.dspDirty(),
          "legacy macro movement cannot change imported sound");
    session.returnAllToMapped();
    check(session.dspDirty() &&
              session.draft().mappingRevision == ResearchWaterMacroMapper::revision.data(),
          "explicit adoption maps legacy state");
    const auto macrosBeforeCalibration = session.draft().macroMappings;
    session.restoreListeningCalibration();
    check(ResearchListeningCalibration::matches(session.draft().engineering) &&
              session.draft().macroMappings == macrosBeforeCalibration &&
              session.draft().listeningCalibration == MappingStatus::mapped,
          "restoring calibration changes only its four gain destinations");
    std::cout << "session tests failures=" << failures << '\n';
    return failures;
}
