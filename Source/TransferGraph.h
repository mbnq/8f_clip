#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TransferGraphComponent : public juce::Component, public juce::Timer
{
public:
    TransferGraphComponent(_8f_clipAudioProcessor& p) : processor(p) { startTimerHz(30); }
    void timerCallback() override { repaint(); }

    void getClipPointCoords(float threshold, float w, float h, float& xRight, float& yRight, float& xLeft, float& yLeft)
    {
        float usableW = w - 2.0f * margin;
        float usableH = h - 2.0f * margin;

        xRight = margin + ((threshold + 1.0f) * 0.5f) * usableW;
        yRight = h - (margin + ((threshold + 1.0f) * 0.5f) * usableH);

        xLeft = margin + ((-threshold + 1.0f) * 0.5f) * usableW;
        yLeft = h - (margin + ((-threshold + 1.0f) * 0.5f) * usableH);
    }

    void getSoftnessPointCoords(float threshold, float softPct, float w, float h, float& xRight, float& yRight, float& xLeft, float& yLeft)
    {
        float usableW = w - 2.0f * margin;
        float usableH = h - 2.0f * margin;

        float testInput = 1.0f;

        float excessR = testInput - threshold;
        float softValOutR = threshold + (1.0f - threshold) * std::tanh(excessR / (1.0f - threshold + 0.0001f));
        if (softValOutR > 1.0f) softValOutR = 1.0f;
        float outputR = threshold + softPct * (softValOutR - threshold);

        xRight = margin + ((testInput + 1.0f) * 0.5f) * usableW;
        yRight = h - (margin + ((outputR + 1.0f) * 0.5f) * usableH);

        xLeft = margin + ((-testInput + 1.0f) * 0.5f) * usableW;
        yLeft = h - (margin + ((-outputR + 1.0f) * 0.5f) * usableH);
    }

    bool isNearAnyPoint(const juce::Point<float>& mousePos, float w, float h)
    {
        float clipVal = processor.apvts.getRawParameterValue("CLIP")->load();
        float clipPct = clipVal / 100.0f;
        float softVal = processor.apvts.getRawParameterValue("SOFTNESS")->load();
        float softPct = softVal / 100.0f;
        float threshold = juce::jmax(0.0f, 1.0f - clipPct);

        float x1R, y1R, x1L, y1L;
        getClipPointCoords(threshold, w, h, x1R, y1R, x1L, y1L);

        float x2R, y2R, x2L, y2L;
        getSoftnessPointCoords(threshold, softPct, w, h, x2R, y2R, x2L, y2L);

        float dist1R = mousePos.getDistanceFrom({ x1R, y1R });
        float dist1L = mousePos.getDistanceFrom({ x1L, y1L });
        float dist2R = mousePos.getDistanceFrom({ x2R, y2R });
        float dist2L = mousePos.getDistanceFrom({ x2L, y2L });

        return (dist1R < 15.0f || dist1L < 15.0f || dist2R < 15.0f || dist2L < 15.0f);
    }

    void mouseMove(const juce::MouseEvent& event) override
    {
        float w = (float)getWidth();
        float h = (float)getHeight();

        if (isNearAnyPoint(event.getPosition().toFloat(), w, h))
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        }
        else
        {
            setMouseCursor(juce::MouseCursor::NormalCursor);
        }
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        float w = (float)getWidth();
        float h = (float)getHeight();

        float clipVal = processor.apvts.getRawParameterValue("CLIP")->load();
        float clipPct = clipVal / 100.0f;
        float softVal = processor.apvts.getRawParameterValue("SOFTNESS")->load();
        float softPct = softVal / 100.0f;
        float threshold = juce::jmax(0.0f, 1.0f - clipPct);

        float x1R, y1R, x1L, y1L;
        getClipPointCoords(threshold, w, h, x1R, y1R, x1L, y1L);

        float x2R, y2R, x2L, y2L;
        getSoftnessPointCoords(threshold, softPct, w, h, x2R, y2R, x2L, y2L);

        auto mousePos = event.getPosition().toFloat();
        float dist1R = mousePos.getDistanceFrom({ x1R, y1R });
        float dist1L = mousePos.getDistanceFrom({ x1L, y1L });
        float dist2R = mousePos.getDistanceFrom({ x2R, y2R });
        float dist2L = mousePos.getDistanceFrom({ x2L, y2L });

        if (dist1R < 15.0f || dist1L < 15.0f)
        {
            draggedPoint = 1;
            activePointSide = (mousePos.x >= w * 0.5f) ? 1 : -1;
            setMouseCursor(juce::MouseCursor::DraggingHandCursor);
            if (auto* p = processor.apvts.getParameter("CLIP")) p->beginChangeGesture();
        }
        else if (dist2R < 15.0f || dist2L < 15.0f)
        {
            draggedPoint = 2;
            activePointSide = (mousePos.x >= w * 0.5f) ? 1 : -1;

            initialSoftnessVal = softVal;
            dragMouseDownPos = event.getPosition();
            setMouseCursor(juce::MouseCursor::DraggingHandCursor);

            if (auto* p = processor.apvts.getParameter("SOFTNESS")) p->beginChangeGesture();
        }
        else
        {
            draggedPoint = 0;
        }
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (draggedPoint == 0) return;

        float w = (float)getWidth();
        float usableW = w - 2.0f * margin;
        float centerX = w * 0.5f;
        auto mousePos = event.getPosition().toFloat();

        if (draggedPoint == 1)
        {
            if (activePointSide > 0)
                mousePos.x = juce::jlimit(centerX, w - margin, mousePos.x);
            else
                mousePos.x = juce::jlimit(margin, centerX, mousePos.x);

            float normX = std::abs(mousePos.x - centerX) / (usableW * 0.5f);
            float threshold = juce::jlimit(0.0f, 1.0f, normX);
            float clipVal = (1.0f - threshold) * 100.0f;

            if (auto* clipParam = processor.apvts.getParameter("CLIP"))
            {
                clipParam->setValueNotifyingHost(clipParam->convertTo0to1(clipVal));
            }
        }
        else if (draggedPoint == 2)
        {
            float deltaY = 0.0f;
            if (activePointSide > 0)
            {
                deltaY = (float)(dragMouseDownPos.y - event.getPosition().y);
            }
            else
            {
                deltaY = (float)(event.getPosition().y - dragMouseDownPos.y);
            }

            float newSoftVal = juce::jlimit(0.0f, 100.0f, initialSoftnessVal + deltaY * 0.25f);

            if (auto* softParam = processor.apvts.getParameter("SOFTNESS"))
            {
                softParam->setValueNotifyingHost(softParam->convertTo0to1(newSoftVal));
            }
        }
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        if (draggedPoint == 1)
        {
            if (auto* p = processor.apvts.getParameter("CLIP")) p->endChangeGesture();
        }
        else if (draggedPoint == 2)
        {
            if (auto* p = processor.apvts.getParameter("SOFTNESS")) p->endChangeGesture();
        }
        draggedPoint = 0;
        activePointSide = 0;

        if (isNearAnyPoint(event.getPosition().toFloat(), (float)getWidth(), (float)getHeight()))
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        else
            setMouseCursor(juce::MouseCursor::NormalCursor);
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        float w = (float)getWidth();
        float h = (float)getHeight();

        float clipVal = processor.apvts.getRawParameterValue("CLIP")->load();
        float clipPct = clipVal / 100.0f;
        float softVal = processor.apvts.getRawParameterValue("SOFTNESS")->load();
        float softPct = softVal / 100.0f;
        float threshold = juce::jmax(0.0f, 1.0f - clipPct);

        float x1R, y1R, x1L, y1L;
        getClipPointCoords(threshold, w, h, x1R, y1R, x1L, y1L);

        float x2R, y2R, x2L, y2L;
        getSoftnessPointCoords(threshold, softPct, w, h, x2R, y2R, x2L, y2L);

        auto mousePos = event.getPosition().toFloat();
        float dist1R = mousePos.getDistanceFrom({ x1R, y1R });
        float dist1L = mousePos.getDistanceFrom({ x1L, y1L });
        float dist2R = mousePos.getDistanceFrom({ x2R, y2R });
        float dist2L = mousePos.getDistanceFrom({ x2L, y2L });

        if (dist1R < 15.0f || dist1L < 15.0f)
        {
            if (auto* p = processor.apvts.getParameter("CLIP"))
            {
                p->beginChangeGesture();
                p->setValueNotifyingHost(p->convertTo0to1(0.0f));
                p->endChangeGesture();
            }
        }
        else if (dist2R < 15.0f || dist2L < 15.0f)
        {
            if (auto* p = processor.apvts.getParameter("SOFTNESS"))
            {
                p->beginChangeGesture();
                p->setValueNotifyingHost(p->convertTo0to1(0.0f));
                p->endChangeGesture();
            }
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::whitesmoke);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
        g.setColour(juce::Colours::lightgrey);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);

        float w = (float)getWidth();
        float h = (float)getHeight();
        float usableW = w - 2.0f * margin;
        float usableH = h - 2.0f * margin;

        g.setColour(juce::Colours::whitesmoke.darker(0.1f));
        g.drawVerticalLine(int(w / 2), margin, h - margin);
        g.drawHorizontalLine(int(h / 2), margin, w - margin);

        float clipVal = processor.apvts.getRawParameterValue("CLIP")->load();
        float clipPct = clipVal / 100.0f;
        float softVal = processor.apvts.getRawParameterValue("SOFTNESS")->load();
        float softPct = softVal / 100.0f;
        float threshold = juce::jmax(0.0f, 1.0f - clipPct);

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
                float softValOut = sign * (threshold + (1.0f - threshold) * std::tanh(excess / (1.0f - threshold + 0.0001f)));
                if (softValOut > 1.0f) softValOut = 1.0f;
                output = hardVal + softPct * (softValOut - hardVal);
            }

            float screenX = margin + (normX * usableW);
            float screenY = h - (margin + ((output + 1.0f) * 0.5f * usableH));

            if (i == 0) p.startNewSubPath(screenX, screenY);
            else p.lineTo(screenX, screenY);
        }

        g.setColour(juce::Colours::dodgerblue);
        g.strokePath(p, juce::PathStrokeType(2.5f));

        // --- PUNKTY 1 (CLIP) ---
        float x1R, y1R, x1L, y1L;
        getClipPointCoords(threshold, w, h, x1R, y1R, x1L, y1L);

        g.setColour(juce::Colours::dodgerblue);
        g.fillEllipse(x1R - 6.0f, y1R - 6.0f, 12.0f, 12.0f);
        g.setColour(juce::Colours::white);
        g.fillEllipse(x1R - 3.0f, y1R - 3.0f, 6.0f, 6.0f);

        g.setColour(juce::Colours::dodgerblue);
        g.fillEllipse(x1L - 6.0f, y1L - 6.0f, 12.0f, 12.0f);
        g.setColour(juce::Colours::white);
        g.fillEllipse(x1L - 3.0f, y1L - 3.0f, 6.0f, 6.0f);

        // --- PUNKTY 2 (SOFTNESS) ---
        float x2R, y2R, x2L, y2L;
        getSoftnessPointCoords(threshold, softPct, w, h, x2R, y2R, x2L, y2L);

        g.setColour(juce::Colours::limegreen);
        g.fillEllipse(x2R - 6.0f, y2R - 6.0f, 12.0f, 12.0f);
        g.setColour(juce::Colours::white);
        g.fillEllipse(x2R - 3.0f, y2R - 3.0f, 6.0f, 6.0f);

        g.setColour(juce::Colours::limegreen);
        g.fillEllipse(x2L - 6.0f, y2L - 6.0f, 12.0f, 12.0f);
        g.setColour(juce::Colours::white);
        g.fillEllipse(x2L - 3.0f, y2L - 3.0f, 6.0f, 6.0f);

        g.setColour(juce::Colours::darkgrey);
        g.setFont(12.0f);
        g.drawText("CLIPPING FUNCTION", 8, 4, 150, 20, juce::Justification::centredLeft);
    }

private:
    _8f_clipAudioProcessor& processor;
    int draggedPoint = 0;
    int activePointSide = 0;
    float initialSoftnessVal = 0.0f;
    juce::Point<int> dragMouseDownPos;

    static constexpr float margin = 6.0f; // <--- Zmieñ tê wartoœæ, aby dopasowaæ margines (np. 6.0f lub 8.0f)
};