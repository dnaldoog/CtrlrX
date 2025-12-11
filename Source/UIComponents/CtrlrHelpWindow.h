/*
  ==============================================================================

    CtrlrHelpWindow.h
    Created: 11 Dec 2025 12:43:29pm
    Author:  zan64

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "CtrlrTransferDumpHelp.h"

// A window that shows a help component inside a viewport for scrolling
class CtrlrHelpWindow : public juce::DocumentWindow
{
public:
    CtrlrHelpWindow(const juce::String& title, juce::Component* helpContent)
        : DocumentWindow(title,
            juce::Colours::lightgrey,
            juce::DocumentWindow::allButtons)
    {
        viewport = std::make_unique<juce::Viewport>();
        viewport->setViewedComponent(helpContent, false); // false: don't resize height
        viewport->setScrollBarsShown(true, true);         // show vertical & horizontal if needed

        setContentOwned(viewport.get(), true);

        centreWithSize(800, 600);
        setResizable(true, true);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        delete this; // self-delete when user closes window
    }

private:
    std::unique_ptr<juce::Viewport> viewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrHelpWindow)
};
