#include "PluginEditor.h"
#include <cmath>

VST3PlusVocalsDesignV3Editor::VST3PlusVocalsDesignV3Editor(VST3PlusVocalsDesignV3Processor& p)
: AudioProcessorEditor(&p)
{
 setSize(1280,800);
 addAndMakeVisible(load); addAndMakeVisible(analyse);
 addAndMakeVisible(file); addAndMakeVisible(status);
 file.setText("No reference loaded",juce::dontSendNotification);
 status.setText("Vocal WAV recommended • normal WAV / AIFF / FLAC also supported",juce::dontSendNotification);
 for(auto* l:{&file,&status}) l->setColour(juce::Label::textColourId,juce::Colours::white);

 addAndMakeVisible(key); addAndMakeVisible(scale);
 for(auto s:{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}) key.addItem(s,key.getNumItems()+1);
 for(auto s:{"Minor","Major"}) scale.addItem(s,scale.getNumItems()+1);
 key.setSelectedId(2); scale.setSelectedId(1);

 const char* labels[]={
  "Tune","Comp","De-Esser","EQ","Saturation","Air","Reverb","Delay","Level",
  "Width","Timing","Pitch","EQ","Reverb","Delay","Pan","Level",
  "Width","Pitch","Distortion","Filter","Delay","Reverb","Pan","Level",
  "EQ","Threshold","Ratio","Attack","Release","Reverb","Size","Decay","Pre-Delay",
  "Delay Time","Feedback","Delay Mix","Drive","Sat Mix","Chorus Rate","Chorus Depth",
  "Input","Dry / Wet","Main Mix","Output"};
 for(auto* n:labels) addKnob(n);

 load.onClick=[this]{loadFile();};
 analyse.onClick=[this]{analyseFile();};
}

void VST3PlusVocalsDesignV3Editor::addKnob(const juce::String& name)
{
 auto k=std::make_unique<Knob>();
 k->label.setText(name,juce::dontSendNotification);
 k->label.setJustificationType(juce::Justification::centred);
 k->label.setColour(juce::Label::textColourId,juce::Colours::white);
 k->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
 k->slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,52,16);
 k->slider.setRange(0,100,1); k->slider.setValue(50);
 k->slider.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xff934dff));
 k->slider.setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff242a38));
 k->slider.setColour(juce::Slider::thumbColourId,juce::Colour(0xffd8c8ff));
 addAndMakeVisible(k->label); addAndMakeVisible(k->slider);
 knobs.push_back(std::move(k));
}

void VST3PlusVocalsDesignV3Editor::loadFile()
{
 chooser=std::make_unique<juce::FileChooser>("Choose vocal or song reference",juce::File{},"*.wav;*.aiff;*.aif;*.flac");
 chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
 [this](const juce::FileChooser& fc){
  auto f=fc.getResult();
  if(f.existsAsFile()){
   reference=f; file.setText("Loaded: "+f.getFileName(),juce::dontSendNotification);
   status.setText("Reference loaded • press ANALYSE",juce::dontSendNotification);
  }
 });
}

void VST3PlusVocalsDesignV3Editor::analyseFile()
{
 if(!reference.existsAsFile()){status.setText("Load a reference first",juce::dontSendNotification);return;}
 juce::AudioFormatManager fm; fm.registerBasicFormats();
 std::unique_ptr<juce::AudioFormatReader> r(fm.createReaderFor(reference));
 if(!r){status.setText("Audio could not be decoded",juce::dontSendNotification);return;}
 auto len=juce::jmin<juce::int64>(r->lengthInSamples,(juce::int64)(r->sampleRate*60.0));
 if(len<=0){status.setText("Reference contains no audio",juce::dontSendNotification);return;}
 int ch=juce::jlimit(1,2,(int)r->numChannels);
 juce::AudioBuffer<float>a(ch,(int)len);
 if(!r->read(&a,0,(int)len,0,true,true)){status.setText("Reference read failed",juce::dontSendNotification);return;}
 double sq=0,diff=0,side=0,mid=0;float peak=0;long long count=0;
 for(int c=0;c<ch;++c){auto*x=a.getReadPointer(c);float prev=0;for(int i=0;i<(int)len;++i){float v=x[i];sq+=(double)v*v;float d=v-prev;diff+=(double)d*d;prev=v;peak=juce::jmax(peak,std::abs(v));++count;}}
 if(ch==2){auto*l=a.getReadPointer(0);auto*rr=a.getReadPointer(1);for(int i=0;i<(int)len;++i){float m=.5f*(l[i]+rr[i]),s=.5f*(l[i]-rr[i]);mid+=(double)m*m;side+=(double)s*s;}}
 double rms=std::sqrt(sq/juce::jmax<long long>(1,count));
 double bright=std::sqrt(diff/juce::jmax<long long>(1,count))/juce::jmax(1e-7,rms);
 double width=ch==2?std::sqrt(side/juce::jmax(1.0,mid)):0.0;
 // UI-only matching in this stable design build.
 double seed[]={35+bright*8,15+bright*10,35,45+bright*5,15+bright*8,25+bright*15,15+width*20,8+width*15,50,
                55+width*35,35+width*20,20,45,20+width*20,15+width*15,50,40,
                65+width*30,25,25+bright*10,45,30+width*20,30+width*20,50,35,
                50,35,40,25,45,35,45,50,25,30,40,35,25,35,40,45,50,60,50,50};
 for(size_t i=0;i<knobs.size();++i)knobs[i]->slider.setValue(juce::jlimit(0.0,100.0,seed[i]),juce::dontSendNotification);
 status.setText("Reference analysed • MAIN / DOUBLES / ADLIBS controls matched",juce::dontSendNotification);
}

void VST3PlusVocalsDesignV3Editor::drawPanel(juce::Graphics& g,juce::Rectangle<int> r,const juce::String& title,juce::Colour accent)
{
 g.setColour(juce::Colour(0xff0c1018));g.fillRoundedRectangle(r.toFloat(),8);
 g.setColour(juce::Colour(0xff293044));g.drawRoundedRectangle(r.toFloat(),8,1);
 g.setColour(accent);g.setFont(17.f);g.drawText(title,r.removeFromTop(34).reduced(12,4),juce::Justification::centredLeft);
}

void VST3PlusVocalsDesignV3Editor::paint(juce::Graphics& g)
{
 auto purple=juce::Colour(0xff914cff),cyan=juce::Colour(0xff36d9ff);
 g.fillAll(juce::Colour(0xff05070c));
 // subtle cracked/tech frame substitute
 g.setColour(juce::Colour(0xff151a24)); for(int x=0;x<getWidth();x+=48) g.drawVerticalLine(x,0.f,(float)getHeight());
 g.setColour(purple);g.fillRect(0,0,getWidth(),4);
 g.setColour(juce::Colours::white);g.setFont(32.f);g.drawText("VST3 VocalChain +",26,12,390,42,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xffc8b5ff));g.setFont(13.f);g.drawText("by 29yuro",188,50,120,18,juce::Justification::centredLeft);

 drawPanel(g,{20,76,820,126},"REFERENCE SONG / VOCAL",purple);
 drawPanel(g,{854,76,406,126},"KEY & SCALE",purple);
 drawPanel(g,{20,216,400,245},"MAIN VOCAL",purple);
 drawPanel(g,{440,216,400,245},"DOUBLES",juce::Colour(0xff7255ff));
 drawPanel(g,{860,216,400,245},"ADLIBS",juce::Colour(0xffa84cff));
 drawPanel(g,{20,475,1240,205},"MORE EFFECTS  •  EQ  •  COMPRESSOR  •  REVERB  •  DELAY  •  SATURATION  •  CHORUS",cyan);
 drawPanel(g,{20,692,1240,90},"INPUT     •     DRY / WET     •     MAIN MIX     •     OUTPUT",purple);
}

void VST3PlusVocalsDesignV3Editor::resized()
{
 load.setBounds(42,116,190,34); analyse.setBounds(242,116,150,34);
 file.setBounds(410,108,405,24); status.setBounds(410,140,405,26);
 key.setBounds(880,116,150,32);scale.setBounds(1042,116,180,32);

 int i=0;
 auto place=[&](int start,int count,int x,int y,int cols,int dx,int dy){
  for(int j=0;j<count;++j){auto&k=*knobs[(size_t)(start+j)];int cx=x+(j%cols)*dx,cy=y+(j/cols)*dy;k.label.setBounds(cx,cy,82,18);k.slider.setBounds(cx,cy+17,82,82);}
 };
 place(0,9,34,257,5,74,102);
 place(9,8,454,257,4,92,102);
 place(17,8,874,257,4,92,102);
 place(25,16,35,518,8,150,88);
 // bottom 4
 place(41,4,170,704,4,275,70);
}
