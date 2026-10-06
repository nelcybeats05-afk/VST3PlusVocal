#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class VST3PlusVocalsSafeV2Editor final:public juce::AudioProcessorEditor,private juce::Timer{public:explicit VST3PlusVocalsSafeV2Editor(VST3PlusVocalsSafeV2Processor&);void paint(juce::Graphics&)override;void resized()override;private:VST3PlusVocalsSafeV2Processor&p;struct K{juce::Slider s;juce::Label l;VST3PlusVocalsSafeV2Processor::Param pm;};std::vector<std::unique_ptr<K>>k;juce::TextButton load{"LOAD VOCAL WAV"},match{"ANALYSE + BUILD CHAIN"};juce::Label file,status,mainR,dblR,adR;std::unique_ptr<juce::FileChooser>chooser;juce::File ref;void addK(const char*,VST3PlusVocalsSafeV2Processor::Param);void analyse();void timerCallback()override;};
