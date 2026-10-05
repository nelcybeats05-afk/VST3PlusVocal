#include "PluginProcessor.h"
#include "PluginEditor.h"
VST3PlusVocalsSafeProcessor::VST3PlusVocalsSafeProcessor()
: AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
                                  .withOutput("Output",juce::AudioChannelSet::stereo(),true)) {}
bool VST3PlusVocalsSafeProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
 auto i=l.getMainInputChannelSet(),o=l.getMainOutputChannelSet();
 return i==o && (i==juce::AudioChannelSet::mono() || i==juce::AudioChannelSet::stereo());
}
void VST3PlusVocalsSafeProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
 juce::ScopedNoDenormals n;
 for(int c=getTotalNumInputChannels();c<getTotalNumOutputChannels();++c) b.clear(c,0,b.getNumSamples());
}
juce::AudioProcessorEditor* VST3PlusVocalsSafeProcessor::createEditor(){return new VST3PlusVocalsSafeEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new VST3PlusVocalsSafeProcessor();}
