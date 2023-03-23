/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectrumAnalyzerTestAudioProcessor::SpectrumAnalyzerTestAudioProcessor()
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

SpectrumAnalyzerTestAudioProcessor::~SpectrumAnalyzerTestAudioProcessor()
{
}

//==============================================================================
const juce::String SpectrumAnalyzerTestAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectrumAnalyzerTestAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumAnalyzerTestAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumAnalyzerTestAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectrumAnalyzerTestAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectrumAnalyzerTestAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectrumAnalyzerTestAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectrumAnalyzerTestAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpectrumAnalyzerTestAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpectrumAnalyzerTestAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpectrumAnalyzerTestAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SpectrumAnalyzerTestAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectrumAnalyzerTestAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SpectrumAnalyzerTestAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (copyToFifo) pushNextSampleToFifo(buffer, 0, 2, abstractFifoInput, audioFifoInput);

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool SpectrumAnalyzerTestAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpectrumAnalyzerTestAudioProcessor::createEditor()
{
    return new SpectrumAnalyzerTestAudioProcessorEditor (*this);
}

//==============================================================================
void SpectrumAnalyzerTestAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpectrumAnalyzerTestAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void SpectrumAnalyzerTestAudioProcessor::setCopyToFifo(bool _copyToFifo)
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

void SpectrumAnalyzerTestAudioProcessor::pushNextSampleToFifo(const juce::AudioBuffer<float>& buffer, int startChannel, int numChannels,
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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrumAnalyzerTestAudioProcessor();
}
