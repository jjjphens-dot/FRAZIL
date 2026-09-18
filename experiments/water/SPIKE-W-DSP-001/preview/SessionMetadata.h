#pragma once

#include "PreviewBuildInfo.h"

#include <cstdint>
#include <juce_core/juce_core.h>

namespace frazil::water::preview {
struct SourceMetadata final {
    juce::String name; // Filename only; audio bytes and machine-specific paths are not embedded.
    double sampleRate{};
    int channels{};
    std::int64_t frames{};
    bool operator==(const SourceMetadata&) const = default;
};
struct BuildMetadata final {
    juce::String commit{kPreviewGitCommit}, state{kPreviewGitState}, variant{kPreviewBuildVariant},
        compiler{kPreviewCompiler};
};
} // namespace frazil::water::preview
