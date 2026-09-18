#include "preview/AuditionMonitor.h"
#include "preview/ResearchSessionModel.h"

#include <iostream>

int runAuditionTests() {
    using namespace frazil::water::preview;
    int failures{};
    const auto check = [&](bool ok, const char* message) {
        if (!ok) {
            ++failures;
            std::cerr << "FAIL audition: " << message << '\n';
        }
    };
    for (double rate : {44100., 48000., 96000.}) {
        AuditionMonitor monitor;
        monitor.prepare(rate, MonitorMode::processed, 1);
        for (auto mode : {MonitorMode::dry, MonitorMode::processed, MonitorMode::residual}) {
            for (double db : {0., 18., 36.}) {
                const float trim = static_cast<float>(std::pow(10., db / 20.));
                monitor.setTargets(mode, .125f, trim);
                frazil::water::research::StereoFrame y{};
                for (int i = 0; i < 1200; ++i)
                    y = monitor.process({.2f, 0}, {.01f, 0});
                const float expected = ((mode == MonitorMode::residual ? 0.f : .2f) +
                                        (mode == MonitorMode::dry ? 0.f : .01f * trim)) *
                                       .125f;
                check(std::abs(y[0] - expected) < 1e-6 && y[1] == 0,
                      "Source/Full/WaterOnly equation and isolated channel");
            }
        }
        monitor.prepare(rate, MonitorMode::residual, 1);
        monitor.setTargets(MonitorMode::residual, 1, 1);
        for (int i = 0; i < 1200; ++i)
            monitor.process({}, {.01f, 0});
        monitor.setTargets(MonitorMode::residual, 1, 8);
        float previous = .01f;
        for (int i = 0; i < static_cast<int>(rate * .01); ++i) {
            const auto y = monitor.process({}, {.01f, 0});
            check(y[0] >= previous && y[0] - previous < .001, "trim ramps without a step");
            previous = y[0];
        }
        check(std::abs(previous - .08f) < 1e-6, "trim reaches target in 10 ms");
    }
    ResearchSessionModel session;
    const auto config = session.draft().engineering.moduleJson();
    check(session.draft().auditionETrimDb == 18 && session.draft().monitorGainDb == -18 &&
              session.draft().engineering.protect.depth == 0,
          "new session Focus defaults with Protect OFF");
    session.setAuditionTrim(36);
    check(!session.dspDirty() && session.sessionDirty() &&
              session.applied().auditionETrimDb == 36 &&
              session.draft().engineering.moduleJson() == config,
          "trim live and excluded from DSP config");
    session.setAuditionTrim(37);
    check(session.applied().auditionETrimDb == 36, "invalid trim cannot publish");
    session.applyValidated();
    session.capture(0);
    session.setAuditionTrim(0);
    session.restoreValidated(*session.slot(0));
    check(session.applied().auditionETrimDb == 36, "A/B retains trim");
    return failures;
}
