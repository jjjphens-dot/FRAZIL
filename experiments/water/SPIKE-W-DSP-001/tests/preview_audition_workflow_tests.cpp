#include "preview/ResearchAuditionWorkflow.h"
#include "preview/ResearchOperationHistory.h"

#include <iostream>

int runAuditionWorkflowTests() {
    using namespace frazil::water::preview;
    int failures{};
    const auto check = [&](bool ok, const char* message) {
        if (!ok) {
            ++failures;
            std::cerr << "FAIL audition workflow: " << message << '\n';
        }
    };
    ResearchSessionModel model;
    ResearchOperations operations(model);
    ResearchAuditionWorkflow workflow(model);
    int stop{}, prepare{}, start{};
    bool valid = true, source = true;
    workflow.stop = [&] { ++stop; };
    workflow.prepare = [&](const PreviewSettings&) {
        ++prepare;
        return valid ? juce::String{} : juce::String{"Invalid"};
    };
    workflow.start = [&] {
        ++start;
        return juce::String{};
    };
    workflow.sourceReady = [&] { return source; };
    workflow.status = [](const juce::String&) {};
    operations.onPrepareBegin = [&] { workflow.beginPrepare(); };
    operations.onCompleted = [&](bool macro) { workflow.completed(macro); };
    const auto sound = ChangeOrigin::soundLeadUI;
    operations.begin("Motion", sound, true, true, true, 0);
    for (int i = 1; i <= 100; ++i) {
        operations.begin("Motion", sound, true, true, false, i);
        model.setMacro(MacroId::motion, .5 + i * .004, sound);
    }
    check(stop == 1 && prepare == 0 && start == 0 && model.dspDirty(),
          "no interim prepare or restart");
    operations.finish();
    check(stop == 1 && prepare == 1 && start == 1 && !model.dspDirty() &&
              operations.history().size() == 1,
          "100 callbacks become one stop/prepare/apply/restart and one operation");
    operations.begin("Motion", sound, true, true, true, 1000);
    const double original = model.draft().water.motion;
    model.setMacro(MacroId::motion, .2, sound);
    model.setMacro(MacroId::motion, original, sound);
    operations.finish();
    check(stop == 2 && prepare == 2 && start == 2 && operations.history().size() == 1,
          "no-op gesture resumes audition without history");
    operations.action("Engineering", ChangeOrigin::engineeringUI, true, true,
                      [&] { model.setMacro(MacroId::size, .8, ChangeOrigin::engineeringUI); });
    check(stop == 3 && prepare == 2 && start == 2 && model.dspDirty(),
          "Engineering never auto applies/restarts");
    valid = false;
    const auto applied = model.applied().engineering.values;
    operations.action("Size", sound, true, true, [&] { model.setMacro(MacroId::size, .9, sound); });
    check(prepare == 3 && start == 2 && model.applied().engineering.values == applied,
          "invalid gesture preserves applied state and never restarts");
    valid = true;
    source = false;
    operations.action("Size", sound, true, true, [&] { model.setMacro(MacroId::size, .7, sound); });
    check(prepare == 4 && start == 2 && !model.dspDirty(),
          "missing source applies values without starting device");
    source = true;
    workflow.autoAudition = false;
    operations.action("Size", sound, true, true, [&] { model.setMacro(MacroId::size, .6, sound); });
    check(prepare == 4 && start == 2 && model.dspDirty(), "Auto Audition OFF retains draft");
    const auto beforeStop = stop;
    operations.action("Protect", sound, false, false,
                      [&] { model.setProtect(ProtectId::depth, .7, sound); });
    check(stop == beforeStop && prepare == 4 && start == 2,
          "live Protect never triggers audition lifecycle");
    return failures;
}
