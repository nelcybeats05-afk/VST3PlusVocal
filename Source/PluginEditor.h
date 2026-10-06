#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class VST3PlusVocalsLoadFirstEditor final : public juce::AudioProcessorEditor
{
public:
 explicit VST3PlusVocalsLoadFirstEditor(VST3PlusVocalsLoadFirstProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 juce::TextButton load{"LOAD REFERENCE"};
 juce::TextButton analyse{"ANALYSE"};
 juce::Label file,status,mainTitle,doublesTitle,adlibsTitle;
 std::unique_ptr<juce::FileChooser> chooser;
 juce::File reference;
 void loadFile();
 void analyseFile();
};
