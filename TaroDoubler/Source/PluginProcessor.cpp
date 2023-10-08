/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroDoublerAudioProcessor::TaroDoublerAudioProcessor()
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

TaroDoublerAudioProcessor::~TaroDoublerAudioProcessor()
{
}

//==============================================================================
const juce::String TaroDoublerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TaroDoublerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TaroDoublerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TaroDoublerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TaroDoublerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TaroDoublerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TaroDoublerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TaroDoublerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TaroDoublerAudioProcessor::getProgramName (int index)
{
    return {};
}

void TaroDoublerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TaroDoublerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumInputChannels();
    spec.maximumBlockSize = samplesPerBlock;

    delayLine1.reset();
    delayLine1.prepare(spec);
    delayLine1.setDelay(24000);
    delayLine2.reset();
    delayLine2.prepare(spec);
    delayLine2.setDelay(24000);
    delayLine3.reset();
    delayLine3.prepare(spec);
    delayLine3.setDelay(24000);
    delayLine4.reset();
    delayLine4.prepare(spec);
    delayLine4.setDelay(24000);
    delayLine5.reset();
    delayLine5.prepare(spec);
    delayLine5.setDelay(24000);

    mySampleRate = sampleRate;

    delayTime.reset(sampleRate, samplesPerBlock / sampleRate);
}

void TaroDoublerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TaroDoublerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TaroDoublerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    delayTime.setTargetValue(apvts.getRawParameterValue("Time")->load());
    // 通过秒数和采样率得出延迟多少sample
    int delayTimeInSamples = delayTime.getNextValue() * mySampleRate;
    delayLine1.setDelay(delayTimeInSamples);
    delayLine2.setDelay(delayTimeInSamples + apvts.getRawParameterValue("Offset1")->load());
    delayLine3.setDelay(delayTimeInSamples + apvts.getRawParameterValue("Offset2")->load());
    delayLine4.setDelay(delayTimeInSamples + apvts.getRawParameterValue("Offset3")->load());
    delayLine5.setDelay(delayTimeInSamples + apvts.getRawParameterValue("Offset4")->load());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* inSamples = buffer.getReadPointer(channel);
        auto* outSamples = buffer.getWritePointer(channel);

        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            // 从当前DelayLine上拿出一个sample
            float delayedSample1 = delayLine1.popSample(channel);
            float delayedSample2 = delayLine2.popSample(channel);
            float delayedSample3 = delayLine3.popSample(channel);
            float delayedSample4 = delayLine4.popSample(channel);
            float delayedSample5 = delayLine5.popSample(channel);
            // 放回DelayLine上的buffer大小的sample
            float inDelay1 = inSamples[i] + (apvts.getRawParameterValue("Feedback")->load() * delayedSample1);
            float inDelay2 = inSamples[i] + (apvts.getRawParameterValue("Feedback")->load() * delayedSample2);
            float inDelay3 = inSamples[i] + (apvts.getRawParameterValue("Feedback")->load() * delayedSample3);
            float inDelay4 = inSamples[i] + (apvts.getRawParameterValue("Feedback")->load() * delayedSample4);
            float inDelay5 = inSamples[i] + (apvts.getRawParameterValue("Feedback")->load() * delayedSample5);
            // 存入inDelay
            delayLine1.pushSample(channel, inDelay1);
            delayLine2.pushSample(channel, inDelay2);
            delayLine3.pushSample(channel, inDelay3);
            delayLine4.pushSample(channel, inDelay4);
            delayLine5.pushSample(channel, inDelay5);

            // 最后的输出
            outSamples[i] = inSamples[i] + delayedSample1 + delayedSample2 + delayedSample3 + delayedSample4 + delayedSample5;
        }
    }
    buffer.applyGain(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("Gain")->load()));
}

//==============================================================================
bool TaroDoublerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TaroDoublerAudioProcessor::createEditor()
{
    //return new TaroDoublerAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TaroDoublerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    apvts.state.writeToStream(stream);
}

void TaroDoublerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);

    if (tree.isValid()) {
        apvts.state = tree;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout TaroDoublerAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    using namespace juce;

    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Time", 1 },
        "Time",
        NormalisableRange<float>(0.01f, 0.5f, 0.01f, 1.f),
        0.05f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Feedback", 1 },
        "Feedback",
        0.01f,
        0.9f,
        0.25f));

    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Offset1", 1 },
        "Offset1",
        -1000.f,
        1000.f,
        -250.f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Offset2", 1 },
        "Offset2",
        -1000.f,
        1000.f,
        250.f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Offset3", 1 },
        "Offset3",
        -1000.f,
        1000.f,
        -500.f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Offset4", 1 },
        "Offset4",
        -1000.f,
        1000.f,
        500.f));

    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Gain", 1 },
        "Gain(dB)",
        -10.f,
        10.f,
        0.f));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TaroDoublerAudioProcessor();
}
