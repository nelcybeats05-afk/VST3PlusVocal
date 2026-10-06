#include "PluginProcessor.h"
#include "PluginEditor.h"
VST3PlusVocalsDesignV3Processor::VST3PlusVocalsDesignV3Processor()
: AudioProcessor(BusesProperties()
 .withInput("Input",juce::AudioChannelSet::stereo(),true)
 .withOutput("Output",juce::AudioChannelSet::stereo(),true)) {}
bool VST3PlusVocalsDesignV3Processor::isBusesLayoutSupported(const BusesLayout& l) const
{
 const auto in=l.getMainInputChannelSet(),out=l.getMainOutputChannelSet();
 return in==out&&(in==juce::AudioChannelSet::mono()||in==juce::AudioChannelSet::stereo());
}
void VST3PlusVocalsDesignV3Processor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
 juce::ScopedNoDenormals n;
 for(int c=getTotalNumInputChannels();c<getTotalNumOutputChannels();++c)
  b.clear(c,0,b.getNumSamples());
}
juce::AudioProcessorEditor* VST3PlusVocalsDesignV3Processor::createEditor()
{return new VST3PlusVocalsDesignV3Editor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{return new VST3PlusVocalsDesignV3Processor();}
