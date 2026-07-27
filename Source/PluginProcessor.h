#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <vector>
#include <array>

// Dziedziczymy dodatkowo po Listenerze, aby nas³uchiwaæ zmian ga³ki OS
class _8f_clipAudioProcessor : public juce::AudioProcessor,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    _8f_clipAudioProcessor();
    ~_8f_clipAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Metoda wywo³ywana automatycznie, gdy zmieni siê wartoœæ obserwowanego parametru
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<float> currentOutputLevel{ 0.0f };

    static constexpr int waveformSize = 2048;
    std::vector<float> waveformHistory;
    std::atomic<int> waveformIndex{ 0 };

private:
    std::array<std::unique_ptr<juce::dsp::Oversampling<float>>, 4> oversamplers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(_8f_clipAudioProcessor)
};