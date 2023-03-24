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
    // ����ܹ���֤�����ڱ߽�֮������κ�����ʱ������ʹ�� setPaintingIsUnclipped(true) ���������
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

// �������Ƶ�����ݻ�������ָ��Ƶ��ķ��ȼ��𣬲�����ת��Ϊ�ֱ�ֵ
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
    // ��ȡ��ǰ�ؼ��Ĵ�С�����������Ⱥ͸߶�
    const auto bounds = getLocalBounds().toFloat();
    const auto width = bounds.getWidth();
    const auto height = bounds.getHeight();

    // ��ȡƽ�����뻺������ƽ����������������ݣ��������ݽ��д���
    const auto* fftDataInput = avgInput.getReadPointer(0);
    const auto* fftDataOutput = avgOutput.getReadPointer(0);

    // ͨ��juce::ScopedLock����Թ�����Դ���м�����������߳�ͬʱ���ʹ�����Դ
    juce::ScopedLock lockedForReading(pathCreationLock);

    // ���·������inP��outP
    inP.clear();
    outP.clear();

    // ��FFT����д�����������������ӵ�·������inP��outP��
    // ����ÿ��Ƶ�㣬��������ȼ���ķֱ�ֵ��������ת��Ϊy������
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

    // ����·������outP���������ɫ
    g.setColour(juce::Colour{ 0x6b9acd32 });
    g.fillPath(outP);

    // ����·������inP���������ɫ
    g.setColour(baseColor.brighter(0.18f).withAlpha(juce::uint8(182)));
    g.fillPath(inP);
}

// ���ݿؼ���С��FFT��С����Ƶ���λ�ã�������Ƶ�����������fftPointsSize
void SpectrumAnalyzer::resized()
{
    // ��ȡ�ؼ��ı߽磨bounds�������������ȣ�getWidth������ÿ��Ƶ���x������Ŀ�����ӣ�widthFactor��
    const auto bounds = getLocalBounds();
    auto widthFactor = bounds.getWidth() / 10.0f;
    // ��ȡ�����ʣ�sampleRate����FFT��С��fftSize��
    auto sampleRate = float(processor.getSampleRate());
    auto fftSize = fftInput.getSize();
    
    // ��ʼ��fftPointsSizeΪ0��������һ��Ƶ��ĵ�һ��bin����������Ϊ0
    fftPointsSize = 0;
    int lastX = 0;
    fftPoints[0].firstBinIndex = 0;

    // ͨ��ѭ������ÿ��Ƶ���λ��
    int i = 0;
    while (i < processor.fftSize)
    {
        // ���Ȼ�ȡ��ǰƵ�㣨fftPoint��
        fftPoint& point = fftPoints[fftPointsSize];
        // �������һ��bin����������Ϊi
        point.firstBinIndex = i;
        // ����һ��Ƶ���x����������Ϊ��Ƶ���x�����꣨lastX��
        point.x = lastX;

        // ��ѭ���м�����һ��Ƶ���x�����꣨x��
        int x = lastX;
        while ((x <= lastX) && (i < processor.fftSize))
        {
            ++i;
            // �ڼ�����һ��Ƶ���x������ʱ�����ȼ����Ƶ���Ƶ������Ӧ��λ�ã�pos��
            auto pos = std::log(((sampleRate * i) / fftSize) / 20.f) / std::log(2.0f);
            // Ȼ����ת��Ϊx������
            // ���x������С�ڵ�����һ��Ƶ���x�����꣬������Ϊ��ͬ��Ƶ��
            // ���x�����������һ��Ƶ���x�����꣬������Ϊ�µ�Ƶ��
            x = juce::roundToInt((pos > 0.0f) ? (widthFactor * pos) + 0.5f : 0);
        }

        // �����Ƶ������һ��bin��������i-1��
        point.lastBinIndex = i - 1;

        // ����Ƶ���������fftPointsSize����1
        ++fftPointsSize;
        // ����һ��Ƶ���x���������Ϊ��ǰƵ���x������
        lastX = x;
    }
}

// ��δ���������Ƶ�׷����ģ���Ҫ�Ƕ���Ƶ���ݽ���FFT�任���Ի�ȡƵ����Ϣ
void SpectrumAnalyzer::drawNextFrame()
{
    // ����ͨ��whileѭ���ӻ������ж�ȡ��Ƶ���ݣ�ֱ���������п��õ����������ڵ���FFT���뻺�����Ĵ�СΪֹ
    while (processor.abstractFifoInput.getNumReady() >= fftInput.getSize())
    {
        // �������FFT���뻺������
        fftBufferInput.clear();

        // ��ͨ��prepareToRead()������ȡ��FIFO���뻺�����ж�ȡ���ݵ���ʼλ�úͿ��С
        int start1, block1, start2, block2;
        processor.abstractFifoInput.prepareToRead(fftInput.getSize(), start1, block1, start2, block2);

        // ��FIFO���뻺�����е����ݸ��Ƶ�FFT���뻺������
        if (block1 > 0) fftBufferInput.copyFrom(0, 0, processor.audioFifoInput.getReadPointer(0, start1), block1);
        if (block2 > 0) fftBufferInput.copyFrom(0, block1, processor.audioFifoInput.getReadPointer(0, start2), block2);
        processor.abstractFifoInput.finishedRead((block1 + block2) / 2);

        // ͨ��Hann���ں��������ݽ��м�Ȩ����
        hannWindow.multiplyWithWindowingTable(fftBufferInput.getWritePointer(0), size_t(fftInput.getSize()));
        fftInput.performFrequencyOnlyForwardTransform(fftBufferInput.getWritePointer(0));
        // ��FFT���뻺�����е����ݽ���FFT�任���������������FFT�����������
        juce::ScopedLock lockedForWriting(pathCreationLock);
        avgInput.addFrom(0, 0, avgInput.getReadPointer(avgInputPtr), avgInput.getNumSamples(), -1.0f);
        avgInput.copyFrom(avgInputPtr, 0, fftBufferInput.getReadPointer(0), avgInput.getNumSamples(), 1.0f / (avgInput.getNumSamples() * (avgInput.getNumChannels() - 1)));
        avgInput.addFrom(0, 0, avgInput.getReadPointer(avgInputPtr), avgInput.getNumSamples());

        // ��FFT����������е����ݽ���ƽ�������������������ƽ�����뻺������
        if (++avgInputPtr == avgInput.getNumChannels()) avgInputPtr = 1;

        // �ظ��������裬ֱ���������п��õ�����������������FFT���뻺����Ϊֹ
        // ��Ҫ˵�����ǣ������е�juce::ScopedLock lockedForWriting(pathCreationLock)���ڼ�����������߳�ͬʱ���ʹ�����Դ�����е�pathCreationLock��һ���������������ڱ���������Դ�ķ���
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

// �����һ��FFT���Ѿ�׼���ã������drawNextFrame()������ƽ������������е����ݻ��Ƶ���Ļ�ϣ���ͨ��repaint()����ˢ����Ļ
void SpectrumAnalyzer::timerCallback()
{
    if (!processor.nextFFTBlockReady.load()) return;

    // ��ƽ������������е����ݻ��Ƶ���Ļ�ϣ�������ʾƵ����Ϣ
    drawNextFrame();
    processor.nextFFTBlockReady.store(false);
    repaint();
}