/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroNoiseGateAudioProcessorEditor::TaroNoiseGateAudioProcessorEditor (TaroNoiseGateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);
    addAndMakeVisible (gainSlider);
    
    gainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "Gain", gainSlider);
}

TaroNoiseGateAudioProcessorEditor::~TaroNoiseGateAudioProcessorEditor()
{
}

//==============================================================================
void TaroNoiseGateAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void TaroNoiseGateAudioProcessorEditor::resized()
{
    gainSlider.setBounds (getWidth() / 2 - 100, getHeight() / 2 - 50, 200, 100);
}
