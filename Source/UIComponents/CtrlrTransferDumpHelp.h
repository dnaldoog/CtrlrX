/*
  ==============================================================================

    CtrlrTransferDumpHelp.h
    Created: 11 Dec 2025 7:05:43am
    Author:  zan64

  ==============================================================================
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class CtrlrTransferDumpHelp : public juce::Component
{
public:
    CtrlrTransferDumpHelp();

    void resized() override;

private:
    juce::TextEditor helpText;
};
