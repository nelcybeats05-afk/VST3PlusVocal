#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VST3PlusVocalAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit VST3PlusVocalAudioProcessorEditor(VST3PlusVocalAudioProcessor&);
    ~VST3PlusVocalAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override {}

private:
    VST3PlusVocalAudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST3PlusVocalAudioProcessorEditor)
};
