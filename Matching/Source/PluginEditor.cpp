/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MatchingAudioProcessorEditor::MatchingAudioProcessorEditor (MatchingAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    analyzer(audioProcessor),
    responseCurveComponent(audioProcessor),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider)
{
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    p.setCopyToFifo(true);

    setSize(800, 500);

}

MatchingAudioProcessorEditor::~MatchingAudioProcessorEditor()
{
}

//==============================================================================
void MatchingAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;

    g.fillAll(Colours::black);

    //Path curve;

    //auto bounds = getLocalBounds();
    //auto center = bounds.getCentre();

    //g.setFont(Font("Iosevka Term Slab", 30, 0)); //https://github.com/be5invis/Iosevka

    //String title{ "PFM::C++ FOR MUSICIANS" };
    //g.setFont(30);
    //auto titleWidth = g.getCurrentFont().getStringWidth(title);

    //curve.startNewSubPath(center.x, 32);
    //curve.lineTo(center.x - titleWidth * 0.45f, 32);

    //auto cornerSize = 20;
    //auto curvePos = curve.getCurrentPosition();
    //curve.quadraticTo(curvePos.getX() - cornerSize, curvePos.getY(),
    //    curvePos.getX() - cornerSize, curvePos.getY() - 16);
    //curvePos = curve.getCurrentPosition();
    //curve.quadraticTo(curvePos.getX(), 2,
    //    curvePos.getX() - cornerSize, 2);

    //curve.lineTo({ 0.f, 2.f });
    //curve.lineTo(0.f, 0.f);
    //curve.lineTo(center.x, 0.f);
    //curve.closeSubPath();

    //g.setColour(Colour(97u, 18u, 167u));
    //g.fillPath(curve);

    //curve.applyTransform(AffineTransform().scaled(-1, 1));
    //curve.applyTransform(AffineTransform().translated(getWidth(), 0));
    //g.fillPath(curve);


    //g.setColour(Colour(255u, 154u, 1u));
    //g.drawFittedText(title, bounds, juce::Justification::centredTop, 1);

    //g.setColour(Colours::grey);
    //g.setFont(14);
    //g.drawFittedText("LowCut", lowCutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);
    //g.drawFittedText("Peak", peakQualitySlider.getBounds(), juce::Justification::centredBottom, 1);
    //g.drawFittedText("HighCut", highCutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);

    //auto buildDate = Time::getCompilationDate().toString(true, false);
    //auto buildTime = Time::getCompilationDate().toString(false, true);
    //g.setFont(12);
    //g.drawFittedText("Build: " + buildDate + "\n" + buildTime, highCutSlopeSlider.getBounds().withY(6), Justification::topRight, 2);
}

void MatchingAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(4);

    auto analyzerEnabledArea = bounds.removeFromTop(5);

    analyzerEnabledArea.setWidth(50);
    analyzerEnabledArea.setX(5);
    analyzerEnabledArea.removeFromTop(2);

    // analyzerEnabledButton.setBounds(analyzerEnabledArea);

    bounds.removeFromTop(5);

    float hRatio = 75.f / 100.f; //JUCE_LIVE_CONSTANT(25) / 100.f;
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio); //change from 0.33 to 0.25 because I needed peak hz text to not overlap the slider thumb

    analyzer.setBounds(responseArea);
    // responseCurveComponent.setBounds(responseArea);

    bounds.removeFromTop(5);

    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    // lowcutBypassButton.setBounds(lowCutArea.removeFromTop(25));
    //lowCutFreqSlider.setBounds(lowCutArea.removeFromLeft(lowCutArea.getWidth() * 0.5));
    lowCutFreqSlider.setBounds(lowCutArea.getX() + 5, lowCutArea.getY(), 90, 90);
    lowCutSlopeSlider.setBounds(lowCutArea.getX() + 100, lowCutArea.getY(), 90, 90);

    // highcutBypassButton.setBounds(highCutArea.removeFromTop(25));
    highCutFreqSlider.setBounds(highCutArea.getX() + 5, highCutArea.getY(), 90, 90);
    highCutSlopeSlider.setBounds(highCutArea.getX() + 100, highCutArea.getY(), 90, 90);

    // peakBypassButton.setBounds(bounds.removeFromTop(25));
    peakFreqSlider.setBounds(bounds.getX() + 5, bounds.getY(), 80, 80);
    peakGainSlider.setBounds(bounds.getX() + 95, bounds.getY(), 80, 80);
    peakQualitySlider.setBounds(bounds.getX() + 185, bounds.getY(), 80, 80);
}

std::vector<juce::Component*> MatchingAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
        &responseCurveComponent,
        &analyzer

        /*&lowcutBypassButton,
        &peakBypassButton,
        &highcutBypassButton,
        &analyzerEnabledButton*/
    };
}