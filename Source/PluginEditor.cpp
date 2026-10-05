#include "PluginEditor.h"
#include <cmath>

VST3PlusVocalsAudioProcessorEditor::VST3PlusVocalsAudioProcessorEditor(VST3PlusVocalsAudioProcessor& proc)
    : AudioProcessorEditor(&proc), p(proc)
{
    setSize(1240, 820);

    style.addItemList({"Clean Trap","Dark Trap","Airy","Distorted","Wide Doubles","Underground","R&B","Custom"}, 1);
    style.setSelectedId(1);
    key.addItemList({"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},1);
    key.setSelectedId(1);
    scale.addItemList({"Minor","Major"},1);
    scale.setSelectedId(1);

    for (auto* c : { &style, &key, &scale }) addAndMakeVisible(*c);
    for (auto* b : { &applyPrompt, &loadReference, &analyseReference }) addAndMakeVisible(*b);

    aiPrompt.setMultiLine(false);
    aiPrompt.setTextToShowWhenEmpty("z.B. Absent / dark melodic trap / airy R&B / wide doubles ...",
                                    juce::Colour(0xff777b89));
    addAndMakeVisible(aiPrompt);
    addAndMakeVisible(referenceLabel);
    addAndMakeVisible(aiStatus);
    addAndMakeVisible(analysisLabel);

    referenceLabel.setText("Reference: none", juce::dontSendNotification);
    aiStatus.setText("LOCAL AI CHAIN ASSISTANT: ready", juce::dontSendNotification);
    analysisLabel.setText("Reference Match: waiting for audio", juce::dontSendNotification);
    for (auto* l : { &referenceLabel, &aiStatus, &analysisLabel })
        l->setColour(juce::Label::textColourId, juce::Colour(0xffc7b8ff));

    const char* names[] = {
        "Tune","Comp","De-Esser","EQ","Saturation","Air","Reverb","Delay","Level",
        "Width","Timing","Pitch","EQ","Reverb","Delay","Pan","Level",
        "Width","Pitch","Distortion","Filter","Delay","Reverb","Pan","Level",
        "Input","Dry/Wet","Output"
    };
    const char* ids[] = {
        "tune","comp","deess","eq","sat","air","rev","delay","level",
        "dw","dt","dp","deq","dr","dd","dpan","dl",
        "aw","ap","dist","filter","ad","ar","apan","al",
        "input","mix","output"
    };
    for (int i=0; i<28; ++i) addKnob(names[i], ids[i]);

    applyPrompt.onClick = [this] { applyPromptChain(); };
    aiPrompt.onReturnKey = [this] { applyPromptChain(); };

    loadReference.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser>(
            "Choose a reference song", juce::File{}, "*.wav;*.aiff;*.flac;*.mp3");

        chooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f.existsAsFile())
                {
                    referenceFile = f;
                    referenceLabel.setText("Reference: " + f.getFileName(), juce::dontSendNotification);
                    analysisLabel.setText("Reference Match: loaded - press ANALYSE + MATCH",
                                          juce::dontSendNotification);
                }
            });
    };

    analyseReference.onClick = [this] { analyseAndMatchReference(); };
    startTimerHz(20);
}

void VST3PlusVocalsAudioProcessorEditor::addKnob(const char* name, const char* id)
{
    auto k = std::make_unique<Knob>();
    k->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    k->label.setText(name, juce::dontSendNotification);
    k->label.setJustificationType(juce::Justification::centred);
    k->label.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(k->slider);
    addAndMakeVisible(k->label);
    k->attachment = std::make_unique<Attachment>(p.apvts, id, k->slider);
    knobs.push_back(std::move(k));
}

void VST3PlusVocalsAudioProcessorEditor::setParam(const char* id, float value)
{
    if (auto* param = p.apvts.getParameter(id))
        param->setValueNotifyingHost(param->convertTo0to1(value));
}

void VST3PlusVocalsAudioProcessorEditor::applyPromptChain()
{
    auto text = aiPrompt.getText().toLowerCase().trim();

    // Baseline
    setParam("comp",38); setParam("deess",25); setParam("sat",12);
    setParam("air",22); setParam("rev",18); setParam("delay",9);
    setParam("dw",125); setParam("dt",18); setParam("dl",-13);
    setParam("aw",145); setParam("dist",18); setParam("ad",26);
    setParam("ar",30); setParam("al",-16);

    if (text.contains("absent"))
    {
        setParam("tune",78); setParam("comp",48); setParam("deess",32);
        setParam("eq",2.5f); setParam("sat",24); setParam("air",34);
        setParam("rev",24); setParam("delay",18);
        setParam("dw",165); setParam("dt",26); setParam("dp",-5);
        setParam("dr",28); setParam("dd",20); setParam("dl",-9);
        setParam("aw",175); setParam("ap",5); setParam("dist",32);
        setParam("filter",58); setParam("ad",38); setParam("ar",42); setParam("al",-12);
    }
    else
    {
        if (text.contains("dark"))       { setParam("air",10); setParam("sat",28); setParam("rev",14); }
        if (text.contains("airy"))       { setParam("air",62); setParam("rev",34); setParam("sat",6); }
        if (text.contains("r&b") || text.contains("rnb"))
                                         { setParam("comp",30); setParam("air",42); setParam("rev",30); setParam("delay",14); }
        if (text.contains("distort"))    { setParam("sat",68); setParam("dist",62); }
        if (text.contains("wide"))       { setParam("dw",190); setParam("aw",190); setParam("dl",-8); }
        if (text.contains("dry"))        { setParam("rev",5); setParam("delay",3); }
        if (text.contains("wet"))        { setParam("rev",42); setParam("delay",26); }
        if (text.contains("hard tune") || text.contains("autotune"))
                                         setParam("tune",90);
    }

    aiStatus.setText("AI CHAIN BUILT: " + (text.isEmpty() ? "custom clean vocal" : text),
                     juce::dontSendNotification);
}

void VST3PlusVocalsAudioProcessorEditor::analyseAndMatchReference()
{
    if (!referenceFile.existsAsFile())
    {
        analysisLabel.setText("Reference Match: load a WAV/song first", juce::dontSendNotification);
        return;
    }

    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(referenceFile));

    if (reader == nullptr)
    {
        analysisLabel.setText("Reference Match: unsupported/corrupt audio file", juce::dontSendNotification);
        return;
    }

    const auto maxSamples = (juce::int64) (reader->sampleRate * 90.0);
    const int samples = (int) juce::jmin<juce::int64>(reader->lengthInSamples, maxSamples);
    const int channels = juce::jlimit(1, 2, (int) reader->numChannels);

    if (samples <= 0)
    {
        analysisLabel.setText("Reference Match: empty audio file", juce::dontSendNotification);
        return;
    }

    juce::AudioBuffer<float> audio(channels, samples);
    if (!reader->read(&audio, 0, samples, 0, true, true))
    {
        analysisLabel.setText("Reference Match: could not decode audio", juce::dontSendNotification);
        return;
    }

    double sumSq = 0.0, diffSq = 0.0, sideSq = 0.0, midSq = 0.0;
    float peak = 0.0f;
    juce::int64 count = 0;

    for (int c=0; c<channels; ++c)
    {
        const auto* x = audio.getReadPointer(c);
        float prev = 0.0f;
        for (int n=0; n<samples; ++n)
        {
            const float v = x[n];
            sumSq += (double)v * v;
            const float d = v - prev;
            diffSq += (double)d * d;
            prev = v;
            peak = juce::jmax(peak, std::abs(v));
            ++count;
        }
    }

    if (channels == 2)
    {
        const auto* l = audio.getReadPointer(0);
        const auto* r = audio.getReadPointer(1);
        for (int n=0; n<samples; ++n)
        {
            const float mid = 0.5f * (l[n] + r[n]);
            const float side = 0.5f * (l[n] - r[n]);
            midSq += (double)mid * mid;
            sideSq += (double)side * side;
        }
    }

    const double rms = std::sqrt(sumSq / juce::jmax<juce::int64>(1, count));
    const double brightness = std::sqrt(diffSq / juce::jmax<juce::int64>(1, count))
                            / juce::jmax(0.000001, rms);
    const double crest = peak / juce::jmax(0.000001, rms);
    const double width = channels == 2
        ? std::sqrt(sideSq / juce::jmax(1.0, midSq)) : 0.0;

    // Heuristic reference matching. It analyses the complete song, so these are
    // effect estimates rather than isolated-vocal reconstruction.
    const float comp = juce::jlimit(18.0f, 72.0f, (float)(68.0 - crest * 8.0));
    const float air  = juce::jlimit(5.0f, 70.0f, (float)(brightness * 34.0));
    const float sat  = juce::jlimit(4.0f, 48.0f, (float)(38.0 / juce::jmax(1.0, crest)));
    const float stereo = juce::jlimit(90.0f, 195.0f, (float)(110.0 + width * 75.0));

    setParam("comp", comp);
    setParam("air", air);
    setParam("sat", sat);
    setParam("deess", juce::jlimit(15.0f, 55.0f, air * 0.65f));
    setParam("rev", juce::jlimit(10.0f, 38.0f, 12.0f + (float)width * 24.0f));
    setParam("delay", juce::jlimit(5.0f, 28.0f, 7.0f + (float)width * 16.0f));

    setParam("dw", stereo);
    setParam("dt", juce::jlimit(12.0f, 40.0f, 16.0f + (float)width * 22.0f));
    setParam("dr", juce::jlimit(14.0f, 45.0f, 18.0f + (float)width * 25.0f));
    setParam("dd", juce::jlimit(8.0f, 34.0f, 10.0f + (float)width * 20.0f));
    setParam("dl", juce::jlimit(-18.0f, -7.0f, -15.0f + (float)width * 7.0f));

    setParam("aw", juce::jlimit(115.0f, 195.0f, stereo + 10.0f));
    setParam("dist", juce::jlimit(10.0f, 42.0f, sat * 0.8f));
    setParam("ad", juce::jlimit(18.0f, 48.0f, 22.0f + (float)width * 25.0f));
    setParam("ar", juce::jlimit(22.0f, 55.0f, 26.0f + (float)width * 28.0f));
    setParam("al", juce::jlimit(-20.0f, -10.0f, -17.0f + (float)width * 6.0f));

    analysisLabel.setText(
        "MATCHED • RMS " + juce::String(juce::Decibels::gainToDecibels((float)rms),1)
        + " dB • width " + juce::String(width,2)
        + " • brightness " + juce::String(brightness,2),
        juce::dontSendNotification);
    aiStatus.setText("REFERENCE CHAIN APPLIED to MAIN / DOUBLES / ADLIBS",
                     juce::dontSendNotification);
}

void VST3PlusVocalsAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto purple = juce::Colour(0xff8d5cff);
    g.fillAll(juce::Colour(0xff080a12));
    g.setColour(purple); g.fillRect(0,0,getWidth(),5);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(30.0f, juce::Font::bold));
    g.drawText("VST3+Vocals",22,12,340,42,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xffb9a3ff));
    g.setFont(14.0f);
    g.drawText("AI VocalChain • by 29YURO",24,51,320,20,juce::Justification::centredLeft);

    auto panel = [&](int x,int y,int w,int h,const juce::String& title)
    {
        g.setColour(juce::Colour(0xff111522));
        g.fillRoundedRectangle((float)x,(float)y,(float)w,(float)h,10.0f);
        g.setColour(juce::Colour(0xff30264a));
        g.drawRoundedRectangle(juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h),10.0f,1.0f);
        g.setColour(purple);
        g.setFont(15.0f);
        g.drawText(title,x+12,y+7,w-24,20,juce::Justification::centredLeft);
    };

    panel(18,82,520,142,"AI CHAIN PROMPT");
    panel(552,82,670,142,"REFERENCE SONG → VOCAL EFFECT MATCH");
    panel(18,240,1204,155,"MAIN VOCAL");
    panel(18,410,1204,155,"DOUBLES");
    panel(18,580,1204,155,"ADLIBS + MASTER");

    g.setColour(juce::Colour(0xff202638));
    g.fillRect(28,770,1160,10);
    g.setColour(purple);
    g.fillRect(28,770,(int)(1160 * juce::jlimit(0.0f,1.0f,p.outputPeak)),10);
}

void VST3PlusVocalsAudioProcessorEditor::resized()
{
    style.setBounds(32,116,145,28);
    aiPrompt.setBounds(187,116,335,30);
    applyPrompt.setBounds(32,158,490,36);
    aiStatus.setBounds(32,196,490,22);

    loadReference.setBounds(568,116,190,32);
    analyseReference.setBounds(768,116,190,32);
    key.setBounds(970,116,58,32);
    scale.setBounds(1038,116,78,32);
    referenceLabel.setBounds(568,158,630,24);
    analysisLabel.setBounds(568,186,630,24);

    int i=0;
    auto row=[&](int y,int count)
    {
        int x=31;
        const int w=116;
        const int step=130;
        for(int j=0;j<count;++j,++i)
        {
            knobs[i]->label.setBounds(x,y,w,18);
            knobs[i]->slider.setBounds(x,y+18,w,100);
            x += step;
        }
    };
    row(266,9);
    row(436,8);
    row(606,8);

    for (; i<(int)knobs.size(); ++i)
    {
        const int x=825+(i-25)*125;
        knobs[i]->label.setBounds(x,606,112,18);
        knobs[i]->slider.setBounds(x,624,112,100);
    }
}
