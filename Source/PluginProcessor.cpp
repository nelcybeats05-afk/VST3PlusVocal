#include "PluginProcessor.h"
#include "PluginEditor.h"
VST3PlusVocalsLoadFirstProcessor::VST3PlusVocalsLoadFirstProcessor()
: AudioProcessor(BusesProperties()
 .withInput("Input",juce::AudioChannelSet::stereo(),true)
 .withOutput("Output",juce::AudioChannelSet::stereo(),true)) {}
bool VST3PlusVocalsLoadFirstProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
 const auto in=l.getMainInputChannelSet(),out=l.getMainOutputChannelSet();
 return in==out&&(in==juce::AudioChannelSet::mono()||in==juce::AudioChannelSet::stereo());
}
void VST3PlusVocalsLoadFirstProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
 juce::ScopedNoDenormals n;
 for(int c=getTotalNumInputChannels();c<getTotalNumOutputChannels();++c)
  b.clear(c,0,b.getNumSamples());
}
juce::AudioProcessorEditor* VST3PlusVocalsLoadFirstProcessor::createEditor()
{return new VST3PlusVocalsLoadFirstEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{return new VST3PlusVocalsLoadFirstProcessor();}
