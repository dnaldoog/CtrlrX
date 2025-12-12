#include "CtrlrMarkdownParser.h"

// ----------------------------- Fonts ---------------------------------
juce::Font CtrlrMarkdownParser::normalFont(float h)
{
    // default system font (safe)
    return juce::Font(h);
}

juce::Font CtrlrMarkdownParser::boldFont(float h)
{
    return juce::Font(h, juce::Font::bold);
}

juce::Font CtrlrMarkdownParser::italicFont(float h)
{
    return juce::Font(h, juce::Font::italic);
}

juce::Font CtrlrMarkdownParser::getMonospaceFont(float h)
{
#if JUCE_WINDOWS
    return juce::Font("Consolas", h, juce::Font::plain);
#elif JUCE_MAC
    return juce::Font("Menlo", h, juce::Font::plain);
#else
    return juce::Font("Monospace", h, juce::Font::plain);
#endif
}

// ----------------------------- Helpers ---------------------------------
void CtrlrMarkdownParser::addHeading(juce::AttributedString& as, const juce::String& text, float size, juce::Colour colour)
{
    as.append(text + "\n", juce::Font(size, juce::Font::bold), colour);
}

void CtrlrMarkdownParser::addHorizontalRule(juce::AttributedString& as)
{
    // Use plain ASCII dashes to avoid Unicode glyph problems.
    as.append("--------------------------------------------------\n", juce::Font(14.0f), juce::Colours::lightgrey);
}

void CtrlrMarkdownParser::addCodeLine(juce::AttributedString& as, const juce::String& line)
{
    as.append(line + "\n", getMonospaceFont(14.0f), juce::Colours::darkgrey);
}

void CtrlrMarkdownParser::addListItem(juce::AttributedString& as, const juce::String& text)
{
    // bullet + styled contents
    as.append("• ", normalFont(), juce::Colours::darkred);
    appendInlineStyled(as, text);
}

bool CtrlrMarkdownParser::isHorizontalRuleLine(const juce::String& rawLine)
{
    // legacy support: accept lines that are only '-' '*' or '_' (3+)
    juce::String s = rawLine.removeCharacters(" ").trim();

    if (s.length() < 3)
        return false;

    juce_wchar first = s[0];
    if (first != '-' && first != '*' && first != '_')
        return false;

    for (int i = 1; i < s.length(); ++i)
        if (s[i] != first)
            return false;

    return true;
}

juce::String CtrlrMarkdownParser::stripInlineCode(const juce::String& s)
{
    juce::String out = s;
    bool inCode = false;
    for (int i = 0; i < out.length(); ++i)
    {
        if (out[i] == '`')
        {
            out = out.replaceSection(i, 1, "");
            --i;
            inCode = !inCode;
        }
    }
    return out;
}

// ----------------------------- Inline formatting ------------------------------
// ----------------------------- Inline formatting ------------------------------
void CtrlrMarkdownParser::appendInlineStyled(juce::AttributedString& as, const juce::String& raw)
{
    // replace <br> with actual newline
    juce::String s = raw.replace("<br>", "\n");

    enum Mode { Normal, Bold, Italic, Code };
    Mode mode = Normal;
    juce::Colour currentColour = juce::Colours::black;

    juce::String buffer;

    auto flush = [&]() {
        if (buffer.isEmpty()) return;
        switch (mode)
        {
        case Normal: as.append(buffer, normalFont(), currentColour); break;
        case Bold:   as.append(buffer, boldFont(), currentColour); break;
        case Italic: as.append(buffer, italicFont(), currentColour); break;
        case Code:   as.append(buffer, getMonospaceFont(13.0f), juce::Colours::darkorange); break;
        }
        buffer.clear();
        };

    int i = 0;
    const int L = s.length();

    while (i < L)
    {
        // Check for <span style="color:...">
        if (s[i] == '<' && s.substring(i).startsWith("<span style=\"color:"))
        {
            flush();

            // Find the color value
            int colorStart = i + 19; // length of "<span style=\"color:"
            int colorEnd = s.indexOfChar(colorStart, '"');

            if (colorEnd > colorStart)
            {
                juce::String colorStr = s.substring(colorStart, colorEnd).trim();

                // Parse color (support named colors and hex)
                if (colorStr.startsWith("#"))
                {
                    // Hex color: #RRGGBB or #RGB
                    currentColour = juce::Colour::fromString(colorStr);
                }
                else
                {
                    // Named color
                    currentColour = juce::Colours::findColourForName(colorStr, juce::Colours::black);
                }

                // Skip past the closing >
                i = s.indexOfChar(colorEnd, '>') + 1;
                continue;
            }
        }

        // Check for </span> to reset color
        if (s[i] == '<' && s.substring(i).startsWith("</span>"))
        {
            flush();
            currentColour = juce::Colours::black;
            i += 7; // length of "</span>"
            continue;
        }

        // escape backslash: \* \_ \` -> literal char
        if (s[i] == '\\' && i + 1 < L)
        {
            buffer += s.substring(i + 1, i + 2);
            i += 2;
            continue;
        }

        // inline code `...`
        if (s[i] == '`')
        {
            flush();
            mode = (mode == Code) ? Normal : Code;
            ++i;
            continue;
        }

        // bold ** ... **
        if (s[i] == '*' && i + 1 < L && s[i + 1] == '*')
        {
            flush();
            mode = (mode == Bold) ? Normal : Bold;
            i += 2;
            continue;
        }

        // italic * or _
        if (s[i] == '*' || s[i] == '_')
        {
            flush();
            mode = (mode == Italic) ? Normal : Italic;
            ++i;
            continue;
        }

        // newline from <br>
        if (s[i] == '\n')
        {
            flush();
            as.append("\n", normalFont());
            ++i;
            continue;
        }

        buffer += s[i];
        ++i;
    }

    flush();
    // ensure a newline end-of-line in layout
    as.append("\n", normalFont());
}

// ----------------------------- Block parser ----------------------------------
std::vector<CtrlrMarkdownParser::MarkdownBlock> CtrlrMarkdownParser::parseToBlocks(const juce::String& md)
{
    std::vector<MarkdownBlock> blocks;

    // Convert <br> early so inline parser can use '\n'
    juce::String text = md.replace("<br>", "\n");

    juce::StringArray lines;
    lines.addLines(text);

    bool inCodeBlock = false;
    juce::AttributedString paragraph;
    paragraph.setLineSpacing(4.0f);

    auto flushParagraph = [&]() {
        if (paragraph.getText().trim().isNotEmpty() || paragraph.getNumAttributes() > 0)
        {
            MarkdownBlock b;
            b.isHorizontalRule = false;
            b.content = paragraph;
            blocks.push_back(std::move(b));
            paragraph = juce::AttributedString();
            paragraph.setLineSpacing(4.0f);
        }
        };


    for (int i = 0; i < lines.size(); ++i)
    {
        juce::String line = lines[i].trimEnd();

        // fenced code block toggle
        if (line.startsWith("```"))
        {
            flushParagraph();
            inCodeBlock = !inCodeBlock;
            continue;
        }

        if (inCodeBlock)
        {
            // each code line becomes its own block (simpler)
            MarkdownBlock cb;
            cb.isHorizontalRule = false;
            juce::AttributedString as;
            as.setLineSpacing(4.0f);
            addCodeLine(as, line);
            cb.content = as;
            blocks.push_back(std::move(cb));
            continue;
        }

        // prefer explicit <hr> marker; fall back to legacy detection
        if (line == "<hr>" || isHorizontalRuleLine(line))
        {
            flushParagraph();
            MarkdownBlock hr;
            hr.isHorizontalRule = true;
            blocks.push_back(std::move(hr));
            continue;
        }

        // Headings (standalone blocks)
        if (line.startsWith("### "))
        {
            flushParagraph();
            MarkdownBlock hb; hb.isHorizontalRule = false;
            juce::AttributedString as; as.setLineSpacing(4.0f);
            addHeading(as, line.substring(4).trim(), 18.0f, juce::Colours::black);
            hb.content = as;
            blocks.push_back(std::move(hb));
            continue;
        }
        if (line.startsWith("## "))
        {
            flushParagraph();
            MarkdownBlock hb; hb.isHorizontalRule = false;
            juce::AttributedString as; as.setLineSpacing(4.0f);
            addHeading(as, line.substring(3).trim(), 24.0f, juce::Colours::black);
            hb.content = as;
            blocks.push_back(std::move(hb));
            continue;
        }
        if (line.startsWith("# "))
        {
            flushParagraph();
            MarkdownBlock hb; hb.isHorizontalRule = false;
            juce::AttributedString as; as.setLineSpacing(4.0f);
            addHeading(as, line.substring(2).trim(), 32.0f, juce::Colours::black);
            hb.content = as;
            blocks.push_back(std::move(hb));
            continue;
        }

        // List item - make as small block
        if (line.trimStart().startsWith("- "))
        {
            flushParagraph();
            MarkdownBlock lb; lb.isHorizontalRule = false;
            juce::AttributedString as; as.setLineSpacing(4.0f);
            addListItem(as, line.substring(line.indexOfChar('-') + 2).trim());
            lb.content = as;
            blocks.push_back(std::move(lb));
            continue;
        }

        // empty line => paragraph break
        if (line.isEmpty())
        {
            flushParagraph();
            continue;
        }

        // Append inline-styled line to paragraph.
        // To keep inline attributes intact we append into paragraph directly as a new line.
        {
            juce::AttributedString tmp; tmp.setLineSpacing(4.0f);
            appendInlineStyled(tmp, line);
            // If paragraph empty just take tmp; otherwise flush paragraph and push tmp as its own block
            if (paragraph.getNumAttributes() > 0 ||
                paragraph.getText().trim().isNotEmpty())

            {
                paragraph = tmp;
            }
            else
            {
                flushParagraph();
                MarkdownBlock tb; tb.isHorizontalRule = false; tb.content = tmp;
                blocks.push_back(std::move(tb));
            }
        }
    }

    flushParagraph();
    return blocks;
}

// ----------------------------- Convenience parse -> single AttributedString ----------------
juce::AttributedString CtrlrMarkdownParser::parse(const juce::String& md)
{
    auto blocks = parseToBlocks(md);
    juce::AttributedString out;
    out.setLineSpacing(4.0f);

    for (auto& b : blocks)
    {
        if (b.isHorizontalRule)
        {
            addHorizontalRule(out);
        }
        else
        {
            // append block content directly
            // preserve attribute runs by appending text and attributes: easiest is to append its plain representation using default font
            // but we want to keep inline style: append the content's plain string with attributes is non-trivial,
            // so we append the whole content by converting to plain string but also re-styling simple runs:
            // Simpler: append the content as-is by constructing a new AttributedString copy
            out.append(b.content);
        }
    }

    return out;
}

// ----------------------------- Plain text fallback --------------------------
juce::String CtrlrMarkdownParser::parseToPlainText(const juce::String& md)
{
    // very simple: strip markdown markers
    juce::StringArray lines; lines.addLines(md);
    juce::String out;
    bool inCodeBlock = false;
    for (auto& ln : lines)
    {
        juce::String line = ln.trimEnd();
        if (line.startsWith("```")) { inCodeBlock = !inCodeBlock; continue; }
        if (inCodeBlock) { out << line << "\n"; continue; }

        if (line.startsWith("# ")) out << line.substring(2).trim() << "\n";
        else if (line.startsWith("## ")) out << line.substring(3).trim() << "\n";
        else if (line.startsWith("### ")) out << line.substring(4).trim() << "\n";
        else if (line.trimStart().startsWith("- ")) out << "• " << line.substring(line.indexOfChar('-') + 2).trim() << "\n";
        else
        {
            juce::String t = line.replace("**", "").replace("*", "").replace("_", "");
            t = stripInlineCode(t);
            out << t << "\n";
        }
    }
    return out;
}
