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
    check(session.draft().engineering.moduleJson() == baselineConfig &&
              WaterMacroMapper::targets(MacroId::size) == MappingStatus::unmapped,
          "unmapped macro cannot invent DSP curve");
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
    std::cout << "session tests failures=" << failures << '\n';
    return failures;
}
