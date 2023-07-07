/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor (SpectrumAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // timer
    juce::Timer::startTimerHz(60.0f);

    addAndMakeVisible(spectrum);

    spectrum.setInterceptsMouseClicks(false, false);
    spectrum.prepareToPaintSpectrum(audioProcessor.getNumBins(), audioProcessor.getFFTData(), audioProcessor.getSampleRate() / (float)audioProcessor.getFFTSize());

    setSize (600, 500);
}

SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor()
{
}

//==============================================================================
void SpectrumAnalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SpectrumAnalyzerAudioProcessorEditor::resized()
{
    juce::Rectangle<int> area(getLocalBounds());
    juce::Rectangle<int> spectrumArea = area;
    spectrum.setBounds(spectrumArea);
}

void SpectrumAnalyzerAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.isFFTBlockReady())
    {
        // not bypassed, repaint at the same time
        //(1<<11)
        // create a temp ddtData because sometimes pushNextSampleIntoFifo will replace the original
        // fftData after doingProcess and before painting.
        float tempFFTData[2 * 2048] = { 0 };
        memmove(tempFFTData, audioProcessor.getFFTData(), sizeof(tempFFTData));
        // doing process, fifo data to fft data
        audioProcessor.processFFT(tempFFTData);
        // prepare to paint the spectrum
        spectrum.prepareToPaintSpectrum(audioProcessor.getNumBins(), tempFFTData, audioProcessor.getSampleRate() / (float)audioProcessor.getFFTSize());

        //graphPanel.repaint();
        spectrum.repaint();
        //multiband.repaint();
        //bandPanel.repaint();
        //globalPanel.repaint();
        repaint();
    }
}
