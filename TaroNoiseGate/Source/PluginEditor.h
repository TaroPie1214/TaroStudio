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
class TaroNoiseGateAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TaroNoiseGateAudioProcessorEditor (TaroNoiseGateAudioProcessor&);
    ~TaroNoiseGateAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider gainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainSliderAttachment;
    
    TaroNoiseGateAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TaroNoiseGateAudioProcessorEditor)
};
