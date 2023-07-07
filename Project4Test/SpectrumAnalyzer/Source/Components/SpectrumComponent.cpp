/*
  ==============================================================================

    SpectrumComponent.cpp
    Created: 26 Jun 2023 4:36:31pm
    Author:  sunwei06

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SpectrumComponent.h"

const int SpectrumComponent::frequenciesForLines[] = { 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000 };
const int SpectrumComponent::numberOfLines = 28;
//==============================================================================
SpectrumComponent::SpectrumComponent() : numberOfBins(1024), mBinWidth(44100 / (float)2048)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

SpectrumComponent::~SpectrumComponent()
{
}

void SpectrumComponent::paint(juce::Graphics& g)
{
    // paint background
    g.setColour(juce::Colour(40, 40, 40));
    g.fillAll();

    // paint horizontal lines and frequency numbers
    g.setColour(juce::Colours::lightgrey.withAlpha(0.2f));
    g.drawLine(0, getHeight() / 5, getWidth(), getHeight() / 5, 1);

    // 使用浅灰色和透明度为0.2的颜色绘制一条水平线，位置为组件高度的1/5。然后根据预定义的频率数组绘制垂直线，并在特定频率处绘制相应的频率文本。
    for (int i = 0; i < numberOfLines; ++i)
    {
        const double proportion = frequenciesForLines[i] / 20000.0;
        int xPos = transformToLog(proportion * 20000) * (getWidth());
        g.drawVerticalLine(xPos, getHeight() / 5, getHeight());
        if (frequenciesForLines[i] == 10 || frequenciesForLines[i] == 100 || frequenciesForLines[i] == 200)
            g.drawFittedText(static_cast<juce::String> (frequenciesForLines[i]), xPos - 30, 0, 60, getHeight() / 5, juce::Justification::centred, 2);
        else if (frequenciesForLines[i] == 1000 || frequenciesForLines[i] == 10000 || frequenciesForLines[i] == 2000)
            g.drawFittedText(static_cast<juce::String> (frequenciesForLines[i] / 1000) + "k", xPos - 30, 0, 60, getHeight() / 5, juce::Justification::centred, 2);
        else if (frequenciesForLines[i] == 20)
            g.drawFittedText(static_cast<juce::String> (frequenciesForLines[i]), xPos - 30, 0, 60, getHeight() / 5, juce::Justification::right, 2);
        else if (frequenciesForLines[i] == 20000)
            g.drawFittedText(static_cast<juce::String> (frequenciesForLines[i] / 1000) + "k", xPos - 30, 0, 60, getHeight() / 5, juce::Justification::left, 2);
    }

    // 根据预定义的坐标位置，绘制垂直方向上的dB文本。
    // paint vertical db numbers
    //    float fontWidth = 50;
    //    float fontHeight = getHeight() / 5;
    //    float centerAlign = fontHeight / 2;
    //    g.drawFittedText("-20 db", 0, getHeight() / 6 * 2 - centerAlign, fontWidth, fontHeight, juce::Justification::centred, 2);
    //    g.drawFittedText("-40 db", 0, getHeight() / 6 * 3 - centerAlign, fontWidth, fontHeight, juce::Justification::centred, 2);
    //    g.drawFittedText("-60 db", 0, getHeight() / 6 * 4 - centerAlign, fontWidth, fontHeight, juce::Justification::centred, 2);
    //    g.drawFittedText("-80 db", 0, getHeight() / 6 * 5 - centerAlign, fontWidth, fontHeight, juce::Justification::centred, 2);

    // 设置颜色为白色，调用paintSpectrum()函数绘制频谱图像。然后对当前频谱图像进行透明度调整、移动和绘制。
    // paint current spectrum
    g.setColour(juce::Colours::white);
    paintSpectrum();
    currentSpectrumImage.multiplyAllAlphas(0.9);
    currentSpectrumImage.moveImageSection(0, 10, 0, 0, currentSpectrumImage.getWidth(), currentSpectrumImage.getHeight());
    g.drawImageAt(currentSpectrumImage, 0, 0);

    // 绘制峰值频谱：对峰值频谱图像进行透明度调整，并在原点位置绘制。
    // paint peak spectrum
    maxSpectrumImage.multiplyAllAlphas(0.5);
    g.drawImageAt(maxSpectrumImage, 0, 0);

    // 绘制峰值文本：根据鼠标的位置和最大分贝值，决定是否绘制峰值文本。如果鼠标在组件范围内且最大分贝值大于-99.9，并且鼠标悬停在最大分贝点附近，绘制包含最大分贝值和最大频率的文本框。
    // paint peak text
    float mouseX = getMouseXYRelative().getX();
    float mouseY = getMouseXYRelative().getY();

    if (mouseX > 0 && mouseX < getWidth()
        && mouseY > 0 && mouseY < getHeight())
    {
        mouseOver = true;
    }
    else
    {
        mouseOver = false;
    }

    // 处理没有鼠标悬停的情况：如果鼠标不在组件范围内或最大分贝值小于-99.9，重置最大分贝值、最大频率和最大分贝点的位置，并将最大数据数组中的所有元素重置为0。
    if (maxDecibelValue >= -99.9f && mouseOver)
    {
        float boxWidth = 100.0f;
        g.setColour(juce::Colours::lightgrey);
        g.drawText(juce::String(maxDecibelValue, 1) + " db", maxDecibelPoint.getX() - boxWidth / 2.0f, maxDecibelPoint.getY() - boxWidth / 4.0f, boxWidth, boxWidth, juce::Justification::centred);
        g.drawText(juce::String(static_cast<int> (maxFreq)) + " Hz", maxDecibelPoint.getX() - boxWidth / 2.0f, maxDecibelPoint.getY(), boxWidth, boxWidth, juce::Justification::centred);
    }
    else
    {
        maxDecibelValue = -100.0f;
        maxFreq = 0.0f;
        maxDecibelPoint.setXY(-10.0f, -10.0f);
        for (int i = 0; i < 1024; i++)
        {
            maxData[i] = 0;
        }
    }
}

void SpectrumComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    currentSpectrumImage = currentSpectrumImage.rescaled(getWidth(), getHeight());
    maxSpectrumImage = maxSpectrumImage.rescaled(getWidth(), getHeight());
}

void SpectrumComponent::paintSpectrum()
{
    // this method is to paint spectrogram
    
    // 初始化绘图对象：创建juce::Graphics对象gCurrent和gMax，用于绘制当前频谱和峰值频谱。
    // init graphics
    juce::Graphics gCurrent(currentSpectrumImage);
    juce::Graphics gMax(maxSpectrumImage);

    // 获取组件的宽度和高度，并设置最小分贝值（mindB）为-100.0，最大分贝值（maxdB）为0.0。
    auto width = getLocalBounds().getWidth();
    auto height = getLocalBounds().getHeight();
    auto mindB = -100.0f;
    auto maxdB = 0.0f;

    // 创建两个juce::Path对象currentSpecPath和maxSpecPath，分别用于绘制当前频谱和峰值频谱的路径。
    juce::Path currentSpecPath;
    currentSpecPath.startNewSubPath(0, height);

    juce::Path maxSpecPath;
    maxSpecPath.startNewSubPath(0, height + 1);
    int resolution = 2;
    // 循环遍历频谱数据，计算每个频点的分贝值和相对高度，并将点连接起来形成路径。
    for (int i = 1; i < numberOfBins; i += resolution)
    {
        // sample range [0, 1] to decibel range[-∞, 0] to [0, 1]
        auto fftSize = 1 << 11;
        float currentDecibel = juce::Decibels::gainToDecibels(spectrumData[i] / static_cast<float> (numberOfBins));
        float maxDecibel = juce::Decibels::gainToDecibels(maxData[i])
            - juce::Decibels::gainToDecibels(static_cast<float> (fftSize));
        float yPercent = juce::jmap(juce::jlimit(mindB, maxdB, currentDecibel),
            mindB,
            maxdB,
            0.0f,
            1.0f);
        float yMaxPercent = juce::jmap(juce::jlimit(mindB, maxdB, maxDecibel),
            mindB,
            maxdB,
            0.0f,
            1.0f);
        //skip some points to save cpu
    /*    if (i > numberOfBins / 8 && i % 2 != 0) continue;
        if (i > numberOfBins / 4 && i % 3 != 0) continue;
        if (i > numberOfBins / 2 && i % 4 != 0) continue;
        if (i > numberOfBins / 4 * 3 && i % 10 != 0) continue;*/

        // connect points
        double currentFreq = i * mBinWidth;
        float currentX = transformToLog(currentFreq) * width;
        float currentY = juce::jmap(yPercent, 0.0f, 1.0f, (float)height, 0.0f);
        float maxY = juce::jmap(yMaxPercent, 0.0f, 1.0f, (float)height, 0.0f);
        currentSpecPath.lineTo(currentX, currentY);

        maxSpecPath.lineTo(currentX, maxY);

        if (currentDecibel > maxDecibelValue)
        {
            maxDecibelValue = currentDecibel;
            maxFreq = currentFreq;
            maxDecibelPoint.setXY(currentX, currentY);
        }
        if (spectrumData[i] > maxData[i])
        {
            maxData[i] = spectrumData[i];
        }

        // reference: https://docs.juce.com/master/tutorial_spectrum_analyser.html
    }

    // this step is to round the path
    juce::Path roundedCurrentPath = currentSpecPath.createPathWithRoundedCorners(10.0f);

    // draw the outline of the path
    roundedCurrentPath.lineTo(width, height);
    roundedCurrentPath.lineTo(0, height);
    roundedCurrentPath.closeSubPath();

    juce::Path roundedMaxPath = maxSpecPath.createPathWithRoundedCorners(10.0f);
    roundedMaxPath.lineTo(width, height + 1);
    //    roundedMaxPath.lineTo(0, height);
    //    roundedMaxPath.closeSubPath();

    gCurrent.setColour(juce::Colour(233, 208, 63));

    juce::ColourGradient grad(juce::Colours::red.withAlpha(0.8f), 0, 0, juce::Colour(233, 208, 63).withAlpha(0.8f), 0, getLocalBounds().getHeight(), false);

    gCurrent.setGradientFill(grad);
    gCurrent.fillPath(roundedCurrentPath);
    //    g.strokePath(roundedPath, juce::PathStrokeType(2));

    if (mouseOver)
    {
        gMax.setColour(juce::Colours::white);
        gMax.strokePath(roundedMaxPath, juce::PathStrokeType(2));
        gMax.drawEllipse(maxDecibelPoint.getX() - 2.0f, maxDecibelPoint.getY() + 10.0f, 4.0f, 4.0f, 1.0f);
    }
}

void SpectrumComponent::prepareToPaintSpectrum(int numBins, float* data, float binWidth)
{
    numberOfBins = numBins;
    memmove(spectrumData, data, sizeof(spectrumData));
    mBinWidth = binWidth;
}

float SpectrumComponent::transformToLog(double valueToTransform) // freq to x
{
    // input: 20-20000
    // output: x
    auto value = juce::mapFromLog10(valueToTransform, 20.0, 20000.0);
    return static_cast<float> (value);
}

float SpectrumComponent::transformFromLog(double between0and1) // x to freq
{
    // input: 0.1-0.9 x pos
    // output: freq

    auto value = juce::mapToLog10(between0and1, 20.0, 20000.0);
    return static_cast<float> (value);
}
