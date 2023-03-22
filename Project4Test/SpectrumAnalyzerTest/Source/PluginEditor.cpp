/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectrumAnalyzerTestAudioProcessorEditor::SpectrumAnalyzerTestAudioProcessorEditor (SpectrumAnalyzerTestAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), analyzer(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(analyzer);
    p.setCopyToFifo(true);
    setSize (400, 300);
}

SpectrumAnalyzerTestAudioProcessorEditor::~SpectrumAnalyzerTestAudioProcessorEditor()
{
}

//==============================================================================
void SpectrumAnalyzerTestAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour{ 0xff000000 });
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    /*g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);*/
}

void SpectrumAnalyzerTestAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    analyzer.setBounds(0, 0, 400, 300);
}
