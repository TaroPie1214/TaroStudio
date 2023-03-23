#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

class FrequencyCurve : public juce::Component,
                       public juce::AudioProcessorValueTreeState::Listener
{
    public:
        FrequencyCurve (TaroMatchingAudioProcessor&);
        ~FrequencyCurve() override;

        void paint (juce::Graphics&) override;
        void resized() override;


    private:
        TaroMatchingAudioProcessor& processor;

        void parameterChanged (const juce::String&, float) override;

        std::mutex mutex;

        juce::Colour baseColor {0xff011523};

        juce::Path frequencyCurvePath;

        std::vector<double> frequencies;
        std::vector<juce::Point<float>> frequenciesPoints;
        std::array<std::vector<double>, 5> magnitudes;
        std::vector<double> magnitudesOut;
        void drawFrequencyCurve();


        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencyCurve)
};