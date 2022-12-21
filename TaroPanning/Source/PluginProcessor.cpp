/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroPanningAudioProcessor::TaroPanningAudioProcessor()
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

TaroPanningAudioProcessor::~TaroPanningAudioProcessor()
{
}

//==============================================================================
const juce::String TaroPanningAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TaroPanningAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TaroPanningAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TaroPanningAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TaroPanningAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TaroPanningAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TaroPanningAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TaroPanningAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TaroPanningAudioProcessor::getProgramName (int index)
{
    return {};
}

void TaroPanningAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TaroPanningAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void TaroPanningAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TaroPanningAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TaroPanningAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto currentRate = apvts.getRawParameterValue("Rate")->load();
    auto currentPan = apvts.getRawParameterValue("Pan")->load();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block {buffer};

    // 定义周期
    float periodicity = 1 / currentRate;
    // 定义采样率
    int sampleRate = getSampleRate();
    // 开始循环遍历block里所有的sample
    for (int sample = 0; sample < block.getNumSamples(); ++sample)
    {
        // 分离左右声道
        auto* channelDataLeft = buffer.getWritePointer(0);
        auto* channelDataRight = buffer.getWritePointer(1);
            
        // 当前时刻，通过已读取的采样数除以采样率得到
        float currentTime = static_cast<float>(sampleCount) / static_cast<float>(sampleRate);
        
        auto panValue = currentPan * std::sin(juce::MathConstants<float>::twoPi * currentRate * currentTime);
        
        auto x = panValue / 200. + 0.5;
        
        auto rightAmp = sqrt(x);
        auto leftAmp = sqrt(1 - x);
        
        // 应用到sample上
        channelDataLeft[sample] *= leftAmp;
        channelDataRight[sample] *= rightAmp;
        
        // 如果当前的sampleCount小于一个周期，那么sampleCount自增
        if (currentTime < periodicity)
        {
            sampleCount++;
        }
        // 如果等于或大于了，则让sampleCount归零
        // 此举是为了限制sampleCount的大小，总不能无限大吧，在一个周期里循环就行了
        else
        {
            sampleCount = 0;
        }
    }
}

//==============================================================================
bool TaroPanningAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TaroPanningAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
    //return new TaroPanningAudioProcessorEditor (*this);
}

//==============================================================================
void TaroPanningAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    apvts.state.writeToStream(stream);
}

void TaroPanningAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);
            
    if (tree.isValid()) {
        apvts.state = tree;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout TaroPanningAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    
    using namespace juce;
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { "Rate", 1 },
                                                     "Rate",
                                                     NormalisableRange<float>(0, 20, 1, 1), 5));
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { "Pan", 1 },
                                                     "Pan",
                                                     NormalisableRange<float>(-100, 100, 1, 1), 0));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TaroPanningAudioProcessor();
}
