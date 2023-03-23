/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/SpectrumAnalyzer.h"
//==============================================================================
/**
*/
class SpectrumAnalyzerTestAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SpectrumAnalyzerTestAudioProcessorEditor (SpectrumAnalyzerTestAudioProcessor&);
    ~SpectrumAnalyzerTestAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SpectrumAnalyzerTestAudioProcessor& audioProcessor;

    SpectrumAnalyzer analyzer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzerTestAudioProcessorEditor)
};
