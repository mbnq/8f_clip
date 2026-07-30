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
    static void show(juce::Component* parentComponent, int currentStyleIndex, std::function<void(int)> callback)
    {
        juce::PopupMenu m;
        m.addItem(1, "Reset Window Size");

        juce::PopupMenu styleMenu;
        styleMenu.addItem(10, "Bright Blue", true, currentStyleIndex == 0);
        styleMenu.addItem(11, "Dark Red", true, currentStyleIndex == 1);
        m.addSubMenu("Change Style", styleMenu);

        m.addItem(2, "About");

        static ContextMenuLookAndFeel menuLookAndFeel;
        m.setLookAndFeel(&menuLookAndFeel);
        m.showMenuAsync(juce::PopupMenu::Options(), callback);
    }
};