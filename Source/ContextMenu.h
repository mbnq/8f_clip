#pragma once
#include <JuceHeader.h>

class ContextMenu
{
public:
    static void show(juce::Component* parentComponent, std::function<void()> resetSizeCallback, std::function<void()> showAboutCallback)
    {
        juce::PopupMenu m;
        m.addItem(1, "Reset Window Size");
        m.addItem(2, "About");

        m.showMenuAsync(juce::PopupMenu::Options(), [resetSizeCallback, showAboutCallback](int choice)
            {
                if (choice == 1)
                {
                    if (resetSizeCallback)
                        resetSizeCallback();
                }
                else if (choice == 2)
                {
                    if (showAboutCallback)
                        showAboutCallback();
                }
            });
    }
};