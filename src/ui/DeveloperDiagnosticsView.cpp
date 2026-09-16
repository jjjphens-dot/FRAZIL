#include "DeveloperDiagnosticsView.h"

#include "plugin/DeveloperDiagnostics.h"

#include <array>

namespace frazil::ui {
namespace {
const auto kMutedText = juce::Colour(0xff9db4bc);
const auto kAccent = juce::Colour(0xff5ed0ba);
const auto kWarning = juce::Colour(0xffffab70);
constexpr int kScaleHeight = 20;
} // namespace

DeveloperDiagnosticsView::DeveloperDiagnosticsView() {
    for (auto* label :
         {&headingLabel_, &runtimeLabel_, &routingLabel_, &finiteLabel_, &levelsLabel_}) {
        label->setFont(juce::FontOptions(11.0f));
        label->setColour(juce::Label::textColourId, kMutedText);
        label->setBorderSize(juce::BorderSize<int>(0));
        addAndMakeVisible(*label);
    }
    headingLabel_.setText("RUNTIME", juce::dontSendNotification);
    headingLabel_.setColour(juce::Label::textColourId, kAccent);
    headingLabel_.setFont(juce::FontOptions(12.0f));
    finiteLabel_.setJustificationType(juce::Justification::centredRight);
    levelsLabel_.setText("LEVELS  /  RMS fill  |  Peak line", juce::dontSendNotification);
    levelsLabel_.setTooltip("Aggregate channels; latest observed block only. No peak hold.");
    routingLabel_.setTooltip("Current Host routing snapshot; developer override may differ.");
    runtimeLabel_.setTooltip("Block: latest / prepared maximum, in samples.");
    addAndMakeVisible(inputMeter_);
    addAndMakeVisible(outputMeter_);
    update({}, "Parallel");
}

void DeveloperDiagnosticsView::update(const plugin::DeveloperDiagnosticsSnapshot& diagnostics,
                                      const juce::String& routing) {
    runtimeLabel_.setText(juce::String(diagnostics.sampleRateHz / 1000.0f, 1) + " kHz | block " +
                              juce::String(diagnostics.latestBlockSize) + " / " +
                              juce::String(diagnostics.preparedBlockSize) + " | " +
                              juce::String(diagnostics.channelCount) + " ch",
                          juce::dontSendNotification);
    routingLabel_.setText("Host: " + routing, juce::dontSendNotification);
    finiteLabel_.setText(diagnostics.finite ? "FINITE OK" : "FINITE NO",
                         juce::dontSendNotification);
    finiteLabel_.setColour(juce::Label::textColourId, diagnostics.finite ? kAccent : kWarning);
    inputMeter_.setLevels(diagnostics.inputPeak, diagnostics.inputRms);
    outputMeter_.setLevels(diagnostics.outputPeak, diagnostics.outputRms);
}

void DeveloperDiagnosticsView::paint(juce::Graphics& graphics) {
    const auto scale = getLocalBounds().removeFromBottom(kScaleHeight);
    graphics.setFont(juce::FontOptions(10.0f));
    graphics.setColour(kMutedText);
    constexpr std::array<int, 6> kTicks{-60, -36, -24, -12, -6, 0};
    for (const auto tick : kTicks) {
        const auto x =
            static_cast<int>(DeveloperLevelMeter::positionForDecibels(static_cast<float>(tick)) *
                             static_cast<float>(scale.getWidth() - 1));
        graphics.drawVerticalLine(x, static_cast<float>(scale.getY()),
                                  static_cast<float>(scale.getY() + 3));
        const auto labelX = juce::jlimit(0, scale.getWidth() - 24, x - 12);
        graphics.drawText(juce::String(tick), labelX, scale.getY() + 3, 24, 16,
                          juce::Justification::centred);
    }
}

void DeveloperDiagnosticsView::resized() {
    auto area = getLocalBounds();
    headingLabel_.setBounds(area.removeFromTop(18));
    runtimeLabel_.setBounds(area.removeFromTop(16));
    auto health = area.removeFromTop(18);
    finiteLabel_.setBounds(health.removeFromRight(86));
    routingLabel_.setBounds(health);
    levelsLabel_.setBounds(area.removeFromTop(18));
    area.removeFromBottom(kScaleHeight);
    inputMeter_.setBounds(area.removeFromTop(area.getHeight() / 2));
    outputMeter_.setBounds(area);
}

} // namespace frazil::ui
