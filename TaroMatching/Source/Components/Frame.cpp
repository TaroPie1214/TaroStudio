/*
  ==============================================================================

    Frame.cpp
    Created: 23 Mar 2023 1:35:42pm
    Author:  sunwei06

  ==============================================================================
*/

#include "Frame.h"

Frame::Frame(int maxWidth, int maxHeight)
    : spectrumBackground{ juce::Image::PixelFormat::ARGB, maxWidth, maxHeight, true },
    spectrumForeground{ juce::Image::PixelFormat::ARGB, maxWidth, maxHeight, true },
    openSansBold{ juce::Typeface::createSystemTypefaceFor(BinaryData::OpenSansCondensedBold_ttf, BinaryData::OpenSansCondensedBold_ttfSize) }
{
    setOpaque(true);
}

void Frame::paint(juce::Graphics& g)
{
    g.drawImageAt(spectrumBackground, 0, 0);
}

void Frame::paintOverChildren(juce::Graphics& g)
{
    g.drawImageAt(spectrumForeground, 0, 0);
}

void Frame::resized()
{
    drawSpectrumBackground();
    drawSpectrumForeground();
}

void Frame::drawSpectrumBackground()
{
    const auto bounds = getLocalBounds();
    const auto width = bounds.getWidth();
    const auto height = bounds.getHeight();

    juce::Graphics g{ spectrumBackground };
    g.setColour(baseColor);
    g.fillRect(bounds);

    g.setColour(baseColor.brighter(0.036f));
    g.drawHorizontalLine(int(height * 0.5f), 0.0f, float(width));

    for (auto& bandGain : bandGains)
    {
        int pos = juce::jmap(bandGain, -24.0f, 24.0f, 1.0f, 0.0f) * height;
        g.drawHorizontalLine(pos, 0, width);
    }

    for (auto& freq : freqsB)
    {
        int pos = juce::roundToInt((std::log(freq / 20.0f) / std::log(2.0f)) / 10.0f * width);
        g.drawVerticalLine(pos, 0, height);
    }

    g.setColour(baseColor.brighter(0.082f));
    for (auto& freq : freqsA)
    {
        int pos = juce::roundToInt((std::log(freq / 20.0f) / std::log(2.0f)) / 10.0f * width);
        g.drawVerticalLine(pos, 0, height);
    }
}

void Frame::drawSpectrumForeground()
{
    const auto bounds = getLocalBounds();
    const auto width = bounds.getWidth();
    const auto height = bounds.getHeight();

    spectrumForeground.clear(spectrumForeground.getBounds());

    juce::Graphics g{ spectrumForeground };
    g.setColour(baseColor.brighter(0.4f));
    g.setFont(openSansBold);

    for (auto& bandGain : bandGains)
    {
        int pos = juce::roundToInt(juce::jmap(bandGain, -24.0f, 24.0f, 1.0f, 0.0f) * height);
        g.drawText(juce::String{ bandGain }, width - 44, pos - 14, 42, 28, juce::Justification::centredRight);
    }

    for (auto& gain : gains)
    {
        int pos = juce::roundToInt(juce::jmap(gain, -84.0f, 6.0f, 1.0f, 0.0f) * height);
        g.drawText(juce::String{ std::abs(gain) }, 2, pos - 14, 42, 28, juce::Justification::centredLeft);
    }

    g.setFont(openSansBold);
    g.setFont(15.0f);
    g.setColour(baseColor.brighter(2.8f));
    for (auto& freq : freqsA)
    {
        int pos = juce::roundToInt((std::log(freq / 20.0f) / std::log(2.0f)) / 10.0f * width);
        juce::String str;
        if (freq >= 1000)
        {
            str << (freq / 1000);
            str << "k";
        }
        else
        {
            str << freq;
        }

        g.drawText(str, pos - 21, (height * 0.5f) - 14, 42, 28, juce::Justification::centred);
    }
}