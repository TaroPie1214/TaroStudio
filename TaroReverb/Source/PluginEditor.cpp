/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
TaroReverbAudioProcessorEditor::TaroReverbAudioProcessorEditor (TaroReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    roomSizeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    roomSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, true, 100, 30);
    addAndMakeVisible (roomSizeSlider);
    roomSizeSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "Room Size", roomSizeSlider);
    
    dampingSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxRight, true, 100, 30);
    addAndMakeVisible (dampingSlider);
    dampingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "Damping", dampingSlider);
    
    widthSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, true, 100, 30);
    addAndMakeVisible (widthSlider);
    widthSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "Width", widthSlider);
    
    dryWetSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxRight, true, 100, 30);
    addAndMakeVisible (dryWetSlider);
    dryWetSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "Dry/Wet", dryWetSlider);
    
    
    
    setSize (400, 300);
    
    sizeLabel.setText ("Size", juce::NotificationType::dontSendNotification);
    sizeLabel.attachToComponent (&roomSizeSlider, false);
    
    dampLabel.setText ("Damp", juce::NotificationType::dontSendNotification);
    dampLabel.attachToComponent (&dampingSlider, false);
    
    widthLabel.setText ("Width", juce::NotificationType::dontSendNotification);
    widthLabel.attachToComponent (&widthSlider, false);

    dwLabel.setText ("Dry/Wet", juce::NotificationType::dontSendNotification);
    dwLabel.attachToComponent (&dryWetSlider, false);
}

TaroReverbAudioProcessorEditor::~TaroReverbAudioProcessorEditor()
{
}

//==============================================================================
void TaroReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);
    g.drawText ("roomSize", 0, getHeight() / 5 - 30, 50, 60, juce::Justification::centred, true);
}

void TaroReverbAudioProcessorEditor::resized()
{
    roomSizeSlider.setBounds (getWidth() / 2 - 150, getHeight() / 5 - 30, 350, 60);
    dampingSlider.setBounds (getWidth() / 2 - 150, getHeight() / 5 + 30, 350, 60);
    widthSlider.setBounds (getWidth() / 2 - 150, getHeight() / 5 + 90, 350, 60);
    dryWetSlider.setBounds (getWidth() / 2 - 150, getHeight() / 5 + 150, 350, 60);
}
