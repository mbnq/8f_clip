#include "PluginProcessor.h"
#include "PluginEditor.h"

_8f_clipAudioProcessorEditor::_8f_clipAudioProcessorEditor(_8f_clipAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    transferGraph(p), waveformDisplay(p), outputMeter(p)
{
    setLookAndFeel(&jasnyStyl);

    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    addAndMakeVisible(gainSlider);
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "GAIN", gainSlider);

    clipSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    clipSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    addAndMakeVisible(clipSlider);
    clipAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "CLIP", clipSlider);

    softnessSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    softnessSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
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
    auto sliderArea = area.removeFromBottom(110);
    int w = sliderArea.getWidth() / 3;

    g.drawText("GAIN", sliderArea.getX(), sliderArea.getY() - 15, w, 15, juce::Justification::centred);
    g.drawText("CLIP", sliderArea.getX() + w, sliderArea.getY() - 15, w, 15, juce::Justification::centred);
    g.drawText("SOFTNESS", sliderArea.getX() + (w * 2), sliderArea.getY() - 15, w, 15, juce::Justification::centred);
}

void _8f_clipAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(15);

    auto sliderArea = area.removeFromBottom(110);
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