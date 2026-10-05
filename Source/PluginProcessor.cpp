#include "PluginProcessor.h"
#include "PluginEditor.h"
VST3PlusVocalAudioProcessor::VST3PlusVocalAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
                                  .withOutput("Output",juce::AudioChannelSet::stereo(),true)) {}
bool VST3PlusVocalAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
 auto in=l.getMainInputChannelSet(); auto out=l.getMainOutputChannelSet();
 return in==out && (in==juce::AudioChannelSet::mono() || in==juce::AudioChannelSet::stereo());
}
void VST3PlusVocalAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {
 juce::ScopedNoDenormals n;
 for(int c=getTotalNumInputChannels();c<getTotalNumOutputChannels();++c) b.clear(c,0,b.getNumSamples());
}
juce::AudioProcessorEditor* VST3PlusVocalAudioProcessor::createEditor(){ return new VST3PlusVocalAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){ return new VST3PlusVocalAudioProcessor(); }
