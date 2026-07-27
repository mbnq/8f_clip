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
}

_8f_clipAudioProcessor::~_8f_clipAudioProcessor() {}

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
void _8f_clipAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) { juce::ignoreUnused(sampleRate, samplesPerBlock); }
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
    float clipPct = apvts.getRawParameterValue("CLIP")->load() / 100.0f;
    float softPct = apvts.getRawParameterValue("SOFTNESS")->load() / 100.0f;

    float gainLinear = juce::Decibels::decibelsToGain(gainDb);
    float threshold = juce::jmax(0.01f, clipPct);

    float maxOutputMagnitude = 0.0f;
    int writeIdx = waveformIndex.load();

    // Liczniki do wolniejszego zapisu fali (zamiast sztywnego modulo zale¿¹cego od SR)
    static int decimationCounter = 0;

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
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

            // Zapis z decymacj¹ (co 16. próbkê dla p³ynnego i wolnego ruchu fali)
            if (channel == 0) {
                decimationCounter++;
                if (decimationCounter >= 256) {
                    decimationCounter = 0;
                    int safeIdx = writeIdx % waveformSize;
                    if (safeIdx >= 0 && safeIdx < (int)waveformHistory.size()) {
                        waveformHistory[safeIdx] = output;
                    }
                    writeIdx++;
                }
            }

            if (std::abs(output) > maxOutputMagnitude)
                maxOutputMagnitude = std::abs(output);
        }
    }

    waveformIndex.store(writeIdx);
    currentOutputLevel.store(maxOutputMagnitude);
}

bool _8f_clipAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* _8f_clipAudioProcessor::createEditor() { return new _8f_clipAudioProcessorEditor(*this); }
void _8f_clipAudioProcessor::getStateInformation(juce::MemoryBlock& destData) { juce::ignoreUnused(destData); }
void _8f_clipAudioProcessor::setStateInformation(const void* data, int sizeInBytes) { juce::ignoreUnused(data, sizeInBytes); }

juce::AudioProcessorValueTreeState::ParameterLayout _8f_clipAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN", "Gain", -24.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CLIP", "Clip", 0.0f, 100.0f, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SOFTNESS", "Softness", 0.0f, 100.0f, 0.0f));
    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new _8f_clipAudioProcessor(); }