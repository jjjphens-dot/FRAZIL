#pragma once

#include "../app/StateModel.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace frazil::plugin {

// Converts the JUCE/APVTS transport into the JUCE-free application state contract. All methods
// are control/message-boundary operations; none may be called from processBlock().
class HostStateAdapter final {
  public:
    static juce::ValueTree serialize(juce::AudioProcessorValueTreeState&);
    static StateModel::DeserializeResult deserialize(const juce::ValueTree&);
    // Returns false when safe defaults were applied because the input was invalid or incomplete.
    static bool restore(juce::AudioProcessorValueTreeState&, const juce::ValueTree&);
};

} // namespace frazil::plugin
