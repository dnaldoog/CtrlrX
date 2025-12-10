/*
  ==============================================================================

    CtrlrTransferDumpHelp.cpp
    Created: 11 Dec 2025 7:05:43am
    Author:  zan64

  ==============================================================================
*/
#include <JuceHeader.h>
#include "CtrlrTransferDumpHelp.h"

CtrlrTransferDumpHelp::CtrlrTransferDumpHelp()
{
    addAndMakeVisible(helpText);

    helpText.setMultiLine(true);
    helpText.setReadOnly(true);
    helpText.setScrollbarsShown(true);
    helpText.setCaretVisible(false);
    helpText.setPopupMenuEnabled(true);
    helpText.setWantsKeyboardFocus(false);

    auto mdFile = juce::File::getCurrentWorkingDirectory()
        .getChildFile("Source/Resources/Doc/BulkReadWriteDump.md");

    juce::String content = mdFile.existsAsFile()
        ? mdFile.loadFileAsString()
        : "Help file not found.";

    helpText.setText(content);
}

void CtrlrTransferDumpHelp::resized()
{
    helpText.setBounds(getLocalBounds());
}
