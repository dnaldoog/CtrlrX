#include "CtrlrFontManager.h"
#include "CtrlrLog.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelResource.h"
#include "CtrlrPanel/CtrlrPanelResourceManager.h"
#include "stdafx.h"

CtrlrFontManager::CtrlrFontManager(CtrlrManager &_owner) : owner(_owner) {
	reloadFonts();
}

CtrlrFontManager::~CtrlrFontManager() {}

void CtrlrFontManager::reloadFonts() {
	reloadOSFonts();
	reloadBuiltInFonts();
	reloadImportedFonts();
	reloadJuceFonts();
}

void CtrlrFontManager::reloadOSFonts() {
	osFonts.clear();
	osTypefaceNames.clear();
	osTypefaceStyles.clear();

	osTypefaceNames = Font::findAllTypefaceNames();
	Font::findFonts(osFonts);
}

void CtrlrFontManager::reloadJuceFonts() {
	juceFonts.clear();

	juceFonts.add(Font(Font::getDefaultMonospacedFontName(), 12.0f, Font::plain));
	juceFonts.add(Font("Tahoma", 14.0f, Font::plain)); // <--- Forces Arial instead of system Verdana
	juceFonts.add(Font(Font::getDefaultSerifFontName(), 12.0f, Font::plain));
}

int CtrlrFontManager::getNumBuiltInFonts() {
	return (11);
}

void CtrlrFontManager::reloadBuiltInFonts() {
	builtInFonts.clear();

	for (int i = 0; i < getNumBuiltInFonts(); i++) {
		builtInFonts.add(CtrlrFontManager::getBuiltInFont(i));
	}
}

void CtrlrFontManager::reloadImportedFonts(CtrlrPanel *panelToLoadFrom) {
	importedFonts.clear();
	CtrlrPanel *panel = nullptr;

	if (panelToLoadFrom) {
		panel = panelToLoadFrom;
	} else {
		panel = owner.getActivePanel();
	}

	if (panel) {
		CtrlrPanelResourceManager &manager = panel->getResourceManager();

		for (int i = 0; i < manager.getNumResources(); i++) {
			if (manager.getResource(i)->getType() == FontRes) {
				importedFonts.add(manager.getResource(i)->asFont());
			}
		}
	}
}

const Array<Font> &CtrlrFontManager::getOsFontArray() {
	return (osFonts);
}

void CtrlrFontManager::fillCombo(ComboBox &comboToFill, const bool showOsFonts, const bool showBuiltInFonts,
								 const bool showImportedFonts, const bool showJuceFonts) {
	comboToFill.clear();
	int i = 0;

	if (showJuceFonts) {
		comboToFill.addSectionHeading("OS Default fonts");
		for (i = 0; i < juceFonts.size(); i++) {
			comboToFill.addItem(juceFonts[i].getTypefaceName(), allFontCount + i + 1);
		}

		allFontCount += i;
	}

	if (showBuiltInFonts) {
		comboToFill.addSectionHeading("Ctrlr Built-In fonts");
		for (i = 0; i < builtInFonts.size(); i++) {
			comboToFill.addItem(builtInFonts[i].getTypefaceName(), allFontCount + i + 1);
		}

		allFontCount += i;
	}

	if (showImportedFonts) {
		reloadImportedFonts();

		comboToFill.addSectionHeading("Imported fonts (from resources)");
		for (i = 0; i < importedFonts.size(); i++) {
			comboToFill.addItem(importedFonts[i].getTypefaceName(), allFontCount + i + 1);
		}

		allFontCount += i;
	}

	if (showOsFonts) {
		comboToFill.addSectionHeading("OS Fonts");
		for (i = 0; i < osFonts.size(); i++) {
			comboToFill.addItem(osFonts[i].getTypefaceName(), allFontCount + i + 1);
		}

		allFontCount += i;
	}
}

Font CtrlrFontManager::getFont(const int fontIndex) {
	if (fontIndex >= 0 && fontIndex < juceFonts.size()) {
		return (juceFonts[fontIndex]);
	}
	if (fontIndex >= juceFonts.size() && fontIndex < (builtInFonts.size() + juceFonts.size())) {
		return (builtInFonts[fontIndex - juceFonts.size()]);
	} else if (fontIndex >= builtInFonts.size() &&
			   fontIndex < (importedFonts.size() + builtInFonts.size() + juceFonts.size())) {
		return (importedFonts[fontIndex - juceFonts.size() - builtInFonts.size()]);
	} else if (fontIndex >= (importedFonts.size() + builtInFonts.size()) &&
			   fontIndex < (importedFonts.size() + juceFonts.size() + builtInFonts.size() + osFonts.size())) {
		return (osFonts[fontIndex - juceFonts.size() - builtInFonts.size() - importedFonts.size()]);
	}

	return (builtInFonts[0]);
}

Font CtrlrFontManager::getBuiltInFont(const String &fontResourceName) {
	int dataSize = 0;
	const char *dataPointer = BinaryData::getNamedResource(fontResourceName.toUTF8(), dataSize);

	if (dataSize <= 0 || dataPointer == nullptr)
		return Font(); // was: Font(Font::getDefaultSansSerifFontName(), 12.0f, Font::plain)

	auto tf = Typeface::createSystemTypefaceFor(dataPointer, (size_t)dataSize);

	if (tf != nullptr)
		return Font(FontOptions(tf).withPointHeight(12.0f));

	return Font();
}
const Font CtrlrFontManager::getFont(const char *fontData, const size_t fontDataSize) {
	if (fontData == nullptr || fontDataSize == 0)
		return Font();
	auto tf = Typeface::createSystemTypefaceFor(fontData, fontDataSize);
	if (tf != nullptr)
		return Font(FontOptions(tf).withPointHeight(12.0f));
	return Font();
}

Font CtrlrFontManager::getFont(const File &fontFile) {
	MemoryBlock data;
	if (!fontFile.loadFileAsData(data) || data.getSize() == 0)
		return Font();
	auto tf = Typeface::createSystemTypefaceFor(data.getData(), data.getSize());
	if (tf != nullptr)
		return Font(FontOptions(tf).withPointHeight(12.0f));
	return Font();
}

const Font CtrlrFontManager::getFontFromString(const String &string) {
	if (!string.contains(";")) {
		if (string.isEmpty()) {
			return Font(FontOptions(12.0f));
		}
		return Font::fromString(string);
	}

	StringArray fontProps;
	fontProps.addTokens(string, ";", "\"\'");
	Font font;

	if (fontProps[fontTypefaceName].isNotEmpty()) {
		String typefaceName = fontProps[fontTypefaceName];

		// 1. Map generic <sans-serif> directly to Arial so modern OS doesn't render Verdana
		if (typefaceName.isEmpty() || typefaceName == "<sans-serif>" || typefaceName == "<Sans-Serif>" ||
			typefaceName == Font::getDefaultSansSerifFontName()) {
			typefaceName = "Arial";
		}

		// 2. Resolve Font Set and Typeface
		if (fontProps[fontSet].isNotEmpty() && fontProps[fontSet].getIntValue() >= 0) {
			int setIdx = fontProps[fontSet].getIntValue();
			Array<Font> &fontSetToUse = getFontSet((const FontSet)setIdx);

			for (int i = 0; i < fontSetToUse.size(); ++i) {
				if (fontSetToUse[i].getTypefaceName() == typefaceName) {
					font = fontSetToUse[i];
					break;
				}
			}
		} else {
			/* Fall back to OS / JUCE font matching */
			font.setTypefaceName(typefaceName);
		}

		// 3. Parse Height with Safe Default (12.0f keeps text unclipped)
		float fontHeight = 12.0f;
		if (fontProps[fontHeight].isNotEmpty()) {
			float parsedHeight = fontProps[fontHeight].getFloatValue();
			if (parsedHeight > 0.0f)
				fontHeight = parsedHeight;
		}

		// 4. Extract Styles and Declare styleName
		bool isBold = (fontProps[fontBold].isNotEmpty() && fontProps[fontBold].getIntValue() != 0);
		bool isItalic = (fontProps[fontItalic].isNotEmpty() && fontProps[fontItalic].getIntValue() != 0);
		bool isUnderline = (fontProps[fontUnderline].isNotEmpty() && fontProps[fontUnderline].getIntValue() != 0);

		String styleName = "Regular";
		if (isBold && isItalic)
			styleName = "Bold Italic";
		else if (isBold)
			styleName = "Bold";
		else if (isItalic)
			styleName = "Italic";

		// 5. Construct Font via FontOptions
		FontOptions options;

		bool isCustomEmbeddedFont =
			(fontProps[fontSet].isNotEmpty() && (fontProps[fontSet].getIntValue() == builtInFontSet ||
												 fontProps[fontSet].getIntValue() == importedFontSet));

		if (isCustomEmbeddedFont && font.getTypefacePtr() != nullptr) {
			// Embedded/Built-in fonts: use binary typeface pointer safely
			options = FontOptions(font.getTypefacePtr()).withPointHeight(fontHeight);
		} else {
			// Standard OS fonts: use typeface name & style string
			options = FontOptions(typefaceName, fontHeight, Font::plain).withStyle(styleName);
		}

		font = Font(options);

		// 6. Apply Underline & Kerning
		font.setUnderline(isUnderline);

		if (fontProps[fontKerning].isNotEmpty())
			font.setExtraKerningFactor(fontProps[fontKerning].getFloatValue());

		if (fontProps[fontHorizontalScale].isNotEmpty()) {
			float scale = fontProps[fontHorizontalScale].getFloatValue();
			if (scale > 0.0f)
				font.setHorizontalScale(scale);
		}
	}

	return font;
}

const String CtrlrFontManager::getStringFromFont(const Font &_font) {
	Font font(_font);
	StringArray fontProps;
	fontProps.add(font.getTypefaceName());
	fontProps.add(String(font.getHeight()));
	fontProps.add(String((int)font.isBold()));
	fontProps.add(String((int)font.isItalic()));
	fontProps.add(String((int)font.isUnderlined()));
	fontProps.add(String(font.getExtraKerningFactor()));
	fontProps.add(String(font.getHorizontalScale()));
	fontProps.add(String((uint8)getFontSetEnum(font)));

	return (fontProps.joinIntoString(";"));
}

const String CtrlrFontManager::getDefaultMonoFontName() {
	if (osTypefaceNames.contains("Courier New")) {
		return ("Courier New");
	} else {
		return (Font::getDefaultMonospacedFontName());
	}
}

const Font CtrlrFontManager::getBuiltInFont(const int fontIndex) {
	Font f;

	switch (fontIndex) {
	case 0:
		f = getBuiltInFont("FONT_LCD_ttf");
		break;
	case 1:
		f = getBuiltInFont("FONT_Digital7_ttf");
		break;
	case 2:
		f = getBuiltInFont("FONT_DottyShadow_ttf");
		break;
	case 3:
		f = getBuiltInFont("FONT_ZX81_ttf");
		break;
	case 4:
		f = getBuiltInFont("FONT_Invasion2000_ttf");
		break;
	case 5:
		f = getBuiltInFont("FONT_Digit_ttf");
		break;
	case 6:
		f = getBuiltInFont("FONT_Computerfont_ttf");
		break;
	case 7:
		f = getBuiltInFont("FONT_Electronic_Highway_Sign_ttf");
		break;
	case 8:
		f = getBuiltInFont("FONT_Karmatic_Arcade_ttf");
		break;
	case 9:
		f = getBuiltInFont("FONT_60sekuntia_ttf");
		break;
	case 10:
		f = getBuiltInFont("FONT_WarenhausStandard_ttf");
		break;

	default:
		break;
	}

	return (f);
}

CtrlrFontManager::FontSet CtrlrFontManager::getFontSetEnum(const Font &font) {
	for (int i = 0; i < importedFonts.size(); i++) {
		if (importedFonts[i].getTypefaceName() == font.getTypefaceName())
			return (importedFontSet);
	}

	for (int i = 0; i < builtInFonts.size(); i++) {
		if (builtInFonts[i].getTypefaceName() == font.getTypefaceName())
			return (builtInFontSet);
	}

	for (int i = 0; i < osFonts.size(); i++) {
		if (osFonts[i].getTypefaceName() == font.getTypefaceName())
			return (osFontSet);
	}

	for (int i = 0; i < juceFonts.size(); i++) {
		if (juceFonts[i].getTypefaceName() == font.getTypefaceName())
			return (juceFontSet);
	}

	return (unknownFontSet);
}

Array<Font> &CtrlrFontManager::getFontSet(const FontSet fontSetToFetch) {
	switch (fontSetToFetch) {
	case osFontSet:
		return (osFonts);
	case builtInFontSet:
		return (builtInFonts);
	case importedFontSet:
		return (importedFonts);
	case juceFontSet:
	case unknownFontSet:
	default:
		return (juceFonts);
		break;
	}
}

Font CtrlrFontManager::getDefaultLargeFont() {
	return (defaultFont.withHeight(defaultFont.getHeight() * 1.25f));
}

Font CtrlrFontManager::getDefaultSmallFont() {
	return (defaultFont.withHeight(defaultFont.getHeight() * 0.75f));
}

Font CtrlrFontManager::getDefaultNormalFont() {
	return (defaultFont);
}
