#pragma once

#include "PreviewBuildInfo.h"

#include <juce_core/juce_core.h>

namespace frazil::water::preview {
// Message-thread-only JSONL writer. No global logger and no callback access.
// Failure is visible through status(); logging failure does not mutate DSP state.
class PreviewSessionLogger final {
  public:
    PreviewSessionLogger() {
        const auto directory =
            juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("FRAZIL/Logs/WaterPreview");
        const auto result = directory.createDirectory();
        if (result.failed()) {
            error_ = result.getErrorMessage();
            return;
        }
        file_ = directory.getNonexistentChildFile(
            "session-" + juce::Time::getCurrentTime().formatted("%Y%m%d-%H%M%S"), ".jsonl");
        stream_ = file_.createOutputStream();
        if (!stream_ || stream_->failedToOpen()) {
            error_ = "Cannot open Preview session log";
            return;
        }
        auto* fields = new juce::DynamicObject;
        fields->setProperty("build", kPreviewBuildVariant);
        fields->setProperty("revision", kPreviewGitCommit);
        fields->setProperty("gitState", kPreviewGitState);
        fields->setProperty("os", juce::SystemStats::getOperatingSystemName());
        fields->setProperty("juce", juce::SystemStats::getJUCEVersion());
        write("app_start", juce::var(fields));
    }
    void write(const juce::String& event, juce::var fields = {}) {
        if (!stream_ || error_.isNotEmpty())
            return;
        auto* record = new juce::DynamicObject;
        record->setProperty("event", event);
        record->setProperty("utc", juce::Time::getCurrentTime().toISO8601(true));
        record->setProperty("fields", fields);
        stream_->writeText(juce::JSON::toString(juce::var(record), true) + "\n", false, false,
                           "\n");
        stream_->flush();
        if (stream_->getStatus().failed())
            error_ = stream_->getStatus().getErrorMessage();
    }
    juce::String status() const {
        return error_.isEmpty() ? file_.getFullPathName() : error_;
    }

  private:
    juce::File file_;
    std::unique_ptr<juce::FileOutputStream> stream_;
    juce::String error_;
};
} // namespace frazil::water::preview
