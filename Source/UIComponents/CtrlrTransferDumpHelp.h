#pragma once
#include <JuceHeader.h>
#include "CtrlrMarkdownParser.h"

// A help component that displays markdown content in a scrollable viewport
class CtrlrTransferDumpHelp : public juce::Component
{
public:
    CtrlrTransferDumpHelp();
    ~CtrlrTransferDumpHelp() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AttributedString attributedContent;
    juce::TextLayout layout;   // JUCE class for layouting text
};
