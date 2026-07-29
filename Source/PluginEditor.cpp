#include "PluginProcessor.h"
#include "PluginEditor.h"

_8f_clipAudioProcessorEditor::_8f_clipAudioProcessorEditor(_8f_clipAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    transferGraph(p), waveformDisplay(p), outputMeter(p)
{
    setLookAndFeel(&jasnyStyl);

    // GAIN
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setRotaryParameters(juce::MathConstants<float>::pi,
        juce::MathConstants<float>::pi * 3.0f, true);
    addAndMakeVisible(gainSlider);
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "GAIN", gainSlider);

    // CLIP
    clipSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    clipSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    clipSlider.setTextValueSuffix(" %");
    clipSlider.setRotaryParameters(0.0f, juce::MathConstants<float>::pi * 2.0f, true);
    addAndMakeVisible(clipSlider);
    clipAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "CLIP", clipSlider);

    // SOFTNESS
    softnessSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    softnessSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    softnessSlider.setTextValueSuffix(" %");
    softnessSlider.setRotaryParameters(0.0f,
        juce::MathConstants<float>::pi * 2.0f, true);
    addAndMakeVisible(softnessSlider);
    softnessAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "SOFTNESS", softnessSlider);

    // OVERSAMPLING
    osSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    osSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    osSlider.setRotaryParameters(0.0f,
        juce::MathConstants<float>::pi * 2.0f, true);
    addAndMakeVisible(osSlider);
    osAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "OS", osSlider);

    addAndMakeVisible(transferGraph);
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(outputMeter);

    // Dodajemy interaktywne logo do widoku
    addAndMakeVisible(logoComponent);

    setResizable(true, true);
    setResizeLimits(550, 400, 1200, 900);

    // Odczytujemy zapisany rozmiar lub bierzemy domyślny
    int savedWidth = audioProcessor.apvts.state.getProperty("uiWidth", 680);
    int savedHeight = audioProcessor.apvts.state.getProperty("uiHeight", 500);
    setSize(savedWidth, savedHeight);
}

_8f_clipAudioProcessorEditor::~_8f_clipAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void _8f_clipAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::darkgrey);
    g.setFont(juce::FontOptions(12.0f));

    auto area = getLocalBounds().reduced(15);
    auto sliderArea = area.removeFromBottom(100);

    int availableWidthForSliders = sliderArea.getWidth() - 100;
    int w = availableWidthForSliders / 4;

    // Podpisy umieszczone na samym dole obszaru suwaków (pod knobami)
    float labelY = sliderArea.getY() + sliderArea.getHeight() - 15;
    g.drawText("GAIN", sliderArea.getX(), labelY, w, 15, juce::Justification::centred);
    g.drawText("CLIP", sliderArea.getX() + w, labelY, w, 15, juce::Justification::centred);
    g.drawText("SOFTNESS", sliderArea.getX() + (w * 2), labelY, w, 15, juce::Justification::centred);
    g.drawText("OVERSAMPLING", sliderArea.getX() + (w * 3), labelY, w, 15, juce::Justification::centred);
}

void _8f_clipAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(15);

    auto sliderArea = area.removeFromBottom(100);
    int availableWidthForSliders = sliderArea.getWidth() - 100;
    int sliderWidth = availableWidthForSliders / 4;

    // Zostawiamy 15 pikseli na dole dla napisów pod pokrętłami
    auto actualSlidersArea = sliderArea.removeFromTop(sliderArea.getHeight() - 15);

    gainSlider.setBounds(actualSlidersArea.removeFromLeft(sliderWidth).reduced(5));
    clipSlider.setBounds(actualSlidersArea.removeFromLeft(sliderWidth).reduced(5));
    softnessSlider.setBounds(actualSlidersArea.removeFromLeft(sliderWidth).reduced(5));
    osSlider.setBounds(actualSlidersArea.removeFromLeft(sliderWidth).reduced(5));

    // Pozycjonowanie interaktywnego bloku logo w prawym dolnym rogu
    logoComponent.setBounds(actualSlidersArea.removeFromLeft(100));

    area.removeFromBottom(15);

    int meterWidth = 70;
    outputMeter.setBounds(area.removeFromRight(meterWidth));
    area.removeFromRight(15);

    auto displayArea = area;
    int halfHeight = displayArea.getHeight() / 2;

    transferGraph.setBounds(displayArea.removeFromTop(halfHeight).reduced(0, 4));
    waveformDisplay.setBounds(displayArea.reduced(0, 4));
}