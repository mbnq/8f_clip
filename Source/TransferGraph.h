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
        xRight = ((threshold + 1.0f) * 0.5f) * w;
        yRight = h - ((threshold + 1.0f) * 0.5f * h);

        xLeft = ((-threshold + 1.0f) * 0.5f) * w;
        yLeft = h - ((-threshold + 1.0f) * 0.5f * h);
    }

    void getSoftnessPointCoords(float threshold, float softPct, float w, float h, float& xRight, float& yRight, float& xLeft, float& yLeft)
    {
        float testInput = juce::jlimit(threshold + 0.02f, 0.98f, threshold + (1.0f - threshold) * 0.5f);
        if (threshold > 0.85f)
            testInput = juce::jlimit(threshold + 0.005f, 0.995f, 0.995f);

        float excessR = testInput - threshold;
        float softValOutR = threshold + (1.0f - threshold) * std::tanh(excessR / (1.0f - threshold + 0.0001f));
        if (softValOutR > 1.0f) softValOutR = 1.0f;
        float outputR = threshold + softPct * (softValOutR - threshold);

        xRight = ((testInput + 1.0f) * 0.5f) * w;
        yRight = h - ((outputR + 1.0f) * 0.5f * h);

        xLeft = ((-testInput + 1.0f) * 0.5f) * w;
        yLeft = h - ((-outputR + 1.0f) * 0.5f * h);
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
            draggedPoint = 1; // Niebieskie punkty (CLIP)
            activePointSide = (mousePos.x >= w * 0.5f) ? 1 : -1;
            if (auto* p = processor.apvts.getParameter("CLIP")) p->beginChangeGesture();
        }
        else if (dist2R < 15.0f || dist2L < 15.0f)
        {
            draggedPoint = 2; // Zielone punkty (SOFTNESS)
            activePointSide = (mousePos.x >= w * 0.5f) ? 1 : -1;

            initialSoftnessVal = softVal;
            dragMouseDownPos = event.getPosition();

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
        float h = (float)getHeight();
        auto mousePos = event.getPosition().toFloat();
        float centerX = w * 0.5f;

        if (draggedPoint == 1)
        {
            if (activePointSide > 0)
                mousePos.x = juce::jlimit(centerX, w, mousePos.x);
            else
                mousePos.x = juce::jlimit(0.0f, centerX, mousePos.x);

            float normX = std::abs(mousePos.x - centerX) / centerX;
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
                // Prawy punkt: ruch w górê zwiêksza wartoœæ
                deltaY = (float)(dragMouseDownPos.y - event.getPosition().y);
            }
            else
            {
                // Lewy punkt: ruch w dó³ zwiêksza wartoœæ
                deltaY = (float)(event.getPosition().y - dragMouseDownPos.y);
            }

            float newSoftVal = juce::jlimit(0.0f, 100.0f, initialSoftnessVal + deltaY * 0.25f);

            if (auto* softParam = processor.apvts.getParameter("SOFTNESS"))
            {
                softParam->setValueNotifyingHost(softParam->convertTo0to1(newSoftVal));
            }
        }
    }

    void mouseUp(const juce::MouseEvent&) override
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
    }

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
    int draggedPoint = 0;          // 0 = brak, 1 = CLIP, 2 = SOFTNESS
    int activePointSide = 0;       // 1 = prawa strona, -1 = lewa strona
    float initialSoftnessVal = 0.0f; // Pocz¹tkowa wartoœæ softness przy klikniêciu
    juce::Point<int> dragMouseDownPos; // Pozycja myszy w momencie klikniêcia
};