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
    // Èç¹ûÄÜ¹»±£Ö¤²»»áÔÚ±ß½çÖ®Íâ»æÖÆÈÎºÎÄÚÈÝÊ±£¬¿ÉÒÔÊ¹ÓÃ setPaintingIsUnclipped(true) À´Ìá¸ßÐÔÄÜ
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

// ¼ÆËã¸ø¶¨ÆµÓòÊý¾Ý»º³åÇøÖÐÖ¸¶¨ÆµµãµÄ·ù¶È¼¶±ð£¬²¢½«Æä×ª»»Îª·Ö±´Öµ
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
    // »ñÈ¡µ±Ç°¿Ø¼þµÄ´óÐ¡£¬²¢¼ÆËã³ö¿í¶ÈºÍ¸ß¶È
    const auto bounds = getLocalBounds().toFloat();
    const auto width = bounds.getWidth();
    const auto height = bounds.getHeight();

    // »ñÈ¡Æ½¾ùÊäÈë»º³åÇøºÍÆ½¾ùÊä³ö»º³åÇøµÄÊý¾Ý£¬²¢¶ÔÊý¾Ý½øÐÐ´¦Àí
    const auto* fftDataInput = avgInput.getReadPointer(0);
    const auto* fftDataOutput = avgOutput.getReadPointer(0);

    // Í¨¹ýjuce::ScopedLock¶ÔÏó¶Ô¹²Ïí×ÊÔ´½øÐÐ¼ÓËø£¬±ÜÃâ¶àÏß³ÌÍ¬Ê±·ÃÎÊ¹²Ïí×ÊÔ´
    juce::ScopedLock lockedForReading(pathCreationLock);

    // Çå¿ÕÂ·¾¶¶ÔÏóinPºÍoutP
    inP.clear();
    outP.clear();

    // ¶ÔFFTµã½øÐÐ´¦Àí£¬½«´¦ÀíºóµÄÊý¾ÝÌí¼Óµ½Â·¾¶¶ÔÏóinPºÍoutPÖÐ
    // ¶ÔÓÚÃ¿¸öÆµµã£¬¼ÆËãÆä·ù¶È¼¶±ðµÄ·Ö±´Öµ£¬²¢½«Æä×ª»»ÎªyÖá×ø±ê
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

    // »æÖÆÂ·¾¶¶ÔÏóoutP£¬²¢Ìî³äÑÕÉ«
    g.setColour(juce::Colour{ 0x6b9acd32 });
    g.fillPath(outP);

    // »æÖÆÂ·¾¶¶ÔÏóinP£¬²¢Ìî³äÑÕÉ«
    g.setColour(baseColor.brighter(0.18f).withAlpha(juce::uint8(182)));
    g.fillPath(inP);
}

// ¸ù¾Ý¿Ø¼þ´óÐ¡ºÍFFT´óÐ¡¼ÆËãÆµµãµÄÎ»ÖÃ£¬²¢¸ù¾ÝÆµµãµÄÊýÁ¿¸üÐÂfftPointsSize
void SpectrumAnalyzer::resized()
{
    // »ñÈ¡¿Ø¼þµÄ±ß½ç£¨bounds£©£¬²¢¸ù¾ÝÆä¿í¶È£¨getWidth£©¼ÆËãÃ¿¸öÆµµãµÄxÖá×ø±êµÄ¿í¶ÈÒò×Ó£¨widthFactor£©
    const auto bounds = getLocalBounds();
    auto widthFactor = bounds.getWidth() / 10.0f;
    // »ñÈ¡²ÉÑùÂÊ£¨sampleRate£©ºÍFFT´óÐ¡£¨fftSize£©
    auto sampleRate = float(processor.getSampleRate());
    auto fftSize = fftInput.getSize();
    
    // ³õÊ¼»¯fftPointsSizeÎª0£¬²¢½«µÚÒ»¸öÆµµãµÄµÚÒ»¸öbinµÄË÷ÒýÉèÖÃÎª0
    fftPointsSize = 0;
    int lastX = 0;
    fftPoints[0].firstBinIndex = 0;

    // Í¨¹ýÑ­»·¼ÆËãÃ¿¸öÆµµãµÄÎ»ÖÃ
    int i = 0;
    while (i < processor.fftSize)
    {
        // Ê×ÏÈ»ñÈ¡µ±Ç°Æµµã£¨fftPoint£©
        fftPoint& point = fftPoints[fftPointsSize];
        // ²¢½«ÆäµÚÒ»¸öbinµÄË÷ÒýÉèÖÃÎªi
        point.firstBinIndex = i;
        // ½«ÉÏÒ»¸öÆµµãµÄxÖá×ø±êÉèÖÃÎª¸ÃÆµµãµÄxÖá×ø±ê£¨lastX£©
        point.x = lastX;

        // ÔÚÑ­»·ÖÐ¼ÆËãÏÂÒ»¸öÆµµãµÄxÖá×ø±ê£¨x£©
        int x = lastX;
        while ((x <= lastX) && (i < processor.fftSize))
        {
            ++i;
            // ÔÚ¼ÆËãÏÂÒ»¸öÆµµãµÄxÖá×ø±êÊ±£¬Ê×ÏÈ¼ÆËã¸ÃÆµµãµÄÆµÂÊËù¶ÔÓ¦µÄÎ»ÖÃ£¨pos£©
            auto pos = std::log(((sampleRate * i) / fftSize) / 20.f) / std::log(2.0f);
            // È»ºó½«Æä×ª»»ÎªxÖá×ø±ê
            // Èç¹ûxÖá×ø±êÐ¡ÓÚµÈÓÚÉÏÒ»¸öÆµµãµÄxÖá×ø±ê£¬Ôò½«ÆäÊÓÎªÏàÍ¬µÄÆµµã
            // Èç¹ûxÖá×ø±ê´óÓÚÉÏÒ»¸öÆµµãµÄxÖá×ø±ê£¬Ôò½«ÆäÊÓÎªÐÂµÄÆµµã
            x = juce::roundToInt((pos > 0.0f) ? (widthFactor * pos) + 0.5f : 0);
        }

        // ¼ÆËã¸ÃÆµµãµÄ×îºóÒ»¸öbinµÄË÷Òý£¨i-1£©
        point.lastBinIndex = i - 1;

        // ½«¸ÃÆµµãµÄÊýÁ¿£¨fftPointsSize£©¼Ó1
        ++fftPointsSize;
        // ½«ÉÏÒ»¸öÆµµãµÄxÖá×ø±ê¸üÐÂÎªµ±Ç°ÆµµãµÄxÖá×ø±ê
        lastX = x;
    }
}

// Õâ¶Î´úÂëÊÇÓÃÓÚÆµÆ×·ÖÎöµÄ£¬Ö÷ÒªÊÇ¶ÔÒôÆµÊý¾Ý½øÐÐFFT±ä»»£¬ÒÔ»ñÈ¡ÆµÆ×ÐÅÏ¢
void SpectrumAnalyzer::drawNextFrame()
{
    // Ê×ÏÈÍ¨¹ýwhileÑ­»·´Ó»º³åÇøÖÐ¶ÁÈ¡ÒôÆµÊý¾Ý£¬Ö±µ½»º³åÇøÖÐ¿ÉÓÃµÄÊý¾ÝÁ¿´óÓÚµÈÓÚFFTÊäÈë»º³åÇøµÄ´óÐ¡ÎªÖ¹
    while (processor.abstractFifoInput.getNumReady() >= fftInput.getSize())
    {
        // ½Ó×ÅÇå¿ÕFFTÊäÈë»º³åÇø£¬
        fftBufferInput.clear();

        // ²¢Í¨¹ýprepareToRead()·½·¨»ñÈ¡´ÓFIFOÊäÈë»º³åÇøÖÐ¶ÁÈ¡Êý¾ÝµÄÆðÊ¼Î»ÖÃºÍ¿é´óÐ¡
        int start1, block1, start2, block2;
        processor.abstractFifoInput.prepareToRead(fftInput.getSize(), start1, block1, start2, block2);

        // ½«FIFOÊäÈë»º³åÇøÖÐµÄÊý¾Ý¸´ÖÆµ½FFTÊäÈë»º³åÇøÖÐ
        if (block1 > 0) fftBufferInput.copyFrom(0, 0, processor.audioFifoInput.getReadPointer(0, start1), block1);
        if (block2 > 0) fftBufferInput.copyFrom(0, block1, processor.audioFifoInput.getReadPointer(0, start2), block2);
        processor.abstractFifoInput.finishedRead((block1 + block2) / 2);

        // Í¨¹ýHann´°¿Úº¯Êý¶ÔÊý¾Ý½øÐÐ¼ÓÈ¨´¦Àí
        hannWindow.multiplyWithWindowingTable(fftBufferInput.getWritePointer(0), size_t(fftInput.getSize()));
        fftInput.performFrequencyOnlyForwardTransform(fftBufferInput.getWritePointer(0));
        // ¶ÔFFTÊäÈë»º³åÇøÖÐµÄÊý¾Ý½øÐÐFFT±ä»»£¬²¢½«½á¹û±£´æÔÚFFTÊä³ö»º³åÇøÖÐ
        juce::ScopedLock lockedForWriting(pathCreationLock);
        avgInput.addFrom(0, 0, avgInput.getReadPointer(avgInputPtr), avgInput.getNumSamples(), -1.0f);
        avgInput.copyFrom(avgInputPtr, 0, fftBufferInput.getReadPointer(0), avgInput.getNumSamples(), 1.0f / (avgInput.getNumSamples() * (avgInput.getNumChannels() - 1)));
        avgInput.addFrom(0, 0, avgInput.getReadPointer(avgInputPtr), avgInput.getNumSamples());

        // ¶ÔFFTÊä³ö»º³åÇøÖÐµÄÊý¾Ý½øÐÐÆ½¾ù»¯´¦Àí£¬½«½á¹û±£´æÔÚÆ½¾ùÊäÈë»º³åÇøÖÐ
        if (++avgInputPtr == avgInput.getNumChannels()) avgInputPtr = 1;

        // ÖØ¸´ÉÏÊö²½Öè£¬Ö±µ½»º³åÇøÖÐ¿ÉÓÃµÄÊý¾ÝÁ¿²»×ãÒÔÌîÂúFFTÊäÈë»º³åÇøÎªÖ¹
        // ÐèÒªËµÃ÷µÄÊÇ£¬´úÂëÖÐµÄjuce::ScopedLock lockedForWriting(pathCreationLock)ÓÃÓÚ¼ÓËø£¬±ÜÃâ¶àÏß³ÌÍ¬Ê±·ÃÎÊ¹²Ïí×ÊÔ´¡£ÆäÖÐµÄpathCreationLockÊÇÒ»¸ö»¥³âËø¶ÔÏó£¬ÓÃÓÚ±£»¤¹²Ïí×ÊÔ´µÄ·ÃÎÊ
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

// Èç¹ûÏÂÒ»¸öFFT¿éÒÑ¾­×¼±¸ºÃ£¬Ôòµ÷ÓÃdrawNextFrame()º¯Êý½«Æ½¾ùÊä³ö»º³åÇøÖÐµÄÊý¾Ý»æÖÆµ½ÆÁÄ»ÉÏ£¬²¢Í¨¹ýrepaint()º¯ÊýË¢ÐÂÆÁÄ»
void SpectrumAnalyzer::timerCallback()
{
    if (!processor.nextFFTBlockReady.load()) return;

    // ½«Æ½¾ùÊä³ö»º³åÇøÖÐµÄÊý¾Ý»æÖÆµ½ÆÁÄ»ÉÏ£¬ÓÃÓÚÏÔÊ¾ÆµÆ×ÐÅÏ¢
    drawNextFrame();
    processor.nextFFTBlockReady.store(false);
    repaint();
}