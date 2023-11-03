/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroDelayAudioProcessor::TaroDelayAudioProcessor()
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

TaroDelayAudioProcessor::~TaroDelayAudioProcessor()
{
}

//==============================================================================
const juce::String TaroDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TaroDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TaroDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TaroDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TaroDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TaroDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TaroDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TaroDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TaroDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void TaroDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TaroDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumInputChannels();
    spec.maximumBlockSize = samplesPerBlock;

    delayLine.reset();
    delayLine.prepare(spec);
    delayLine.setDelay(24000);
}

void TaroDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TaroDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TaroDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 通过秒数和采样率得出延迟多少sample
    int delayTimeInSamples = *apvts.getRawParameterValue("Time") * getSampleRate();
    delayLine.setDelay(delayTimeInSamples);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* inSamples = buffer.getReadPointer(channel);
        auto* outSamples = buffer.getWritePointer(channel);

        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            // 拿
            float delayedSample = delayLine.popSample(channel);
            float inDelay = inSamples[i] + *apvts.getRawParameterValue("Feedback") * delayedSample;
            // 放
            delayLine.pushSample(channel, inDelay);
            outSamples[i] = inSamples[i] + delayedSample;
        }
    }
}

//==============================================================================
bool TaroDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TaroDelayAudioProcessor::createEditor()
{
    //return new TaroDelayAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TaroDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TaroDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout TaroDelayAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    using namespace juce;

    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Time", 1 },
        "Time",
        0.01f,
        0.9f,
        0.25f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Feedback", 1 },
        "Feedback",
        0.01f,
        0.9f,
        0.25f));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TaroDelayAudioProcessor();
}
