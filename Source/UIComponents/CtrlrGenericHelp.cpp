#include "CtrlrGenericHelp.h"
#include "CtrlrMarkdownParser.h"

namespace {
const float LINEHEIGHT = 4.0f; // padding above the first block
const float LEFTMARGIN = 12.0f;
const float RIGHTMARGIN = 12.0f;
const float LINESPACING = 4.0f;	 // extra space between lines inside a text run
const float RULE_HEIGHT = 20.0f; // vertical space taken by a horizontal rule
const float TABLE_GAP = 4.0f;	 // space below a table
const float CELL_PAD_X = 8.0f;
const float CELL_PAD_Y = 4.0f;
const float MIN_COL_WIDTH = 40.0f;
const float MIN_ROW_HEIGHT = 18.0f;

// Width of the widest line if the text were never wrapped
float measureNaturalWidth(const juce::AttributedString &as) {
	juce::TextLayout layout;
	layout.createLayout(as, 100000.0f);

	float w = 0.0f;
	for (int i = 0; i < layout.getNumLines(); ++i)
		w = juce::jmax(w, layout.getLine(i).getLineBoundsX().getLength());

	return w + 2.0f; // small slack so rounding never causes an extra wrap
}

float measureHeight(const juce::AttributedString &as, float width) {
	juce::TextLayout layout;
	layout.createLayout(as, juce::jmax(1.0f, width));
	return layout.getHeight();
}
} // namespace

// TextLayout::getHeight() can come back too small (trailing newlines, mixed font sizes,
// line spacing), so work out the real vertical extent from the lines themselves.
void measureLayoutExtent(const juce::TextLayout &layout, float &top, float &height) {
	top = 0.0f;
	height = layout.getHeight();

	if (layout.getNumLines() == 0)
		return;

	float minY = 1.0e9f, maxY = -1.0e9f;

	for (int i = 0; i < layout.getNumLines(); ++i) {
		const auto bounds = layout.getLine(i).getLineBoundsY();
		minY = juce::jmin(minY, bounds.getStart());
		maxY = juce::jmax(maxY, bounds.getEnd());
	}

	top = minY;
	height = juce::jmax(height, maxY - minY);
}

CtrlrGenericHelp::CtrlrGenericHelp(const char* mdData, int mdSize)
{
    juce::String content = (mdData != nullptr)
        ? juce::String::fromUTF8(mdData, mdSize)
        : "Help file not found.";

	buildSegments(content);
	layoutSegments(800.0f - (LEFTMARGIN + RIGHTMARGIN));

	setSize(800, juce::jmax(600, (int)contentHeight));
}

void CtrlrGenericHelp::buildSegments(const juce::String &markdown) {
	segments.clear();
	plainText = CtrlrMarkdownParser::parseToPlainText(markdown);

	const auto blocks = CtrlrMarkdownParser::parseToBlocks(markdown);

	// Consecutive text blocks are merged into one run so line layout is unchanged
	juce::AttributedString run;
	run.setLineSpacing(LINESPACING);
	bool runHasContent = false;

	auto flushRun = [&]() {
		if (runHasContent) {
			Segment s;
			s.type = Segment::Type::Text;
			s.text = run;
			segments.push_back(std::move(s));
		}

		run = juce::AttributedString();
		run.setLineSpacing(LINESPACING);
		runHasContent = false;
	};

	for (const auto &b : blocks) {
		if (b.isHorizontalRule) {
			flushRun();
			Segment s;
			s.type = Segment::Type::Rule;
			segments.push_back(std::move(s));
		} else if (b.isTable) {
			flushRun();
			Segment s;
			s.type = Segment::Type::Table;
			s.cells = b.tableRows;
			segments.push_back(std::move(s));
		} else {
			run.append(b.content);
			runHasContent = true;
		}
	}

	flushRun();
}

void CtrlrGenericHelp::layoutTable(Segment &seg, float availableWidth) {
	const int numRows = (int)seg.cells.size();
	const int numCols = numRows > 0 ? (int)seg.cells[0].size() : 0;

	seg.colWidths.clear();
	seg.rowHeights.clear();

	if (numCols == 0) {
		seg.height = 0.0f;
		return;
	}

	// 1. Natural (unwrapped) width of each column
	juce::Array<float> natural;
	natural.insertMultiple(0, 0.0f, numCols);

	for (const auto &row : seg.cells)
		for (int c = 0; c < numCols; ++c)
			natural.set(c, juce::jmax(natural[c], measureNaturalWidth(row[(size_t)c]) + 2.0f * CELL_PAD_X));

	float totalNatural = 0.0f;
	for (float w : natural)
		totalNatural += w;

	// 2. Column widths: natural if it fits, otherwise narrow columns keep their natural width
	//    and the wide ones share what is left in proportion to their natural width
	seg.colWidths = natural;

	if (totalNatural > availableWidth) {
		juce::Array<bool> fixedCol;
		fixedCol.insertMultiple(0, false, numCols);

		float remaining = availableWidth;
		int unfixed = numCols;
		bool changed = true;

		while (changed && unfixed > 0) {
			changed = false;
			const float share = remaining / (float)unfixed;

			for (int c = 0; c < numCols; ++c) {
				if (!fixedCol[c] && natural[c] <= share) {
					fixedCol.set(c, true);
					seg.colWidths.set(c, natural[c]);
					remaining -= natural[c];
					--unfixed;
					changed = true;
				}
			}
		}

		float unfixedNatural = 0.0f;
		for (int c = 0; c < numCols; ++c)
			if (!fixedCol[c])
				unfixedNatural += natural[c];

		if (unfixedNatural > 0.0f)
			for (int c = 0; c < numCols; ++c)
				if (!fixedCol[c])
					seg.colWidths.set(c, juce::jmax(MIN_COL_WIDTH, remaining * natural[c] / unfixedNatural));
	}

	// 3. Row heights from the wrapped cell text
	float total = 0.0f;

	for (int r = 0; r < numRows; ++r) {
		float h = MIN_ROW_HEIGHT;

		for (int c = 0; c < numCols; ++c)
			h = juce::jmax(h, measureHeight(seg.cells[(size_t)r][(size_t)c], seg.colWidths[c] - 2.0f * CELL_PAD_X));

		const float rowH = h + 2.0f * CELL_PAD_Y;
		seg.rowHeights.add(rowH);
		total += rowH;
	}

	seg.height = total + TABLE_GAP;
}

void CtrlrGenericHelp::layoutSegments(float availableWidth) {
	float y = LINEHEIGHT;

	for (auto &seg : segments) {
		switch (seg.type) {
		case Segment::Type::Text: {
			seg.layout.createLayout(seg.text, availableWidth);

			float top = 0.0f, h = 0.0f;
			measureLayoutExtent(seg.layout, top, h);

			// DBG("text run: getHeight=" << seg.layout.getHeight() << " extent=" << h << " top=" << top);

			seg.textOffsetY = -top;
			seg.height = h;
			break;
		}

		case Segment::Type::Rule:
			seg.height = RULE_HEIGHT;
			break;

		case Segment::Type::Table:
			layoutTable(seg, availableWidth);
			break;
		}

		y += seg.height;
	}

	contentHeight = y + LEFTMARGIN;
}

void CtrlrGenericHelp::drawTable(juce::Graphics &g, const Segment &seg, float x, float y) {
	if (seg.rowHeights.isEmpty())
		return;

	float tableW = 0.0f, tableH = 0.0f;
	for (float w : seg.colWidths)
		tableW += w;
	for (float h : seg.rowHeights)
		tableH += h;

	// Header background
	g.setColour(juce::Colour(0xfff0f0f0));
	g.fillRect(x, y, tableW, seg.rowHeights[0]);

	// Cell text
	float cy = y;
	for (int r = 0; r < seg.rowHeights.size(); ++r) {
		float cx = x;
		for (int c = 0; c < seg.colWidths.size(); ++c) {
			const juce::Rectangle<float> area(cx + CELL_PAD_X, cy + CELL_PAD_Y, seg.colWidths[c] - 2.0f * CELL_PAD_X,
											  seg.rowHeights[r] - 2.0f * CELL_PAD_Y);

			juce::TextLayout layout;
			layout.createLayout(seg.cells[(size_t)r][(size_t)c], juce::jmax(1.0f, area.getWidth()));
			layout.draw(g, area);

			cx += seg.colWidths[c];
		}
		cy += seg.rowHeights[r];
	}

	// Grid
	g.setColour(juce::Colour(0xffbbbbbb));
	g.drawRect(x, y, tableW, tableH, 1.0f);

	float ly = y;
	for (int r = 0; r < seg.rowHeights.size() - 1; ++r) {
		ly += seg.rowHeights[r];
		g.drawHorizontalLine((int)ly, x, x + tableW);
	}

	float lx = x;
	for (int c = 0; c < seg.colWidths.size() - 1; ++c) {
		lx += seg.colWidths[c];
		g.drawVerticalLine((int)lx, y, y + tableH);
	}
}

void CtrlrGenericHelp::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

	const float availableWidth = (float)getWidth() - (LEFTMARGIN + RIGHTMARGIN);
	float y = LINEHEIGHT;

	for (const auto &seg : segments) {
		switch (seg.type) {
		case Segment::Type::Text:
			seg.layout.draw(
				g, juce::Rectangle<float>(LEFTMARGIN, y + seg.textOffsetY, availableWidth, seg.layout.getHeight()));
			break;

		case Segment::Type::Rule:
			g.setColour(juce::Colours::lightgrey);
			g.drawHorizontalLine((int)(y + RULE_HEIGHT * 0.5f), LEFTMARGIN, LEFTMARGIN + availableWidth * 0.9f);
			break;

		case Segment::Type::Table:
			drawTable(g, seg, LEFTMARGIN, y);
			break;
		}

		y += seg.height;
	}
}

void CtrlrGenericHelp::resized()
{
	layoutSegments(juce::jmax(100.0f, (float)getWidth() - (LEFTMARGIN + RIGHTMARGIN)));

	// Repaint without calling setSize() recursively inside resized()
	repaint();
}

void CtrlrGenericHelp::mouseDown(const juce::MouseEvent& e)
{
    // Right-click or Ctrl+Click to copy all text
    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        menu.addItem(1, "Copy All Text", true);
        menu.addItem(2, "Select All (Ctrl+A)", false);  // Visual hint

		menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
			if (result == 1)
				juce::SystemClipboard::copyTextToClipboard(plainText);
		});
	}
}

void CtrlrGenericHelp::parentSizeChanged()
{
    const int newWidth = getParentWidth();
    if (newWidth > 0 && newWidth != getWidth())
        setSize(newWidth, getHeight());
}

void CtrlrGenericHelp::setMarkdownContent(const juce::String& markdown)
{
	buildSegments(markdown);
	layoutSegments(juce::jmax(100.0f, (float)getWidth() - (LEFTMARGIN + RIGHTMARGIN)));
	repaint();
}

int CtrlrGenericHelp::getRequiredHeight() const
{
    return juce::roundToInt(contentHeight);
}