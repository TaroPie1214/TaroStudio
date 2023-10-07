/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroChorusAudioProcessor::TaroChorusAudioProcessor()
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

TaroChorusAudioProcessor::~TaroChorusAudioProcessor()
{
}

//==============================================================================
const juce::String TaroChorusAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TaroChorusAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TaroChorusAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TaroChorusAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TaroChorusAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TaroChorusAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TaroChorusAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TaroChorusAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TaroChorusAudioProcessor::getProgramName (int index)
{
    return {};
}

void TaroChorusAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TaroChorusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    spec.sampleRate = sampleRate;
    
    chorus.prepare(spec);
    chorus.reset();
}

void TaroChorusAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TaroChorusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TaroChorusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    chorus.setRate(*apvts.getRawParameterValue("Rate"));
    chorus.setDepth(*apvts.getRawParameterValue("Depth"));
    chorus.setCentreDelay(*apvts.getRawParameterValue("CentreDelay"));
    chorus.setFeedback(*apvts.getRawParameterValue("Feedback"));
    chorus.setMix(*apvts.getRawParameterValue("Mix"));

    juce::dsp::AudioBlock<float> sampleBlock (buffer);
    chorus.process(juce::dsp::ProcessContextReplacing<float>(sampleBlock));

    float gainInDecibels = *apvts.getRawParameterValue("Gain");
    sampleBlock.multiplyBy(juce::Decibels::decibelsToGain(gainInDecibels));
}

//==============================================================================
bool TaroChorusAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TaroChorusAudioProcessor::createEditor()
{
//    return new TaroChorusAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void TaroChorusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    apvts.state.writeToStream(stream);
}

void TaroChorusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);
        
    if (tree.isValid()) {
        apvts.state = tree;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout TaroChorusAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    using namespace juce;
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { "Rate", 1 },
                                                     "Rate",
                                                     NormalisableRange<float>(0.0f, 99.f, 0.1f, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { "Depth", 1 },
                                                     "Depth",
                                                     NormalisableRange<float>(0.0f, 1.f, 0.01f, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { "CentreDelay", 1 },
                                                     "CentreDelay",
                                                     NormalisableRange<float>(1.f, 100.f, 0.1f, 1), 1));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { "Feedback", 1 },
                                                     "Feedback",
                                                     NormalisableRange<float>(-1.f, 1.f, 0.01f, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { "Mix", 1 },
                                                     "Mix",
                                                     NormalisableRange<float>(0.0f, 1.f, 0.01f, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Gain", 1 },
                                                     "Gain(dB)",
                                                     NormalisableRange<float>(-10.f, 10.f, 0.1f, 1), 0));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TaroChorusAudioProcessor();
}
