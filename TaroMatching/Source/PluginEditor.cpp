/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TaroMatchingAudioProcessorEditor::TaroMatchingAudioProcessorEditor (TaroMatchingAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    spectrumAnalyzer(p),
    frame(1000, 900),
    lowShelfFreqAttachment(audioProcessor.apvts, "LowShelf Freq", lowShelfFreq),
    lowShelfQualityAttachment(audioProcessor.apvts, "LowShelf Quality", lowShelfQuality),
    lowShelfGainAttachment(audioProcessor.apvts, "LowShelf Gain", lowShelfGain),
    highShelfFreqAttachment(audioProcessor.apvts, "HighShelf Freq", highShelfFreq),
    highShelfQualityAttachment(audioProcessor.apvts, "HighShelf Quality", highShelfQuality),
    highShelfGainAttachment(audioProcessor.apvts, "HighShelf Gain", highShelfGain),
    peakFreqAttachment1(audioProcessor.apvts, "Peak Freq1", peakFreq1),
    peakQualityAttachment1(audioProcessor.apvts, "Peak Quality1", peakQuality1),
    peakGainAttachment1(audioProcessor.apvts, "Peak Gain1", peakGain1),
    peakFreqAttachment2(audioProcessor.apvts, "Peak Freq2", peakFreq2),
    peakQualityAttachment2(audioProcessor.apvts, "Peak Quality2", peakQuality2),
    peakGainAttachment2(audioProcessor.apvts, "Peak Gain2", peakGain2)
{
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    
    p.setCopyToFifo(true);
    setSize (1000, 900);
}

TaroMatchingAudioProcessorEditor::~TaroMatchingAudioProcessorEditor()
{
}

//==============================================================================
void TaroMatchingAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void TaroMatchingAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto spectrumAnalyzerArea = bounds.removeFromTop(getHeight() * 0.75);
    spectrumAnalyzer.setBounds(spectrumAnalyzerArea);
    frame.setBounds(spectrumAnalyzerArea);

    auto lowArea = bounds.removeFromLeft(getWidth() * 0.25);
    auto peakArea1 = bounds.removeFromLeft(getWidth() * 0.33);
    auto peakArea2 = bounds.removeFromLeft(getWidth() * 0.5);
    auto highArea = bounds;

    lowShelfFreq.setBounds(lowArea.getX(), lowArea.getY(), 70, 70);
    lowShelfQuality.setBounds(lowArea.getX(), lowArea.getY() + 70, 70, 70);
    lowShelfGain.setBounds(lowArea.getX(), lowArea.getY() + 140, 70, 70);

    peakFreq1.setBounds(peakArea1.getX(), peakArea1.getY(), 70, 70);
    peakQuality1.setBounds(peakArea1.getX(), peakArea1.getY() + 70, 70, 70);
    peakGain1.setBounds(peakArea1.getX(), peakArea1.getY() + 140, 70, 70);

    peakFreq2.setBounds(peakArea2.getX(), peakArea2.getY(), 70, 70);
    peakQuality2.setBounds(peakArea2.getX(), peakArea2.getY() + 70, 70, 70);
    peakGain2.setBounds(peakArea2.getX(), peakArea2.getY() + 140, 70, 70);

    highShelfFreq.setBounds(highArea.getX()-100, highArea.getY(), 70, 70);
    highShelfQuality.setBounds(highArea.getX()-100, highArea.getY() + 70, 70, 70);
    highShelfGain.setBounds(highArea.getX()-100, highArea.getY() + 140, 70, 70);
}

std::vector<juce::Component*> TaroMatchingAudioProcessorEditor::getComps()
{
    return
    {
        &frame,
        &spectrumAnalyzer,
        &lowShelfFreq,
        &lowShelfQuality,
        &lowShelfGain,
        &highShelfFreq,
        &highShelfQuality,
        &highShelfGain,
        &peakFreq1,
        &peakQuality1,
        &peakGain1,
        &peakFreq2,
        &peakQuality2,
        &peakGain2

        /*&lowcutBypassButton,
        &peakBypassButton,
        &highcutBypassButton,
        &analyzerEnabledButton*/
    };
}