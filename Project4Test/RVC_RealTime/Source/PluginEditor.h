/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class RVC_RealTimeAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RVC_RealTimeAudioProcessorEditor (RVC_RealTimeAudioProcessor&);
    ~RVC_RealTimeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RVC_RealTimeAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RVC_RealTimeAudioProcessorEditor)
};
