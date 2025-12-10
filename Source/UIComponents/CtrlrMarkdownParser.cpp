#include <JuceHeader.h>
#include "CtrlrMarkdownParser.h"

std::vector<CtrlrMarkdownParser::Line> CtrlrMarkdownParser::parseLines(const juce::String& markdown)
{
    std::vector<Line> result;
    auto lines = juce::StringArray::fromLines(markdown);

    for (auto& l : lines)
    {
        Line line;
        line.text = l.trim();

        if (line.text.startsWith("## "))      line.fontSize = 18;
        else if (line.text.startsWith("# "))  line.fontSize = 20;
        else                                  line.fontSize = 14;

        line.bold = line.text.contains("**");
        line.italic = line.text.contains("_");

        line.text = line.text.replace("**", "").replace("_", "");

        result.push_back(line);
    }
    return result;
}
