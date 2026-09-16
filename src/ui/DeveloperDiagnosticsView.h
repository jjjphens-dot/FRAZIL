#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace frazil::plugin {
struct DeveloperDiagnosticsSnapshot;
}

namespace frazil::ui {

// Editor-owned, message-thread presentation of an existing latest-block snapshot.
// Receives values only; never accesses a processor, APVTS or diagnostics transport.
class DeveloperDiagnosticsView final : public juce::Component {
  public:
    DeveloperDiagnosticsView();
    void update(const plugin::DeveloperDiagnosticsSnapshot&, const juce::String& routing);
    void resized() override;

  private:
    juce::Label summaryLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeveloperDiagnosticsView)
};

} // namespace frazil::ui
