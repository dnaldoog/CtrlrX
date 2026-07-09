/*
  ==============================================================================

    CtrlrHelpWindow.h
    Created: 11 Dec 2025 12:43:29pm
    Author:  zan64

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


// A window that shows a help component inside a viewport for scrolling
class CtrlrHelpWindow : public juce::DocumentWindow
{
public:
    CtrlrHelpWindow(const juce::String& title,
        juce::Component* helpContent)
        : juce::DocumentWindow(title,
            juce::Colours::lightgrey,
            juce::DocumentWindow::allButtons,
            true)
    {
        auto* viewport = new juce::Viewport();
        viewport->setViewedComponent(helpContent, true);
        viewport->setScrollBarsShown(true, true);

        // Windows scrollbar visibility fix (safe on Linux)
        viewport->setLookAndFeel(&helpLookAndFeel);

        setUsingNativeTitleBar(true);
        setContentOwned(viewport, true);

        setResizable(true, true);
        setResizeLimits(400, 300, 4096, 4096);

        centreWithSize(800, 600);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        delete this;
    }

private:
    class HelpLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        int getDefaultScrollbarWidth() override
        {
            return 14; // makes Windows scrollbar visible
        }
    };

    HelpLookAndFeel helpLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrHelpWindow)
};