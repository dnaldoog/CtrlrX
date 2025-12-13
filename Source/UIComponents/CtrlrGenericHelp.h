#pragma once
#include <JuceHeader.h>
#include "CtrlrMarkdownParser.h"

class CtrlrGenericHelp : public juce::Component
{
public:
    CtrlrGenericHelp(const char* mdData, int mdSize);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void parentSizeChanged() override;
private:
    juce::AttributedString attributedContent;
    juce::String plainText;
    float contentHeight = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrGenericHelp)
};
