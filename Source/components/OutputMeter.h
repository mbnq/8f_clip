#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

class OutputMeterComponent : public juce::Component, public juce::Timer
{
public:
    OutputMeterComponent(_8f_clipAudioProcessor& p) : processor(p) { startTimerHz(100); }

    void timerCallback() override
    {
        float currentLevel = processor.currentOutputLevel.load();

        displayLevel = currentLevel;

        if (currentLevel > peakLevel) {
            peakLevel = currentLevel;
            peakHoldTimer = 45;
        }
        else if (peakHoldTimer > 0) {
            peakHoldTimer--;
        }
        else {
            peakLevel -= 0.008f;
            if (peakLevel < 0.0f) peakLevel = 0.0f;
        }
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::whitesmoke);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
        g.setColour(juce::Colours::lightgrey);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);

        float h = (float)getHeight() - 38.0f;
        float fillHeight = juce::jmin(1.0f, displayLevel) * h;
        float peakHeight = juce::jmin(1.0f, peakLevel) * h;

        float meterWidth = 14.0f;
        float meterX = (getWidth() - meterWidth) / 2.0f;

        g.setColour(juce::Colours::lightgrey.withAlpha(0.5f));
        g.fillRoundedRectangle(meterX, 24.0f, meterWidth, h, 3.0f);

        g.setColour(juce::Colours::dodgerblue);
        g.fillRoundedRectangle(meterX, 24.0f + (h - fillHeight), meterWidth, fillHeight, 3.0f);

        g.setColour(juce::Colours::darkgrey);
        g.fillRect(meterX - 2.0f, 24.0f + (h - peakHeight), meterWidth + 4.0f, 2.0f);

        g.setColour(juce::Colours::darkgrey);
        g.setFont(9.0f);
        g.drawText("0dB", getWidth() / 2 - 15, 8, 30, 10, juce::Justification::centred);
        g.drawText("-6", getWidth() / 2 - 15, (int)(24.0f + h * 0.5f - 5), 30, 10, juce::Justification::centred);
        g.drawText("-inf", getWidth() / 2 - 15, (int)(24.0f + h + 2), 30, 10, juce::Justification::centred);
    }

private:
    _8f_clipAudioProcessor& processor;
    float displayLevel = 0.0f;
    float peakLevel = 0.0f;
    int peakHoldTimer = 0;
};