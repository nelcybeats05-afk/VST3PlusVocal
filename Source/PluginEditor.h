#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class VST3PlusVocalsSafeEditor final : public juce::AudioProcessorEditor
{
public:
 explicit VST3PlusVocalsSafeEditor(VST3PlusVocalsSafeProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 juce::TextButton load{"LOAD VOCAL WAV"}, analyse{"ANALYSE VOCAL"};
 juce::Label file,status,mainL,doublesL,adlibsL;
 std::unique_ptr<juce::FileChooser> chooser;
 juce::File ref;
 std::array<juce::Slider,12> knobs;
 std::array<juce::Label,12> labels;
 void runAnalysis();
};
