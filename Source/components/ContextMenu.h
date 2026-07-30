#pragma once
#include <JuceHeader.h>

class ContextMenuLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ContextMenuLookAndFeel(int currentStyleIndex)
    {
        setStyle(currentStyleIndex);
    }

    void setStyle(int currentStyleIndex)
    {
        if (currentStyleIndex == 1) // dark red
        {
            setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff222222));
            setColour(juce::PopupMenu::textColourId, juce::Colours::whitesmoke);
            setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::indianred.withAlpha(0.4f));
            setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        }
        else // bright blue
        {
            setColour(juce::PopupMenu::backgroundColourId, juce::Colours::white);
            setColour(juce::PopupMenu::textColourId, juce::Colours::darkgrey);
            setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::dodgerblue.withAlpha(0.2f));
            setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::dodgerblue.darker(0.2f));
        }
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

        static ContextMenuLookAndFeel menuLookAndFeel(currentStyleIndex);
        menuLookAndFeel.setStyle(currentStyleIndex);

        m.setLookAndFeel(&menuLookAndFeel);
        m.showMenuAsync(juce::PopupMenu::Options(), callback);
    }
};