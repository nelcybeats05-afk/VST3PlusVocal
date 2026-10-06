#include "PluginEditor.h"
#include <cmath>
VST3PlusVocalsLoadFirstEditor::VST3PlusVocalsLoadFirstEditor(VST3PlusVocalsLoadFirstProcessor& p)
:AudioProcessorEditor(&p)
{
 setSize(1120,700);
 for(auto*b:{&load,&analyse})addAndMakeVisible(*b);
 for(auto*l:{&file,&status,&mainTitle,&doublesTitle,&adlibsTitle})
 {
  addAndMakeVisible(*l);
  l->setColour(juce::Label::textColourId,juce::Colours::white);
 }
 file.setText("No reference loaded",juce::dontSendNotification);
 status.setText("Load an isolated vocal WAV for best results, or a normal WAV/AIFF/FLAC song.",juce::dontSendNotification);
 mainTitle.setText("MAIN VOCAL",juce::dontSendNotification);
 doublesTitle.setText("DOUBLES",juce::dontSendNotification);
 adlibsTitle.setText("ADLIBS",juce::dontSendNotification);
 load.onClick=[this]{loadFile();};
 analyse.onClick=[this]{analyseFile();};
}
void VST3PlusVocalsLoadFirstEditor::loadFile()
{
 chooser=std::make_unique<juce::FileChooser>(
  "Choose vocal or song reference",juce::File{},"*.wav;*.aiff;*.aif;*.flac");
 chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
 [this](const juce::FileChooser& fc)
 {
  auto f=fc.getResult();
  if(f.existsAsFile())
  {
   reference=f;
   file.setText("Loaded: "+f.getFileName(),juce::dontSendNotification);
   status.setText("Reference loaded. Press ANALYSE.",juce::dontSendNotification);
  }
 });
}
void VST3PlusVocalsLoadFirstEditor::analyseFile()
{
 if(!reference.existsAsFile())
 {status.setText("Load a reference first.",juce::dontSendNotification);return;}
 juce::AudioFormatManager fm;fm.registerBasicFormats();
 std::unique_ptr<juce::AudioFormatReader> r(fm.createReaderFor(reference));
 if(!r){status.setText("Audio could not be decoded.",juce::dontSendNotification);return;}
 auto len=juce::jmin<juce::int64>(r->lengthInSamples,(juce::int64)(r->sampleRate*60.0));
 if(len<=0){status.setText("Reference contains no audio.",juce::dontSendNotification);return;}
 int ch=juce::jlimit(1,2,(int)r->numChannels);
 juce::AudioBuffer<float>a(ch,(int)len);
 if(!r->read(&a,0,(int)len,0,true,true))
 {status.setText("Reference read failed.",juce::dontSendNotification);return;}
 double sq=0,diff=0;float peak=0;long long count=0;
 for(int c=0;c<ch;++c)
 {
  auto*x=a.getReadPointer(c);float prev=0;
  for(int i=0;i<(int)len;++i)
  {
   float v=x[i];sq+=(double)v*v;
   float d=v-prev;diff+=(double)d*d;prev=v;
   peak=juce::jmax(peak,std::abs(v));++count;
  }
 }
 const double rms=std::sqrt(sq/juce::jmax<long long>(1,count));
 const double brightness=std::sqrt(diff/juce::jmax<long long>(1,count))/juce::jmax(1.0e-7,rms);
 status.setText("Analysis OK  |  Level "+juce::String(juce::Decibels::gainToDecibels((float)rms,-100.f),1)
                +" dB  |  Peak "+juce::String(juce::Decibels::gainToDecibels(peak,-100.f),1)
                +" dB  |  Tone "+juce::String(brightness,2),juce::dontSendNotification);
}
void VST3PlusVocalsLoadFirstEditor::paint(juce::Graphics& g)
{
 auto purple=juce::Colour(0xff8d4dff);
 g.fillAll(juce::Colour(0xff070910));
 g.setColour(purple);g.fillRect(0,0,getWidth(),5);
 g.setColour(juce::Colours::white);g.setFont(30.f);
 g.drawText("VST3 VocalChain +",24,15,420,40,juce::Justification::centredLeft);
 g.setFont(14.f);g.setColour(juce::Colour(0xffc2b0ff));
 g.drawText("by 29yuro  •  LOAD-FIRST BUILD",26,54,400,22,juce::Justification::centredLeft);
 auto box=[&](int x,int y,int w,int h,const char*t)
 {
  g.setColour(juce::Colour(0xff10141f));g.fillRoundedRectangle((float)x,(float)y,(float)w,(float)h,10);
  g.setColour(juce::Colour(0xff332650));g.drawRoundedRectangle({(float)x,(float)y,(float)w,(float)h},10,1);
  g.setColour(purple);g.drawText(t,x+14,y+9,w-28,22,juce::Justification::centredLeft);
 };
 box(20,90,1080,145,"REFERENCE SONG / VOCAL");
 box(20,250,345,390,"MAIN VOCAL");
 box(387,250,345,390,"DOUBLES");
 box(754,250,346,390,"ADLIBS");
 g.setColour(juce::Colour(0xff181d2a));
 for(int col=0;col<3;++col)
  for(int i=0;i<6;++i)
  {
   int x=48+col*367+(i%3)*98,y=330+(i/3)*135;
   g.fillEllipse((float)x,(float)y,70,70);
   g.setColour(purple);g.drawEllipse((float)x,(float)y,70,70,2);
   g.setColour(juce::Colour(0xff181d2a));
  }
}
void VST3PlusVocalsLoadFirstEditor::resized()
{
 load.setBounds(42,128,210,38);analyse.setBounds(266,128,170,38);
 file.setBounds(460,119,610,26);status.setBounds(460,150,610,28);
 mainTitle.setBounds(38,270,180,24);doublesTitle.setBounds(405,270,180,24);adlibsTitle.setBounds(772,270,180,24);
}
