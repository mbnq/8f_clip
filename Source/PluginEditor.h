#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "components/ContextMenu.h"
#include "components/WaveformDisplay.h"
#include "components/TransferGraph.h"
#include "components/OutputMeter.h"
#include "components/Themes.h"
#include "components/LogoInfo.h"

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

    juce::DropShadowEffect gainShadow, clipShadow, softnessShadow, osShadow;

    TransferGraphComponent transferGraph;
    WaveformDisplayComponent waveformDisplay;
    OutputMeterComponent outputMeter;

    LogoComponent logoComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(_8f_clipAudioProcessorEditor)
};