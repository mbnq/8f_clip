#include "PluginProcessor.h"
#include "PluginEditor.h"

_8f_clipAudioProcessorEditor::_8f_clipAudioProcessorEditor(_8f_clipAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    transferGraph(p), waveformDisplay(p), outputMeter(p)
{
    setLookAndFeel(&jasnyStyl);

    // GAIN (domyślna 0 dB wypada na godzinie 12 przy zakresie od π do 3π)
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setRotaryParameters(juce::MathConstants<float>::pi,
        juce::MathConstants<float>::pi * 3.0f, true);
    addAndMakeVisible(gainSlider);
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "GAIN", gainSlider);

    // CLIP (domyślna 0% wypada na godzinie 12 przy zakresie od 0 do 2π)
    clipSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    clipSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    clipSlider.setTextValueSuffix(" %");
    clipSlider.setRotaryParameters(0.0f,
        juce::MathConstants<float>::pi * 2.0f, true);
    addAndMakeVisible(clipSlider);
    clipAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "CLIP", clipSlider);

    // SOFTNESS (domyślna 0% wypada na godzinie 12 przy zakresie od 0 do 2π)
    softnessSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    softnessSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    softnessSlider.setTextValueSuffix(" %");
    softnessSlider.setRotaryParameters(0.0f,
        juce::MathConstants<float>::pi * 2.0f, true);
    addAndMakeVisible(softnessSlider);
    softnessAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "SOFTNESS", softnessSlider);

    addAndMakeVisible(transferGraph);
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(outputMeter);

    setResizable(true, true);
    setResizeLimits(550, 400, 1200, 900);
    setSize(680, 500);
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
    int w = sliderArea.getWidth() / 3;

    g.drawText("GAIN", sliderArea.getX(), sliderArea.getY() - 15, w, 15, juce::Justification::centred);
    g.drawText("CLIP", sliderArea.getX() + w, sliderArea.getY() - 15, w, 15, juce::Justification::centred);
    g.drawText("SOFTNESS", sliderArea.getX() + (w * 2), sliderArea.getY() - 15, w, 15, juce::Justification::centred);
}

void _8f_clipAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(15);

    auto sliderArea = area.removeFromBottom(100);
    int sliderWidth = sliderArea.getWidth() / 3;

    gainSlider.setBounds(sliderArea.removeFromLeft(sliderWidth).reduced(5));
    clipSlider.setBounds(sliderArea.removeFromLeft(sliderWidth).reduced(5));
    softnessSlider.setBounds(sliderArea.reduced(5));

    area.removeFromBottom(15);

    int meterWidth = 70;
    outputMeter.setBounds(area.removeFromRight(meterWidth));
    area.removeFromRight(15);

    auto displayArea = area;
    int halfHeight = displayArea.getHeight() / 2;

    transferGraph.setBounds(displayArea.removeFromTop(halfHeight).reduced(0, 4));
    waveformDisplay.setBounds(displayArea.reduced(0, 4));
}