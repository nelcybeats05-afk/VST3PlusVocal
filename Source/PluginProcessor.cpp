#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

static std::unique_ptr<juce::RangedAudioParameter> f(const char* id,const char* name,float lo,float hi,float def)
{
    return std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{id,1}, name, juce::NormalisableRange<float>{lo,hi}, def);
}

VST3PlusVocalsMatchProcessor::VST3PlusVocalsMatchProcessor()
: AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
                                  .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
  state(*this,nullptr,"STATE",createLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout VST3PlusVocalsMatchProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto A=[&](const char*i,const char*n,float a,float b,float d){p.push_back(f(i,n,a,b,d));};
    A("input","Input",-18,18,0); A("mix","Dry Wet",0,100,100); A("output","Output",-18,18,0);
    A("mcomp","Main Comp",0,100,35); A("msat","Main Sat",0,100,12); A("mair","Main Air",0,100,22);
    A("mrev","Main Reverb",0,100,18); A("mdel","Main Delay",0,100,8); A("mlevel","Main Level",-24,12,0);
    A("dwidth","Double Width",0,200,135); A("dtime","Double Timing",5,45,18); A("drev","Double Reverb",0,100,18);
    A("ddel","Double Delay",0,100,12); A("dlevel","Double Level",-48,6,-14);
    A("awidth","Adlib Width",0,200,155); A("adist","Adlib Dist",0,100,22); A("arev","Adlib Reverb",0,100,30);
    A("adel","Adlib Delay",0,100,28); A("alevel","Adlib Level",-48,6,-17);
    return {p.begin(),p.end()};
}

void VST3PlusVocalsMatchProcessor::setParam(const juce::String& id,float v)
{
    if(auto* q=state.getParameter(id))
        q->setValueNotifyingHost(q->convertTo0to1(v));
}

bool VST3PlusVocalsMatchProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    auto in=l.getMainInputChannelSet(), out=l.getMainOutputChannelSet();
    return in==out && (in==juce::AudioChannelSet::mono() || in==juce::AudioChannelSet::stereo());
}

void VST3PlusVocalsMatchProcessor::prepareToPlay(double sr,int bs)
{
    sampleRateHz=sr;
    juce::dsp::ProcessSpec spec{sr,(juce::uint32)bs,(juce::uint32)getTotalNumOutputChannels()};
    reverb.prepare(spec); reverb.reset();
    delay.prepare(spec); delay.reset();
}

void VST3PlusVocalsMatchProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
    juce::ScopedNoDenormals guard;
    if(b.getNumSamples()==0) return;

    juce::AudioBuffer<float> dry;
    dry.makeCopyOf(b);
    b.applyGain(juce::Decibels::decibelsToGain(state.getRawParameterValue("input")->load()));

    const float comp=state.getRawParameterValue("mcomp")->load()/100.0f;
    const float sat=state.getRawParameterValue("msat")->load()/100.0f;
    const float air=state.getRawParameterValue("mair")->load()/100.0f;

    for(int c=0;c<b.getNumChannels();++c)
    {
        auto* x=b.getWritePointer(c);
        float prev=0.0f;
        for(int n=0;n<b.getNumSamples();++n)
        {
            float v=x[n];
            float av=std::abs(v);
            if(av>0.32f)
            {
                float ratio=1.0f+comp*6.0f;
                v=std::copysign(0.32f+(av-0.32f)/ratio,v);
            }
            v=std::tanh(v*(1.0f+sat*3.5f))/(1.0f+sat);
            float hi=v-prev; prev=v;
            x[n]=v+hi*air*0.20f;
        }
    }

    juce::dsp::Reverb::Parameters rp;
    rp.roomSize=.42f; rp.damping=.52f; rp.width=1.0f;
    rp.wetLevel=state.getRawParameterValue("mrev")->load()/100.0f*.42f;
    rp.dryLevel=1.0f;
    reverb.setParameters(rp);
    juce::dsp::AudioBlock<float> block(b);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    reverb.process(ctx);

    float delMix=state.getRawParameterValue("mdel")->load()/100.0f;
    delay.setDelay((float)(sampleRateHz*.235));
    for(int c=0;c<b.getNumChannels();++c)
    {
        auto* x=b.getWritePointer(c);
        for(int n=0;n<b.getNumSamples();++n)
        {
            float d=delay.popSample(c);
            delay.pushSample(c,x[n]+d*.22f);
            x[n]+=d*delMix*.4f;
        }
    }

    // Safe stereo widening/parallel layers without extra DSP objects.
    if(b.getNumChannels()==2)
    {
        auto* L=b.getWritePointer(0); auto* R=b.getWritePointer(1);
        const float dw=state.getRawParameterValue("dwidth")->load()/200.0f;
        const float dl=juce::Decibels::decibelsToGain(state.getRawParameterValue("dlevel")->load());
        const float aw=state.getRawParameterValue("awidth")->load()/200.0f;
        const float al=juce::Decibels::decibelsToGain(state.getRawParameterValue("alevel")->load());
        const float dist=state.getRawParameterValue("adist")->load()/100.0f;
        for(int n=0;n<b.getNumSamples();++n)
        {
            float mono=.5f*(dry.getSample(0,n)+dry.getSample(1,n));
            float dbl=mono*dl;
            float ad=std::tanh(mono*(1.0f+dist*4.0f))*al;
            L[n]+=dbl*(1.0f+.18f*dw)+ad*(1.0f+.25f*aw);
            R[n]+=dbl*(1.0f-.18f*dw)+ad*(1.0f-.25f*aw);
        }
    }

    float wet=state.getRawParameterValue("mix")->load()/100.0f;
    for(int c=0;c<b.getNumChannels();++c)
        for(int n=0;n<b.getNumSamples();++n)
            b.setSample(c,n,dry.getSample(c,n)*(1.0f-wet)+b.getSample(c,n)*wet);

    b.applyGain(juce::Decibels::decibelsToGain(
        state.getRawParameterValue("mlevel")->load()+state.getRawParameterValue("output")->load()));
    outputPeak=b.getMagnitude(0,b.getNumSamples());
}

void VST3PlusVocalsMatchProcessor::getStateInformation(juce::MemoryBlock& d)
{
    if(auto xml=state.copyState().createXml()) copyXmlToBinary(*xml,d);
}
void VST3PlusVocalsMatchProcessor::setStateInformation(const void* d,int s)
{
    if(auto xml=getXmlFromBinary(d,s)) state.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* VST3PlusVocalsMatchProcessor::createEditor()
{
    return new VST3PlusVocalsMatchEditor(*this);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VST3PlusVocalsMatchProcessor();
}
