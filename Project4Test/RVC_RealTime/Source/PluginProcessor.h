/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "postAudio.h"

//==============================================================================
/**
*/
class RVC_RealTimeAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    RVC_RealTimeAudioProcessor();
    ~RVC_RealTimeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void setAudioBufferSize(int newNumChannels, int newNumSamples, bool keepExistingContent = false, bool clearExtraSpace = false, bool avoidRealLocationg = false);
    juce::AudioBuffer<float>& RVC_RealTimeAudioProcessor::getAudioBuffer();
    void readWav();

    void write2CircularBuffer(juce::AudioBuffer<float>& inputBuffer);
    void postAudio2Backend(juce::AudioBuffer<float>& buffer);
    void postWav();

private:
    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> audioBuffer;
    int audioBufferSampleRate = 48000; 
    bool firstProcess = true;

    juce::AudioBuffer<float> circularBuffer{ 1, 48000 }; // 长度为1s音频内容的circularBuffer
    int writePointer = 0;

    postAudio postAudioThread;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RVC_RealTimeAudioProcessor)
};
