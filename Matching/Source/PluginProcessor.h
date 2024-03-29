/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "dsp/Filter.h"
#include "dsp/SingleChannelSampleFifo.h"

//==============================================================================
/**
*/
class MatchingAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MatchingAudioProcessor();
    ~MatchingAudioProcessor() override;

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

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

    enum
    {
        fftOrder = 11,
        fftSize = 1 << fftOrder,
    };
    std::atomic<bool> nextFFTBlockReady{ false };
    juce::AbstractFifo abstractFifoInput{ 1 };
    juce::AudioBuffer<float> audioFifoInput;
    juce::AbstractFifo abstractFifoOutput{ 1 };
    juce::AudioBuffer<float> audioFifoOutput;
    void setCopyToFifo(bool _copyToFifo);
    std::atomic<bool> copyToFifo{ false };
    void pushNextSampleToFifo(const juce::AudioBuffer<float>& buffer, int startChannel, int numChannels,
        juce::AbstractFifo& absFifo, juce::AudioBuffer<float>& fifo);

    using BlockType = juce::AudioBuffer<float>;
    SingleChannelSampleFifo<BlockType> leftChannelFifo{ Channel::Left };
    SingleChannelSampleFifo<BlockType> rightChannelFifo{ Channel::Right };

private:
    // 初始化两个声道的Chain
    MonoChain leftChain, rightChain;

    juce::dsp::Oscillator<float> osc;

    FilterSettings filterSettings;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MatchingAudioProcessor)
};
