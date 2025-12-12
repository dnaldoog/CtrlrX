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
    CtrlrHelpWindow(const juce::String& title, juce::Component* helpContent)
        : DocumentWindow(title,
            juce::Colours::lightgrey,
            juce::DocumentWindow::allButtons,
            true)  // Use native title bar
    {
        viewport = std::make_unique<juce::Viewport>();
        viewport->setViewedComponent(helpContent, false);
        viewport->setScrollBarsShown(true, true);
        setContentOwned(viewport.get(), true);

        setUsingNativeTitleBar(true);

        centreWithSize(800, 600);
        setResizable(true, true);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        delete this;
    }

private:
    std::unique_ptr<juce::Viewport> viewport;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrHelpWindow)
};