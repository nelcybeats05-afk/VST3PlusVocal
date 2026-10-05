#include "PluginProcessor.h"
#include "PluginEditor.h"

static std::unique_ptr<juce::RangedAudioParameter> fp(const char* id,const char* name,float mn,float mx,float def) {
 return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(mn,mx),def);
}
VST3PlusVocalAudioProcessor::VST3PlusVocalAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
                                  .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
  apvts(*this,nullptr,"STATE",createLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout VST3PlusVocalAudioProcessor::createLayout(){
 std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
 p.push_back(fp("input","Input",-24,24,0)); p.push_back(fp("drywet","DryWet",0,100,100)); p.push_back(fp("output","Output",-24,24,0));
 p.push_back(fp("tune","Tune",0,100,25)); p.push_back(fp("comp","Comp",0,100,35)); p.push_back(fp("deesser","DeEsser",0,100,25));
 p.push_back(fp("eq","EQ",-12,12,0)); p.push_back(fp("sat","Saturation",0,100,10)); p.push_back(fp("air","Air",0,100,20));
 p.push_back(fp("reverb","Reverb",0,100,18)); p.push_back(fp("delay","Delay",0,100,8)); p.push_back(fp("mainlevel","Main Level",-24,12,0));
 p.push_back(fp("dwidth","Double Width",0,200,120)); p.push_back(fp("dtiming","Double Timing",0,100,20)); p.push_back(fp("dpitch","Double Pitch",-24,24,0));
 p.push_back(fp("deq","Double EQ",-12,12,0)); p.push_back(fp("drev","Double Reverb",0,100,20)); p.push_back(fp("ddelay","Double Delay",0,100,10));
 p.push_back(fp("dpan","Double Pan",-100,100,0)); p.push_back(fp("dlevel","Double Level",-48,12,-12));
 p.push_back(fp("awidth","Adlib Width",0,200,140)); p.push_back(fp("apitch","Adlib Pitch",-24,24,0)); p.push_back(fp("adist","Adlib Dist",0,100,15));
 p.push_back(fp("afilter","Adlib Filter",0,100,50)); p.push_back(fp("adelay","Adlib Delay",0,100,25)); p.push_back(fp("arev","Adlib Reverb",0,100,30));
 p.push_back(fp("apan","Adlib Pan",-100,100,0)); p.push_back(fp("alevel","Adlib Level",-48,12,-15));
 return {p.begin(),p.end()};
}
bool VST3PlusVocalAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const{
 auto i=l.getMainInputChannelSet(),o=l.getMainOutputChannelSet();
 return i==o && (i==juce::AudioChannelSet::mono()||i==juce::AudioChannelSet::stereo());
}
void VST3PlusVocalAudioProcessor::prepareToPlay(double s,int block){
 sr=s;
 juce::dsp::ProcessSpec spec{s,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};
 reverb.prepare(spec); reverb.reset(); delay.prepare(spec); delay.reset();
}
void VST3PlusVocalAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals nd;
 auto dry=b;
 auto inGain=juce::Decibels::decibelsToGain(apvts.getRawParameterValue("input")->load());
 b.applyGain(inGain);
 inputPeak=b.getMagnitude(0,b.getNumSamples());
 float sat=apvts.getRawParameterValue("sat")->load()/100.0f;
 for(int c=0;c<b.getNumChannels();++c){ auto* x=b.getWritePointer(c); for(int n=0;n<b.getNumSamples();++n) x[n]=std::tanh(x[n]*(1.0f+sat*3.0f))/(1.0f+sat); }
 juce::dsp::Reverb::Parameters rp; rp.roomSize=.35f; rp.damping=.45f; rp.width=1.0f;
 rp.wetLevel=apvts.getRawParameterValue("reverb")->load()/100.0f*.45f; rp.dryLevel=1.0f; reverb.setParameters(rp);
 juce::dsp::AudioBlock<float> block(b); juce::dsp::ProcessContextReplacing<float> ctx(block); reverb.process(ctx);
 float dw=apvts.getRawParameterValue("drywet")->load()/100.0f;
 for(int c=0;c<b.getNumChannels();++c) for(int n=0;n<b.getNumSamples();++n) b.setSample(c,n,dry.getSample(c,n)*(1-dw)+b.getSample(c,n)*dw);
 b.applyGain(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("output")->load()));
 outputPeak=b.getMagnitude(0,b.getNumSamples());
}
void VST3PlusVocalAudioProcessor::getStateInformation(juce::MemoryBlock& d){ if(auto xml=apvts.copyState().createXml()) copyXmlToBinary(*xml,d); }
void VST3PlusVocalAudioProcessor::setStateInformation(const void* d,int s){ if(auto xml=getXmlFromBinary(d,s)) apvts.replaceState(juce::ValueTree::fromXml(*xml)); }
juce::AudioProcessorEditor* VST3PlusVocalAudioProcessor::createEditor(){ return new VST3PlusVocalAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){ return new VST3PlusVocalAudioProcessor(); }
