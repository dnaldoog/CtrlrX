#pragma once
#include <JuceHeader.h>
#include "CtrlrMarkdownParser.h"

#pragma once
#include <JuceHeader.h>
#include "CtrlrMarkdownParser.h"

class CtrlrGenericHelp : public juce::Component
{
public:
    CtrlrGenericHelp(const char* mdData, int mdSize);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    float contentHeight = 0.0f;
    juce::AttributedString attributedContent;
};

