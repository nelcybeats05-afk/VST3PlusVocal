#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VST3PlusVocalsMatchEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VST3PlusVocalsMatchEditor(VST3PlusVocalsMatchProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    VST3PlusVocalsMatchProcessor& p;
    using Att=juce::AudioProcessorValueTreeState::SliderAttachment;
    struct K { juce::Slider s; juce::Label l; std::unique_ptr<Att> a; };
    std::vector<std::unique_ptr<K>> knobs;

    juce::TextButton load{"LOAD VOCAL WAV"};
    juce::TextButton match{"ANALYSE + MATCH EFFECTS"};
    juce::Label fileLabel,statusLabel,mainResult,doubleResult,adlibResult;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::File reference;

    void addKnob(const char*,const char*);
    void analyse();
    void timerCallback() override { repaint(); }
};
