#pragma once
#include <JuceHeader.h>
#include "CtrlrMarkdownParser.h"

class CtrlrGenericHelp : public juce::Component
{
public:
    CtrlrGenericHelp(const char* mdData, int mdSize);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void parentSizeChanged() override;
    void setMarkdownContent(const juce::String& markdown);
    int getRequiredHeight() const;

private:
	// One drawable piece of the document: a run of text, a horizontal rule, or a table
	struct Segment {
			enum class Type { Text, Rule, Table };

			Type type = Type::Text;
			juce::AttributedString text;
			juce::TextLayout layout;
			float textOffsetY = 0.0f; // Text: shifts the layout so its top line sits at the segment top
			std::vector<std::vector<juce::AttributedString>> cells; // Table (row 0 = header)
			juce::Array<float> colWidths, rowHeights;				// Table, set by layoutTable()
			float height = 0.0f;
	};

	void buildSegments(const juce::String &markdown);
	void layoutSegments(float availableWidth);
	static void layoutTable(Segment &seg, float availableWidth);
	static void drawTable(juce::Graphics &g, const Segment &seg, float x, float y);

	std::vector<Segment> segments;
	juce::String plainText;
    float contentHeight = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrGenericHelp)
};