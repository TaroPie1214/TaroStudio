/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroMatchingAudioProcessor::TaroMatchingAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), filters(apvts)
#endif
{
}

TaroMatchingAudioProcessor::~TaroMatchingAudioProcessor()
{
}

//==============================================================================
const juce::String TaroMatchingAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TaroMatchingAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TaroMatchingAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TaroMatchingAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TaroMatchingAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TaroMatchingAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TaroMatchingAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TaroMatchingAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TaroMatchingAudioProcessor::getProgramName (int index)
{
    return {};
}

void TaroMatchingAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TaroMatchingAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    filters.prepare(spec);
}

void TaroMatchingAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TaroMatchingAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TaroMatchingAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (copyToFifo) pushNextSampleToFifo(buffer, 0, 2, abstractFifoInput, audioFifoInput);

    juce::dsp::AudioBlock<float> block(buffer);

    filters.process(block);

    if (copyToFifo) pushNextSampleToFifo(buffer, 0, 2, abstractFifoOutput, audioFifoOutput);
}

//==============================================================================
bool TaroMatchingAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TaroMatchingAudioProcessor::createEditor()
{
    return new TaroMatchingAudioProcessorEditor (*this);
}

//==============================================================================
void TaroMatchingAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TaroMatchingAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


void TaroMatchingAudioProcessor::setCopyToFifo(bool _copyToFifo)
{
    if (_copyToFifo)
    {
        abstractFifoInput.setTotalSize(int(getSampleRate()));
        abstractFifoOutput.setTotalSize(int(getSampleRate()));

        audioFifoInput.setSize(1, int(getSampleRate()));
        audioFifoOutput.setSize(1, int(getSampleRate()));

        abstractFifoInput.reset();
        abstractFifoOutput.reset();

        audioFifoInput.clear();
        audioFifoOutput.clear();
    }

    copyToFifo.store(_copyToFifo);
}

void TaroMatchingAudioProcessor::pushNextSampleToFifo(const juce::AudioBuffer<float>& buffer, int startChannel, int numChannels,
    juce::AbstractFifo& absFifo, juce::AudioBuffer<float>& fifo)
{
    if (absFifo.getFreeSpace() < buffer.getNumSamples()) return;

    int start1, block1, start2, block2;
    absFifo.prepareToWrite(buffer.getNumSamples(), start1, block1, start2, block2);
    fifo.copyFrom(0, start1, buffer.getReadPointer(startChannel), block1);

    if (block2 > 0)
        fifo.copyFrom(0, start2, buffer.getReadPointer(startChannel, block1), block2);

    for (int channel = startChannel + 1; channel < startChannel + numChannels; ++channel)
    {
        if (block1 > 0) fifo.addFrom(0, start1, buffer.getReadPointer(channel), block1);
        if (block2 > 0) fifo.addFrom(0, start2, buffer.getReadPointer(channel, block1), block2);
    }

    absFifo.finishedWrite(block1 + block2);
    nextFFTBlockReady.store(true);
}

juce::AudioProcessorValueTreeState::ParameterLayout TaroMatchingAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add (std::make_unique<juce::AudioParameterFloat> ("LowShelf Freq",
                                                             "LowShlef Freq",
                                                             juce::NormalisableRange<float> (20.f, 20000.f, 1.f, 0.25f),
                                                             100.f,
                                                             juce::String(),
                                                             juce::AudioProcessorParameter::genericParameter,
                                                             [](float value, int) { return (value < 1000.0f) ?
                                                                 juce::String (value, 0) + " Hz" :
                                                                 juce::String (value / 1000.0f, 1) + " kHz"; },
                                                             nullptr));    
    layout.add (std::make_unique<juce::AudioParameterFloat> ("LowShelf Quality",
                                                             "LowShelf Quality",
                                                             juce::NormalisableRange<float> (0.1f, 10.f, 0.01f, 0.25f),
                                                             0.71f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("LowShelf Gain",
                                                             "LowShelf Gain",
                                                             juce::NormalisableRange<float> (-10.f, 0.f, 0.1f, 1.f),
                                                             -2.f));   

    layout.add (std::make_unique<juce::AudioParameterFloat> ("HighShelf Freq",
                                                             "HighShelf Freq",
                                                             juce::NormalisableRange<float> (20.f, 20000.f, 1.f, 0.25f),
                                                             5000.f,
                                                             juce::String(),
                                                             juce::AudioProcessorParameter::genericParameter,
                                                             [](float value, int) { return (value < 1000.0f) ?
                                                                 juce::String (value, 0) + " Hz" :
                                                                 juce::String (value / 1000.0f, 1) + " kHz"; },
                                                             nullptr));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("HighShelf Quality",
                                                             "HighShelf Quality",
                                                             juce::NormalisableRange<float> (0.1f, 10.f, 0.01f, 0.25f),
                                                             0.71f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("HighShelf Gain",
                                                             "HighShelf Gain",
                                                             juce::NormalisableRange<float> (-10.f, 0.f, 0.1f, 1.f),
                                                             -2.f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("Peak Freq1",
                                                             "Peak Freq1",
                                                             juce::NormalisableRange<float> (20.f, 20000.f, 1.f, 0.25f),
                                                             500.f,
                                                             juce::String(),
                                                             juce::AudioProcessorParameter::genericParameter,
                                                             [](float value, int) { return (value < 1000.0f) ?
                                                                 juce::String (value, 0) + " Hz" :
                                                                 juce::String (value / 1000.0f, 1) + " kHz"; },
                                                             nullptr));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("Peak Quality1",
                                                             "Peak Quality1",
                                                             juce::NormalisableRange<float> (0.1f, 10.f, 0.01f, 0.25f),
                                                             0.71f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("Peak Gain1",
                                                             "Peak Gain1",
                                                             juce::NormalisableRange<float> (-10.f, 0.f, 0.1f, 1.f),
                                                             -2.f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("Peak Freq2",
                                                             "Peak Freq2",
                                                             juce::NormalisableRange<float> (20.f, 20000.f, 1.f, 0.25f),
                                                             2000.f,
                                                             juce::String(),
                                                             juce::AudioProcessorParameter::genericParameter,
                                                             [](float value, int) { return (value < 1000.0f) ?
                                                                 juce::String (value, 0) + " Hz" :
                                                                 juce::String (value / 1000.0f, 1) + " kHz"; },
                                                             nullptr));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("Peak Quality2",
                                                             "Peak Quality2",
                                                             juce::NormalisableRange<float> (0.1f, 10.f, 0.01f, 0.25f),
                                                             0.71f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("Peak Gain2",
                                                             "Peak Gain2",
                                                             juce::NormalisableRange<float> (-10.f, 0.f, 0.1f, 1.f),
                                                             -2.f));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TaroMatchingAudioProcessor();
}
