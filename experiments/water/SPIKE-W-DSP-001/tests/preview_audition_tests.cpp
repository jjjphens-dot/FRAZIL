#include "preview/AuditionMonitor.h"
#include "preview/MonitorOverRange.h"
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
    MonitorOverRange warning;
    warning.observe(1.f);
    check(!warning.consume(), "exact full scale is not over-range");
    warning.observe(std::nextafter(1.f, 2.f));
    warning.observe(.1f);
    check(warning.consume() && !warning.consume(),
          "brief over-range survives quiet blocks and consumes once");
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
    // The actual driver replaces source/E, without E Trim or a DSP reset; ramp both directions.
    AuditionMonitor driverMonitor;
    driverMonitor.prepare(48000, MonitorMode::processed, 64);
    driverMonitor.setTargets(MonitorMode::processed, .125f, 64, true);
    frazil::water::research::StereoFrame driverOutput{};
    for (int i = 0; i < 600; ++i)
        driverOutput = driverMonitor.process({.2f, 0}, {.01f, 0}, {.4f, 0});
    check(std::abs(driverOutput[0] - .05f) < 1e-6f && driverOutput[1] == 0,
          "actual driver only, excludes E trim");
    driverMonitor.setTargets(MonitorMode::dry, .125f, 64, false);
    for (int i = 0; i < 600; ++i)
        driverOutput = driverMonitor.process({.2f, 0}, {.01f, 0}, {.4f, 0});
    check(std::abs(driverOutput[0] - .025f) < 1e-6f, "return to normal monitor");
    // Every diagnostic source has its own ramp, so changing between solos cannot step.
    DiagnosticFrames signals;
    for (std::size_t i = 1; i < signals.signals.size(); ++i)
        signals.signals[i] = {static_cast<float>(i) * .01f, 0};
    AuditionMonitor diagnostic;
    diagnostic.prepareDiagnostics(48000, MonitorMode::processed, 2, DiagnosticSignal::none);
    for (std::size_t i = 1; i < signals.signals.size(); ++i) {
        const auto selection = static_cast<DiagnosticSignal>(i);
        diagnostic.setDiagnosticTargets(MonitorMode::processed, 1, 2, selection);
        frazil::water::research::StereoFrame output{};
        for (int n = 0; n < 600; ++n)
            output = diagnostic.processDiagnostics({.1f, 0}, {.02f, 0}, signals);
        check(std::abs(output[0] - signals.signals[i][0] * (isDriver(selection) ? 1.f : 2.f)) <
                      1e-6 &&
                  output[1] == 0,
              "solo uses E trim; driver bypasses trim; no dry/protected-E leak");
    }
    diagnostic.setDiagnosticTargets(MonitorMode::processed, 1, 2, DiagnosticSignal::bubble);
    float last = .07f;
    for (int i = 0; i < 480; ++i) {
        const auto output = diagnostic.processDiagnostics({.1f, 0}, {.02f, 0}, signals);
        check(std::abs(output[0] - last) < .001f, "diagnostic-to-diagnostic 10ms ramp");
        last = output[0];
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
