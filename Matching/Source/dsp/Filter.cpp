/*
  ==============================================================================

    Filter.cpp
    Created: 8 Feb 2023 2:51:49pm
    Author:  香芋派Taro

  ==============================================================================
*/

#include "Filter.h"

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

    settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5f;
    settings.peakBypassed = apvts.getRawParameterValue("Peak Bypassed")->load() > 0.5f;
    settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5f;

    return settings;
}

Coefficients FilterSettings::makePeakFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                               chainSettings.peakFreq,
                                                               chainSettings.peakQuality,
                                                               juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
}

void updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}

void FilterSettings::updateHighCutFilters(const ChainSettings &chainSettings, double sampleRate, MonoChain& leftChain, MonoChain& rightChain)
{
    auto highCutCoefficients = makeHighCutFilter(chainSettings, sampleRate);
    
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
    
    leftChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    rightChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    
    updateCutFilter(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
    updateCutFilter(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
}

void FilterSettings::updateLowCutFilters(const ChainSettings &chainSettings, double sampleRate, MonoChain& leftChain, MonoChain& rightChain)
{
    auto cutCoefficients = makeLowCutFilter(chainSettings, sampleRate);
    
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    
    leftChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    rightChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    
    updateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
}

void FilterSettings::updatePeakFilter(const ChainSettings &chainSettings, double sampleRate, MonoChain& leftChain, MonoChain& rightChain)
{
    auto peakCoefficients = makePeakFilter(chainSettings, sampleRate);

    leftChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    rightChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);

    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
//    leftChain.get<ChainPositions::Peak>().coefficients = peakCoefficients;
//    rightChain.get<ChainPositions::Peak>().coefficients = peakCoefficients;
}

void FilterSettings::updateFilters(ChainSettings chainSettings, double sampleRate, MonoChain& leftChain, MonoChain& rightChain)
{
//    auto chainSettings = getChainSettings(apvts);
    
    updateLowCutFilters(chainSettings, sampleRate, leftChain, rightChain);
    updatePeakFilter(chainSettings, sampleRate, leftChain, rightChain);
    updateHighCutFilters(chainSettings, sampleRate, leftChain, rightChain);
}
