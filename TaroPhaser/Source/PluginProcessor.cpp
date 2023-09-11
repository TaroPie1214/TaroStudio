/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroPhaserAudioProcessor::TaroPhaserAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

TaroPhaserAudioProcessor::~TaroPhaserAudioProcessor()
{
}

//==============================================================================
const juce::String TaroPhaserAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TaroPhaserAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TaroPhaserAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TaroPhaserAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TaroPhaserAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TaroPhaserAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TaroPhaserAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TaroPhaserAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TaroPhaserAudioProcessor::getProgramName (int index)
{
    return {};
}

void TaroPhaserAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TaroPhaserAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    spec.sampleRate = sampleRate;

    phaser.prepare(spec);
    phaser.reset();
}

void TaroPhaserAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TaroPhaserAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TaroPhaserAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    phaser.setRate(*apvts.getRawParameterValue("Rate"));
    phaser.setDepth(*apvts.getRawParameterValue("Depth"));
    phaser.setCentreFrequency(*apvts.getRawParameterValue("CentreFrequency"));
    phaser.setFeedback(*apvts.getRawParameterValue("Feedback"));
    phaser.setMix(*apvts.getRawParameterValue("Mix"));

    juce::dsp::AudioBlock<float> sampleBlock(buffer);
    phaser.process(juce::dsp::ProcessContextReplacing<float>(sampleBlock));

    float gainInDecibels = *apvts.getRawParameterValue("Gain");
    sampleBlock.multiplyBy(juce::Decibels::decibelsToGain(gainInDecibels));
}

//==============================================================================
bool TaroPhaserAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TaroPhaserAudioProcessor::createEditor()
{
    //return new TaroPhaserAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TaroPhaserAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    apvts.state.writeToStream(stream);
}

void TaroPhaserAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);

    if (tree.isValid()) {
        apvts.state = tree;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout TaroPhaserAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    using namespace juce;

    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Rate", 1 },
        "Rate",
        NormalisableRange<float>(0.0f, 99.f, 0.1f, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Depth", 1 },
        "Depth",
        NormalisableRange<float>(0.0f, 1.f, 0.01f, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "CentreFrequency", 1 },
        "CentreFrequency",
        NormalisableRange<float>(0.0f, 20000.f, 1.f, 1), 5000));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Feedback", 1 },
        "Feedback",
        NormalisableRange<float>(-1.f, 1.f, 0.01f, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Mix", 1 },
        "Mix",
        NormalisableRange<float>(0.0f, 1.f, 0.01f, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Gain", 1 },
        "Gain(dB)",
        NormalisableRange<float>(-10.0f, 10.f, 0.1f, 1), 0));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TaroPhaserAudioProcessor();
}
