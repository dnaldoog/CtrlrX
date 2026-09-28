#include "CtrlrMarkdownParser.h"
namespace
{
constexpr float LINE_SPACING = 4.0f; // Increase from 4.0f to 8.0f for more space

// juce::Colour::fromString() expects 8 hex digits (ARGB), so a CSS-style "#rrggbb" would come out
// fully transparent. Accepts #rgb, #rrggbb, #aarrggbb (JUCE order) or a colour name.
juce::Colour parseCssColour(const juce::String& value)
{
	const juce::String v = value.trim();

	if (v.startsWith("#"))
	{
		juce::String hex = v.substring(1);

		if (hex.length() == 3)
		{
			juce::String expanded;
			for (int k = 0; k < 3; ++k)
				expanded << juce::String::charToString(hex[k]) << juce::String::charToString(hex[k]);
			hex = expanded;
		}

		if (hex.containsOnly("0123456789abcdefABCDEF"))
		{
			if (hex.length() == 6)
				return juce::Colour(0xff000000u | (juce::uint32) hex.getHexValue32());
			if (hex.length() == 8)
				return juce::Colour((juce::uint32) hex.getHexValue32());
		}

		return juce::Colours::black;
	}

	return juce::Colours::findColourForName(v, juce::Colours::black);
}
} // namespace

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
    // Explicitly create UTF-8 bullet character
    juce::String bullet = juce::CharPointer_UTF8("\xe2\x80\xa2 "); // • in UTF-8
    as.append(bullet, normalFont(), juce::Colours::darkred);
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
void CtrlrMarkdownParser::appendInlineStyled(juce::AttributedString& as, const juce::String& raw)
{
	// Convert <br> tags (any case) directly into real line breaks
	juce::String s = raw.replace("<br>", "\n", true)
						 .replace("<br/>", "\n", true)
						 .replace("<br />", "\n", true)
						 .replace("&gt;", ">")
						 .replace("&lt;", "<")
						 .replace("&amp;", "&")
						 .replace("&quot;", "\"")
						 .replace("&#39;", "'")
						 .replace("&#215;", "\\*")
						 .replace("&times;", "\\*");

	// Independent flags instead of a single mode, so ***bold italic*** works
	bool bold = false;
	bool italic = false;
	bool code = false;
	juce::Colour currentColour = juce::Colours::black;

	juce::String buffer;

	auto flush = [&]() {
		if (buffer.isEmpty())
			return;

		if (code)
			as.append(buffer, getMonospaceFont(16.0f), juce::Colours::purple);
		else if (bold && italic)
			as.append(buffer, boldFont().withStyle(juce::Font::bold | juce::Font::italic), currentColour);
		else if (bold)
			as.append(buffer, boldFont(), currentColour);
		else if (italic)
			as.append(buffer, italicFont(), currentColour);
		else
			as.append(buffer, normalFont(), currentColour);

		buffer.clear();
	};

	int i = 0;
	const int L = s.length();

	while (i < L)
	{
		// Inside `code`, everything is literal until the closing backtick
		if (code && s[i] != '`')
		{
			buffer += s[i];
			++i;
			continue;
		}

		// Check for <span style="color:...">
		if (s[i] == '<' && s.substring(i).startsWith("<span style=\"color:"))
		{
			const int colorStart = i + 19;
			const int colorEnd = s.indexOfChar(colorStart, '"');
			const int tagEnd = colorEnd > colorStart ? s.indexOfChar(colorEnd, '>') : -1;

			if (tagEnd > colorEnd)
			{
				flush();
				const juce::String colorStr =
					s.substring(colorStart, colorEnd).upToFirstOccurrenceOf(";", false, false).trim();
				currentColour = parseCssColour(colorStr);
				i = tagEnd + 1;
				continue;
			}
			// Malformed tag: fall through and treat '<' as ordinary text
		}

		// Check for </span>
		if (s[i] == '<' && s.substring(i).startsWith("</span>"))
		{
			flush();
			currentColour = juce::Colours::black;
			i += 7;
			continue;
		}

		// Escape backslash
		if (s[i] == '\\' && i + 1 < L)
		{
			buffer += s.substring(i + 1, i + 2);
			i += 2;
			continue;
		}

		// Code `...`
		if (s[i] == '`')
		{
			flush();
			code = !code;
			++i;
			continue;
		}

		// Bold **...**
		if (s[i] == '*' && i + 1 < L && s[i + 1] == '*')
		{
			flush();
			bold = !bold;
			i += 2;
			continue;
		}

		// An underscore between two word characters (snake_case, FILE_NAMES) is literal
		if (s[i] == '_')
		{
			const bool prevIsWord = i > 0 && juce::CharacterFunctions::isLetterOrDigit(s[i - 1]);
			const bool nextIsWord = i + 1 < L && juce::CharacterFunctions::isLetterOrDigit(s[i + 1]);

			if (prevIsWord && nextIsWord)
			{
				buffer += s[i];
				++i;
				continue;
			}
		}

		// Italic * or _
		if (s[i] == '*' || s[i] == '_')
		{
			flush();
			italic = !italic;
			++i;
			continue;
		}

		// Process actual newline characters (including converted <br>)
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
	// REMOVED automatic trailing newline here
}

// ----------------------------- Block parser ----------------------------------
std::vector<CtrlrMarkdownParser::MarkdownBlock> CtrlrMarkdownParser::parseToBlocks(const juce::String &md) {
	std::vector<MarkdownBlock> blocks;

	// Split strictly by raw markdown lines without preemptively replacing <br>
	juce::StringArray lines;
	lines.addLines(md);

	bool inCodeBlock = false;
	juce::AttributedString paragraph;

	auto flushParagraph = [&]() {
		if (paragraph.getText().isNotEmpty()) {
			MarkdownBlock b;
			b.isHorizontalRule = false;
			b.content = paragraph;
			blocks.push_back(std::move(b));
			paragraph = juce::AttributedString();
		}
	};

	// Headings drop inline-code backticks so `name` in a heading doesn't show the backticks literally
	auto addHeadingBlock = [&](const juce::String &text, float size) {
		flushParagraph();
		MarkdownBlock hb;
		hb.isHorizontalRule = false;
		juce::AttributedString as;
		addHeading(as, stripInlineCode(text.trim()), size, juce::Colours::black);
		hb.content = as;
		blocks.push_back(std::move(hb));
	};

	for (int i = 0; i < lines.size(); ++i) {
		juce::String line = lines[i].trimEnd();

		// Fences may be indented (e.g. inside a list)
		if (line.trimStart().startsWith("```")) {
			flushParagraph();
			inCodeBlock = !inCodeBlock;
			continue;
		}

		if (inCodeBlock) {
			MarkdownBlock cb;
			cb.isHorizontalRule = false;
			juce::AttributedString as;
			addCodeLine(as, line);
			cb.content = as;
			blocks.push_back(std::move(cb));
			continue;
		}

		if (line.equalsIgnoreCase("<hr>") || line.equalsIgnoreCase("<hr/>") || line.equalsIgnoreCase("<hr />") ||
			isHorizontalRuleLine(line)) {
			flushParagraph();
			MarkdownBlock hr;
			hr.isHorizontalRule = true;
			blocks.push_back(std::move(hr));
			continue;
		}

		// Headings
		if (line.startsWith("#### ")) {
			addHeadingBlock(line.substring(5), 16.0f);
			continue;
		}
		if (line.startsWith("### ")) {
			addHeadingBlock(line.substring(4), 18.0f);
			continue;
		}
		if (line.startsWith("## ")) {
			addHeadingBlock(line.substring(3), 24.0f);
			continue;
		}
		if (line.startsWith("# ")) {
			addHeadingBlock(line.substring(2), 32.0f);
			continue;
		}

		// List item: "- ", "* " or "+ ", optionally indented for nesting
		const juce::String trimmedLine = line.trimStart();
		if (trimmedLine.startsWith("- ") || trimmedLine.startsWith("* ") || trimmedLine.startsWith("+ ")) {
			flushParagraph();
			MarkdownBlock lb;
			lb.isHorizontalRule = false;
			juce::AttributedString as;

			const int level = (line.length() - trimmedLine.length()) / 2;
			if (level > 0)
				as.append(juce::String::repeatedString("    ", level), normalFont(), juce::Colours::black);

			addListItem(as, trimmedLine.substring(2).trim());
			as.append("\n", normalFont());
			lb.content = as;
			blocks.push_back(std::move(lb));
			continue;
		}

		// Empty line => flush paragraph
		if (line.isEmpty()) {
			flushParagraph();
			continue;
		}

		// Append line content to the active paragraph block
		if (paragraph.getText().isNotEmpty())
			paragraph.append("\n", normalFont()); // Space between text lines within a paragraph

		appendInlineStyled(paragraph, line);
	}

	flushParagraph();
	return blocks;
}

// ----------------------------- Convenience parse -> single AttributedString ----------------
juce::AttributedString CtrlrMarkdownParser::parse(const juce::String& md)
{
    auto blocks = parseToBlocks(md);
    juce::AttributedString out;
    out.setLineSpacing(LINE_SPACING);

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
	juce::String s = md.replace("<br>", "\n", true)
						 .replace("<br/>", "\n", true)
						 .replace("<br />", "\n", true)
						 .replace("&gt;", ">")
						 .replace("&lt;", "<")
						 .replace("&amp;", "&")
						 .replace("&quot;", "\"")
						 .replace("&#39;", "'")
						 .replace("&#215;", juce::String::charToString(0x00d7))
						 .replace("&times;", juce::String::charToString(0x00d7));

	// Use the entity-decoded text (the original decoded into 's' but then split 'md')
	juce::StringArray lines; lines.addLines(s);
    juce::String out;
    bool inCodeBlock = false;
	const juce::String bullet = juce::CharPointer_UTF8("\xe2\x80\xa2 "); // • in UTF-8
    for (auto& ln : lines)
    {
        juce::String line = ln.trimEnd();
        if (line.trimStart().startsWith("```")) { inCodeBlock = !inCodeBlock; continue; }
        if (inCodeBlock) { out << line << "\n"; continue; }

		const juce::String trimmedLine = line.trimStart();

        if (line.startsWith("# ")) out << line.substring(2).trim() << "\n";
        else if (line.startsWith("## ")) out << line.substring(3).trim() << "\n";
        else if (line.startsWith("### ")) out << line.substring(4).trim() << "\n";
        else if (line.startsWith("#### ")) out << line.substring(5).trim() << "\n";
        else if (trimmedLine.startsWith("- ") || trimmedLine.startsWith("* ") || trimmedLine.startsWith("+ "))
            out << bullet << trimmedLine.substring(2).trim() << "\n";
        else
        {
            juce::String t = line.replace("**", "").replace("*", "").replace("_", "");
            t = stripInlineCode(t);
            out << t << "\n";
        }
    }
    return out;
}
