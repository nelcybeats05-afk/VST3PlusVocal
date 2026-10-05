#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

static std::unique_ptr<juce::RangedAudioParameter> makeFloat(
    const char* id, const char* name, float minV, float maxV, float defV)
{
    return std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { id, 1 }, name,
        juce::NormalisableRange<float> { minV, maxV }, defV);
}

VST3PlusVocalsAudioProcessor::VST3PlusVocalsAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout VST3PlusVocalsAudioProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto add = [&p](const char* id, const char* name, float a, float b, float d)
    { p.push_back(makeFloat(id, name, a, b, d)); };

    add("input","Input",-24,24,0); add("mix","Dry Wet",0,100,100); add("output","Output",-24,24,0);
    add("tune","Tune",0,100,30); add("comp","Compression",0,100,35); add("deess","De Esser",0,100,20);
    add("eq","EQ",-12,12,0); add("sat","Saturation",0,100,12); add("air","Air",0,100,20);
    add("rev","Reverb",0,100,18); add("delay","Delay",0,100,8); add("level","Main Level",-24,12,0);

    add("dw","Doubles Width",0,200,125); add("dt","Doubles Timing",0,100,18); add("dp","Doubles Pitch",-24,24,0);
    add("deq","Doubles EQ",-12,12,0); add("dr","Doubles Reverb",0,100,20); add("dd","Doubles Delay",0,100,12);
    add("dpan","Doubles Pan",-100,100,0); add("dl","Doubles Level",-48,12,-12);

    add("aw","Adlibs Width",0,200,145); add("ap","Adlibs Pitch",-24,24,0); add("dist","Adlibs Distortion",0,100,20);
    add("filter","Adlibs Filter",0,100,50); add("ad","Adlibs Delay",0,100,28); add("ar","Adlibs Reverb",0,100,30);
    add("apan","Adlibs Pan",-100,100,0); add("al","Adlibs Level",-48,12,-15);

    return { p.begin(), p.end() };
}

bool VST3PlusVocalsAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    const auto in = l.getMainInputChannelSet();
    const auto out = l.getMainOutputChannelSet();
    return in == out && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void VST3PlusVocalsAudioProcessor::prepareToPlay(double sampleRate, int blockSize)
{
    sr = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) blockSize,
                                  (juce::uint32) getTotalNumOutputChannels() };
    mainReverb.prepare(spec); mainReverb.reset();
    mainDelay.prepare(spec); mainDelay.reset();
    doubleDelay.prepare(spec); doubleDelay.reset();
    adlibDelay.prepare(spec); adlibDelay.reset();
    highPass.prepare(spec); lowPass.prepare(spec);
    highPass.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    lowPass.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    highPass.setCutoffFrequency(70.0f);
    lowPass.setCutoffFrequency(18000.0f);
}

void VST3PlusVocalsAudioProcessor::processBlock(juce::AudioBuffer<float>& b, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = b.getNumChannels();
    const int samples = b.getNumSamples();
    if (channels == 0 || samples == 0) return;

    juce::AudioBuffer<float> original;
    original.makeCopyOf(b);

    b.applyGain(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("input")->load()));
    inputPeak = b.getMagnitude(0, samples);

    juce::dsp::AudioBlock<float> block(b);
    juce::dsp::ProcessContextReplacing<float> context(block);
    highPass.process(context);
    lowPass.process(context);

    const float comp = apvts.getRawParameterValue("comp")->load() / 100.0f;
    const float sat = apvts.getRawParameterValue("sat")->load() / 100.0f;
    const float air = apvts.getRawParameterValue("air")->load() / 100.0f;
    const float eq = apvts.getRawParameterValue("eq")->load() / 12.0f;

    for (int c = 0; c < channels; ++c)
    {
        auto* x = b.getWritePointer(c);
        float prev = 0.0f;
        for (int n = 0; n < samples; ++n)
        {
            float v = x[n];
            const float av = std::abs(v);
            if (av > 0.35f)
            {
                const float ratio = 1.0f + comp * 7.0f;
                v = std::copysign(0.35f + (av - 0.35f) / ratio, v);
            }
            v = std::tanh(v * (1.0f + sat * 4.0f)) / (1.0f + sat);
            const float high = v - prev;
            prev = v;
            x[n] = v + high * (air * 0.22f + eq * 0.08f);
        }
    }

    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.42f; rp.damping = 0.50f; rp.width = 1.0f;
    rp.wetLevel = apvts.getRawParameterValue("rev")->load() / 100.0f * 0.45f;
    rp.dryLevel = 1.0f;
    mainReverb.setParameters(rp);
    mainReverb.process(context);

    const float mainDelayMix = apvts.getRawParameterValue("delay")->load() / 100.0f;
    mainDelay.setDelay((float) (sr * 0.24));
    for (int c = 0; c < channels; ++c)
    {
        auto* x = b.getWritePointer(c);
        for (int n = 0; n < samples; ++n)
        {
            const float d = mainDelay.popSample(c);
            mainDelay.pushSample(c, x[n] + d * 0.25f);
            x[n] += d * mainDelayMix * 0.45f;
        }
    }

    // Parallel doubles generated from the incoming vocal.
    const float dLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("dl")->load());
    const float dWidth = apvts.getRawParameterValue("dw")->load() / 200.0f;
    const float dTiming = 0.012f + apvts.getRawParameterValue("dt")->load() / 100.0f * 0.045f;
    doubleDelay.setDelay((float) (sr * dTiming));

    // Parallel adlib-style layer. This is a generated effect layer, not source separation.
    const float aLevel = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("al")->load());
    const float aDist = apvts.getRawParameterValue("dist")->load() / 100.0f;
    const float aDelayMix = apvts.getRawParameterValue("ad")->load() / 100.0f;
    adlibDelay.setDelay((float) (sr * 0.31));

    for (int c = 0; c < channels; ++c)
    {
        auto* out = b.getWritePointer(c);
        const auto* src = original.getReadPointer(c);
        const float side = (channels > 1 ? (c == 0 ? -1.0f : 1.0f) : 0.0f);

        for (int n = 0; n < samples; ++n)
        {
            const float doubled = doubleDelay.popSample(c);
            doubleDelay.pushSample(c, src[n]);
            const float doubleGain = dLevel * (0.65f + 0.35f * dWidth) * (1.0f + side * 0.08f * dWidth);

            const float ad = adlibDelay.popSample(c);
            const float driven = std::tanh(src[n] * (1.0f + aDist * 5.0f));
            adlibDelay.pushSample(c, driven + ad * 0.20f);

            out[n] += doubled * doubleGain;
            out[n] += ad * aLevel * aDelayMix * 0.8f;
        }
    }

    const float wet = apvts.getRawParameterValue("mix")->load() / 100.0f;
    for (int c = 0; c < channels; ++c)
        for (int n = 0; n < samples; ++n)
            b.setSample(c, n, original.getSample(c,n) * (1.0f - wet) + b.getSample(c,n) * wet);

    const float totalDb = apvts.getRawParameterValue("level")->load()
                        + apvts.getRawParameterValue("output")->load();
    b.applyGain(juce::Decibels::decibelsToGain(totalDb));
    outputPeak = b.getMagnitude(0, samples);
}

void VST3PlusVocalsAudioProcessor::getStateInformation(juce::MemoryBlock& d)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary(*xml, d);
}
void VST3PlusVocalsAudioProcessor::setStateInformation(const void* d, int s)
{
    if (auto xml = getXmlFromBinary(d, s))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* VST3PlusVocalsAudioProcessor::createEditor()
{
    return new VST3PlusVocalsAudioProcessorEditor(*this);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VST3PlusVocalsAudioProcessor();
}
