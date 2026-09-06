#include "app/AudioEngine.h"

#include <array>
#include <cmath>
#include <iostream>

namespace
{
int failures = 0;

void expect(bool condition, const char* description)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

void testAudioEnginePreservesInput()
{
    AudioEngine engine;
    engine.prepare(48000.0, 64, 2);

    juce::AudioBuffer<float> buffer(2, 4);
    buffer.copyFrom(0, 0, std::array<float, 4> { 0.25f, -0.5f, 0.75f, -1.0f }.data(), 4);
    buffer.copyFrom(1, 0, std::array<float, 4> { -0.125f, 0.25f, -0.375f, 0.5f }.data(), 4);

    const auto leftBefore = buffer.getSample(0, 2);
    const auto rightBefore = buffer.getSample(1, 3);
    engine.process(buffer);

    expect(std::abs(buffer.getSample(0, 2) - leftBefore) < 1.0e-6f, "left channel remains unchanged");
    expect(std::abs(buffer.getSample(1, 3) - rightBefore) < 1.0e-6f, "right channel remains unchanged");
}

void testAudioEngineResetIsSafe()
{
    AudioEngine engine;
    engine.prepare(44100.0, 32, 1);
    engine.reset();

    juce::AudioBuffer<float> buffer(1, 0);
    engine.process(buffer);
    expect(true, "zero-length buffer is accepted by the M0 pass-through engine");
}
}

int main()
{
    testAudioEnginePreservesInput();
    testAudioEngineResetIsSafe();

    if (failures != 0)
        return 1;

    std::cout << "FRAZIL unit tests passed (2 cases)\n";
    return 0;
}
