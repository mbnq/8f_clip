#include "PluginProcessor.h"
#include "PluginEditor.h"

_8f_clipAudioProcessor::_8f_clipAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    waveformHistory.resize(waveformSize, 0.0f);

    for (int i = 0; i < 4; ++i)
    {
        oversamplers[i] = std::make_unique<juce::dsp::Oversampling<float>>(
            2,
            i + 1,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            true
        );
    }

    apvts.addParameterListener("OS", this);
}

_8f_clipAudioProcessor::~_8f_clipAudioProcessor()
{
    apvts.removeParameterListener("OS", this);
}

const juce::String _8f_clipAudioProcessor::getName() const { return JucePlugin_Name; }
bool _8f_clipAudioProcessor::acceptsMidi() const { return false; }
bool _8f_clipAudioProcessor::producesMidi() const { return false; }
bool _8f_clipAudioProcessor::isMidiEffect() const { return false; }
double _8f_clipAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int _8f_clipAudioProcessor::getNumPrograms() { return 1; }
int _8f_clipAudioProcessor::getCurrentProgram() { return 0; }
void _8f_clipAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }
const juce::String _8f_clipAudioProcessor::getProgramName(int index) { juce::ignoreUnused(index); return {}; }
void _8f_clipAudioProcessor::changeProgramName(int index, const juce::String& newName) { juce::ignoreUnused(index, newName); }

void _8f_clipAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate);
    for (auto& os : oversamplers)
    {
        os->initProcessing(samplesPerBlock);
        os->reset();
    }

    int osMode = (int)apvts.getRawParameterValue("OS")->load();
    if (osMode > 0)
        setLatencySamples(oversamplers[osMode - 1]->getLatencyInSamples());
    else
        setLatencySamples(0);
}

void _8f_clipAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool _8f_clipAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
#endif
}
#endif

void _8f_clipAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float gainDb = apvts.getRawParameterValue("GAIN")->load();
    float clipVal = apvts.getRawParameterValue("CLIP")->load();
    float clipPct = clipVal / 100.0f;
    float softPct = apvts.getRawParameterValue("SOFTNESS")->load() / 100.0f;

    int osMode = (int)apvts.getRawParameterValue("OS")->load();

    float gainLinear = juce::Decibels::decibelsToGain(gainDb);
    // Normalna skala: 0% = brak ciêcia (threshold = 1.0), 100% = max ciêcie (threshold = 0.01)
    float threshold = juce::jmax(0.01f, 1.0f - clipPct);

    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::AudioBlock<float> processBlock = audioBlock;

    if (osMode > 0)
    {
        processBlock = oversamplers[osMode - 1]->processSamplesUp(audioBlock);
    }

    for (int channel = 0; channel < processBlock.getNumChannels(); ++channel)
    {
        auto* channelData = processBlock.getChannelPointer(channel);
        for (int sample = 0; sample < processBlock.getNumSamples(); ++sample)
        {
            float input = channelData[sample] * gainLinear;
            float output = input;

            if (std::abs(input) <= threshold) {
                output = input;
            }
            else {
                float sign = (input > 0.0f) ? 1.0f : -1.0f;
                float absIn = std::abs(input);
                float hardVal = sign * threshold;
                float excess = absIn - threshold;
                float softVal = sign * (threshold + (1.0f - threshold) * std::tanh(excess / (1.0f - threshold + 0.0001f)));
                if (softVal > 1.0f) softVal = 1.0f;
                output = hardVal + softPct * (softVal - hardVal);
            }
            channelData[sample] = output;
        }
    }

    if (osMode > 0)
    {
        oversamplers[osMode - 1]->processSamplesDown(audioBlock);
    }

    float maxOutputMagnitude = 0.0f;
    int writeIdx = waveformIndex.load();
    static int decimationCounter = 0;

    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            if (channel == 0) {
                decimationCounter++;
                if (decimationCounter >= 256) {
                    decimationCounter = 0;
                    int safeIdx = writeIdx % waveformSize;
                    if (safeIdx >= 0 && safeIdx < (int)waveformHistory.size()) {
                        waveformHistory[safeIdx] = channelData[sample];
                    }
                    writeIdx++;
                }
            }

            if (std::abs(channelData[sample]) > maxOutputMagnitude)
                maxOutputMagnitude = std::abs(channelData[sample]);
        }
    }

    waveformIndex.store(writeIdx);
    currentOutputLevel.store(maxOutputMagnitude);
}

bool _8f_clipAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* _8f_clipAudioProcessor::createEditor() { return new _8f_clipAudioProcessorEditor(*this); }

void _8f_clipAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void _8f_clipAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout _8f_clipAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN", "Gain", -24.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CLIP", "Clip", 0.0f, 100.0f, 0.0f)); // Domyœlnie 0% (brak ciêcia)
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SOFTNESS", "Softness", 0.0f, 100.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("OS", "Oversampling",
        juce::StringArray{ "OFF", "2X", "4X", "8X", "16X" }, 0));

    return { params.begin(), params.end() };
}

void _8f_clipAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "OS")
    {
        int osMode = juce::roundToInt(newValue);
        int newLatency = (osMode > 0) ? oversamplers[osMode - 1]->getLatencyInSamples() : 0;
        setLatencySamples(newLatency);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new _8f_clipAudioProcessor(); }