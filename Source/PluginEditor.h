#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class VST3PlusVocalAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
 explicit VST3PlusVocalAudioProcessorEditor(VST3PlusVocalAudioProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override {}
};
