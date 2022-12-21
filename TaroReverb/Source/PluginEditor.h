/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "NameLabel.h"

//==============================================================================
/**
*/
class TaroReverbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TaroReverbAudioProcessorEditor (TaroReverbAudioProcessor&);
    ~TaroReverbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider roomSizeSlider;
    juce::Slider dampingSlider;
    juce::Slider widthSlider;
    juce::Slider dryWetSlider;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomSizeSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetSliderAttachment;
    
    NameLabel sizeLabel,
              dampLabel,
              widthLabel,
              dwLabel;
    
    TaroReverbAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TaroReverbAudioProcessorEditor)
};
