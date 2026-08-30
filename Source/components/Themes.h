#pragma once
#include <JuceHeader.h>

namespace ThemeColors
{
    enum CustomColorIds
    {
        panelBackgroundId = 0x1100101,
        panelOutlineId    = 0x1100102
    };
}

class BaseTheme : public juce::LookAndFeel_V4
{
public:
    struct Parameters
    {
        juce::Colour windowBackground;
        juce::Colour rotaryFill;
        juce::Colour rotaryOutline;
        juce::Colour thumbColor;
        juce::Colour panelBackground;
        juce::Colour panelOutline;
        juce::Colour textColor;
        juce::Colour linearTrackStart;
        juce::Colour linearTrackEnd;
    };

    static constexpr float ROTARY_THUMB_WIDTH = 9.0f;
    static constexpr float ROTARY_OUTLINE_WIDTH = 5.0f;
    static constexpr float ROTARY_BOUNDS_REDUCTION = 6.0f;
    static constexpr float LINEAR_TRACK_HEIGHT = 6.0f;
    static constexpr float LINEAR_THUMB_SIZE = 10.0f;
    static constexpr float LINEAR_TRACK_CORNER_RADIUS = 3.0f;
    static constexpr float TEXT_FONT_SIZE = 11.0f;

    BaseTheme(const Parameters& params)
    {
        setColour(juce::ResizableWindow::backgroundColourId, params.windowBackground);
        setColour(juce::Slider::rotarySliderFillColourId, params.rotaryFill);
        setColour(juce::Slider::thumbColourId, params.thumbColor);
        setColour(juce::Slider::rotarySliderOutlineColourId, params.rotaryOutline);
        setColour(ThemeColors::panelBackgroundId, params.panelBackground);
        setColour(ThemeColors::panelOutlineId, params.panelOutline);

        cachedParams = params;
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(ROTARY_BOUNDS_REDUCTION);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto center = bounds.getCentre();
        auto rx = center.x - radius;
        auto ry = center.y - radius;
        auto rw = radius * 2.0f;

        g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
        g.drawEllipse(rx, ry, rw, rw, ROTARY_OUTLINE_WIDTH);

        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        juce::Path valueArc;
        valueArc.addCentredArc(center.x, center.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, juce::PathStrokeType(ROTARY_OUTLINE_WIDTH, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        auto thumbPointX = center.x + radius * std::sin(angle);
        auto thumbPointY = center.y - radius * std::cos(angle);
        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumbPointX - ROTARY_THUMB_WIDTH / 2.0f, thumbPointY - ROTARY_THUMB_WIDTH / 2.0f, ROTARY_THUMB_WIDTH, ROTARY_THUMB_WIDTH);

        auto text = slider.getTextFromValue(slider.getValue());
        auto suffix = slider.getTextValueSuffix();
        if (suffix.isNotEmpty() && !text.contains(suffix))
            text += suffix;

        g.setColour(cachedParams.textColor);
        g.setFont(juce::FontOptions(TEXT_FONT_SIZE, juce::Font::bold));
        g.drawText(text, bounds, juce::Justification::centred, true);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (slider.getSliderStyle() == juce::Slider::LinearHorizontal)
        {
            drawLinearTrack(g, x, y, width, height, sliderPos);
        }
        else
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

protected:
    Parameters cachedParams;

    virtual void drawLinearTrack(juce::Graphics& g, int x, int y, int width, int height, float sliderPos)
    {
        auto trackWidth = (float)width;
        auto trackX = (float)x;
        auto trackY = (float)y + ((float)height - LINEAR_TRACK_HEIGHT) * 0.5f;
        auto trackRight = trackX + trackWidth;

        juce::Rectangle<float> trackBounds(trackX, trackY, trackWidth, LINEAR_TRACK_HEIGHT);

        juce::ColourGradient trackGrad(
            cachedParams.linearTrackStart, trackX, trackY,
            cachedParams.linearTrackEnd, trackRight, trackY, false
        );
        g.setGradientFill(trackGrad);
        g.fillRoundedRectangle(trackBounds, LINEAR_TRACK_CORNER_RADIUS);

        float thumbX = juce::jlimit(trackX, trackRight - LINEAR_THUMB_SIZE, sliderPos - LINEAR_THUMB_SIZE * 0.5f);
        float thumbY = trackY + (LINEAR_TRACK_HEIGHT - LINEAR_THUMB_SIZE) * 0.5f;

        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumbX, thumbY, LINEAR_THUMB_SIZE, LINEAR_THUMB_SIZE);
    }
};

class BrightBlueStyle : public BaseTheme
{
public:
    BrightBlueStyle() : BaseTheme({
        juce::Colours::white,
        juce::Colours::darkgrey,
        juce::Colours::lightgrey,
        juce::Colours::darkgrey,
        juce::Colours::whitesmoke,
        juce::Colours::lightgrey,
        juce::Colours::darkgrey,
        juce::Colours::lightgrey.darker(0.15f),
        juce::Colours::lightgrey.withAlpha(0.4f)
    }) {}

    enum BruColors
    {
        panelBackgroundId = ThemeColors::panelBackgroundId,
        panelOutlineId = ThemeColors::panelOutlineId
    };
};

class DarkRedStyle : public BaseTheme
{
public:
    DarkRedStyle() : BaseTheme({
        juce::Colour(0xff222222),
        juce::Colours::indianred,
        juce::Colours::darkgrey,
        juce::Colours::whitesmoke,
        juce::Colour(0xff181818),
        juce::Colour(0xff333333),
        juce::Colours::whitesmoke,
        juce::Colours::darkgrey.withAlpha(0.8f),
        juce::Colours::darkgrey.withAlpha(0.3f)
    }) {}
};