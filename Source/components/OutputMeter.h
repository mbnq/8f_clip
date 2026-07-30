#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "Themes.h"

class OutputMeterComponent : public juce::Component, public juce::Timer
{
public:
    OutputMeterComponent(_8f_clipAudioProcessor& p) : processor(p) { startTimerHz(100); }

    void timerCallback() override
    {
        float targetLevel = processor.currentOutputLevel.load();

        if (targetLevel > displayLevel)
        {
            displayLevel = targetLevel;
        }
        else
        {
            displayLevel -= 0.005f;
            if (displayLevel < targetLevel)
                displayLevel = targetLevel;
        }

        if (displayLevel > peakLevel) {
            peakLevel = displayLevel;
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
        auto bgColour = getLookAndFeel().findColour(BrightBlueStyle::panelBackgroundId);
        auto outlineColour = getLookAndFeel().findColour(BrightBlueStyle::panelOutlineId);
        auto textColour = bgColour.contrasting(0.6f);

        g.setColour(bgColour);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
        g.setColour(outlineColour);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);

        float h = (float)getHeight() - 38.0f;
        float fillHeight = juce::jmin(1.0f, displayLevel) * h;
        float peakHeight = juce::jmin(1.0f, peakLevel) * h;

        float meterWidth = 14.0f;
        float meterX = (getWidth() - meterWidth) / 2.0f;

        g.setColour(outlineColour.withAlpha(0.5f));
        g.fillRoundedRectangle(meterX, 24.0f, meterWidth, h, 3.0f);

        juce::ColourGradient fillGradient(
            juce::Colours::blue,
            meterX, 24.0f + h,
            juce::Colours::red,
            meterX, 24.0f,
            false
        );
        fillGradient.addColour(0.50, juce::Colours::green);
        fillGradient.addColour(0.85, juce::Colours::orange);
        fillGradient.addColour(0.95, juce::Colours::red);

        g.setGradientFill(fillGradient);
        g.fillRoundedRectangle(meterX, 24.0f + (h - fillHeight), meterWidth, fillHeight, 3.0f);

        g.setColour(textColour);
        g.fillRect(meterX - 2.0f, 24.0f + (h - peakHeight), meterWidth + 4.0f, 2.0f);

        g.setColour(textColour);
        g.setFont(9.0f);

        g.drawText("0dB", getWidth() / 2 - 15, 8, 30, 10, juce::Justification::centred);

        float y_1 = 24.0f + (1.0f - 0.89125f) * h - 5.0f;
        g.drawText("-1", getWidth() / 2 - 15, (int)y_1, 30, 10, juce::Justification::centred);

        float y_3 = 24.0f + (1.0f - 0.7079f) * h - 5.0f;
        g.drawText("-3", getWidth() / 2 - 15, (int)y_3, 30, 10, juce::Justification::centred);

        g.drawText("-6", getWidth() / 2 - 15, (int)(24.0f + h * 0.5f - 5), 30, 10, juce::Justification::centred);

        float y_12 = 24.0f + (1.0f - 0.2512f) * h - 5.0f;
        g.drawText("-12", getWidth() / 2 - 15, (int)y_12, 30, 10, juce::Justification::centred);

        g.drawText("-inf", getWidth() / 2 - 15, (int)(24.0f + h + 2), 30, 10, juce::Justification::centred);
    }

private:
    _8f_clipAudioProcessor& processor;
    float displayLevel = 0.0f;
    float peakLevel = 0.0f;
    int peakHoldTimer = 0;
};