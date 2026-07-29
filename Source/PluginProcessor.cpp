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
    waveMinHistory.resize(waveformSize, 0.0f);
    waveMaxHistory.resize(waveformSize, 0.0f);

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

    decimationCounter = 0;
    blockMin = 0.0f;
    blockMax = 0.0f;

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

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float gainDb = apvts.getRawParameterValue("GAIN")->load();
    float clipVal = apvts.getRawParameterValue("CLIP")->load();
    float clipPct = clipVal / 100.0f;
    float softPct = apvts.getRawParameterValue("SOFTNESS")->load() / 100.0f;
    float zoomVal = apvts.getRawParameterValue("ZOOM")->load(); // Pobieramy wartoœæ suwaka prêdkoœci/zoomu

    int osMode = (int)apvts.getRawParameterValue("OS")->load();

    float gainLinear = juce::Decibels::decibelsToGain(gainDb);
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

    // Zamierzony maksymalny poziom wyjœcia przy danym CLIP i SOFTNESS
    // (dok³adnie ten sam sufit co w formule waveshapera i na wykresie TRANSFER FUNCTION)
    float ceiling = threshold + softPct * (1.0f - threshold);

    // --- BEZPIECZNIK OSTATECZNY ---
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            if (channelData[sample] > ceiling)  channelData[sample] = ceiling;
            if (channelData[sample] < -ceiling) channelData[sample] = -ceiling;
        }
    }
    // -----------------------------

    // --- ZBIERANJE SZCZYTÓW STEROWANE PARAMETREM ZOOM ---
    float maxOutputMagnitude = 0.0f;
    int numSamples = buffer.getNumSamples();
    // decimationCounter, blockMin, blockMax s¹ teraz polami klasy — brak deklaracji tutaj

    // Im wy¿szy zoom, tym rzadszy zapis próbek, co daje bardzo powolny i gêsty ruch
    // Odwrócenie kierunku: ruch w lewo (mniejsze wartoœci) daje wolniejszy ruch/zoom, ruch w prawo przyspiesza
    int decimationTarget = juce::jlimit(4, 128, (int)juce::jmap(zoomVal, 0.01f, 0.1f, 128.0f, 4.0f));

    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float s = channelData[sample];
            if (s < blockMin) blockMin = s;
            if (s > blockMax) blockMax = s;

            if (std::abs(s) > maxOutputMagnitude)
                maxOutputMagnitude = std::abs(s);

            decimationCounter++;
            if (decimationCounter >= decimationTarget)
            {
                decimationCounter = 0;
                int writeIdx = waveformIndex.load() % waveformSize;
                if (writeIdx >= 0 && writeIdx < waveformSize)
                {
                    waveMinHistory[writeIdx] = blockMin;
                    waveMaxHistory[writeIdx] = blockMax;
                }
                waveformIndex.store(waveformIndex.load() + 1);
                blockMin = 0.0f;
                blockMax = 0.0f;
            }
        }
    }
    // ----------------------------------------------------

    currentOutputLevel.store(maxOutputMagnitude);
}

bool _8f_clipAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* _8f_clipAudioProcessor::createEditor() { return new _8f_clipAudioProcessorEditor(*this); }

void _8f_clipAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    // Zapisujemy aktualny rozmiar okna do stanu APVTS, jeœli edytor jest otwarty
    if (auto* editor = getActiveEditor())
    {
        state.setProperty("uiWidth", editor->getWidth(), nullptr);
        state.setProperty("uiHeight", editor->getHeight(), nullptr);
    }

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

            // Odczytujemy zapisany rozmiar okna ze stanu
            int savedWidth = apvts.state.getProperty("uiWidth", 680);
            int savedHeight = apvts.state.getProperty("uiHeight", 500);

            if (auto* editor = getActiveEditor())
            {
                editor->setSize(savedWidth, savedHeight);
            }
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout _8f_clipAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN", "Gain", -12.0f, 12.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CLIP", "Clip", 0.0f, 100.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SOFTNESS", "Softness", 0.0f, 100.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("OS", "Oversampling",
        juce::StringArray{ "OFF", "2X", "4X", "8X", "16X" }, 0));
	// zoom slider for waveform display, range from 0.05 to 4.0, default 0.1
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ZOOM", "Zoom", 0.01f, 0.1f, 0.05f));

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