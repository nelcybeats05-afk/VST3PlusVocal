#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VST3PlusVocalsDesignV3Editor final : public juce::AudioProcessorEditor
{
public:
 explicit VST3PlusVocalsDesignV3Editor(VST3PlusVocalsDesignV3Processor&);
 void paint(juce::Graphics&) override;
 void resized() override;

private:
 struct Knob { juce::Slider slider; juce::Label label; };
 std::vector<std::unique_ptr<Knob>> knobs;
 juce::TextButton load{"LOAD REFERENCE"}, analyse{"ANALYSE"};
 juce::Label file,status;
 juce::ComboBox key,scale;
 std::unique_ptr<juce::FileChooser> chooser;
 juce::File reference;

 void addKnob(const juce::String&);
 void loadFile();
 void analyseFile();
 void drawPanel(juce::Graphics&, juce::Rectangle<int>, const juce::String&, juce::Colour);
};
