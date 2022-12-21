/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroGainAudioProcessorEditor::TaroGainAudioProcessorEditor (TaroGainAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);
    addAndMakeVisible (gainSlider);
    
    gainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "Gain", gainSlider);
    
    setSize (400, 300);
}

TaroGainAudioProcessorEditor::~TaroGainAudioProcessorEditor()
{
}

//==============================================================================
void TaroGainAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);
}

void TaroGainAudioProcessorEditor::resized()
{
    gainSlider.setBounds (getWidth() / 2 - 100, getHeight() / 2 - 50, 200, 100);
}
