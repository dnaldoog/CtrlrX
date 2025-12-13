#pragma once
#include <JuceHeader.h>
#include "CtrlrMarkdownParser.h"

class CtrlrGenericHelp : public juce::Component
{
public:
    CtrlrGenericHelp(const char* mdData, int mdSize);
    ~CtrlrGenericHelp();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    
private:
    class CustomScrollbarLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        CustomScrollbarLookAndFeel()
        {
            // Set scrollbar colors
            setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xff666666));
            setColour(juce::ScrollBar::trackColourId, juce::Colour(0xffe0e0e0));
            setColour(juce::ScrollBar::backgroundColourId, juce::Colour(0xfff5f5f5));
        }
        
        int getDefaultScrollbarWidth() override { return 16; }
    };
    
    class ContentComponent : public juce::Component
    {
    public:
        ContentComponent(const std::vector<CtrlrMarkdownParser::MarkdownBlock>& blocks);
        void paint(juce::Graphics& g) override;
        void calculateLayout(float width);
        
    private:
        std::vector<CtrlrMarkdownParser::MarkdownBlock> blocks;
        float contentHeight = 0.0f;
    };
    
    std::unique_ptr<juce::Viewport> viewport;
    std::unique_ptr<ContentComponent> content;
    std::unique_ptr<juce::ResizableCornerComponent> resizer;
    juce::ComponentBoundsConstrainer resizeLimits;
    juce::String plainText;
    CustomScrollbarLookAndFeel scrollbarLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrGenericHelp)
};