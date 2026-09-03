#ifndef CTRLR_INLINE_UTILITIES_GUI
#define CTRLR_INLINE_UTILITIES_GUI

#include "../UIComponents/CtrlrWindowManagers/CtrlrDialogWindow.h"
#include "CtrlrMacros.h"
#include <JuceHeader.h>
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h> // Make sure this is included for LookAndFeel_V4

#pragma once

#include <JuceHeader.h> // or your standard JUCE include

namespace gui {

    static inline
    DrawableButton *createDrawableButton(const String &buttonName, const String &svgData,
										 const String &svgDataDown="",
										 const MouseCursor cursor = MouseCursor::PointingHandCursor) {
        std::unique_ptr<XmlElement> svgXml (XmlDocument::parse(svgData));
        std::unique_ptr<Drawable> drawable (Drawable::createFromSVG(*svgXml));

        std::unique_ptr<XmlElement> svgXmlDown (XmlDocument::parse(svgDataDown));
        std::unique_ptr<Drawable> drawableDown (svgXmlDown ? Drawable::createFromSVG(*svgXmlDown) : nullptr);

        auto btn = new DrawableButton(buttonName, DrawableButton::ImageFitted);
        btn->setImages(drawable.get(),
                       nullptr,
                       drawableDown.get(),
                       nullptr,
                       nullptr,
                       nullptr,
                       nullptr,
                       nullptr);
        btn->setMouseCursor(cursor);
        return btn;
    }

    static inline
    Drawable *createDrawable(const String &svgData) {
        std::unique_ptr<XmlElement> svgXml (XmlDocument::parse(svgData));
        return Drawable::createFromSVG(*svgXml).release();
    }

    static const inline void
    drawSelectionRectangle(Graphics &g, int width, int height, Colour base = Colour(HIGHLIGHT_COLOUR),
                           const float baseSaturation = 0.9f, const float baseAlpha = 0.9f,
                           const float gradientMin = 0.2f, const float gradientMax = 0.25f) {
        Colour baseColour(base.withMultipliedSaturation(baseSaturation).withMultipliedAlpha(baseAlpha));
        const float mainBrightness = baseColour.getBrightness();
        const float mainAlpha = baseColour.getFloatAlpha();
        Path outline;
        outline.addRoundedRectangle(0, 0, width, height, 4.0f, 4.0f, false, false, false, false);
        g.setGradientFill(ColourGradient(baseColour.brighter(gradientMin), 0.0f, 0.0f,
                                         baseColour.darker(gradientMax), 0.0f, height, false));
        g.fillPath(outline);

        g.setColour(Colours::white.withAlpha(0.4f * mainAlpha * mainBrightness * mainBrightness));
        g.strokePath(outline, PathStrokeType(1.0f),
                     AffineTransform::translation(0.0f, 1.0f).scaled(1.0f, (height - 1.6f) / height));
        g.setColour(Colours::black.withAlpha(0.4f * mainAlpha));
        g.strokePath(outline, PathStrokeType(1.0f));
    }

    static const inline void
    drawSelectionRectangle(Graphics &g, int x, int y, int width, int height, Colour base = Colour(HIGHLIGHT_COLOUR),
                           const float baseSaturation = 0.9f, const float baseAlpha = 0.9f) {
        Colour baseColour(base.withMultipliedSaturation(baseSaturation).withMultipliedAlpha(baseAlpha));
        const float mainBrightness = baseColour.getBrightness();
        const float mainAlpha = baseColour.getFloatAlpha();
        Path outline;

        outline.addRoundedRectangle(x, y, width, height, 4.0f, 4.0f, false, false, false, false);

        g.setGradientFill(ColourGradient(baseColour.brighter(0.2f), x, y, baseColour.darker(0.25f), x, height, false));
        g.fillPath(outline);

        g.setColour(Colours::white.withAlpha(0.4f * mainAlpha * mainBrightness * mainBrightness));
        g.strokePath(outline, PathStrokeType(1.0f),
                     AffineTransform::translation(0.0f, 1.0f).scaled(1.0f, (height - 1.6f) / height));
        g.setColour(Colours::black.withAlpha(0.4f * mainAlpha));
        g.strokePath(outline, PathStrokeType(1.0f));
    }

    // static LookAndFeel_V4::ColourScheme colourSchemeFromProperty(const var &property) {} // Updated v5.6.34. Moved to CtrlrInlineUtilitiesGUI.cpp

    // Declare your custom ColourScheme getter functions
    juce::LookAndFeel_V4::ColourScheme getJetBlackColourScheme();
    juce::LookAndFeel_V4::ColourScheme getYamDxColourScheme();
    juce::LookAndFeel_V4::ColourScheme getAkApcColourScheme();
    juce::LookAndFeel_V4::ColourScheme getAkMpcColourScheme();
    juce::LookAndFeel_V4::ColourScheme getLexiBlueColourScheme();
    juce::LookAndFeel_V4::ColourScheme getKurzGreenColourScheme();
    juce::LookAndFeel_V4::ColourScheme getKorGreyColourScheme();
    juce::LookAndFeel_V4::ColourScheme getKorGoldColourScheme();
    juce::LookAndFeel_V4::ColourScheme getArturOrangeColourScheme();
    juce::LookAndFeel_V4::ColourScheme getAiraGreenColourScheme();

    // Your existing colourSchemeFromProperty function
    juce::LookAndFeel_V4::ColourScheme colourSchemeFromProperty(const juce::var &property);

    // The central LookAndFeel factory function
    // Keep the optional 'colourSchemeProperty' parameter, it's not directly used here but the signature is fine.
    juce::LookAndFeel* createLookAndFeelFromDescription(const juce::String& description,
                                                        const juce::var& colourSchemeProperty = juce::var(),
                                                        bool returnDefaultV4ForUnknown = true);

    // Your existing colourSchemeFromProperty function (can now call createLookAndFeelFromDescription if it uses properties
    // that map directly to the names used in createLookAndFeelFromDescription)
    juce::LookAndFeel_V4::ColourScheme colourSchemeFromProperty(const juce::var &property);
inline void initLookAndFeelDefaults(juce::LookAndFeel &lf) {
	lf.setColour(juce::ScrollBar::backgroundColourId, juce::Colour(0xffffffff));
	lf.setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xffababab));
	lf.setColour(juce::ScrollBar::trackColourId, juce::Colour(0xffff0000));

	lf.setColour(juce::TextEditor::highlightColourId, juce::Colours::antiquewhite);
	lf.setColour(juce::TextEditor::highlightedTextColourId, juce::Colour(0xff000000));
	lf.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff000000));

	lf.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff000000));
	lf.setColour(juce::ProgressBar::foregroundColourId, juce::Colour(0xff0f0f0f));

	lf.setColour(juce::CodeEditorComponent::backgroundColourId, juce::Colours::white);
	lf.setColour(juce::CodeEditorComponent::defaultTextColourId, juce::Colours::black);

	lf.setColour(0x1007001,
				 juce::Colours::black); // placeholder lf.setColour(CtrlrPropertyComponent::labelTextColourId)
}
}

namespace LNF {

struct ColourMapping {
		juce::Identifier treePropId;
		int juceColourId;
};

// --- Overload 1: Core Logic ---
inline void applyLookAndFeelState(juce::Component &targetComp, juce::ValueTree &ownerTree,
								  const juce::Identifier &customFlagId, std::initializer_list<ColourMapping> mappings) {
	// If missing from tree, default to 0 (User Mode)
	const var propVal = ownerTree.getProperty(customFlagId, 0);

	// Matches XML: 0 (or false) = "Using my colours", 1 (or true) = "Using LookAndFeel colours"
	const bool useUserSettings = !(propVal.equals(var(0)) || propVal.equals(var(false)));

	if (useUserSettings) {
		// --- USER MODE ---
		for (const auto &map : mappings) {
			if (ownerTree.hasProperty(map.treePropId)) {
				juce::Colour col = VAR2COLOUR(ownerTree.getProperty(map.treePropId));
				targetComp.setColour(map.juceColourId, col);
			}
		}
	} else {
		// --- LNF MODE ---
		for (const auto &map : mappings) {
			targetComp.removeColour(map.juceColourId);
		}

		// Tells JUCE to flush cached colors and fetch from active LNF theme
		targetComp.sendLookAndFeelChange();
	}

	targetComp.repaint();
}

// --- Overload 2: Positional Parameter Version ---
inline void applyLookAndFeelState(juce::Component &targetComp, juce::ValueTree &ownerTree,
								  const juce::Identifier &customFlagId, const juce::Identifier &bgColourOnId,
								  const juce::Identifier &bgColourOffId, int juceBgColourOnId, int juceBgColourOffId,
								  const juce::Identifier &textColourOnId, const juce::Identifier &textColourOffId,
								  int juceTextColourOnId, int juceTextColourOffId) {
	applyLookAndFeelState(targetComp, ownerTree, customFlagId,
						  {{bgColourOnId, juceBgColourOnId},
						   {bgColourOffId, juceBgColourOffId},
						   {textColourOnId, juceTextColourOnId},
						   {textColourOffId, juceTextColourOffId}});
}
/*************************************************
 *
 *
 *
 ************************************************/
// Font-equivalent of applyLookAndFeelState. Same custom-flag-driven idea, but
// since JUCE has no generic "font ID" the way it has ColourId, each font target
// needs its own setter callback instead of a plain int.
// inline void applyFontState(
// 	juce::Component &targetComp, juce::ValueTree &ownerTree, const juce::Identifier &customFlagId,
// 	std::initializer_list<std::pair<juce::Identifier, std::function<void(const juce::Font &)>>> fontMappings) {
// 	const bool isCustom = (bool)ownerTree.getProperty(customFlagId, false);

// 	for (auto &mapping : fontMappings) {
// 		if (isCustom && ownerTree.hasProperty(mapping.first)) {
// 			juce::String descriptor = ownerTree.getProperty(mapping.first).toString();
// 			DBG("Applying custom font for property " << mapping.first.toString() << ": " << descriptor);
// 			// juce::Font font = CtrlrFontManager::getInstance()->getFontFromString(descriptor);
// 			// mapping.second(font); // call whatever setter this target needs
// 		}
// 		// else: leave it alone — same "let the theme own it" behavior as colours
// 	}
// }

/**
 * Call this ONLY when the user clicks "Freeze LNF to User Settings"
 * or when initializing default properties on a brand-new component.
 */
inline void freezeLnfToUserSettings(juce::Component &targetComp, juce::ValueTree &ownerTree,
									const juce::Identifier &customFlagId, const juce::Identifier &colourOnId,
									const juce::Identifier &colourOffId, int juceColourOnId, int juceColourOffId) {
	juce::LookAndFeel &currentLNF = targetComp.getLookAndFeel();
	juce::Colour lnfOn = currentLNF.findColour(juceColourOnId);
	juce::Colour lnfOff = currentLNF.findColour(juceColourOffId);

	// Save current LNF colors into the tree as user's starting point
	ownerTree.setProperty(colourOnId, lnfOn.toDisplayString(true), nullptr);
	ownerTree.setProperty(colourOffId, lnfOff.toDisplayString(true), nullptr);

	// Turn ON custom mode
	ownerTree.setProperty(customFlagId, true, nullptr);

	targetComp.setColour(juceColourOnId, lnfOn);
	targetComp.setColour(juceColourOffId, lnfOff);
	targetComp.repaint();
}

} // namespace LNF
/**************************************************************************************** */

namespace CtrlrThemeLookup {

struct PanelThemePalette {
		juce::Colour textColour;	// Text / Label colour
		juce::Colour outlineColour; // Toggle box / Border outline
		juce::Colour tickAccent;	// Checkmark / Radio dot accent
};

static PanelThemePalette getPaletteForScheme(const juce::String &schemeName) {
	if (schemeName == "V4 Light") {
		return {juce::Colour(0xff000000), juce::Colour(0x60000000), juce::Colour(0xff000000)};
	} else if (schemeName == "Lexi Blue") {
		return {juce::Colour(0xffffffff), juce::Colour(0xff515459), juce::Colour(0xff5794c7)};
	} else if (schemeName == "Kurz Green") {
		return {juce::Colour(0xffffffff), juce::Colour(0xff515459), juce::Colour(0xff00a66e)};
	} else if (schemeName == "Artur Orange") {
		return {juce::Colour(0xffffffff), juce::Colour(0xff515459), juce::Colour(0xffe24a21)};
	} else if (schemeName == "V3" || schemeName == "V2" || schemeName == "V1") {
		return {juce::Colour(0xff000000), juce::Colour(0xff0000ff), juce::Colour(0xff0000ff)};
	}

	// Default Dark Fallback
	return {juce::Colour(0xffffffff), juce::Colour(0xff666666), juce::Colour(0xffffffff)};
}
} // namespace CtrlrThemeLookup
/* USAGE */
// PanelThemePalette palette = CtrlrThemeLookup::getPanelThemePaletteForScheme(activeScheme);

// // Apply explicitly
// ctrlrButton->setColour(juce::ToggleButton::textColourId, palette.textColour);
// ctrlrButton->setColour(juce::ToggleButton::tickDisabledColourId, palette.outlineColour);
// ctrlrButton->setColour(juce::ToggleButton::tickColourId, palette.tickAccent);
#endif
