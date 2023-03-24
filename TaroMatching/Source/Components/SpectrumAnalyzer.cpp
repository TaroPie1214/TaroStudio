/*
  ==============================================================================

    SpectrumAnalyzer.cpp
    Created: 20 Mar 2023 11:54:27am
    Author:  TaroPie

  ==============================================================================
*/

#include "SpectrumAnalyzer.h"
#include <JuceHeader.h>

SpectrumAnalyzer::SpectrumAnalyzer(TaroMatchingAudioProcessor& TaroMatchingProcessor) : processor{ TaroMatchingProcessor }
{
    // 如果能够保证不会在边界之外绘制任何内容时，可以使用 setPaintingIsUnclipped(true) 来提高性能
    setPaintingIsUnclipped(true);

    avgInput.clear();
    avgOutput.clear();

    fftPoints.resize(processor.fftSize);

    {
        juce::ScopedLock lockedForWriting(pathCreationLock);
        inP.preallocateSpace(processor.fftSize * 2);
        outP.preallocateSpace(processor.fftSize * 2);
    }

    startTimerHz(30);
}

// 计算给定频域数据缓冲区中指定频点的幅度级别，并将其转换为分贝值
float SpectrumAnalyzer::getFftPointLevel(const float* buffer, const fftPoint& point)
{
    float level = 0.0f;

    for (int i = point.firstBinIndex; i <= point.lastBinIndex; ++i)
    {
        if (buffer[i] > level) level = buffer[i];
    }

    return juce::Decibels::gainToDecibels(level, mindB);
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
    // 获取当前控件的大小，并计算出宽度和高度
    const auto bounds = getLocalBounds().toFloat();
    const auto width = bounds.getWidth();
    const auto height = bounds.getHeight();

    // 获取平均输入缓冲区和平均输出缓冲区的数据，并对数据进行处理
    const auto* fftDataInput = avgInput.getReadPointer(0);
    const auto* fftDataOutput = avgOutput.getReadPointer(0);

    // 通过juce::ScopedLock对象对共享资源进行加锁，避免多线程同时访问共享资源
    juce::ScopedLock lockedForReading(pathCreationLock);

    // 清空路径对象inP和outP
    inP.clear();
    outP.clear();

    // 对FFT点进行处理，将处理后的数据添加到路径对象inP和outP中
    // 对于每个频点，计算其幅度级别的分贝值，并将其转换为y轴坐标
    {
        fftPoint& point = fftPoints[0];
        float y = juce::jmap(getFftPointLevel(fftDataInput, point), mindB, maxdB, height, 0.0f) + 0.5f;

        inP.startNewSubPath(float(point.x), y);
        outP.startNewSubPath(float(point.x), y);
    }

    for (int i = 0; i < fftPointsSize; ++i)
    {
        fftPoint& point = fftPoints[i];
        float y = juce::jmap(getFftPointLevel(fftDataInput, point), mindB, maxdB, height, 0.0f) + 0.5f;

        inP.lineTo(float(point.x), y);
        outP.lineTo(float(point.x), y);
    }

    for (int i = fftPointsSize - 1; i >= 0; --i)
    {
        fftPoint& point = fftPoints[i];
        float y = juce::jmap(getFftPointLevel(fftDataOutput, point), mindB, maxdB, height, 0.0f) + 0.5f;

        outP.lineTo(float(point.x), y);
    }

    outP.closeSubPath();

    inP.lineTo(width, height);
    inP.lineTo(0.0f, height);
    inP.closeSubPath();

    // 绘制路径对象outP，并填充颜色
    g.setColour(juce::Colour{ 0x6b9acd32 });
    g.fillPath(outP);

    // 绘制路径对象inP，并填充颜色
    g.setColour(baseColor.brighter(0.18f).withAlpha(juce::uint8(182)));
    g.fillPath(inP);
}

// 根据控件大小和FFT大小计算频点的位置，并根据频点的数量更新fftPointsSize
void SpectrumAnalyzer::resized()
{
    // 获取控件的边界（bounds），并根据其宽度（getWidth）计算每个频点的x轴坐标的宽度因子（widthFactor）
    const auto bounds = getLocalBounds();
    auto widthFactor = bounds.getWidth() / 10.0f;
    // 获取采样率（sampleRate）和FFT大小（fftSize）
    auto sampleRate = float(processor.getSampleRate());
    auto fftSize = fftInput.getSize();
    
    // 初始化fftPointsSize为0，并将第一个频点的第一个bin的索引设置为0
    fftPointsSize = 0;
    int lastX = 0;
    fftPoints[0].firstBinIndex = 0;

    // 通过循环计算每个频点的位置
    int i = 0;
    while (i < processor.fftSize)
    {
        // 首先获取当前频点（fftPoint）
        fftPoint& point = fftPoints[fftPointsSize];
        // 并将其第一个bin的索引设置为i
        point.firstBinIndex = i;
        // 将上一个频点的x轴坐标设置为该频点的x轴坐标（lastX）
        point.x = lastX;

        // 在循环中计算下一个频点的x轴坐标（x）
        int x = lastX;
        while ((x <= lastX) && (i < processor.fftSize))
        {
            ++i;
            // 在计算下一个频点的x轴坐标时，首先计算该频点的频率所对应的位置（pos）
            auto pos = std::log(((sampleRate * i) / fftSize) / 20.f) / std::log(2.0f);
            // 然后将其转换为x轴坐标
            // 如果x轴坐标小于等于上一个频点的x轴坐标，则将其视为相同的频点
            // 如果x轴坐标大于上一个频点的x轴坐标，则将其视为新的频点
            x = juce::roundToInt((pos > 0.0f) ? (widthFactor * pos) + 0.5f : 0);
        }

        // 计算该频点的最后一个bin的索引（i-1）
        point.lastBinIndex = i - 1;

        // 将该频点的数量（fftPointsSize）加1
        ++fftPointsSize;
        // 将上一个频点的x轴坐标更新为当前频点的x轴坐标
        lastX = x;
    }
}

// 这段代码是用于频谱分析的，主要是对音频数据进行FFT变换，以获取频谱信息
void SpectrumAnalyzer::drawNextFrame()
{
    // 首先通过while循环从缓冲区中读取音频数据，直到缓冲区中可用的数据量大于等于FFT输入缓冲区的大小为止
    while (processor.abstractFifoInput.getNumReady() >= fftInput.getSize())
    {
        // 接着清空FFT输入缓冲区，
        fftBufferInput.clear();

        // 并通过prepareToRead()方法获取从FIFO输入缓冲区中读取数据的起始位置和块大小
        int start1, block1, start2, block2;
        processor.abstractFifoInput.prepareToRead(fftInput.getSize(), start1, block1, start2, block2);

        // 将FIFO输入缓冲区中的数据复制到FFT输入缓冲区中
        if (block1 > 0) fftBufferInput.copyFrom(0, 0, processor.audioFifoInput.getReadPointer(0, start1), block1);
        if (block2 > 0) fftBufferInput.copyFrom(0, block1, processor.audioFifoInput.getReadPointer(0, start2), block2);
        processor.abstractFifoInput.finishedRead((block1 + block2) / 2);

        // 通过Hann窗口函数对数据进行加权处理
        hannWindow.multiplyWithWindowingTable(fftBufferInput.getWritePointer(0), size_t(fftInput.getSize()));
        fftInput.performFrequencyOnlyForwardTransform(fftBufferInput.getWritePointer(0));
        // 对FFT输入缓冲区中的数据进行FFT变换，并将结果保存在FFT输出缓冲区中
        juce::ScopedLock lockedForWriting(pathCreationLock);
        avgInput.addFrom(0, 0, avgInput.getReadPointer(avgInputPtr), avgInput.getNumSamples(), -1.0f);
        avgInput.copyFrom(avgInputPtr, 0, fftBufferInput.getReadPointer(0), avgInput.getNumSamples(), 1.0f / (avgInput.getNumSamples() * (avgInput.getNumChannels() - 1)));
        avgInput.addFrom(0, 0, avgInput.getReadPointer(avgInputPtr), avgInput.getNumSamples());

        // 对FFT输出缓冲区中的数据进行平均化处理，将结果保存在平均输入缓冲区中
        if (++avgInputPtr == avgInput.getNumChannels()) avgInputPtr = 1;

        // 重复上述步骤，直到缓冲区中可用的数据量不足以填满FFT输入缓冲区为止
        // 需要说明的是，代码中的juce::ScopedLock lockedForWriting(pathCreationLock)用于加锁，避免多线程同时访问共享资源。其中的pathCreationLock是一个互斥锁对象，用于保护共享资源的访问
    }

    while (processor.abstractFifoOutput.getNumReady() >= fftOutput.getSize())
    {
        fftBufferOutput.clear();

        int start1, block1, start2, block2;
        processor.abstractFifoOutput.prepareToRead(fftOutput.getSize(), start1, block1, start2, block2);

        if (block1 > 0) fftBufferOutput.copyFrom(0, 0, processor.audioFifoOutput.getReadPointer(0, start1), block1);
        if (block2 > 0) fftBufferOutput.copyFrom(0, block1, processor.audioFifoOutput.getReadPointer(0, start2), block2);

        processor.abstractFifoOutput.finishedRead((block1 + block2) / 2);

        hannWindow.multiplyWithWindowingTable(fftBufferOutput.getWritePointer(0), size_t(fftOutput.getSize()));
        fftOutput.performFrequencyOnlyForwardTransform(fftBufferOutput.getWritePointer(0));

        juce::ScopedLock lockedForWriting(pathCreationLock);
        avgOutput.addFrom(0, 0, avgOutput.getReadPointer(avgOutputPtr), avgOutput.getNumSamples(), -1.0f);
        avgOutput.copyFrom(avgOutputPtr, 0, fftBufferOutput.getReadPointer(0), avgOutput.getNumSamples(), 1.0f / (avgOutput.getNumSamples() * (avgOutput.getNumChannels() - 1)));
        avgOutput.addFrom(0, 0, avgOutput.getReadPointer(avgOutputPtr), avgOutput.getNumSamples());

        if (++avgOutputPtr == avgOutput.getNumChannels()) avgOutputPtr = 1;
    }
}

// 如果下一个FFT块已经准备好，则调用drawNextFrame()函数将平均输出缓冲区中的数据绘制到屏幕上，并通过repaint()函数刷新屏幕
void SpectrumAnalyzer::timerCallback()
{
    if (!processor.nextFFTBlockReady.load()) return;

    // 将平均输出缓冲区中的数据绘制到屏幕上，用于显示频谱信息
    drawNextFrame();
    processor.nextFFTBlockReady.store(false);
    repaint();
}