#include "PluginProcessor.h"
#include "PluginEditor.h"

_8f_clipAudioProcessorEditor::_8f_clipAudioProcessorEditor(_8f_clipAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), transferGraph(p), waveformDisplay(p), outputMeter(p)
{
    int savedStyle = audioProcessor.apvts.state.getProperty("uiStyle", 1);
    setStyle(savedStyle);

    isTransferGraphHidden = audioProcessor.apvts.state.getProperty("transferGraphHidden", true);
    isOutputMeterHidden = audioProcessor.apvts.state.getProperty("outputMeterHidden", true);

    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setRotaryParameters(juce::MathConstants<float>::pi, juce::MathConstants<float>::pi * 3.0f, true);
    addAndMakeVisible(gainSlider);
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "GAIN", gainSlider);
    gainShadow.setShadowProperties(juce::DropShadow(juce::Colours::black.withAlpha(0.2f), 5, { 0, 3 }));
    gainSlider.setComponentEffect(&gainShadow);

    clipSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    clipSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    clipSlider.setTextValueSuffix(" %");
    clipSlider.setRotaryParameters(0.0f, juce::MathConstants<float>::pi * 2.0f, true);
    addAndMakeVisible(clipSlider);
    clipAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "CLIP", clipSlider);
    clipShadow.setShadowProperties(juce::DropShadow(juce::Colours::black.withAlpha(0.2f), 5, { 0, 3 }));
    clipSlider.setComponentEffect(&clipShadow);

    softnessSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    softnessSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    softnessSlider.setTextValueSuffix(" %");
    softnessSlider.setRotaryParameters(0.0f, juce::MathConstants<float>::pi * 2.0f, true);
    addAndMakeVisible(softnessSlider);
    softnessAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "SOFTNESS", softnessSlider);
    softnessShadow.setShadowProperties(juce::DropShadow(juce::Colours::black.withAlpha(0.2f), 5, { 0, 3 }));
    softnessSlider.setComponentEffect(&softnessShadow);

    osSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    osSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    osSlider.setRotaryParameters(0.0f, juce::MathConstants<float>::pi * 2.0f, true);
    addAndMakeVisible(osSlider);
    osAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "OS", osSlider);

    addAndMakeVisible(transferGraph);
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(outputMeter);
    osShadow.setShadowProperties(juce::DropShadow(juce::Colours::black.withAlpha(0.2f), 5, { 0, 3 }));
    osSlider.setComponentEffect(&osShadow);

    addAndMakeVisible(logoComponent);

    setResizable(true, true);
    
    constexpr int minWidth = 550;
    constexpr int minHeight = 400;
    constexpr int maxWidth = 4096;
    constexpr int maxHeight = 2560;
    
    setResizeLimits(minWidth, minHeight, maxWidth, maxHeight);

    int savedWidth = juce::jlimit(minWidth, maxWidth, static_cast<int>(audioProcessor.apvts.state.getProperty("uiWidth", 680)));
    int savedHeight = juce::jlimit(minHeight, maxHeight, static_cast<int>(audioProcessor.apvts.state.getProperty("uiHeight", 500)));
    setSize(savedWidth, savedHeight);
}

_8f_clipAudioProcessorEditor::~_8f_clipAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void _8f_clipAudioProcessorEditor::setStyle(int styleIndex)
{
    currentStyle = styleIndex;
    if (currentStyle == 1)
        setLookAndFeel(&darkRedStyle);
    else
        setLookAndFeel(&brightBlueStyle);

    repaint();
}

void _8f_clipAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(currentStyle == 1 ? juce::Colours::lightgrey : juce::Colours::darkgrey);
    g.setFont(juce::FontOptions(12.0f));

    auto area = getLocalBounds().reduced(15);
    auto sliderArea = area.removeFromBottom(100);

    int availableWidthForSliders = sliderArea.getWidth() - 100;
    int w = availableWidthForSliders / 4;

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
    auto actualSlidersArea = sliderArea.removeFromTop(sliderArea.getHeight() - 15);

    gainSlider.setBounds(actualSlidersArea.removeFromLeft(sliderWidth).reduced(5));
    clipSlider.setBounds(actualSlidersArea.removeFromLeft(sliderWidth).reduced(5));
    softnessSlider.setBounds(actualSlidersArea.removeFromLeft(sliderWidth).reduced(5));
    osSlider.setBounds(actualSlidersArea.removeFromLeft(sliderWidth).reduced(5));

    logoComponent.setBounds(actualSlidersArea.removeFromLeft(100));

    area.removeFromBottom(15);

    int meterWidth = 70;
    if (!isOutputMeterHidden)
    {
        auto meterBounds = area.removeFromRight(meterWidth);
        meterBounds = meterBounds.reduced(0, 4);
        outputMeter.setBounds(meterBounds);
        outputMeter.setVisible(true);
        area.removeFromRight(15);
    }
    else
    {
        outputMeter.setVisible(false);
    }

    auto displayArea = area;
    int halfHeight = displayArea.getHeight() / 2;

    if (isTransferGraphHidden)
    {
        transferGraph.setVisible(false);
        waveformDisplay.setBounds(displayArea.reduced(0, 4));
    }
    else
    {
        transferGraph.setVisible(true);
        transferGraph.setBounds(displayArea.removeFromTop(halfHeight).reduced(0, 4));
        waveformDisplay.setBounds(displayArea.reduced(0, 4));
    }

    startTimer(500);
}

void _8f_clipAudioProcessorEditor::timerCallback()
{
    stopTimer();
    audioProcessor.apvts.state.setProperty("uiWidth", getWidth(), nullptr);
    audioProcessor.apvts.state.setProperty("uiHeight", getHeight(), nullptr);
    audioProcessor.apvts.state.setProperty("transferGraphHidden", isTransferGraphHidden, nullptr);
    audioProcessor.apvts.state.setProperty("outputMeterHidden", isOutputMeterHidden, nullptr);
}