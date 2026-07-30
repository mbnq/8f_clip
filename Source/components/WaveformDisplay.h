#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "Themes.h"
#include <vector>

class WaveformDisplayComponent : public juce::Component, public juce::Timer
{
public:
    WaveformDisplayComponent(_8f_clipAudioProcessor& p) : processor(p)
    {
        zoomSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        zoomSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(zoomSlider);
        zoomAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "ZOOM", zoomSlider);

        frozenMinHistory.resize(processor.waveformSize, 0.0f);
        frozenMaxHistory.resize(processor.waveformSize, 0.0f);

        startTimerHz(100);
    }

    void timerCallback() override { repaint(); }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (event.getNumberOfClicks() == 2)
        {
            isPaused = !isPaused;
            if (isPaused)
            {
                pausedHead = processor.waveformIndex.load();
                for (int i = 0; i < processor.waveformSize; ++i)
                {
                    frozenMinHistory[i] = processor.waveMinHistory[i];
                    frozenMaxHistory[i] = processor.waveMaxHistory[i];
                }
            }
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bgColour = getLookAndFeel().findColour(BrightBlueStyle::panelBackgroundId);
        auto outlineColour = getLookAndFeel().findColour(BrightBlueStyle::panelOutlineId);

        g.setColour(bgColour);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
        g.setColour(outlineColour);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);

        g.setColour(bgColour.contrasting(0.08f));
        g.drawHorizontalLine(getHeight() / 2, 0.0f, (float)getWidth());

        float w = (float)getWidth();
        float h = (float)getHeight();

        float clipVal = processor.apvts.getRawParameterValue("CLIP")->load();
        float clipPct = clipVal / 100.0f;
        float softVal = processor.apvts.getRawParameterValue("SOFTNESS")->load();
        float softPct = softVal / 100.0f;

        float threshold = juce::jmax(0.01f, 1.0f - clipPct);

        float posThresholdY = (h / 2.0f) - (threshold * (h * 0.4f));
        float negThresholdY = (h / 2.0f) + (threshold * (h * 0.4f));

        g.setColour(juce::Colours::dodgerblue.withAlpha(0.6f));
        g.drawLine(0.0f, posThresholdY, w, posThresholdY, 1.0f);
        g.drawLine(0.0f, negThresholdY, w, negThresholdY, 1.0f);

        if (softVal > 0.0f && clipVal > 0.0f)
        {
            float ceiling = threshold + softPct * (1.0f - threshold);
            float posCeilingY = (h / 2.0f) - (ceiling * (h * 0.4f));
            float negCeilingY = (h / 2.0f) + (ceiling * (h * 0.4f));

            g.setColour(juce::Colours::limegreen.withAlpha(0.7f));
            g.drawLine(0.0f, posCeilingY, w, posCeilingY, 1.0f);
            g.drawLine(0.0f, negCeilingY, w, negCeilingY, 1.0f);
        }

        int size = processor.waveformSize;
        int head = isPaused ? pausedHead : processor.waveformIndex.load();

        g.setColour(juce::Colours::dodgerblue.withAlpha(0.75f));

        for (int i = 0; i < size; ++i)
        {
            int idx = (head + i) % size;
            if (idx < 0 || idx >= size) continue;

            float minSample = isPaused ? frozenMinHistory[idx] : processor.waveMinHistory[idx];
            float maxSample = isPaused ? frozenMaxHistory[idx] : processor.waveMaxHistory[idx];

            float x = ((float)i / (float)size) * w;
            float yMin = (h / 2.0f) - (minSample * (h * 0.4f));
            float yMax = (h / 2.0f) - (maxSample * (h * 0.4f));

            g.drawLine(x, yMin, x, yMax, 1.5f);
        }

        g.setColour(bgColour.contrasting(0.6f));
        g.setFont(12.0f);
        g.drawText("OUTPUT WAVEFORM", 8, 4, 150, 20, juce::Justification::left);

        if (isPaused)
        {
            g.setColour(bgColour.contrasting(0.8f));
            g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
            g.drawText("PAUSED", getLocalBounds(), juce::Justification::centred, false);
        }

        g.setFont(11.0f);
        int sliderRightX = getWidth() - 15;
        int sliderY = getHeight() - 20 + 5;
        g.drawText("-", sliderRightX - 90, sliderY, 12, 15, juce::Justification::centred);
        g.drawText("+", sliderRightX + 3, sliderY, 12, 15, juce::Justification::centred);
    }

    void resized() override
    {
        zoomSlider.setBounds(getWidth() - 95, getHeight() - 20 + 5, 85, 15);
    }

private:
    _8f_clipAudioProcessor& processor;
    juce::Slider zoomSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> zoomAttachment;

    bool isPaused = false;
    int pausedHead = 0;
    std::vector<float> frozenMinHistory;
    std::vector<float> frozenMaxHistory;
};