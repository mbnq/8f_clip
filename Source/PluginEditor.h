#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "components/ContextMenu.h"
#include "components/WaveformDisplay.h"
#include "components/TransferGraph.h"
#include "components/OutputMeter.h"

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

// --- G£ÓWNE OKNO WTYCZKI ---
class _8f_clipAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    _8f_clipAudioProcessorEditor(_8f_clipAudioProcessor&);
    ~_8f_clipAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (event.mods.isRightButtonDown())
        {
            ContextMenu::show(this, [this](int choice)
                {
                    if (choice == 1)
                    {
                        setSize(680, 500);
                    }
                    else if (choice == 2)
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
                });
        }
    }

private:
    _8f_clipAudioProcessor& audioProcessor;

    std::array<std::unique_ptr<juce::dsp::Oversampling<float>>, 4> oversamplers;

    double currentSampleRate = 44100.0; // <--- Dodane
    int decimationCounter = 0;
    float blockMin = 0.0f;
    float blockMax = 0.0f;

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