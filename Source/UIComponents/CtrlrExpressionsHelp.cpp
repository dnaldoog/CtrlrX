/*
  ==============================================================================

    CtrlrExpressionsHelp.cpp
    Created: 11 Dec 2025 7:05:58am
    Author:  zan64

  ==============================================================================
*/
#include <JuceHeader.h>
#include "CtrlrExpressionsHelp.h"

CtrlrExpressionsHelp::CtrlrExpressionsHelp()
{
    addAndMakeVisible(helpText);

    helpText.setMultiLine(true);
    helpText.setReadOnly(true);
    helpText.setScrollbarsShown(true);
    helpText.setCaretVisible(false);
    helpText.setPopupMenuEnabled(true);
    helpText.setWantsKeyboardFocus(false);

    auto mdFile = juce::File::getCurrentWorkingDirectory()
        .getChildFile("Source/Resources/Doc/Expressions.md");

    juce::String content = mdFile.existsAsFile()
        ? mdFile.loadFileAsString()
        : "Help file not found.";

    helpText.setText(content);
}

void CtrlrExpressionsHelp::resized()
{
    helpText.setBounds(getLocalBounds());
}


