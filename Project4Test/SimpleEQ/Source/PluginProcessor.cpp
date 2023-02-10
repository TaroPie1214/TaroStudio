/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
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

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
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
    
    osc.initialise([](float x) { return std::sin(x); });
    
    spec.numChannels = getTotalNumOutputChannels();
    osc.prepare(spec);
    osc.setFrequency(440);
}

void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (//layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    filterSettings.updateFilters(getChainSettings(apvts), getSampleRate(), leftChain, rightChain);
    
    // 以一个buffer创建一个block
    juce::dsp::AudioBlock<float> block(buffer);
    
//    buffer.clear();
//
//    for( int i = 0; i < buffer.getNumSamples(); ++i )
//    {
//        buffer.setSample(0, i, osc.processSample(0));
//    }
//
//    juce::dsp::ProcessContextReplacing<float> stereoContext(block);
//    osc.process(stereoContext);
    
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
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
    return new SimpleEQAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if( tree.isValid() )
    {
        apvts.replaceState(tree);
        filterSettings.updateFilters(getChainSettings(apvts), getSampleRate(), leftChain, rightChain);
    }
}

// getChainSettings的定义，用于在被调用时从apvts中获取所有参数
//ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
//{
//    ChainSettings settings;
//
//    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
//    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
//    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
//    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
//    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
//    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
//    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());
//
//    settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5f;
//    settings.peakBypassed = apvts.getRawParameterValue("Peak Bypassed")->load() > 0.5f;
//    settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5f;
//
//    return settings;
//}

//Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate)
//{
//    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
//                                                               chainSettings.peakFreq,
//                                                               chainSettings.peakQuality,
//                                                               juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
//}

//void SimpleEQAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings)
//{
//    auto peakCoefficients = makePeakFilter(chainSettings, getSampleRate());
//
//    leftChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
//    rightChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
//
//    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
//    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
//}

//void updateCoefficients(Coefficients &old, const Coefficients &replacements)
//{
//    *old = *replacements;
//}

//void SimpleEQAudioProcessor::updateLowCutFilters(const ChainSettings &chainSettings)
//{
//    auto cutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());
//    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
//    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
//
//    leftChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
//    rightChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
//
//    updateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
//    updateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
//}
//
//void SimpleEQAudioProcessor::updateHighCutFilters(const ChainSettings &chainSettings)
//{
//    auto highCutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());
//
//    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
//    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
//
//    leftChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
//    rightChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
//
//    updateCutFilter(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
//    updateCutFilter(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
//}

// 此时已经将apvts中的参数放置到ChainSettings中，然后把里面的参数告诉各个filter
// 再调用各个filter的update函数，解析chainSettings中的内容并应用
//void SimpleEQAudioProcessor::updateFilters()
//{
//    auto chainSettings = getChainSettings(apvts);
//
//    updateLowCutFilters(chainSettings);
//    updatePeakFilter(chainSettings);
//    updateHighCutFilters(chainSettings);
//}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    using namespace juce;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterID { "LowCut Freq", 1 },
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterID { "HighCut Freq", 1 },
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20000.f));
    
        layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterID { "Peak Freq", 1 },
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           750.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterID { "Peak Gain", 1 },
                                                           "Peak Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterID { "Peak Quality", 1 },
                                                           "Peak Quality",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));
    
    StringArray stringArray;
    for( int i = 0; i < 4; ++i )
    {
        String str;
        str << (12 + i*12);
        str << " db/Oct";
        stringArray.add(str);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParameterID { "LowCut Slope", 1 }, "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParameterID { "HighCut Slope", 1 }, "HighCut Slope", stringArray, 0));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterID { "LowCut Bypassed", 1 }, "LowCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterID { "Peak Bypassed", 1 }, "Peak Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterID { "HighCut Bypassed", 1 }, "HighCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterID { "Analyzer Enabled", 1 }, "Analyzer Enabled", true));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}
