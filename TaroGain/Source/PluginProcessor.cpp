/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroGainAudioProcessor::TaroGainAudioProcessor()
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

TaroGainAudioProcessor::~TaroGainAudioProcessor()
{
}

//==============================================================================
const juce::String TaroGainAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TaroGainAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TaroGainAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TaroGainAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TaroGainAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TaroGainAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TaroGainAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TaroGainAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TaroGainAudioProcessor::getProgramName (int index)
{
    return {};
}

void TaroGainAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TaroGainAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // 方式4
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    spec.sampleRate = sampleRate;

    gain.prepare(spec);
    gain.reset();
}

void TaroGainAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TaroGainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TaroGainAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // 禁用浮点数的非规格化处理
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float currentGain = *apvts.getRawParameterValue("Gain");

    // 方式1
    /*for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			channelData[sample] *= juce::Decibels::decibelsToGain(currentGain);
		}
    }*/

    // 方式2
    /*juce::dsp::AudioBlock<float> block(buffer);
    for (int channel = 0; channel < block.getNumChannels(); ++channel)
    {
		auto* channelData = block.getChannelPointer(channel);
        for (int sample = 0; sample < block.getNumSamples(); ++sample)
        {
			channelData[sample] *= juce::Decibels::decibelsToGain(currentGain);
		}
	}*/

    // 方式3
    //buffer.applyGain(juce::Decibels::decibelsToGain(currentGain));
    
    // 方式4
    juce::dsp::AudioBlock<float> block(buffer);
    gain.setGainDecibels(currentGain);
    gain.process(juce::dsp::ProcessContextReplacing<float>(block));
}

//==============================================================================
bool TaroGainAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TaroGainAudioProcessor::createEditor()
{
    //return new TaroGainAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TaroGainAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    apvts.state.writeToStream(stream);
}

void TaroGainAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);
    
    if (tree.isValid()) {
        apvts.state = tree;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TaroGainAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout TaroGainAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    using namespace juce;
    
    layout.add(std::make_unique<AudioParameterFloat>("Gain",
                                                     "Gain(dB)",
                                                     NormalisableRange<float>(-10.f, 10.f, 0.1f, 1), 0.f));
    
    return layout;
}
