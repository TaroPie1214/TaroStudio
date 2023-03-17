/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MatchingAudioProcessor::MatchingAudioProcessor()
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

MatchingAudioProcessor::~MatchingAudioProcessor()
{
}

//==============================================================================
const juce::String MatchingAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MatchingAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MatchingAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MatchingAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MatchingAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MatchingAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MatchingAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MatchingAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MatchingAudioProcessor::getProgramName (int index)
{
    return {};
}

void MatchingAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MatchingAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // 使用dsp module后的常规套路
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    // 分别进行prepare
    leftChain.prepare(spec);
    rightChain.prepare(spec);

    filterSettings.updateFilters(getChainSettings(apvts), getSampleRate(), leftChain, rightChain);

    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);
}

void MatchingAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MatchingAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MatchingAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    filterSettings.updateFilters(getChainSettings(apvts), getSampleRate(), leftChain, rightChain);

    // 以一个buffer创建一个block
    juce::dsp::AudioBlock<float> block(buffer);

    //for (int channel = 0; channel < totalNumInputChannels; ++channel)
    //{
    //    auto* channelData = buffer.getWritePointer (channel);

    //    // ..do something to the data...
    //}
    // 从block中分别提取左右声道
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    // 从block中将待处理的内容更新到Context中
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    // 对内容进行处理
    leftChain.process(leftContext);
    rightChain.process(rightContext);

    //
    leftChannelFifo.update(buffer);
    rightChannelFifo.update(buffer);
}

//==============================================================================
bool MatchingAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MatchingAudioProcessor::createEditor()
{
    return new MatchingAudioProcessorEditor (*this);
}

//==============================================================================
void MatchingAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void MatchingAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        filterSettings.updateFilters(getChainSettings(apvts), getSampleRate(), leftChain, rightChain);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout MatchingAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    using namespace juce;

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterID{ "LowCut Freq", 1 },
        "LowCut Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        20.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterID{ "HighCut Freq", 1 },
        "HighCut Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        20000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterID{ "Peak Freq", 1 },
        "Peak Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        750.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterID{ "Peak Gain", 1 },
        "Peak Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterID{ "Peak Quality", 1 },
        "Peak Quality",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
        1.f));

    StringArray stringArray;
    for (int i = 0; i < 4; ++i)
    {
        String str;
        str << (12 + i * 12);
        str << " db/Oct";
        stringArray.add(str);
    }

    layout.add(std::make_unique<juce::AudioParameterChoice>(ParameterID{ "LowCut Slope", 1 }, "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParameterID{ "HighCut Slope", 1 }, "HighCut Slope", stringArray, 0));

    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterID{ "LowCut Bypassed", 1 }, "LowCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterID{ "Peak Bypassed", 1 }, "Peak Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterID{ "HighCut Bypassed", 1 }, "HighCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterID{ "Analyzer Enabled", 1 }, "Analyzer Enabled", true));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MatchingAudioProcessor();
}
