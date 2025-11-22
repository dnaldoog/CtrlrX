#pragma once

#include "JuceHeader.h"

class CtrlrDialogWindow
{
public:

    // -------------------------
    // Modal dialogs (all platforms)
    // -------------------------
    static int showModalDialog(const juce::String &title,
                               juce::Component *content,
                               const bool resizable = false,
                               juce::Component *parent = nullptr);

#if JUCE_LINUX
    // -------------------------
    // Non-modal dialogs (Linux only)
    // Opens completely offscreen first to prevent flicker
    // Caller owns the returned pointer
    // -------------------------
    static juce::DialogWindow* showNonModalDialog(const juce::String& title,
                                                  juce::Component* content,
                                                  const juce::Colour& backgroundColour = juce::Colours::lightgrey,
                                                  bool escapeKeyCloses = true,
                                                  bool resizable = false);
#endif

private:
    JUCE_DECLARE_NON_COPYABLE(CtrlrDialogWindow)
};
