#pragma once

#include "PreviewSettings.h"
#include "ProtectDiagnostics.h"
#include "SessionMetadata.h"
#include "plugin/DeveloperDiagnostics.h"

#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>

namespace frazil::water::preview {

// Narrow application interface. UI submits values/commands; this owner alone manages devices,
// source buffers, DSP lifecycle and audio-thread state. Public calls are message-thread only.
class PreviewController final {
  public:
    PreviewController();
    ~PreviewController();
    juce::String load(const juce::File& wav);
    juce::String play(const PreviewSettings&);
    // Transaction owner stops once before prepare. Successful prepare validates the actual
    // processing engine; startPrepared attaches the callback without a second prepare/stop.
    juce::String prepareStopped(const PreviewSettings&);
    juce::String startPrepared();
    void stop();
    juce::String validate(const PreviewSettings&) const;
    // Candidate imports/recalls use their recorded rate, independent of the loaded device/source.
    juce::String validate(const PreviewSettings&, double sampleRate) const;
    void setMonitor(MonitorMode, float outputGainDb) noexcept;
    void setAuditionTrim(double decibels) noexcept;
    void setProtectDepth(double depth) noexcept;
    plugin::DeveloperDiagnosticsSnapshot diagnostics() const noexcept;
    ProtectDiagnosticsSnapshot protectDiagnostics() noexcept;
    juce::String sourceDescription() const;
    SourceMetadata sourceMetadata() const;
    double positionSeconds() const noexcept;
    bool playing() const noexcept;
    bool finished() const noexcept;
    bool deviceRateMismatch() const noexcept;
    double sampleRate() const noexcept;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace frazil::water::preview
