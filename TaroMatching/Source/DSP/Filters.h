/*
  ==============================================================================

    Filter.h
    Created: 23 Mar 2023 2:43:36pm
    Author:  sunwei06

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//#include "../PluginProcessor.h"

struct ChainSettings
{
    float lowShelfFreq{ 0 }, highShelfFreq{ 0 }, peakFreq1{ 0 }, peakFreq2{ 0 };
    float lowShelfQuality{ 0.71f }, highShelfQuality{ 0.71f }, peakQuality1{ 0.71f }, peakQuality2{ 0.71f };
    float lowShelfDecibels{ 0 }, highShelfDecibels{ 0 }, peakDecibels1{ 0 }, peakDecibels2{ 0 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

class Filters
{
public:
    Filters(juce::AudioProcessorValueTreeState&);
    void prepare(juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::AudioBlock<float>& block);
    
private:
    juce::AudioProcessorValueTreeState& myapvts;

    double sampleRate = 22050;

    using Filter = juce::dsp::IIR::Filter<float>;
    using MonoChain = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    MonoChain leftChain, rightChain;

    enum ChainPositions
    {
        LowShelf,
        HighShelf,
        Peak1,
        Peak2
    };

    void updateLowShelfFilter(const ChainSettings& chainSettings);
    void updateHighShelfFilter(const ChainSettings& chainSettings);
    void updatePeakFilter1(const ChainSettings& chainSettings);
    void updatePeakFilter2(const ChainSettings& chainSettings);
    void updateFilters();
};