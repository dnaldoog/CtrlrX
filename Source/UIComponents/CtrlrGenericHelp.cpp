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

    // Calculate the required height for the content
    // Use a reasonable width (e.g., 776 = 800 - 24 for margins)
    juce::TextLayout layout;
    layout.createLayout(attributedContent, 776.0f);
    contentHeight = layout.getHeight() + 24.0f; // Add padding

    // Set a reasonable minimum size
    setSize(800, juce::jmax(600, (int)contentHeight));
}

void CtrlrGenericHelp::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    // Draw with proper bounds
    juce::Rectangle<float> textBounds(
        12.0f, 12.0f,
        (float)getWidth() - 24.0f,
        contentHeight);

    attributedContent.draw(g, textBounds);
}

void CtrlrGenericHelp::resized()
{
    // Recalculate layout when resized
    juce::TextLayout layout;
    layout.createLayout(attributedContent, (float)getWidth() - 24.0f);
    contentHeight = layout.getHeight() + 24.0f;

    repaint();
}