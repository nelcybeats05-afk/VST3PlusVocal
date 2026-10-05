#include "PluginEditor.h"
#include <cmath>

VST3PlusVocalsMatchEditor::VST3PlusVocalsMatchEditor(VST3PlusVocalsMatchProcessor& proc)
: AudioProcessorEditor(&proc),p(proc)
{
    setSize(1120,720);
    addAndMakeVisible(load); addAndMakeVisible(match);
    for(auto* l:{&fileLabel,&statusLabel,&mainResult,&doubleResult,&adlibResult})
    {
        addAndMakeVisible(*l);
        l->setColour(juce::Label::textColourId,juce::Colour(0xffc9bbff));
    }
    fileLabel.setText("No vocal reference loaded",juce::dontSendNotification);
    statusLabel.setText("Reference Match ready",juce::dontSendNotification);
    mainResult.setText("MAIN: waiting",juce::dontSendNotification);
    doubleResult.setText("DOUBLES: waiting",juce::dontSendNotification);
    adlibResult.setText("ADLIBS: waiting",juce::dontSendNotification);

    const char* names[]={"Comp","Saturation","Air","Reverb","Delay","Level",
                         "Width","Timing","Reverb","Delay","Level",
                         "Width","Distortion","Reverb","Delay","Level",
                         "Input","Dry/Wet","Output"};
    const char* ids[]={"mcomp","msat","mair","mrev","mdel","mlevel",
                       "dwidth","dtime","drev","ddel","dlevel",
                       "awidth","adist","arev","adel","alevel",
                       "input","mix","output"};
    for(int i=0;i<19;++i) addKnob(names[i],ids[i]);

    load.onClick=[this]
    {
        chooser=std::make_unique<juce::FileChooser>("Choose vocal WAV",juce::File{},"*.wav;*.aiff;*.flac");
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f=fc.getResult();
                if(f.existsAsFile())
                {
                    reference=f;
                    fileLabel.setText("Loaded: "+f.getFileName(),juce::dontSendNotification);
                    statusLabel.setText("Press ANALYSE + MATCH EFFECTS",juce::dontSendNotification);
                }
            });
    };
    match.onClick=[this]{analyse();};
    startTimerHz(15);
}

void VST3PlusVocalsMatchEditor::addKnob(const char* name,const char* id)
{
    auto k=std::make_unique<K>();
    k->s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k->s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,58,17);
    k->l.setText(name,juce::dontSendNotification);
    k->l.setJustificationType(juce::Justification::centred);
    k->l.setColour(juce::Label::textColourId,juce::Colours::white);
    addAndMakeVisible(k->s); addAndMakeVisible(k->l);
    k->a=std::make_unique<Att>(p.state,id,k->s);
    knobs.push_back(std::move(k));
}

void VST3PlusVocalsMatchEditor::analyse()
{
    if(!reference.existsAsFile())
    {
        statusLabel.setText("Load a vocal WAV first",juce::dontSendNotification); return;
    }

    juce::AudioFormatManager fm; fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> r(fm.createReaderFor(reference));
    if(!r)
    {
        statusLabel.setText("Could not read this audio file",juce::dontSendNotification); return;
    }

    auto maxLen=(juce::int64)(r->sampleRate*120.0);
    int n=(int)juce::jmin<juce::int64>(r->lengthInSamples,maxLen);
    int ch=juce::jlimit(1,2,(int)r->numChannels);
    if(n<=0){statusLabel.setText("Audio file is empty",juce::dontSendNotification);return;}

    juce::AudioBuffer<float> a(ch,n);
    if(!r->read(&a,0,n,0,true,true))
    {
        statusLabel.setText("Decode failed",juce::dontSendNotification); return;
    }

    double sq=0,diff=0,side=0,mid=0,zero=0;
    float peak=0;
    long long count=0;
    for(int c=0;c<ch;++c)
    {
        auto*x=a.getReadPointer(c); float prev=0;
        for(int i=0;i<n;++i)
        {
            float v=x[i]; sq+=(double)v*v; float d=v-prev; diff+=(double)d*d;
            if((v>=0)!=(prev>=0)) zero+=1.0; prev=v; peak=juce::jmax(peak,std::abs(v)); ++count;
        }
    }
    if(ch==2)
    {
        auto*l=a.getReadPointer(0);auto*rr=a.getReadPointer(1);
        for(int i=0;i<n;++i)
        {
            float m=.5f*(l[i]+rr[i]),s=.5f*(l[i]-rr[i]);
            mid+=(double)m*m;side+=(double)s*s;
        }
    }

    double rms=std::sqrt(sq/juce::jmax<long long>(1,count));
    double crest=peak/juce::jmax(1.0e-7,rms);
    double bright=std::sqrt(diff/juce::jmax<long long>(1,count))/juce::jmax(1.0e-7,rms);
    double width=ch==2?std::sqrt(side/juce::jmax(1.0,mid)):0.0;
    double zcr=zero/juce::jmax<long long>(1,count);

    float comp=juce::jlimit(20.f,70.f,(float)(68.0-crest*7.0));
    float air=juce::jlimit(8.f,68.f,(float)(bright*30.0+zcr*50.0));
    float sat=juce::jlimit(5.f,45.f,(float)(34.0/juce::jmax(1.0,crest)));
    float rev=juce::jlimit(8.f,38.f,(float)(12.0+width*22.0));
    float del=juce::jlimit(4.f,26.f,(float)(6.0+width*15.0));
    float dw=juce::jlimit(105.f,190.f,(float)(120.0+width*65.0));

    p.setParam("mcomp",comp);p.setParam("msat",sat);p.setParam("mair",air);p.setParam("mrev",rev);p.setParam("mdel",del);
    p.setParam("dwidth",dw);p.setParam("dtime",juce::jlimit(10.f,38.f,(float)(15+width*20)));
    p.setParam("drev",juce::jlimit(10.f,42.f,rev+6));p.setParam("ddel",juce::jlimit(8.f,32.f,del+7));
    p.setParam("dlevel",juce::jlimit(-18.f,-8.f,(float)(-15+width*6)));
    p.setParam("awidth",juce::jlimit(125.f,195.f,dw+12));p.setParam("adist",juce::jlimit(10.f,48.f,sat+10));
    p.setParam("arev",juce::jlimit(18.f,55.f,rev+15));p.setParam("adel",juce::jlimit(16.f,48.f,del+18));
    p.setParam("alevel",juce::jlimit(-21.f,-11.f,(float)(-18+width*5)));

    mainResult.setText("MAIN  • Comp "+juce::String(comp,0)+"  Air "+juce::String(air,0)+"  Rev "+juce::String(rev,0),juce::dontSendNotification);
    doubleResult.setText("DOUBLES • Width "+juce::String(dw,0)+"  Timing matched",juce::dontSendNotification);
    adlibResult.setText("ADLIBS • Wider / wetter / more saturation matched",juce::dontSendNotification);
    statusLabel.setText("REFERENCE MATCH APPLIED",juce::dontSendNotification);
}

void VST3PlusVocalsMatchEditor::paint(juce::Graphics& g)
{
    auto purple=juce::Colour(0xff8e5cff);
    g.fillAll(juce::Colour(0xff080a12));g.setColour(purple);g.fillRect(0,0,getWidth(),5);
    g.setColour(juce::Colours::white);g.setFont(30.f);g.drawText("VST3+Vocals Match",22,14,420,40,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xffbdaaff));g.setFont(14.f);g.drawText("Reference Vocal Effect Matcher • by 29YURO",24,53,430,20,juce::Justification::centredLeft);
    auto box=[&](int x,int y,int w,int h,const char*t){g.setColour(juce::Colour(0xff111522));g.fillRoundedRectangle((float)x,(float)y,(float)w,(float)h,10);g.setColour(juce::Colour(0xff30264a));g.drawRoundedRectangle(juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h),10,1);g.setColour(purple);g.drawText(t,x+12,y+7,w-24,20,juce::Justification::centredLeft);};
    box(18,88,1084,128,"LOAD A VOCAL WAV → ANALYSE → MATCH MAIN / DOUBLES / ADLIBS");
    box(18,232,1084,138,"MAIN VOCAL");box(18,386,1084,138,"DOUBLES");box(18,540,1084,138,"ADLIBS + MASTER");
    g.setColour(juce::Colour(0xff202638));g.fillRect(28,694,1050,9);g.setColour(purple);g.fillRect(28,694,(int)(1050*juce::jlimit(0.f,1.f,p.outputPeak)),9);
}

void VST3PlusVocalsMatchEditor::resized()
{
    load.setBounds(34,122,210,34);match.setBounds(254,122,250,34);
    fileLabel.setBounds(520,116,560,24);statusLabel.setBounds(520,145,560,24);
    mainResult.setBounds(34,177,330,24);doubleResult.setBounds(370,177,330,24);adlibResult.setBounds(706,177,370,24);
    int i=0;
    auto row=[&](int y,int count){int x=32;for(int j=0;j<count;++j,++i){knobs[i]->l.setBounds(x,y,105,18);knobs[i]->s.setBounds(x,y+18,105,92);x+=116;}};
    row(255,6);row(409,5);row(563,5);
    for(;i<(int)knobs.size();++i){int x=720+(i-16)*118;knobs[i]->l.setBounds(x,563,108,18);knobs[i]->s.setBounds(x,581,108,92);}
}
