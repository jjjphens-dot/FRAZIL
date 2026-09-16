#include "DeveloperDiagnosticsView.h"

#include "plugin/DeveloperDiagnostics.h"

namespace frazil::ui {

DeveloperDiagnosticsView::DeveloperDiagnosticsView() {
    summaryLabel_.setText("RUNTIME DIAGNOSTICS", juce::dontSendNotification);
    summaryLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff5ed0ba));
    summaryLabel_.setFont(juce::FontOptions(12.0f));
    summaryLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(summaryLabel_);
}

void DeveloperDiagnosticsView::update(const plugin::DeveloperDiagnosticsSnapshot& diagnostics,
                                      const juce::String& routing) {
    summaryLabel_.setText(
        juce::String::formatted(
            "RUNTIME  %.1f kHz  |  prepared max %d  |  latest %d  |  %d ch  |  route %s\n"
            "Input  peak %.4f  RMS %.4f   |   Output  peak %.4f  RMS %.4f\n"
            "Finite: %s  |  Host snapshot is represented by the controls above",
            diagnostics.sampleRateHz / 1000.0f, diagnostics.preparedBlockSize,
            diagnostics.latestBlockSize, diagnostics.channelCount, routing.toRawUTF8(),
            diagnostics.inputPeak, diagnostics.inputRms, diagnostics.outputPeak,
            diagnostics.outputRms, diagnostics.finite ? "yes" : "NO"),
        juce::dontSendNotification);
}

void DeveloperDiagnosticsView::resized() {
    summaryLabel_.setBounds(getLocalBounds());
}

} // namespace frazil::ui
