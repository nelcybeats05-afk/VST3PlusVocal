#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VST3PlusVocalAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
 explicit VST3PlusVocalAudioProcessorEditor(VST3PlusVocalAudioProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 VST3PlusVocalAudioProcessor& p;
 using SA=juce::AudioProcessorValueTreeState::SliderAttachment;
 struct K { juce::Slider s; juce::Label l; std::unique_ptr<SA> a; };
 std::vector<std::unique_ptr<K>> knobs;
 juce::ComboBox style,key,scale,artist;
 juce::TextButton generate{"GENERATE / APPLY CHAIN"}, loadRef{"LOAD REFERENCE SONG"}, analyse{"READ / ANALYSE"}, autoKey{"AUTO KEY"};
 juce::Label status, ai;
 juce::FileChooser chooser{"Choose a reference song",{}, "*.wav;*.mp3;*.aiff;*.flac"};
 void addKnob(const juce::String&,const juce::String&);
 void applyStyle();
 void timerCallback() override { repaint(); }
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST3PlusVocalAudioProcessorEditor)
};
