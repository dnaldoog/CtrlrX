#pragma once
#include <JuceHeader.h>
#include <vector>

class CtrlrMarkdownParser
{
public:
    struct MarkdownBlock
    {
        bool isHorizontalRule = false;
        juce::AttributedString content; // used when isHorizontalRule == false
    };

    // high-level APIs
    static juce::AttributedString parse(const juce::String& md);            // convenient single AttributedString
    static std::vector<MarkdownBlock> parseToBlocks(const juce::String& md); // block-level parsing
    static juce::String parseToPlainText(const juce::String& md);          // fallback plain text

private:
    // fonts (platform-safe monospace)
    static juce::Font getMonospaceFont(float h = 14.0f);
    static juce::Font normalFont(float h = 16.0f);
    static juce::Font boldFont(float h = 16.0f);
    static juce::Font italicFont(float h = 16.0f);

    // helpers used by both parse and parseToBlocks
    static void addHeading(juce::AttributedString& as, const juce::String& text, float size, juce::Colour colour);
    static void addHorizontalRule(juce::AttributedString& as);   // appends an ASCII HR line
    static void addCodeLine(juce::AttributedString& as, const juce::String& line);
    static void addListItem(juce::AttributedString& as, const juce::String& text);
    static void appendInlineStyled(juce::AttributedString& as, const juce::String& line);

    static bool isHorizontalRuleLine(const juce::String& rawLine); // kept but <hr> preferred
    static juce::String stripInlineCode(const juce::String& s);
};
