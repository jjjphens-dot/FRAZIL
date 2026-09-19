#pragma once
#include "DiagnosticMonitor.h"
#include "PreviewSettings.h"
#include "dsp/primitives/LinearSmoother.h"
namespace frazil::water::preview {
// Audio-owner monitor only. Each signal weight ramps independently in 10 ms, including
// switching between diagnostic sources. No selection changes DSP state or callback lifecycle.
class AuditionMonitor final {
  public:
    void prepare(double rate, MonitorMode mode, float trimGain, bool excitation = false) noexcept {
        prepareDiagnostics(rate, mode, trimGain,
                           excitation ? DiagnosticSignal::modalDriver : DiagnosticSignal::none);
    }
    void setTargets(MonitorMode mode, float outputGain, float trimGain,
                    bool excitation = false) noexcept {
        setDiagnosticTargets(mode, outputGain, trimGain,
                             excitation ? DiagnosticSignal::modalDriver : DiagnosticSignal::none);
    }
    void prepareDiagnostics(double rate, MonitorMode mode, float trimGain,
                            DiagnosticSignal selection) noexcept {
        for (std::size_t i = 0; i < diagnostic_.size(); ++i) {
            diagnostic_[i].prepare(rate, .01);
            const auto signal = static_cast<DiagnosticSignal>(i);
            diagnostic_[i].reset(selection != DiagnosticSignal::none && signal == selection
                                     ? (isDriver(signal) ? 1.f : trimGain)
                                     : 0.f);
        }
        carrier_.prepare(rate, .01);
        effect_.prepare(rate, .01);
        output_.prepare(rate, .01);
        carrier_.reset(selection != DiagnosticSignal::none || mode == MonitorMode::residual ? 0.f
                                                                                            : 1.f);
        effect_.reset(selection != DiagnosticSignal::none || mode == MonitorMode::dry ? 0.f
                                                                                      : trimGain);
        output_.reset(0);
    }
    void setDiagnosticTargets(MonitorMode mode, float outputGain, float trimGain,
                              DiagnosticSignal selection) noexcept {
        for (std::size_t i = 0; i < diagnostic_.size(); ++i) {
            const auto signal = static_cast<DiagnosticSignal>(i);
            diagnostic_[i].setTarget(selection != DiagnosticSignal::none && signal == selection
                                         ? (isDriver(signal) ? 1.f : trimGain)
                                         : 0.f);
        }
        carrier_.setTarget(
            selection != DiagnosticSignal::none || mode == MonitorMode::residual ? 0.f : 1.f);
        effect_.setTarget(
            selection != DiagnosticSignal::none || mode == MonitorMode::dry ? 0.f : trimGain);
        output_.setTarget(outputGain);
    }
    research::StereoFrame process(const research::StereoFrame& source,
                                  const research::StereoFrame& protectedE,
                                  const research::StereoFrame& excitation = {}) noexcept {
        DiagnosticFrames signals;
        signals.at(DiagnosticSignal::modalDriver) = excitation;
        return processDiagnostics(source, protectedE, signals);
    }
    research::StereoFrame processDiagnostics(const research::StereoFrame& source,
                                             const research::StereoFrame& protectedE,
                                             const DiagnosticFrames& signals) noexcept {
        const float x = carrier_.getNextValue(), e = effect_.getNextValue(),
                    g = output_.getNextValue();
        research::StereoFrame result{x * source[0] + e * protectedE[0],
                                     x * source[1] + e * protectedE[1]};
        for (std::size_t i = 0; i < diagnostic_.size(); ++i) {
            const float weight = diagnostic_[i].getNextValue();
            for (std::size_t c = 0; c < result.size(); ++c)
                result[c] += weight * signals.signals[i][c];
        }
        for (auto& value : result)
            value *= g;
        return result;
    }

  private:
    LinearSmoother carrier_, effect_, output_;
    std::array<LinearSmoother, static_cast<std::size_t>(DiagnosticSignal::count)> diagnostic_;
};
} // namespace frazil::water::preview
