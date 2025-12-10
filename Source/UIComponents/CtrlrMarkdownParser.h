#pragma once
#include <JuceHeader.h>

class CtrlrMarkdownParser
{
public:
    struct Line
    {
        juce::String text;
        int fontSize = 14;
        bool bold = false;
        bool italic = false;
    };

    std::vector<Line> parseLines(const juce::String& markdown);
};
