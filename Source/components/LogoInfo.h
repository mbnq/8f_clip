#pragma once
#include <JuceHeader.h>

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

        bandcampButton.setButtonText("[https://oktafonika.bandcamp.com/](https://oktafonika.bandcamp.com/)");
        bandcampButton.setURL(juce::URL("[https://oktafonika.bandcamp.com/](https://oktafonika.bandcamp.com/)"));
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
            "Wersja: 0.4a 2026\n\n"
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
    LogoComponent()
    {
        // Konfiguracja cienia pod logo
        shadowEffect.setShadowProperties(juce::DropShadow(juce::Colours::black.withAlpha(0.25f), 6, { 0, 2 }));
        setComponentEffect(&shadowEffect);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        g.setColour(juce::Colours::darkgrey);

        bounds.removeFromTop(23);
        auto topPart = bounds.removeFromTop(bounds.getHeight() * 0.7f);

        g.setFont(juce::FontOptions(72.0f, juce::Font::bold));
        g.drawText("8f", topPart, juce::Justification::centred, false);

        g.setFont(juce::FontOptions(16.0f));
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
            juce::URL("[https://oktafonika.bandcamp.com/](https://oktafonika.bandcamp.com/)").launchInDefaultBrowser();
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

private:
    juce::DropShadowEffect shadowEffect;
};