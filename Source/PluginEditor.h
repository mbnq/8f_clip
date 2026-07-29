#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class JasnyStyl : public juce::LookAndFeel_V4
{
public:
    JasnyStyl()
    {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::darkgrey);
        setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::lightgrey);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(6.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto center = bounds.getCentre();
        auto rx = center.x - radius;
        auto ry = center.y - radius;
        auto rw = radius * 2.0f;

        g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
        g.drawEllipse(rx, ry, rw, rw, 5.0f);

        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        juce::Path valueArc;
        valueArc.addCentredArc(center.x, center.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        auto thumbWidth = 9.0f;
        auto thumbPointX = center.x + (radius - 5.0f) * std::sin(angle);
        auto thumbPointY = center.y - (radius - 5.0f) * std::cos(angle);
        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumbPointX - thumbWidth / 2.0f, thumbPointY - thumbWidth / 2.0f, thumbWidth, thumbWidth);

        auto text = slider.getTextFromValue(slider.getValue());
        auto suffix = slider.getTextValueSuffix();
        if (suffix.isNotEmpty() && !text.contains(suffix))
        {
            text += suffix;
        }

        g.setColour(juce::Colours::darkgrey);
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(text, bounds, juce::Justification::centred, true);
    }
};

// --- WYSKAKUJ¥CE OKIENKO INFORMACYJNE (ABOUT) ---
class AboutBoxComponent : public juce::Component
{
public:
    AboutBoxComponent()
    {
        closeButton.setButtonText("CLOSE");
        closeButton.onClick = [this]() {
            if (auto* parent = getParentComponent())
                parent->removeChildComponent(this);
            delete this;
            };
        addAndMakeVisible(closeButton);

        bandcampButton.setButtonText("https://oktafonika.bandcamp.com/");
        bandcampButton.setURL(juce::URL("https://oktafonika.bandcamp.com/"));
        bandcampButton.setColour(juce::HyperlinkButton::textColourId, juce::Colours::dodgerblue);
        addAndMakeVisible(bandcampButton);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        g.setColour(juce::Colours::white);
        g.fillRoundedRectangle(bounds, 8.0f);
        g.setColour(juce::Colours::darkgrey);
        g.drawRoundedRectangle(bounds, 8.0f, 1.5f);

        g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        g.drawText("8f_clip by Oktafonika", 15, 15, getWidth() - 30, 25, juce::Justification::centred);

        g.setFont(juce::FontOptions(13.0f));
        juce::String text =
            "8f Audio Clipper\n"
            "Wersja: " + juce::String(JucePlugin_VersionString) + " FREEWARE 2026\n\n"
            "Author: Mateusz \"Oktafonika\" Bieniek\n"
            "Email: mateuszbnk@gmail.com\n\n"
            "Support me by buying my music here:";

        g.drawMultiLineText(text, 20, 58, getWidth() - 40, juce::Justification::centredLeft);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(20);
        closeButton.setBounds(area.removeFromBottom(30).reduced(50, 0));
        bandcampButton.setBounds(20, 152, getWidth() - 40, 20);
    }

private:
    juce::TextButton closeButton;
    juce::HyperlinkButton bandcampButton;
};

// --- INTERAKTYWNY KOMPONENT LOGO (8f + Oktafonika) ---
class LogoComponent : public juce::Component
{
public:
    LogoComponent() = default;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        g.setColour(juce::Colours::darkgrey);

        bounds.removeFromTop(26);
        auto topPart = bounds.removeFromTop(bounds.getHeight() * 0.7f);

        g.setFont(juce::FontOptions(72.0f, juce::Font::bold));
        g.drawText("8f", topPart, juce::Justification::centred, false);

        g.setFont(juce::FontOptions(11.0f));
        g.drawText("Oktafonika", bounds, juce::Justification::centred, false);
    }

    void mouseEnter(const juce::MouseEvent&) override
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (event.mods.isRightButtonDown())
        {
            juce::URL("https://oktafonika.bandcamp.com/").launchInDefaultBrowser();
        }
        else
        {
            if (auto* parent = getTopLevelComponent())
            {
                if (parent->findChildWithID("AboutBox") == nullptr)
                {
                    auto* aboutBox = new AboutBoxComponent();
                    aboutBox->setComponentID("AboutBox");

                    int boxWidth = 360;
                    int boxHeight = 235;

                    int x = (parent->getWidth() - boxWidth) / 2;
                    int y = (parent->getHeight() - boxHeight) / 2;

                    aboutBox->setBounds(x, y, boxWidth, boxHeight);
                    parent->addAndMakeVisible(aboutBox);
                    aboutBox->toFront(true);
                }
            }
        }
    }
};

// --- WYŒWIETLACZ 1: Wykres funkcji clippera ---
class TransferGraphComponent : public juce::Component, public juce::Timer
{
public:
    TransferGraphComponent(_8f_clipAudioProcessor& p) : processor(p) { startTimerHz(30); }
    void timerCallback() override { repaint(); }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::whitesmoke);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
        g.setColour(juce::Colours::lightgrey);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);

        g.setColour(juce::Colours::whitesmoke.darker(0.1f));
        float w = (float)getWidth();
        float h = (float)getHeight();
        g.drawVerticalLine(int(w / 2), 0.0f, h);
        g.drawHorizontalLine(int(h / 2), 0.0f, w);

        float clipVal = processor.apvts.getRawParameterValue("CLIP")->load();
        float clipPct = clipVal / 100.0f;
        float softPct = processor.apvts.getRawParameterValue("SOFTNESS")->load() / 100.0f;
        float threshold = juce::jmax(0.01f, 1.0f - clipPct);

        juce::Path p;
        int numPoints = 100;
        for (int i = 0; i < numPoints; ++i)
        {
            float normX = (float)i / (float)(numPoints - 1);
            float input = normX * 2.0f - 1.0f;
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

            float screenX = normX * w;
            float screenY = h - ((output + 1.0f) * 0.5f * h);

            if (i == 0) p.startNewSubPath(screenX, screenY);
            else p.lineTo(screenX, screenY);
        }

        g.setColour(juce::Colours::dodgerblue);
        g.strokePath(p, juce::PathStrokeType(2.5f));

        g.setColour(juce::Colours::darkgrey);
        g.setFont(12.0f);
        g.drawText("CLIPPING FUNCTION", 8, 4, 150, 20, juce::Justification::left);
    }

private:
    _8f_clipAudioProcessor& processor;
};

// --- WYŒWIETLACZ 2: Podgl¹d fali audio (czyste s³upki bez bia³ych plam w œrodku) ---
class WaveformDisplayComponent : public juce::Component, public juce::Timer
{
public:
    WaveformDisplayComponent(_8f_clipAudioProcessor& p) : processor(p)
    {
        zoomSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        zoomSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(zoomSlider);
        zoomAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "ZOOM", zoomSlider);

        startTimerHz(30);
    }

    void timerCallback() override { repaint(); }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::whitesmoke);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
        g.setColour(juce::Colours::lightgrey);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);

        g.setColour(juce::Colours::darkgrey.withAlpha(0.2f));
        g.drawHorizontalLine(getHeight() / 2, 0.0f, (float)getWidth());

        float w = (float)getWidth();
        float h = (float)getHeight();

        float clipVal = processor.apvts.getRawParameterValue("CLIP")->load();
        float clipPct = clipVal / 100.0f;
        float threshold = juce::jmax(0.01f, 1.0f - clipPct);

        float posThresholdY = (h / 2.0f) - (threshold * (h * 0.4f));
        float negThresholdY = (h / 2.0f) + (threshold * (h * 0.4f));

        g.setColour(juce::Colours::dodgerblue.withAlpha(0.6f));
        g.drawLine(0.0f, posThresholdY, w, posThresholdY, 1.0f);
        g.drawLine(0.0f, negThresholdY, w, negThresholdY, 1.0f);

        int size = processor.waveformSize;
        int head = processor.waveformIndex.load();

        g.setColour(juce::Colours::dodgerblue.withAlpha(0.75f));

        for (int i = 0; i < size; ++i)
        {
            int idx = (head + i) % size;
            if (idx < 0 || idx >= size) continue;

            float minSample = processor.waveMinHistory[idx];
            float maxSample = processor.waveMaxHistory[idx];

            float x = ((float)i / (float)size) * w;
            float yMin = (h / 2.0f) - (minSample * (h * 0.4f));
            float yMax = (h / 2.0f) - (maxSample * (h * 0.4f));

            g.drawLine(x, yMin, x, yMax, 1.5f);
        }

        g.setColour(juce::Colours::darkgrey);
        g.setFont(12.0f);
        g.drawText("OUTPUT WAVEFORM", 8, 4, 150, 20, juce::Justification::left);
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
};

// --- WYŒWIETLACZ 3: Miernik wyjœciowy ---
class OutputMeterComponent : public juce::Component, public juce::Timer
{
public:
    OutputMeterComponent(_8f_clipAudioProcessor& p) : processor(p) { startTimerHz(30); }

    void timerCallback() override
    {
        float currentLevel = processor.currentOutputLevel.load();

        if (currentLevel > displayLevel) {
            displayLevel = currentLevel;
        }
        else {
            displayLevel -= 0.035f;
            if (displayLevel < 0.0f) displayLevel = 0.0f;
        }

        if (currentLevel > peakLevel) {
            peakLevel = currentLevel;
            peakHoldTimer = 45;
        }
        else if (peakHoldTimer > 0) {
            peakHoldTimer--;
        }
        else {
            peakLevel -= 0.02f;
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

// --- G£ÓWNE OKNO WTYCZKI ---
class _8f_clipAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    _8f_clipAudioProcessorEditor(_8f_clipAudioProcessor&);
    ~_8f_clipAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    _8f_clipAudioProcessor& audioProcessor;
    JasnyStyl jasnyStyl;

    juce::Slider gainSlider;
    juce::Slider clipSlider;
    juce::Slider softnessSlider;
    juce::Slider osSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<SliderAttachment> clipAttachment;
    std::unique_ptr<SliderAttachment> softnessAttachment;
    std::unique_ptr<SliderAttachment> osAttachment;

    TransferGraphComponent transferGraph;
    WaveformDisplayComponent waveformDisplay;
    OutputMeterComponent outputMeter;

    LogoComponent logoComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(_8f_clipAudioProcessorEditor)
};