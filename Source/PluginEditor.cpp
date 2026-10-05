#include "PluginEditor.h"
#include <cmath>
VST3PlusVocalsSafeEditor::VST3PlusVocalsSafeEditor(VST3PlusVocalsSafeProcessor& p):AudioProcessorEditor(&p)
{
 setSize(980,620);
 for(auto*b:{&load,&analyse}) addAndMakeVisible(*b);
 for(auto*l:{&file,&status,&mainL,&doublesL,&adlibsL}){addAndMakeVisible(*l);l->setColour(juce::Label::textColourId,juce::Colours::white);}
 file.setText("No WAV loaded",juce::dontSendNotification);status.setText("SAFE reference analyser ready",juce::dontSendNotification);
 mainL.setText("MAIN VOCAL",juce::dontSendNotification);doublesL.setText("DOUBLES",juce::dontSendNotification);adlibsL.setText("ADLIBS",juce::dontSendNotification);
 const char* n[]={"Comp","Air","Sat","Reverb","Width","Timing","Reverb","Level","Width","Dist","Delay","Reverb"};
 for(int i=0;i<12;++i){labels[i].setText(n[i],juce::dontSendNotification);labels[i].setJustificationType(juce::Justification::centred);labels[i].setColour(juce::Label::textColourId,juce::Colours::white);knobs[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,55,17);knobs[i].setRange(0,100,1);addAndMakeVisible(labels[i]);addAndMakeVisible(knobs[i]);}
 load.onClick=[this]{chooser=std::make_unique<juce::FileChooser>("Choose isolated vocal WAV",juce::File{},"*.wav");chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[this](const juce::FileChooser&fc){auto f=fc.getResult();if(f.existsAsFile()){ref=f;file.setText(f.getFileName(),juce::dontSendNotification);status.setText("WAV loaded - press ANALYSE VOCAL",juce::dontSendNotification);}});};
 analyse.onClick=[this]{runAnalysis();};
}
void VST3PlusVocalsSafeEditor::runAnalysis()
{
 if(!ref.existsAsFile()){status.setText("Load a WAV first",juce::dontSendNotification);return;}
 juce::WavAudioFormat wav;std::unique_ptr<juce::AudioFormatReader> r(wav.createReaderFor(new juce::FileInputStream(ref),true));
 if(!r){status.setText("Could not decode WAV",juce::dontSendNotification);return;}
 int n=(int)juce::jmin<juce::int64>(r->lengthInSamples,(juce::int64)(r->sampleRate*60.0));int ch=juce::jlimit(1,2,(int)r->numChannels);
 juce::AudioBuffer<float>a(ch,n);if(!r->read(&a,0,n,0,true,true)){status.setText("WAV read failed",juce::dontSendNotification);return;}
 double sq=0,dif=0,side=0,mid=0;float peak=0;long long count=0;
 for(int c=0;c<ch;++c){auto*x=a.getReadPointer(c);float prev=0;for(int i=0;i<n;++i){float v=x[i];sq+=(double)v*v;float d=v-prev;dif+=(double)d*d;prev=v;peak=juce::jmax(peak,std::abs(v));++count;}}
 if(ch==2){auto*l=a.getReadPointer(0);auto*rr=a.getReadPointer(1);for(int i=0;i<n;++i){float m=.5f*(l[i]+rr[i]),s=.5f*(l[i]-rr[i]);mid+=(double)m*m;side+=(double)s*s;}}
 double rms=std::sqrt(sq/juce::jmax<long long>(1,count));double crest=peak/juce::jmax(1e-7,rms);double bright=std::sqrt(dif/juce::jmax<long long>(1,count))/juce::jmax(1e-7,rms);double width=ch==2?std::sqrt(side/juce::jmax(1.0,mid)):0;
 double vals[]={juce::jlimit(15.0,75.0,70.0-crest*8.0),juce::jlimit(5.0,75.0,bright*32.0),juce::jlimit(5.0,50.0,35.0/juce::jmax(1.0,crest)),juce::jlimit(8.0,45.0,14.0+width*24.0),juce::jlimit(20.0,100.0,55.0+width*40.0),juce::jlimit(10.0,65.0,22.0+width*25.0),juce::jlimit(10.0,55.0,20.0+width*25.0),juce::jlimit(15.0,80.0,48.0+width*20.0),juce::jlimit(30.0,100.0,65.0+width*30.0),juce::jlimit(10.0,65.0,25.0+bright*10.0),juce::jlimit(10.0,70.0,30.0+width*30.0),juce::jlimit(15.0,75.0,35.0+width*30.0)};
 for(int i=0;i<12;++i)knobs[i].setValue(vals[i],juce::dontSendNotification);
 status.setText("ANALYSIS COMPLETE - settings estimated for MAIN / DOUBLES / ADLIBS",juce::dontSendNotification);
}
void VST3PlusVocalsSafeEditor::paint(juce::Graphics&g)
{
 g.fillAll(juce::Colour(0xff090b13));g.setColour(juce::Colour(0xff8e5cff));g.fillRect(0,0,getWidth(),5);g.setColour(juce::Colours::white);g.setFont(28.f);g.drawText("VST3+Vocals SAFE",20,15,400,38,juce::Justification::centredLeft);
 g.setFont(14.f);g.setColour(juce::Colour(0xffbdaaff));g.drawText("WAV Reference Vocal Matcher • scanner-safe build",22,52,500,20,juce::Justification::centredLeft);
}
void VST3PlusVocalsSafeEditor::resized()
{
 load.setBounds(22,88,190,34);analyse.setBounds(222,88,190,34);file.setBounds(430,88,520,24);status.setBounds(430,116,520,24);
 mainL.setBounds(22,165,180,24);doublesL.setBounds(22,315,180,24);adlibsL.setBounds(22,465,180,24);
 for(int i=0;i<4;++i){int x=210+i*170;labels[i].setBounds(x,160,120,20);knobs[i].setBounds(x,180,120,105);}
 for(int i=4;i<8;++i){int x=210+(i-4)*170;labels[i].setBounds(x,310,120,20);knobs[i].setBounds(x,330,120,105);}
 for(int i=8;i<12;++i){int x=210+(i-8)*170;labels[i].setBounds(x,460,120,20);knobs[i].setBounds(x,480,120,105);}
}
