// CtrlrGenericHelp.cpp
#include "CtrlrGenericHelp.h"

CtrlrGenericHelp::CtrlrGenericHelp(const char* mdData, int mdSize)
{
    juce::String content = (mdData != nullptr)
        ? juce::String::fromUTF8(mdData, mdSize)
        : "Help file not found.";

    attributedContent = CtrlrMarkdownParser::parse(content);
    plainText = CtrlrMarkdownParser::parseToPlainText(content);

    // Calculate height
    juce::TextLayout layout;
    layout.createLayout(attributedContent, 776.0f);
    contentHeight = layout.getHeight() + 24.0f;

    setSize(800, juce::jmax(600, (int)contentHeight));
}

void CtrlrGenericHelp::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    juce::Rectangle<float> textBounds(
        12.0f, 12.0f,
        (float)getWidth() - 24.0f,
        contentHeight);

    attributedContent.draw(g, textBounds);
}

void CtrlrGenericHelp::resized()
{
    juce::TextLayout layout;
    layout.createLayout(attributedContent, (float)getWidth() - 24.0f);
    contentHeight = layout.getHeight() + 24.0f;
    repaint();
}

void CtrlrGenericHelp::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown() || e.mods.isCtrlDown())
    {
        // Show context menu for copying all text
        juce::PopupMenu menu;
        menu.addItem(1, "Copy All Text");

        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1)
            {
                juce::SystemClipboard::copyTextToClipboard(plainText);
            }
            });
    }
}

void CtrlrGenericHelp::mouseDrag(const juce::MouseEvent& e)
{
    // Could implement text selection here if needed
}