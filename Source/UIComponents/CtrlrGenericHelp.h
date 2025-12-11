/*
  ==============================================================================

    CtrlrGenericHelp.h
    Created: 11 Dec 2025 1:04:17pm
    Author:  zan64

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h> // must have juce_gui_basics in project
#include "CtrlrMarkdownParser.h"

class CtrlrGenericHelp : public juce::Component
{
public:
    CtrlrGenericHelp(const char* mdData, int mdSize)
    {
        // Load content from BinaryData
        juce::String content = (mdData != nullptr)
            ? juce::String::fromUTF8(mdData, mdSize)
            : "Help file not found.";

        attributedContent = CtrlrMarkdownParser::parse(content);
        layout.createLayout(attributedContent, 800.0f);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::white);
        auto area = getLocalBounds().toFloat().reduced(12); // add padding
        layout.draw(g, area);
    }

    void resized() override
    {
        layout.createLayout(attributedContent, (float)getWidth());
        setSize(getWidth(), (int)layout.getHeight());
    }

private:
    juce::AttributedString attributedContent;
    juce::AttributedStringLayout layout;
};
