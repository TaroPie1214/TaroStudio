/*
  ==============================================================================

    Filter.cpp
    Created: 23 Mar 2023 2:43:36pm
    Author:  sunwei06

  ==============================================================================
*/

#include "Filters.h"

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.lowShelfFreq = apvts.getRawParameterValue("LowShelf Freq")->load();
    settings.lowShelfQuality = apvts.getRawParameterValue("LowShelf Quality")->load();
    settings.lowShelfDecibels = apvts.getRawParameterValue("LowShelf Gain")->load();

    settings.highShelfFreq = apvts.getRawParameterValue("HighShelf Freq")->load();
    settings.highShelfQuality = apvts.getRawParameterValue("HighShelf Quality")->load();
    settings.highShelfDecibels = apvts.getRawParameterValue("HighShelf Gain")->load();

    settings.peakFreq1 = apvts.getRawParameterValue("Peak Freq1")->load();
    settings.peakQuality1 = apvts.getRawParameterValue("Peak Quality1")->load();
    settings.peakDecibels1 = apvts.getRawParameterValue("Peak Gain1")->load();

    settings.peakFreq2 = apvts.getRawParameterValue("Peak Freq2")->load();
    settings.peakQuality2 = apvts.getRawParameterValue("Peak Quality2")->load();
    settings.peakDecibels2 = apvts.getRawParameterValue("Peak Gain2")->load();

    return settings;
}

Filters::Filters(juce::AudioProcessorValueTreeState& apvts) : myapvts{ apvts } {}

void Filters::updateLowShelfFilter(const ChainSettings& chainSettings)
{
    auto lowShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, chainSettings.lowShelfFreq, chainSettings.lowShelfQuality, juce::Decibels::decibelsToGain(chainSettings.lowShelfDecibels));

    *leftChain.get<ChainPositions::LowShelf>().coefficients = *lowShelfCoefficients;
    *rightChain.get<ChainPositions::LowShelf>().coefficients = *lowShelfCoefficients;
}

void Filters::updateHighShelfFilter(const ChainSettings& chainSettings)
{
    auto highShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, chainSettings.highShelfFreq, chainSettings.highShelfQuality, juce::Decibels::decibelsToGain(chainSettings.highShelfDecibels));

    *leftChain.get<ChainPositions::HighShelf>().coefficients = *highShelfCoefficients;
    *rightChain.get<ChainPositions::HighShelf>().coefficients = *highShelfCoefficients;
}

void Filters::updatePeakFilter1(const ChainSettings& chainSettings)
{
    auto peakCoefficients1 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq1, chainSettings.peakQuality1, juce::Decibels::decibelsToGain(chainSettings.peakDecibels1));

    *leftChain.get<ChainPositions::Peak1>().coefficients = *peakCoefficients1;
    *rightChain.get<ChainPositions::Peak1>().coefficients = *peakCoefficients1;
}

void Filters::updatePeakFilter2(const ChainSettings& chainSettings)
{
    auto peakCoefficients2 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq2, chainSettings.peakQuality2, juce::Decibels::decibelsToGain(chainSettings.peakDecibels2));

    *leftChain.get<ChainPositions::Peak2>().coefficients = *peakCoefficients2;
    *rightChain.get<ChainPositions::Peak2>().coefficients = *peakCoefficients2;
}

void Filters::updateFilters()
{
    auto chainSettings = getChainSettings(myapvts);

    updateLowShelfFilter(chainSettings);
    updateHighShelfFilter(chainSettings);
    updatePeakFilter1(chainSettings);
    updatePeakFilter2(chainSettings);
}

void Filters::prepare(juce::dsp::ProcessSpec& spec)
{
    leftChain.prepare(spec);
    rightChain.prepare(spec);

    updateFilters();
}

void Filters::process(juce::dsp::AudioBlock<float>& block)
{
    updateFilters();

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
}