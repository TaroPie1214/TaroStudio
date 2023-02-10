/*
  ==============================================================================

    Filter.h
    Created: 8 Feb 2023 2:51:49pm
    Author:  香芋派Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// 定义滤波器
using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

enum ChainPositions
{
    LowCut,
    Peak,
    HighCut
};

using Coefficients = Filter::CoefficientsPtr;
void updateCoefficients(Coefficients& old, const Coefficients& replacements);

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

// 一个data structure代表着所有的参数，这将增加代码的可读性
struct ChainSettings
{
    float peakFreq { 0 }, peakGainInDecibels{ 0 }, peakQuality {1.f};
    float lowCutFreq { 0 }, highCutFreq { 0 };
    
    Slope lowCutSlope { Slope::Slope_12 }, highCutSlope { Slope::Slope_12 };
    
    bool lowCutBypassed { false }, peakBypassed { false }, highCutBypassed { false };
};

// 声明函数getChainSettings，用于在调用时从apvts中获取所有参数
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

template<int Index, typename ChainType, typename CoefficientType>
void update(ChainType& chain, const CoefficientType& coefficients)
{
    updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
    chain.template setBypassed<Index>(false);
}

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain,
                     const CoefficientType& coefficients,
                     const Slope& slope)
{
    chain.template setBypassed<0>(true);
    chain.template setBypassed<1>(true);
    chain.template setBypassed<2>(true);
    chain.template setBypassed<3>(true);
    
    switch( slope )
    {
        case Slope_48:
        {
            update<3>(chain, coefficients);
        }
        case Slope_36:
        {
            update<2>(chain, coefficients);
        }
        case Slope_24:
        {
            update<1>(chain, coefficients);
        }
        case Slope_12:
        {
            update<0>(chain, coefficients);
        }
    }
}

inline auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate )
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                       sampleRate,
                                                                                       2 * (chainSettings.lowCutSlope + 1));
}

inline auto makeHighCutFilter(const ChainSettings& chainSettings, double sampleRate )
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                                                                                      sampleRate,
                                                                                      2 * (chainSettings.highCutSlope + 1));
}

class FilterSettings
{
public:
//    FilterSettings(MonoChain leftChain, MonoChain rightChain);

    void updatePeakFilter(const ChainSettings& chainSettings, double sampleRate, MonoChain& leftChain, MonoChain& rightChain);
    void updateLowCutFilters(const ChainSettings& chainSettings, double sampleRate, MonoChain& leftChain, MonoChain& rightChain);
    void updateHighCutFilters(const ChainSettings& chainSettings, double sampleRate, MonoChain& leftChain, MonoChain& rightChain);
    
    void updateFilters(ChainSettings chainSettings, double sampleRate, MonoChain& leftChain, MonoChain& rightChain);
    
    Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate);
    
//    void updateCoefficients(Coefficients& old, const Coefficients& replacements);
private:
    // 初始化两个声道的Chain
//    MonoChain leftChain, rightChain;
};
