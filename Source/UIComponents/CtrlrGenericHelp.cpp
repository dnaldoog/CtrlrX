#include "CtrlrGenericHelp.h"

// -------- ContentComponent Implementation --------
CtrlrGenericHelp::ContentComponent::ContentComponent(const std::vector<CtrlrMarkdownParser::MarkdownBlock>& b)
    : blocks(b)
{
    calculateLayout(776.0f);
}

void CtrlrGenericHelp::ContentComponent::calculateLayout(float width)
{
    const float textWidth = width - 24.0f;
    const float blockSpacing = 8.0f;  // Extra space between blocks/paragraphs
    float yPos = 12.0f;

    for (const auto& block : blocks)
    {
        if (block.isHorizontalRule)
        {
            yPos += 20.0f;
        }
        else
        {
            juce::TextLayout layout;
            layout.createLayout(block.content, textWidth);
            yPos += layout.getHeight() + blockSpacing;  // Add extra spacing
        }
    }

    contentHeight = yPos + 12.0f;
    setSize((int)width, (int)contentHeight);
}

void CtrlrGenericHelp::ContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    const float leftMargin = 12.0f;
    const float rightMargin = 12.0f;
    const float textWidth = (float)getWidth() - leftMargin - rightMargin;
    const float blockSpacing = 8.0f;  // Extra space between blocks/paragraphs
    float yPos = 12.0f;

    for (const auto& block : blocks)
    {
        if (block.isHorizontalRule)
        {
            // Draw actual horizontal line (90% width, left aligned)
            float lineWidth = textWidth * 0.9f;
            float lineY = yPos + 10.0f;

            g.setColour(juce::Colours::lightgrey);
            g.drawHorizontalLine((int)lineY, leftMargin, leftMargin + lineWidth);

            yPos += 20.0f;
        }
        else
        {
            juce::TextLayout layout;
            layout.createLayout(block.content, textWidth);

            // Draw the layout
            layout.draw(g, juce::Rectangle<float>(leftMargin, yPos, textWidth, layout.getHeight()));

            yPos += layout.getHeight() + blockSpacing;  // Add extra spacing
        }
    }
}

// -------- CtrlrGenericHelp Implementation --------
CtrlrGenericHelp::CtrlrGenericHelp(const char* mdData, int mdSize)
{
    juce::String mdContent = (mdData != nullptr)
        ? juce::String::fromUTF8(mdData, mdSize)
        : "Help file not found.";

    auto blocks = CtrlrMarkdownParser::parseToBlocks(mdContent);
    plainText = CtrlrMarkdownParser::parseToPlainText(mdContent);

    // Set LookAndFeel on this component (cascades to children including scrollbars)
    setLookAndFeel(&scrollbarLookAndFeel);

    // Create content component
    content = std::make_unique<ContentComponent>(blocks);

    // Create viewport
    viewport = std::make_unique<juce::Viewport>();
    viewport->setViewedComponent(content.get(), false);
    viewport->setScrollBarsShown(true, false);
    addAndMakeVisible(viewport.get());

    // Create resize corner
    resizeLimits.setMinimumSize(400, 300);
    resizeLimits.setMaximumSize(2000, 2000);
    resizer = std::make_unique<juce::ResizableCornerComponent>(this, &resizeLimits);
    addAndMakeVisible(resizer.get());

    setSize(800, 600);
}

CtrlrGenericHelp::~CtrlrGenericHelp()
{
    setLookAndFeel(nullptr);
}

void CtrlrGenericHelp::paint(juce::Graphics& g)
{
    // Background is handled by viewport and content
}

void CtrlrGenericHelp::resized()
{
    auto bounds = getLocalBounds();

    // Position viewport
    viewport->setBounds(bounds);

    // Recalculate content layout for new width
    if (content)
        content->calculateLayout((float)viewport->getMaximumVisibleWidth());

    // Position resize corner
    if (resizer)
        resizer->setBounds(getWidth() - 16, getHeight() - 16, 16, 16);
}

void CtrlrGenericHelp::mouseDown(const juce::MouseEvent& e)
{
    // Right-click or Ctrl+Click to copy all text
    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        menu.addItem(1, "Copy All Text", true);
        menu.addItem(2, "Select All (Ctrl+A)", false);
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1)
            {
                juce::SystemClipboard::copyTextToClipboard(plainText);
            }
            });
    }
}