#include "PluginProcessor.h"
#include "PluginEditor.h"

FRAZILAudioProcessor::FRAZILAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "FRAZIL", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout FRAZILAudioProcessor::createParameterLayout()
{
    using Range = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterBool>("water.enable", "Water Enable", true));
    layout.add(std::make_unique<juce::AudioParameterBool>("ice.enable", "Ice Enable", true));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "routing.mode", "Routing Mode", juce::StringArray { "Parallel", "Water -> Ice", "Ice -> Water" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "parallel.balance", "Parallel Balance", Range { 0.0f, 1.0f, 0.001f }, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "water.amount", "Water Amount", Range { 0.0f, 1.0f, 0.001f }, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "ice.amount", "Ice Amount", Range { 0.0f, 1.0f, 0.001f }, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "input.gain", "Input Gain", Range { -24.0f, 24.0f, 0.01f }, 0.0f, "dB"));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "global.mix", "Global Mix", Range { 0.0f, 1.0f, 0.001f }, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "output.gain", "Output Gain", Range { -24.0f, 24.0f, 0.01f }, 0.0f, "dB"));

    return layout;
}

void FRAZILAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    audioEngine.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

void FRAZILAudioProcessor::releaseResources()
{
    audioEngine.reset();
}

bool FRAZILAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();

    if (input != output)
        return false;

    return input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo();
}

void FRAZILAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    audioEngine.process(buffer);
}

juce::AudioProcessorEditor* FRAZILAudioProcessor::createEditor()
{
    return new FRAZILAudioProcessorEditor(*this);
}

bool FRAZILAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String FRAZILAudioProcessor::getName() const
{
    return "FRAZIL";
}

bool FRAZILAudioProcessor::acceptsMidi() const
{
    return false;
}

bool FRAZILAudioProcessor::producesMidi() const
{
    return false;
}

bool FRAZILAudioProcessor::isMidiEffect() const
{
    return false;
}

double FRAZILAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FRAZILAudioProcessor::getNumPrograms()
{
    return 1;
}

int FRAZILAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FRAZILAudioProcessor::setCurrentProgram(int)
{
}

const juce::String FRAZILAudioProcessor::getProgramName(int)
{
    return {};
}

void FRAZILAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void FRAZILAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData)
{
    const auto state = parameters.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destinationData);
}

void FRAZILAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        const auto state = juce::ValueTree::fromXml(*xml);
        if (state.isValid() && state.hasType(parameters.state.getType()))
            parameters.replaceState(state);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FRAZILAudioProcessor();
}
