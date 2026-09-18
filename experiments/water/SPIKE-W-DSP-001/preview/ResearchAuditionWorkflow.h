#pragma once

#include "ResearchSessionModel.h"

namespace frazil::water::preview {
// Message-thread transaction lifecycle. Injected narrow commands allow exact call-count tests
// without an audio device; production wiring uses the controller's detached prepare/start split.
class ResearchAuditionWorkflow final {
  public:
    explicit ResearchAuditionWorkflow(ResearchSessionModel& session) : session_(session) {}
    std::function<void()> stop;
    std::function<juce::String(const PreviewSettings&)> prepare;
    std::function<juce::String()> start;
    std::function<bool()> sourceReady;
    std::function<void(const juce::String&)> status;
    bool autoAudition{true};
    std::uint64_t stops{}, preparations{}, restarts{};

    void beginPrepare() {
        ++stops;
        stop();
    }
    bool apply(bool stopFirst = true) {
        if (stopFirst)
            beginPrepare();
        if (!validProtectMemory(session_.draft().protectMemory)) {
            status("Protect: retained Low must be less than High.");
            return false;
        }
        ++preparations;
        const auto error = prepare(session_.draft().engineering);
        if (error.isNotEmpty()) {
            status(error);
            return false;
        }
        session_.applyValidated();
        status("Applied research config. Play restarts the fixed seed.");
        return true;
    }
    void completed(bool soundLeadMacro) {
        if (!soundLeadMacro || !autoAudition || !apply(false))
            return;
        if (!sourceReady()) {
            status("Applied. Load the matching source WAV for Auto Audition.");
            return;
        }
        ++restarts;
        const auto error = start();
        status(error.isEmpty()
                   ? "AUTO AUDITION | applied once, restarted from source start / seed 42."
                   : error);
    }

  private:
    ResearchSessionModel& session_;
};
} // namespace frazil::water::preview
