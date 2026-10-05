#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VST3PlusVocalsAudioProcessorEditor final
    : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VST3PlusVocalsAudioProcessorEditor(VST3PlusVocalsAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    VST3PlusVocalsAudioProcessor& p;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<Attachment> attachment;
    };
    std::vector<std::unique_ptr<Knob>> knobs;

    juce::ComboBox style, key, scale;
    juce::TextEditor aiPrompt;
    juce::TextButton applyPrompt { "BUILD AI CHAIN" };
    juce::TextButton loadReference { "LOAD WAV / SONG" };
    juce::TextButton analyseReference { "ANALYSE + MATCH" };
    juce::Label referenceLabel, aiStatus, analysisLabel;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::File referenceFile;

    void addKnob(const char*, const char*);
    void applyPromptChain();
    void analyseAndMatchReference();
    void setParam(const char*, float);
    void timerCallback() override { repaint(); }
};
