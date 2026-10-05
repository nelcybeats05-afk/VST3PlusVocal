#include "PluginEditor.h"
VST3PlusVocalAudioProcessorEditor::VST3PlusVocalAudioProcessorEditor(VST3PlusVocalAudioProcessor& p)
: AudioProcessorEditor(&p){ setSize(700,420); }
void VST3PlusVocalAudioProcessorEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour::fromRGB(12,14,24));
 g.setColour(juce::Colours::white); g.setFont(30.0f);
 g.drawText("VST3PlusVocal",getLocalBounds(),juce::Justification::centred);
}
