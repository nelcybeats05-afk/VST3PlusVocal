#pragma once
#include <JuceHeader.h>

class VST3PlusVocalsMatchProcessor final : public juce::AudioProcessor
{
public:
    VST3PlusVocalsMatchProcessor();
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;

    juce::AudioProcessorValueTreeState state;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void setParam(const juce::String&, float);
    float outputPeak = 0.0f;

private:
    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> delay{192000};
    double sampleRateHz = 44100.0;
};
