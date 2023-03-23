/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/Filters.h"

//==============================================================================
/**
*/
class TaroMatchingAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    TaroMatchingAudioProcessor();
    ~TaroMatchingAudioProcessor() override;

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

    // ”√”⁄Spectrum Analyzer
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

    // apvts
    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS::ParameterLayout createParameterLayout();
    APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

    // Frequency Curve
   /* struct Band : public juce::AudioProcessorValueTreeState::Listener
    {
        Band(TaroMatchingAudioProcessor& eqProcessor, int index) :
            idOn{ std::to_string(index) + "On" },
            idType{ std::to_string(index) + "Type" },
            idFreq{ std::to_string(index) + "Freq" },
            idGain{ std::to_string(index) + "Gain" },
            idQ{ std::to_string(index) + "Q" },
            eqProcessor{ eqProcessor }, index{ index } {}

        bool active{ true };

        const juce::Identifier idOn;
        const juce::Identifier idType;
        const juce::Identifier idFreq;
        const juce::Identifier idGain;
        const juce::Identifier idQ;

        std::atomic<float>* prmOn{ nullptr };
        std::atomic<float>* prmType{ nullptr };
        std::atomic<float>* prmFreq{ nullptr };
        std::atomic<float>* prmGain{ nullptr };
        std::atomic<float>* prmQ{ nullptr };

        void parameterChanged(const juce::String& parameter, float newValue) override;

        void updateFilter();

        int getIndex() { return index; }

        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> processor;

    private:
        TaroMatchingAudioProcessor& eqProcessor;
        int index{ 0 };
    };
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };
    std::array<Band, 5> bands;
    std::array<Band, 5>& getBands() { return bands; }
    juce::AudioProcessorValueTreeState& getVTS() { return apvts; }
    const juce::Identifier idOutputGain;
    std::atomic<float>* prmOutputGain{ nullptr };*/

private:
    Filters filters;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TaroMatchingAudioProcessor)
};
