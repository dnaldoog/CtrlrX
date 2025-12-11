/*
  ==============================================================================

    CtrlrGenericHelp.cpp
    Created: 11 Dec 2025 1:04:17pm
    Author:  zan64

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "CtrlrGenericHelp.h"

CtrlrGenericHelp::CtrlrGenericHelp(const char* mdData, int mdSize)
{
    juce::String content = (mdData != nullptr)
        ? juce::String::fromUTF8(mdData, mdSize)
        : "Help file not found.";

    attributedContent = CtrlrMarkdownParser::parse(content);
}

void CtrlrGenericHelp::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    attributedContent.draw(g, juce::Rectangle<float>(
        12.0f, 12.0f,
        (float)getWidth() - 24.0f, 10000.0f));
}

void CtrlrGenericHelp::resized()
{
    repaint();
}

