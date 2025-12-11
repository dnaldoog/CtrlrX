#include "CtrlrTransferDumpHelp.h"
#include "CtrlrMarkdownParser.h"
#include "BinaryData.h"

CtrlrTransferDumpHelp::CtrlrTransferDumpHelp()
{
    // Load embedded markdown content
    juce::String content;
    if (BinaryData::BulkReadWriteDump_md != nullptr)
        content = juce::String::fromUTF8(BinaryData::BulkReadWriteDump_md,
            BinaryData::BulkReadWriteDump_mdSize);
    else
        content = "Help file not found.";

    // Parse markdown to AttributedString
    attributedContent = CtrlrMarkdownParser::parse(content);

    // Initial layout
    layout.createLayout(attributedContent, 800.0f);

    // Set initial size to match content
    setSize(800, (int)layout.getHeight());
}

void CtrlrTransferDumpHelp::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    auto area = getLocalBounds().reduced(12).toFloat(); // 12 px padding
    layout.draw(g, area);
}

void CtrlrTransferDumpHelp::resized()
{
    layout.createLayout(attributedContent, (float)getWidth() - 24); // account for left+right padding
    setSize(getWidth(), (int)layout.getHeight());
}

