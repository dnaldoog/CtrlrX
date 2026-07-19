#ifndef CTRLR_INLINE_UTILITIES_GUI
#define CTRLR_INLINE_UTILITIES_GUI

#include "CtrlrMacros.h"
#include <juce_gui_basics/juce_gui_basics.h> // Make sure this is included for LookAndFeel_V4

class MyPopupHelper {
	public:
		/**
			Safely shows a popup menu across JUCE 6, 7, and 8.

			@param menuToDisplay   The PopupMenu object you built.
			@param targetComponent The component the menu should align with (usually 'this').
			@param callback        A lambda/function to execute with the resulting integer ID.
			@param componentToTargetForShowAt Optional: If you were originally using m.showAt(someButton),
											  pass that specific button pointer here. Otherwise leave it null.
		*/
		static void showMenuAsyncSafe(juce::PopupMenu &menuToDisplay, juce::Component *targetComponent,
									  std::function<void(int)> callback,
									  juce::Component *componentToTargetForShowAt = nullptr) {
#if JUCE_VERSION >= 0x070000
			// --- Modern JUCE 7/8 Asynchronous Approach ---
			// If a specific showAt target was passed, use it. Otherwise fall back to targetComponent.
			juce::Component *finalTarget =
				(componentToTargetForShowAt != nullptr) ? componentToTargetForShowAt : targetComponent;

			juce::PopupMenu::Options options = juce::PopupMenu::Options().withTargetComponent(finalTarget);

			menuToDisplay.showMenuAsync(options, [callback](int result) {
				callback(result); // Runs your menu handling logic
			});
#else
			// --- Legacy JUCE 6 Synchronous Approach ---
			int result = 0;
			if (componentToTargetForShowAt != nullptr) {
				result = menuToDisplay.showAt(componentToTargetForShowAt); // Emulates old showAt()
			} else {
				result = menuToDisplay.show(); // Emulates old show()
			}

			callback(result);
#endif
		}
};

class AW {
	public:
		enum Icon { None, Question, Warning, Info };

		static bool showNativeDialogBox(Icon icon, const juce::String &title, const juce::String &bodyText,
										bool isOkCancel) {
#if JUCE_VERSION < 0x070000
			// Map down to legacy standard icons
			auto juce6Icon = (icon == Question)	 ? juce::AlertWindow::QuestionIcon
							 : (icon == Warning) ? juce::AlertWindow::WarningIcon
							 : (icon == Info)	 ? juce::AlertWindow::InfoIcon
												 : juce::AlertWindow::NoIcon;

			return juce::AlertWindow::showNativeDialogBox(title, bodyText, isOkCancel);
#else
			// Map down to modern native icons
			auto juce8Icon = (icon == Question)	 ? juce::MessageBoxIconType::QuestionIcon
							 : (icon == Warning) ? juce::MessageBoxIconType::WarningIcon
							 : (icon == Info)	 ? juce::MessageBoxIconType::InfoIcon
												 : juce::MessageBoxIconType::NoIcon;

			if (isOkCancel) {
				juce::NativeMessageBox::showOkCancelBox(juce8Icon, title, bodyText, nullptr,
														juce::ModalCallbackFunction::create([](int) {}));
			} else {
				juce::NativeMessageBox::showMessageBoxAsync(juce8Icon, title, bodyText, nullptr,
															juce::ModalCallbackFunction::create([](int) {}));
			}
			return false;
#endif
		}

		bool AW::showNativeDialogBox(const String &title, const String &bodyText, bool isOkCancel) {
#if JUCE_VERSION < 0x070000
			// --- Legacy JUCE 6 Path (Synchronous) ---
			return juce::AlertWindow::showNativeDialogBox(title, bodyText, isOkCancel);
#else
			// --- Modern JUCE 8 Path (Asynchronous) ---
			using namespace juce;
			MessageBoxIconType icon = isOkCancel ? MessageBoxIconType::QuestionIcon : MessageBoxIconType::InfoIcon;

			if (isOkCancel) {
				NativeMessageBox::showOkCancelBox(icon, title, bodyText, nullptr,
												  ModalCallbackFunction::create([](int result) {
													  // result == 1 means 'OK' / 'Yes' was clicked.
													  // If Ctrlr needs to take action here, trigger a global event or
													  // callback.
												  }));
			} else {
				NativeMessageBox::showMessageBoxAsync(icon, title, bodyText, nullptr,
													  ModalCallbackFunction::create([](int) {}));
			}

			return false; // JUCE 8 fallback return
#endif
		}

		/**
		Executes a custom AlertWindow asynchronously for JUCE 7/8, or synchronously for JUCE 6.
	*/
		static void runCustomAlertAsyncSafe(juce::AlertWindow *alert, std::function<void(int)> callback) {
#if JUCE_VERSION >= 0x070000
			alert->enterModalState(true, juce::ModalCallbackFunction::create([alert, callback](int result) {
									   callback(result);
									   // Safe cleanup if it's a dynamic heap allocation
								   }),
								   true);
#else
			int result = alert->runModalLoop();
			callback(result);
#endif
		}

		/**
			Centralizes the custom button layout engines for JUCE 8 custom components.
			Does absolutely nothing on JUCE 6 to save execution cycles.
		*/
		static void layoutButtonsJUCE8(juce::AlertWindow *alert, const juce::String &okText,
									   const juce::String &cancelText, int bW = 100, int bH = 35) {
#if JUCE_VERSION >= 0x070000
			if (auto *okBtn = alert->getButton(okText)) {
				if (auto *cancelBtn = alert->getButton(cancelText)) {
					const int gap = 15;
					const int totalWidth = (bW * 2) + gap;
					const int startX = (alert->getWidth() - totalWidth) / 2;
					const int yPos = alert->getHeight() - bH - 25;

					okBtn->setBounds(startX, yPos, bW, bH);
					cancelBtn->setBounds(startX + bW + gap, yPos, bW, bH);
				}
			}
#endif
		}
};

namespace gui {

static inline DrawableButton *createDrawableButton(const String &buttonName, const String &svgData,
												   const String &svgDataDown = "",
												   const MouseCursor cursor = MouseCursor::PointingHandCursor) {
	std::unique_ptr<XmlElement> svgXml(XmlDocument::parse(svgData));
	std::unique_ptr<Drawable> drawable(Drawable::createFromSVG(*svgXml));

	std::unique_ptr<XmlElement> svgXmlDown(XmlDocument::parse(svgDataDown));
	std::unique_ptr<Drawable> drawableDown(svgXmlDown ? Drawable::createFromSVG(*svgXmlDown) : nullptr);

	auto btn = new DrawableButton(buttonName, DrawableButton::ImageFitted);
	btn->setImages(drawable.get(), nullptr, drawableDown.get(), nullptr, nullptr, nullptr, nullptr, nullptr);
	btn->setMouseCursor(cursor);
	return btn;
}

static inline Drawable *createDrawable(const String &svgData) {
	std::unique_ptr<XmlElement> svgXml(XmlDocument::parse(svgData));
	return Drawable::createFromSVG(*svgXml).release();
}

static const inline void drawSelectionRectangle(Graphics &g, int width, int height,
												Colour base = Colour(HIGHLIGHT_COLOUR),
												const float baseSaturation = 0.9f, const float baseAlpha = 0.9f,
												const float gradientMin = 0.2f, const float gradientMax = 0.25f) {
	Colour baseColour(base.withMultipliedSaturation(baseSaturation).withMultipliedAlpha(baseAlpha));
	const float mainBrightness = baseColour.getBrightness();
	const float mainAlpha = baseColour.getFloatAlpha();
	Path outline;
	outline.addRoundedRectangle(0, 0, width, height, 4.0f, 4.0f, false, false, false, false);
	g.setGradientFill(ColourGradient(baseColour.brighter(gradientMin), 0.0f, 0.0f, baseColour.darker(gradientMax), 0.0f,
									 height, false));
	g.fillPath(outline);

	g.setColour(Colours::white.withAlpha(0.4f * mainAlpha * mainBrightness * mainBrightness));
	g.strokePath(outline, PathStrokeType(1.0f),
				 AffineTransform::translation(0.0f, 1.0f).scaled(1.0f, (height - 1.6f) / height));
	g.setColour(Colours::black.withAlpha(0.4f * mainAlpha));
	g.strokePath(outline, PathStrokeType(1.0f));
}

static const inline void drawSelectionRectangle(Graphics &g, int x, int y, int width, int height,
												Colour base = Colour(HIGHLIGHT_COLOUR),
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

// static LookAndFeel_V4::ColourScheme colourSchemeFromProperty(const var &property) {} // Updated v5.6.34. Moved to
// CtrlrInlineUtilitiesGUI.cpp

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
std::unique_ptr<juce::LookAndFeel> createLookAndFeelFromDescription(const juce::String &description,
																	const juce::var &colourSchemeProperty = juce::var(),
																	bool returnDefaultV4ForUnknown = true);

// Your existing colourSchemeFromProperty function (can now call createLookAndFeelFromDescription if it uses properties
// that map directly to the names used in createLookAndFeelFromDescription)
juce::LookAndFeel_V4::ColourScheme colourSchemeFromProperty(const juce::var &property);
} // namespace gui
#endif
