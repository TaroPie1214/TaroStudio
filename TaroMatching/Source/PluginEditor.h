/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/SpectrumAnalyzer.h"
#include "Components/Frame.h"
#include "Components/Label.h"
#include "Components/RotarySlider.h"

//==============================================================================
/**
*/
class TaroMatchingAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TaroMatchingAudioProcessorEditor (TaroMatchingAudioProcessor&);
    ~TaroMatchingAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TaroMatchingAudioProcessor& audioProcessor;

    Frame frame;
    SpectrumAnalyzer spectrumAnalyzer;

    RotarySlider lowShelfFreq,
        lowShelfQuality,
        lowShelfGain,
        highShelfFreq,
        highShelfQuality,
        highShelfGain,
        peakFreq1,
        peakQuality1,
        peakGain1,
        peakFreq2,
        peakQuality2,
        peakGain2;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    Attachment lowShelfFreqAttachment,
        lowShelfQualityAttachment,
        lowShelfGainAttachment,
        highShelfFreqAttachment,
        highShelfQualityAttachment,
        highShelfGainAttachment,
        peakFreqAttachment1,
        peakQualityAttachment1,
        peakGainAttachment1,
        peakFreqAttachment2,
        peakQualityAttachment2,
        peakGainAttachment2;

    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TaroMatchingAudioProcessorEditor)
};
