#pragma once
#include <JuceHeader.h>

class ContextMenuLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ContextMenuLookAndFeel()
    {
        setColour(juce::PopupMenu::backgroundColourId, juce::Colours::white);
        setColour(juce::PopupMenu::textColourId, juce::Colours::darkgrey);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::dodgerblue.withAlpha(0.2f));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::dodgerblue.darker(0.2f));
    }
};

class ContextMenu
{
public:
    static void show(juce::Component* parentComponent, std::function<void(int)> callback)
    {
        juce::PopupMenu m;
        m.addItem(1, "Reset Window Size");
        m.addItem(2, "About");

        static ContextMenuLookAndFeel menuLookAndFeel;
        m.setLookAndFeel(&menuLookAndFeel);

        // Wywo³ujemy menu bez dodatkowych modyfikatorów Options – JUCE sam poprawnie zmapuje pozycjê myszy
        m.showMenuAsync(juce::PopupMenu::Options(), callback);
    }
};