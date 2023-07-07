/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/SpectrumComponent.h"

//==============================================================================
/**
*/
class SpectrumAnalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                              public juce::Timer          
{
public:
    SpectrumAnalyzerAudioProcessorEditor (SpectrumAnalyzerAudioProcessor&);
    ~SpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SpectrumAnalyzerAudioProcessor& audioProcessor;

    SpectrumComponent spectrum;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzerAudioProcessorEditor)
};
