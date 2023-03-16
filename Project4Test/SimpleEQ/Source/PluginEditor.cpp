/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics & g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider & slider)
{
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    auto enabled = slider.isEnabled();
    
    g.setColour(enabled ? Colour(97u, 18u, 167u) : Colours::darkgrey );
    g.fillEllipse(bounds);
    
    g.setColour(enabled ? Colour(255u, 154u, 1u) : Colours::grey);
    g.drawEllipse(bounds, 1.f);
    
    if( auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);
        
        p.addRoundedRectangle(r, 2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        
        g.setColour(enabled ? Colours::black : Colours::darkgrey);
        g.fillRect(r);
        
        g.setColour(enabled ? Colours::white : Colours::lightgrey);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void LookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    using namespace juce;
    
    if( auto* pb = dynamic_cast<PowerButton*>(&toggleButton) )
    {
        Path powerButton;
        
        auto bounds = toggleButton.getLocalBounds();
        
        auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
        
        float ang = 30.f; //30.f;
        
        size -= 6;
        
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5,
                                  size * 0.5,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);
        
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
        
        PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
        
        auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);
        
        g.setColour(color);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 2);
    }
    else if( auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton) )
    {
        auto color = ! toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);
        
        g.setColour(color);
        
        auto bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);
        
        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
    }
}
//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAng,
                                      endAng,
                                      *this);
    
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(Colour(0u, 172u, 1u));
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    for( int i = 0; i < numChoices; ++i )
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);
        
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);
        
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
    
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
    
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if( auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param) )
        return choiceParam->getCurrentChoiceName();
    
    juce::String str;
    bool addK = false;
    
    if( auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param) )
    {
        float val = getValue();
        
        if( val > 999.f )
        {
            val /= 1000.f; //1001 / 1000 = 1.001
            addK = true;
        }
        
        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse; //this shouldn't happen!
    }
    
    if( suffix.isNotEmpty() )
    {
        str << " ";
        if( addK )
            str << "k";
        
        str << suffix;
    }
    
    return str;
}
//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "db/Oct"),

responseCurveComponent(audioProcessor),

peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),

lowcutBypassButtonAttachment(audioProcessor.apvts, "LowCut Bypassed", lowcutBypassButton),
peakBypassButtonAttachment(audioProcessor.apvts, "Peak Bypassed", peakBypassButton),
highcutBypassButtonAttachment(audioProcessor.apvts, "HighCut Bypassed", highcutBypassButton),
analyzerEnabledButtonAttachment(audioProcessor.apvts, "Analyzer Enabled", analyzerEnabledButton)
{
    peakFreqSlider.labels.add({0.f, "20Hz"});
    peakFreqSlider.labels.add({1.f, "20kHz"});
    
    peakGainSlider.labels.add({0.f, "-24dB"});
    peakGainSlider.labels.add({1.f, "+24dB"});
    
    peakQualitySlider.labels.add({0.f, "0.1"});
    peakQualitySlider.labels.add({1.f, "10.0"});
    
    lowCutFreqSlider.labels.add({0.f, "20Hz"});
    lowCutFreqSlider.labels.add({1.f, "20kHz"});
    
    highCutFreqSlider.labels.add({0.f, "20Hz"});
    highCutFreqSlider.labels.add({1.f, "20kHz"});
    
    lowCutSlopeSlider.labels.add({0.0f, "12"});
    lowCutSlopeSlider.labels.add({1.f, "48"});
    
    highCutSlopeSlider.labels.add({0.0f, "12"});
    highCutSlopeSlider.labels.add({1.f, "48"});
    
    for( auto* comp : getComps() )
    {
        addAndMakeVisible(comp);
    }
    
    peakBypassButton.setLookAndFeel(&lnf);
    highcutBypassButton.setLookAndFeel(&lnf);
    lowcutBypassButton.setLookAndFeel(&lnf);

    analyzerEnabledButton.setLookAndFeel(&lnf);
    
    auto safePtr = juce::Component::SafePointer<SimpleEQAudioProcessorEditor>(this);
    peakBypassButton.onClick = [safePtr]()
    {
        if( auto* comp = safePtr.getComponent() )
        {
            auto bypassed = comp->peakBypassButton.getToggleState();
            
            comp->peakFreqSlider.setEnabled( !bypassed );
            comp->peakGainSlider.setEnabled( !bypassed );
            comp->peakQualitySlider.setEnabled( !bypassed );
        }
    };
    

    lowcutBypassButton.onClick = [safePtr]()
    {
        if( auto* comp = safePtr.getComponent() )
        {
            auto bypassed = comp->lowcutBypassButton.getToggleState();
            
            comp->lowCutFreqSlider.setEnabled( !bypassed );
            comp->lowCutSlopeSlider.setEnabled( !bypassed );
        }
    };
    
    highcutBypassButton.onClick = [safePtr]()
    {
        if( auto* comp = safePtr.getComponent() )
        {
            auto bypassed = comp->highcutBypassButton.getToggleState();
            
            comp->highCutFreqSlider.setEnabled( !bypassed );
            comp->highCutSlopeSlider.setEnabled( !bypassed );
        }
    };

    analyzerEnabledButton.onClick = [safePtr]()
    {
        if( auto* comp = safePtr.getComponent() )
        {
            auto enabled = comp->analyzerEnabledButton.getToggleState();
            comp->responseCurveComponent.toggleAnalysisEnablement(enabled);
        }
    };
    
    setSize (480, 500);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
    peakBypassButton.setLookAndFeel(nullptr);
    highcutBypassButton.setLookAndFeel(nullptr);
    lowcutBypassButton.setLookAndFeel(nullptr);

    analyzerEnabledButton.setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint(juce::Graphics &g)
{
    using namespace juce;
    
    g.fillAll (Colours::black);
    
    Path curve;
    
    auto bounds = getLocalBounds();
    auto center = bounds.getCentre();
    
    g.setFont(Font("Iosevka Term Slab", 30, 0)); //https://github.com/be5invis/Iosevka
    
    String title { "PFM::C++ FOR MUSICIANS" };
    g.setFont(30);
    auto titleWidth = g.getCurrentFont().getStringWidth(title);
    
    curve.startNewSubPath(center.x, 32);
    curve.lineTo(center.x - titleWidth * 0.45f, 32);
    
    auto cornerSize = 20;
    auto curvePos = curve.getCurrentPosition();
    curve.quadraticTo(curvePos.getX() - cornerSize, curvePos.getY(),
                      curvePos.getX() - cornerSize, curvePos.getY() - 16);
    curvePos = curve.getCurrentPosition();
    curve.quadraticTo(curvePos.getX(), 2,
                      curvePos.getX() - cornerSize, 2);
    
    curve.lineTo({0.f, 2.f});
    curve.lineTo(0.f, 0.f);
    curve.lineTo(center.x, 0.f);
    curve.closeSubPath();
    
    g.setColour(Colour(97u, 18u, 167u));
    g.fillPath(curve);
    
    curve.applyTransform(AffineTransform().scaled(-1, 1));
    curve.applyTransform(AffineTransform().translated(getWidth(), 0));
    g.fillPath(curve);
    
    
    g.setColour(Colour(255u, 154u, 1u));
    g.drawFittedText(title, bounds, juce::Justification::centredTop, 1);
    
    g.setColour(Colours::grey);
    g.setFont(14);
    g.drawFittedText("LowCut", lowCutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("Peak", peakQualitySlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("HighCut", highCutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);
    
    auto buildDate = Time::getCompilationDate().toString(true, false);
    auto buildTime = Time::getCompilationDate().toString(false, true);
    g.setFont(12);
    g.drawFittedText("Build: " + buildDate + "\n" + buildTime, highCutSlopeSlider.getBounds().withY(6), Justification::topRight, 2);
}

void SimpleEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(4);
    
    auto analyzerEnabledArea = bounds.removeFromTop(25);
    
    analyzerEnabledArea.setWidth(50);
    analyzerEnabledArea.setX(5);
    analyzerEnabledArea.removeFromTop(2);
    
    analyzerEnabledButton.setBounds(analyzerEnabledArea);
    
    bounds.removeFromTop(5);
    
    float hRatio = 25.f / 100.f; //JUCE_LIVE_CONSTANT(25) / 100.f;
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio); //change from 0.33 to 0.25 because I needed peak hz text to not overlap the slider thumb

    responseCurveComponent.setBounds(responseArea);
    
    bounds.removeFromTop(5);
    
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    lowcutBypassButton.setBounds(lowCutArea.removeFromTop(25));
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    
    highcutBypassButton.setBounds(highCutArea.removeFromTop(25));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);
    
    peakBypassButton.setBounds(bounds.removeFromTop(25));
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
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
        
        &lowcutBypassButton,
        &peakBypassButton,
        &highcutBypassButton,
        &analyzerEnabledButton
    };
}
