/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroStereoWidthAudioProcessor::TaroStereoWidthAudioProcessor()
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
    gain = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Gain"));
    width = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Width"));
}

TaroStereoWidthAudioProcessor::~TaroStereoWidthAudioProcessor()
{
}

//==============================================================================
const juce::String TaroStereoWidthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TaroStereoWidthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TaroStereoWidthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TaroStereoWidthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TaroStereoWidthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TaroStereoWidthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TaroStereoWidthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TaroStereoWidthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TaroStereoWidthAudioProcessor::getProgramName (int index)
{
    return {};
}

void TaroStereoWidthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TaroStereoWidthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void TaroStereoWidthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TaroStereoWidthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TaroStereoWidthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    auto currentGain = apvts.getRawParameterValue("Gain")->load();
    auto currentWidth = apvts.getRawParameterValue("Width")->load();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block {buffer};
    
    for (int sample = 0; sample < block.getNumSamples(); ++sample)
    {
        // Encoding，通过左右声道得到sides和mid
        auto* channelDataLeft = buffer.getWritePointer(0);
        auto* channelDataRight = buffer.getWritePointer(1);
        
        auto sides = 0.5 * (channelDataLeft[sample] - channelDataRight[sample]);
        auto mid = 0.5 * (channelDataLeft[sample] + channelDataRight[sample]);
        
        // Processing，width的范围是0～2
        sides = currentWidth * sides;
        mid = (2 - currentWidth) * mid;
        
        // Decoding
        channelDataLeft[sample] = (mid + sides) * currentGain;
        channelDataRight[sample] = (mid - sides) * currentGain;
    }
}

//==============================================================================
bool TaroStereoWidthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TaroStereoWidthAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
    //return new TaroStereoWidthAudioProcessorEditor (*this);
}

//==============================================================================
void TaroStereoWidthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    apvts.state.writeToStream(stream);
}

void TaroStereoWidthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);
        
    if (tree.isValid()) {
        apvts.state = tree;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout TaroStereoWidthAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    
    using namespace juce;
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { "Gain", 1 },
                                                     "Gain",
                                                     NormalisableRange<float>(0, 2, 0.1, 1), 1));
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { "Width", 1 },
                                                     "Width",
                                                     NormalisableRange<float>(0, 2, 0.1, 1), 1));
    
    return layout;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TaroStereoWidthAudioProcessor();
}
