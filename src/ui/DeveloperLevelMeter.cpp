#include "DeveloperLevelMeter.h"

#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>
#include <limits>

namespace frazil::ui {
namespace {
const auto kBackground = juce::Colour(0xff101820);
const auto kBorder = juce::Colour(0xff2d4654);
const auto kText = juce::Colour(0xffe8f0f2);
const auto kMutedText = juce::Colour(0xff9db4bc);
const auto kAccent = juce::Colour(0xff5ed0ba);
const auto kWarning = juce::Colour(0xffffab70);

bool isValidAmplitude(float amplitude) noexcept {
    return std::isfinite(amplitude) && amplitude >= 0.0f;
}

float decibelsForAmplitude(float amplitude) noexcept {
    // Preserve quiet and above-full-scale numeric values; the drawing alone has a floor.
    return juce::Decibels::gainToDecibels(amplitude, -std::numeric_limits<float>::infinity());
}

juce::String amplitudeText(float amplitude) {
    if (!isValidAmplitude(amplitude))
        return "INVALID";
    if (amplitude == 0.0f)
        return "-inf";
    const auto decibels = decibelsForAmplitude(amplitude);
    return (decibels > 0.0f ? "+" : "") + juce::String(decibels, 1);
}
} // namespace

DeveloperLevelMeter::DeveloperLevelMeter(const juce::String& name) : name_(name) {
    setTitle(name + " aggregate latest-block level");
    setDescription("RMS fill and current Peak line. dBFS, no peak hold or channel separation.");
}

void DeveloperLevelMeter::setLevels(float peakAmplitude, float rmsAmplitude) {
    peakAmplitude_ = peakAmplitude;
    rmsAmplitude_ = rmsAmplitude;
    repaint();
}

float DeveloperLevelMeter::positionForDecibels(float decibels) noexcept {
    if (std::isnan(decibels))
        return 0.0f;
    return juce::jlimit(0.0f, 1.0f, (decibels - kDisplayFloorDb) / -kDisplayFloorDb);
}

void DeveloperLevelMeter::paint(juce::Graphics& graphics) {
    auto area = getLocalBounds();
    auto heading = area.removeFromTop(16);
    const auto valid = isValidAmplitude(peakAmplitude_) && isValidAmplitude(rmsAmplitude_);
    const auto overRange = valid && (peakAmplitude_ > 1.0f || rmsAmplitude_ > 1.0f);
    graphics.setFont(juce::FontOptions(11.0f));
    graphics.setColour(kMutedText);
    graphics.drawText(name_, heading, juce::Justification::centredLeft);
    graphics.setColour(kWarning);
    graphics.drawText(!valid      ? "INVALID"
                      : overRange ? "OVER 0 dBFS"
                                  : "",
                      heading, juce::Justification::centredRight);

    const auto bar = area.removeFromTop(14).toFloat();
    graphics.setColour(kBackground);
    graphics.fillRect(bar);
    if (valid) {
        graphics.setColour(kAccent);
        graphics.fillRect(bar.withWidth(bar.getWidth() *
                                        positionForDecibels(decibelsForAmplitude(rmsAmplitude_))));
        graphics.setColour(overRange ? kWarning : kText);
        const auto markerX =
            bar.getX() +
            (bar.getWidth() - 2.0f) * positionForDecibels(decibelsForAmplitude(peakAmplitude_));
        graphics.fillRect(markerX, bar.getY(), 2.0f, bar.getHeight());
    }
    graphics.setColour(valid && !overRange ? kBorder : kWarning);
    graphics.drawRect(bar, 1.0f);

    auto numbers = area.removeFromTop(20);
    graphics.setColour(kText);
    graphics.drawText("Peak " + amplitudeText(peakAmplitude_),
                      numbers.removeFromLeft(numbers.getWidth() / 2),
                      juce::Justification::centredLeft);
    graphics.setColour(kAccent);
    graphics.drawText("RMS " + amplitudeText(rmsAmplitude_) + " dBFS", numbers,
                      juce::Justification::centredRight);
}

} // namespace frazil::ui
