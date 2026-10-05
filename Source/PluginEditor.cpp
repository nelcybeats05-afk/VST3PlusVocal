#include "PluginEditor.h"

VST3PlusVocalAudioProcessorEditor::VST3PlusVocalAudioProcessorEditor(
    VST3PlusVocalAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(700, 420);
}

void VST3PlusVocalAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(12, 14, 24));

    g.setColour(juce::Colour::fromRGB(130, 65, 255));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(12.0f), 16.0f, 2.0f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(30.0f, juce::Font::bold));
    g.drawText("VST3PlusVocal", 35, 35, getWidth() - 70, 50,
               juce::Justification::centredLeft);

    g.setColour(juce::Colour::fromRGB(190, 175, 255));
    g.setFont(18.0f);
    g.drawText("by 29YURO", 35, 85, getWidth() - 70, 35,
               juce::Justification::centredLeft);

    g.setColour(juce::Colours::lightgrey);
    g.setFont(17.0f);
    g.drawText("Clean VST3 base - audio passthrough test",
               35, 180, getWidth() - 70, 40, juce::Justification::centred);

    g.drawText("If this opens in FL Studio, the base plugin is working.",
               35, 225, getWidth() - 70, 40, juce::Justification::centred);
}
