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

        setColour(BruColors::panelBackgroundId, juce::Colours::whitesmoke);
        setColour(BruColors::panelOutlineId, juce::Colours::lightgrey);
    }

    enum BruColors
    {
        panelBackgroundId = 0x1100101,
        panelOutlineId
    };

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
        auto thumbPointX = center.x + radius * std::sin(angle);
        auto thumbPointY = center.y - radius * std::cos(angle);
        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumbPointX - thumbWidth / 2.0f, thumbPointY - thumbWidth / 2.0f, thumbWidth, thumbWidth);

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

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (slider.getSliderStyle() == juce::Slider::LinearHorizontal)
        {
            auto trackWidth = (float)width;
            auto trackHeight = 6.0f;
            auto trackX = (float)x;
            auto trackY = (float)y + ((float)height - trackHeight) * 0.5f;

            juce::Rectangle<float> trackBounds(trackX, trackY, trackWidth, trackHeight);

			// waveform speed slider gradient
            juce::ColourGradient trackGrad(
                juce::Colours::lightgrey.darker(0.15f), trackX, trackY,
                juce::Colours::lightgrey.withAlpha(0.4f), trackX + trackWidth, trackY, false
            );
            g.setGradientFill(trackGrad);
            g.fillRoundedRectangle(trackBounds, 3.0f);

            // dot
            auto thumbWidth = 10.0f;
            auto thumbHeight = 10.0f;
            float thumbX = juce::jlimit(trackX, trackX + trackWidth - thumbWidth, sliderPos - thumbWidth * 0.5f);
            float thumbY = trackY + (trackHeight - thumbHeight) * 0.5f;

            g.setColour(findColour(juce::Slider::thumbColourId));
            g.fillEllipse(thumbX, thumbY, thumbWidth, thumbHeight);
        }
        else
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }
};

class DarkRedStyle : public juce::LookAndFeel_V4
{
public:
    DarkRedStyle()
    {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff222222));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::indianred);
        setColour(juce::Slider::thumbColourId, juce::Colours::whitesmoke);
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);

        setColour(BrightBlueStyle::panelBackgroundId, juce::Colour(0xff181818));
        setColour(BrightBlueStyle::panelOutlineId, juce::Colour(0xff333333));
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
        auto thumbPointX = center.x + radius * std::sin(angle);
        auto thumbPointY = center.y - radius * std::cos(angle);
        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumbPointX - thumbWidth / 2.0f, thumbPointY - thumbWidth / 2.0f, thumbWidth, thumbWidth);

        auto text = slider.getTextFromValue(slider.getValue());
        auto suffix = slider.getTextValueSuffix();
        if (suffix.isNotEmpty() && !text.contains(suffix))
        {
            text += suffix;
        }

        g.setColour(juce::Colours::whitesmoke);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(text, bounds, juce::Justification::centred, true);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (slider.getSliderStyle() == juce::Slider::LinearHorizontal)
        {
            auto trackWidth = (float)width;
            auto trackHeight = 6.0f;
            auto trackX = (float)x;
            auto trackY = (float)y + ((float)height - trackHeight) * 0.5f;

            juce::Rectangle<float> trackBounds(trackX, trackY, trackWidth, trackHeight);

            juce::ColourGradient trackGrad(
                juce::Colours::darkgrey.withAlpha(0.8f), trackX, trackY,
                juce::Colours::darkgrey.withAlpha(0.3f), trackX + trackWidth, trackY, false
            );
            g.setGradientFill(trackGrad);
            g.fillRoundedRectangle(trackBounds, 3.0f);

            auto thumbWidth = 10.0f;
            auto thumbHeight = 10.0f;
            float thumbX = juce::jlimit(trackX, trackX + trackWidth - thumbWidth, sliderPos - thumbWidth * 0.5f);
            float thumbY = trackY + (trackHeight - thumbHeight) * 0.5f;

            g.setColour(findColour(juce::Slider::thumbColourId));
            g.fillEllipse(thumbX, thumbY, thumbWidth, thumbHeight);
        }
        else
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }
};