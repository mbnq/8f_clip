#pragma once
#include <JuceHeader.h>

class BrightBlueStyle : public juce::LookAndFeel_V4
{
public:
    BrightBlueStyle()
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

        // arc
        g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
        g.drawEllipse(rx, ry, rw, rw, 5.0f);

        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

		// filling the arc
        juce::Path valueArc;
        valueArc.addCentredArc(center.x, center.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // dot
        auto thumbWidth = 9.0f;
        auto thumbPointX = center.x + radius * std::sin(angle);
        auto thumbPointY = center.y - radius * std::cos(angle);
        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumbPointX - thumbWidth / 2.0f, thumbPointY - thumbWidth / 2.0f, thumbWidth, thumbWidth);

		// txt inside the knob
        auto text = slider.getTextFromValue(slider.getValue());
        auto suffix = slider.getTextValueSuffix();
        if (suffix.isNotEmpty() && !text.contains(suffix))
        {
            text += suffix;
        }

        g.setColour(juce::Colours::darkgrey);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(text, bounds, juce::Justification::centred, true);
    }
};