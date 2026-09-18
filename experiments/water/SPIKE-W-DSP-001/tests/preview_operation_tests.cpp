#include "preview/ResearchOperationHistory.h"

#include <iostream>

int runOperationTests() {
    using namespace frazil::water::preview;
    int failures{};
    const auto check = [&](bool ok, const char* message) {
        if (!ok) {
            std::cerr << "FAIL operations: " << message << '\n';
            ++failures;
        }
    };
    ResearchSessionModel model;
    ResearchOperations operations(model);
    int stops{}, completions{}, macroCompletions{};
    operations.onPrepareBegin = [&] { ++stops; };
    operations.onCompleted = [&](bool macro) {
        ++completions;
        macroCompletions += macro;
    };
    const auto origin = ChangeOrigin::soundLeadUI;
    operations.begin("Size", origin, true, true, true, 0);
    for (int i = 1; i <= 100; ++i) {
        operations.begin("Size", origin, true, true, false, i);
        model.setMacro(MacroId::size, .5 + .003 * i, origin);
        operations.tick(i + 1000); // Mouse gestures never expire on a debounce timer.
    }
    check(operations.history().size() == 0 && stops == 1,
          "100 callbacks stop once, no interim operation");
    operations.finish();
    check(operations.history().size() == 1 && completions == 1 && macroCompletions == 1 &&
              operations.history().at(0).before.water.size == .5 &&
              operations.history().at(0).after.water.size == .8,
          "one physical drag records its endpoints");
    operations.begin("Size", origin, true, true, true, 2000);
    model.setMacro(MacroId::size, .3, origin);
    model.setMacro(MacroId::size, .8, origin);
    operations.finish();
    check(operations.history().size() == 1, "drag returning to start records nothing");
    for (int i = 0; i < 20; ++i) {
        operations.begin("Motion", origin, true, true, false, 3000 + i * 50);
        model.setMacro(MacroId::motion, .01 * i, origin);
        operations.tick(3000 + i * 50 + 49);
    }
    check(operations.history().size() == 1, "wheel stream remains pending");
    operations.tick(4199);
    check(operations.history().size() == 1, "249ms does not flush");
    operations.tick(4200);
    check(operations.history().size() == 2, "250ms stable commits one wheel operation");
    operations.begin("Motion", origin, true, true, false, 4500);
    model.setMacro(MacroId::motion, .7, origin);
    operations.tick(4750);
    check(operations.history().size() == 3, "separated wheel burst commits separately");
    operations.begin("Size", origin, true, true, false, 5000);
    model.setMacro(MacroId::size, .2, origin);
    operations.begin("Decay", origin, true, true, false, 5050);
    check(operations.history().size() == 4, "changing control flushes immediately");
    operations.finish();
    const auto stopBeforeLive = stops;
    int targets{};
    model.onChange = [&] { ++targets; };
    operations.begin("protect.depth", origin, false, false, true, 6000);
    for (int i = 1; i <= 100; ++i) {
        operations.begin("protect.depth", origin, false, false, false, 6000 + i);
        model.setProtect(ProtectId::depth, i * .008, origin);
        check(model.applied().engineering.protect.depth == i * .008,
              "live applied target advances");
    }
    operations.finish();
    check(stops == stopBeforeLive && targets == 100 && operations.history().size() == 5,
          "live Protect sends intermediate targets without stop and records once");
    auto imported = model.draft();
    imported.water.size = .9;
    imported.water.motion = .9;
    imported.water.decay = .9;
    imported.engineering.values[0] = 400;
    operations.action("Import Session", ChangeOrigin::sessionLoad, true, false,
                      [&] { model.restoreValidated(imported); });
    check(operations.history().size() == 6 && operations.history().at(5).type == "Action",
          "composite import records once");
    const auto count = operations.history().size();
    model.applyValidated();
    model.capture(0);
    operations.tick(99999);
    check(operations.history().size() == count, "automatic publication and polling do not record");
    for (int i = 0; i < 51; ++i)
        operations.action("Motion", origin, false, false,
                          [&] { model.setMacro(MacroId::motion, i % 2 ? .2 : .3, origin); });
    check(operations.history().size() == 50 && operations.history().sequence() == 57 &&
              operations.history().at(0).sequence == 8 &&
              operations.history().at(49).sequence == 57,
          "ring retains only newest 50 complete operations with monotonic sequence");
    ResearchSessionModel delayedModel;
    ResearchOperations delayed(delayedModel);
    delayed.begin("Size", origin, true, true, false, 0);
    delayedModel.setMacro(MacroId::size, .3, origin);
    delayed.begin("Size", origin, true, true, false, 300);
    delayedModel.setMacro(MacroId::size, .4, origin);
    delayed.finish();
    check(delayed.history().size() == 2, "late UI timer cannot merge bursts separated by debounce");
    return failures;
}
