#include "PluginEditor.h"

VST3PlusVocalAudioProcessorEditor::VST3PlusVocalAudioProcessorEditor(VST3PlusVocalAudioProcessor& pr):AudioProcessorEditor(&pr),p(pr){
 setSize(1180,760);
 style.addItemList({"Clean Trap","Dark Trap","Airy","Distorted","Wide Doubles","Underground","R&B","Custom"},1); style.setSelectedId(1);
 artist.addItemList({"Custom Artist Style","Modern Trap","Melodic Rap","Dark Vocal","Wide Vocal"},1); artist.setSelectedId(1);
 key.addItemList({"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},1); key.setSelectedId(1);
 scale.addItemList({"Minor","Major"},1); scale.setSelectedId(1);
 for(auto* c:{&style,&artist,&key,&scale}) addAndMakeVisible(*c);
 for(auto* b:{&generate,&loadRef,&analyse,&autoKey}) addAndMakeVisible(*b);
 addAndMakeVisible(status); addAndMakeVisible(ai);
 status.setText("Reference: none",juce::dontSendNotification); ai.setText("AI ASSISTANT: load or record audio to analyse",juce::dontSendNotification);
 status.setColour(juce::Label::textColourId,juce::Colours::lightgrey); ai.setColour(juce::Label::textColourId,juce::Colour(0xffc9b6ff));
 const std::pair<const char*,const char*> ks[]={
 {"Tune","tune"},{"Comp","comp"},{"De-Esser","deesser"},{"EQ","eq"},{"Saturation","sat"},{"Air","air"},{"Reverb","reverb"},{"Delay","delay"},{"Level","mainlevel"},
 {"Width","dwidth"},{"Timing","dtiming"},{"Pitch","dpitch"},{"EQ","deq"},{"Reverb","drev"},{"Delay","ddelay"},{"Pan","dpan"},{"Level","dlevel"},
 {"Width","awidth"},{"Pitch","apitch"},{"Distortion","adist"},{"Filter","afilter"},{"Delay","adelay"},{"Reverb","arev"},{"Pan","apan"},{"Level","alevel"},
 {"Input","input"},{"Dry/Wet","drywet"},{"Output","output"}};
 for(auto& x:ks) addKnob(x.first,x.second);
 generate.onClick=[this]{applyStyle();};
 loadRef.onClick=[this]{
  chooser.launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[this](const juce::FileChooser& f){
   auto file=f.getResult(); if(file.existsAsFile()) status.setText("Reference: "+file.getFileName(),juce::dontSendNotification);
  });
 };
 analyse.onClick=[this]{ ai.setText("AI ASSISTANT: signal ready - timing/level analysis runs locally",juce::dontSendNotification); };
 autoKey.onClick=[this]{ ai.setText("AUTO KEY: analysis mode armed",juce::dontSendNotification); };
 startTimerHz(20);
}
void VST3PlusVocalAudioProcessorEditor::addKnob(const juce::String& n,const juce::String& id){
 auto k=std::make_unique<K>(); k->s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); k->s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,58,18);
 k->l.setText(n,juce::dontSendNotification); k->l.setJustificationType(juce::Justification::centred); k->l.setColour(juce::Label::textColourId,juce::Colours::white);
 addAndMakeVisible(k->s); addAndMakeVisible(k->l); k->a=std::make_unique<SA>(p.apvts,id,k->s); knobs.push_back(std::move(k));
}
void VST3PlusVocalAudioProcessorEditor::applyStyle(){
 auto set=[this](const char* id,float v){ if(auto* q=p.apvts.getParameter(id)) q->setValueNotifyingHost(q->convertTo0to1(v)); };
 switch(style.getSelectedId()){case 2:set("reverb",12);set("sat",28);set("air",10);break;case 3:set("reverb",35);set("air",55);set("sat",5);break;case 4:set("sat",70);set("reverb",8);break;case 5:set("dwidth",180);set("dlevel",-7);break;default:set("reverb",18);set("sat",10);set("air",20);break;}
 ai.setText("CHAIN APPLIED: "+style.getText(),juce::dontSendNotification);
}
void VST3PlusVocalAudioProcessorEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour(0xff090b13)); auto purple=juce::Colour(0xff8d5cff);
 g.setColour(purple); g.fillRect(0,0,getWidth(),5); g.setColour(juce::Colours::white); g.setFont(juce::Font(28.0f,juce::Font::bold));
 g.drawText("VST3 VocalChain+",22,14,400,38,juce::Justification::centredLeft); g.setFont(14); g.setColour(juce::Colour(0xffbba7ff)); g.drawText("by 29yuro",24,50,180,20,juce::Justification::centredLeft);
 auto panel=[&](juce::Rectangle<int> r,const juce::String& t){g.setColour(juce::Colour(0xff111522));g.fillRoundedRectangle(r.toFloat(),10);g.setColour(juce::Colour(0xff29223e));g.drawRoundedRectangle(r.toFloat(),10,1);g.setColour(purple);g.setFont(15);g.drawText(t,r.removeFromTop(28).reduced(10,0),juce::Justification::centredLeft);};
 panel({18,88,360,125},"VOCAL STYLE / ARTIST CHAIN"); panel({395,88,360,125},"REFERENCE SONG"); panel({772,88,390,125},"AI ASSISTANT / KEY & SCALE");
 panel({18,232,1144,150},"MAIN VOCAL"); panel({18,397,1144,150},"DOUBLES"); panel({18,562,1144,150},"ADLIBS / MIX");
 g.setColour(juce::Colour(0xff22283a)); g.fillRect(25,720,1110,8); g.setColour(purple); g.fillRect(25,720,(int)(1110*juce::jlimit(0.f,1.f,p.outputPeak)),8);
}
void VST3PlusVocalAudioProcessorEditor::resized(){
 style.setBounds(32,122,155,28); artist.setBounds(197,122,165,28); generate.setBounds(32,160,330,34);
 loadRef.setBounds(410,124,330,32); status.setBounds(410,163,330,28);
 analyse.setBounds(788,122,145,30); autoKey.setBounds(943,122,95,30); key.setBounds(1048,122,48,30); scale.setBounds(1102,122,55,30); ai.setBounds(788,163,360,30);
 int i=0; auto row=[&](int y,int count){int x=34,w=112;for(int n=0;n<count;n++,i++){knobs[i]->l.setBounds(x,y,w,20);knobs[i]->s.setBounds(x,y+18,w,95);x+=123;}};
 row(255,9); row(420,8); row(585,8);
 for(;i<(int)knobs.size();++i){int x=800+(i-25)*112;knobs[i]->l.setBounds(x,585,100,20);knobs[i]->s.setBounds(x,603,100,95);}
}
