#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
class VST3PlusVocalsSafeV2Processor final : public juce::AudioProcessor {
public:
 VST3PlusVocalsSafeV2Processor(); void prepareToPlay(double,int) override; void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override; void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override{return true;} const juce::String getName() const override{return JucePlugin_Name;}
 bool acceptsMidi() const override{return false;} bool producesMidi() const override{return false;} bool isMidiEffect() const override{return false;} double getTailLengthSeconds() const override{return 2.0;}
 int getNumPrograms() override{return 1;} int getCurrentProgram() override{return 0;} void setCurrentProgram(int) override{} const juce::String getProgramName(int) override{return{};} void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
 enum Param{mainComp,mainSat,mainAir,mainRev,mainDelay,mainLevel,dblWidth,dblTime,dblRev,dblDelay,dblLevel,adWidth,adDist,adRev,adDelay,adLevel,inputGain,dryWet,outputGain,paramCount};
 std::array<juce::AudioParameterFloat*,paramCount> params{}; void setParam(Param,float); float getParam(Param)const; float meter=0;
 static float mins[paramCount],maxs[paramCount],defs[paramCount];
private:
 double sr=44100; std::vector<float> mainL,mainR,dblL,dblR,adL,adR; int mainPos=0,dblPos=0,adPos=0; float revL=0,revR=0;
 static const char* ids[paramCount]; static const char* names[paramCount];
};
