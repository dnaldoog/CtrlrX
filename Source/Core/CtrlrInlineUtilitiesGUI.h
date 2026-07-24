#ifndef CTRLR_INLINE_UTILITIES_GUI
#define CTRLR_INLINE_UTILITIES_GUI

#include "../UIComponents/CtrlrWindowManagers/CtrlrDialogWindow.h"
#include "CtrlrMacros.h"
#include <JuceHeader.h>
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h> // Make sure this is included for LookAndFeel_V4

#pragma once

#include <JuceHeader.h> // or your standard JUCE include

// 1. Forward declaration outside namespace
class CtrlrDialogWindow;

// 2. The namespace block
namespace AW {
// --- Decoupled async dialog (defined in .cpp) ---
void showCustomDialogAsync(const juce::String &title, juce::Component *content, const juce::Colour &backgroundColour,
						   bool resizable, std::function<void(int)> callback = nullptr);

// --- Inline Alert Helpers (defined right here in .h) ---
static inline void runCustomAlertAsyncSafe(juce::AlertWindow *alert, std::function<void(int)> callback) {
#if JUCE_VERSION >= 0x070000
	alert->enterModalState(
		true, juce::ModalCallbackFunction::create([alert, callback](int result) { callback(result); }), true);
#else
	int result = alert->runModalLoop();
	if (callback != nullptr)
		callback(result);
#endif
}

// 1. AlertWindow version
static inline void layoutButtonsJUCE8(juce::AlertWindow *alert, const juce::String &okText,
									  const juce::String &cancelText, int bW = 100, int bH = 35) {
#if JUCE_VERSION >= 0x070000
	if (alert == nullptr)
		return;

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

// 2. Component* overload (calls the AlertWindow version above)
static inline void layoutButtonsJUCE8(juce::Component *component, const juce::String &okText,
									  const juce::String &cancelText, int bW = 100, int bH = 35) {
	if (auto *alert = dynamic_cast<juce::AlertWindow *>(component)) {
		layoutButtonsJUCE8(alert, okText, cancelText, bW, bH);
	}
}

} // namespace AW

class PU {
public:
    // =========================================================================
    // SYNCHRONOUS HELPERS (Return selected item ID integer)
    // =========================================================================

    /** Displays a menu synchronously with layout constraints */
    static int showMenuSync(juce::PopupMenu &menu,
                            int itemIDThatMustBeVisible = 0,
                            int minimumWidth = 0,
                            int maximumNumColumns = 0,
                            int standardItemHeight = 0) 
    {
#if JUCE_VERSION >= 0x070000
        auto options = juce::PopupMenu::Options()
            .withMinimumWidth(minimumWidth)
            .withMaximumNumColumns(maximumNumColumns)
            .withStandardItemHeight(standardItemHeight);

        if (itemIDThatMustBeVisible > 0)
            options = options.withItemThatMustBeVisible(itemIDThatMustBeVisible);

        return showSyncWithOptions(menu, options);
#else
        return menu.show(itemIDThatMustBeVisible, minimumWidth, maximumNumColumns, standardItemHeight);
#endif
    }

    /** Displays a menu synchronously attached to a target Component */
    static int showMenuSyncAtComponent(juce::PopupMenu &menu,
                                       juce::Component *componentToAttachTo,
                                       int standardItemHeight = 0) 
    {
        if (componentToAttachTo == nullptr)
            return showMenuSync(menu, 0, 0, 0, standardItemHeight);

#if JUCE_VERSION >= 0x070000
        auto options = juce::PopupMenu::Options()
            .withTargetComponent(componentToAttachTo)
            .withStandardItemHeight(standardItemHeight);

        return showSyncWithOptions(menu, options);
#else
        return menu.showAt(componentToAttachTo, standardItemHeight);
#endif
    }

    /** Displays a menu synchronously attached to a Screen Area */
    static int showMenuSyncAtArea(juce::PopupMenu &menu,
                                  const juce::Rectangle<int> &areaToAttachTo,
                                  int standardItemHeight = 0) 
    {
#if JUCE_VERSION >= 0x070000
        auto options = juce::PopupMenu::Options()
            .withTargetScreenArea(areaToAttachTo)
            .withStandardItemHeight(standardItemHeight);

        return showSyncWithOptions(menu, options);
#else
        return menu.showAt(areaToAttachTo, standardItemHeight);
#endif
    }

	static int showSyncWithOptions(juce::PopupMenu &menu, const juce::PopupMenu::Options &options) {
#if JUCE_VERSION >= 0x070000
		// JUCE 8 non-blocking async execution (returns 0 immediately to caller)
		menu.showMenuAsync(options, nullptr);
		return 0;
#else
		// Legacy JUCE 6 synchronous execution
		return menu.showWithOptionalTargetComponent(options.getTargetComponent());
#endif
	}

	// =========================================================================
	// ASYNCHRONOUS HELPERS (Non-blocking, uses callback)
	// =========================================================================

	/** Safely shows a popup menu asynchronously across JUCE versions */
	static void showMenuAsyncSafe(juce::PopupMenu &menuToDisplay,
                                  juce::Component *targetComponent,
                                  std::function<void(int)> callback,
                                  juce::Component *componentToTargetForShowAt = nullptr) 
    {
        juce::Component *finalTarget =
            (componentToTargetForShowAt != nullptr) ? componentToTargetForShowAt : targetComponent;

#if JUCE_VERSION >= 0x070000
        auto options = juce::PopupMenu::Options();
        if (finalTarget != nullptr)
            options = options.withTargetComponent(finalTarget);

        menuToDisplay.showMenuAsync(options, [callback](int result) {
            if (callback) callback(result);
        });
#else
        int result = 0;
        if (componentToTargetForShowAt != nullptr) {
            result = menuToDisplay.showAt(componentToTargetForShowAt);
        } else {
            result = menuToDisplay.show();
        }

        if (callback) callback(result);
#endif
    }

private:
	// #if JUCE_VERSION >= 0x070000
	//     /** Private helper to run modern JUCE async menus synchronously */
	// static void showSyncWithOptions(juce::PopupMenu& menu, const juce::PopupMenu::Options& options,
	// std::function<void(int)> callback = nullptr)
	// {
	//     menu.showMenuAsync(options, juce::ModalComponentManager::Callback::create([callback](int result)
	//     {
	//         if (callback)
	//             callback(result);
	//     }));
	// }
	// #endif
};
/**************************************************************************************************/
namespace AW {
	// public:
		enum Icon { None, Question, Warning, Info, NoIcon };

		/**************************************************************************************************/
		/**
		 * JUCE 8 Async Popup Menu Wrapper
		 * Handles non-blocking popup menu execution across all JUCE versions.
		 *
		 * @param menu      The juce::PopupMenu instance to present
		 * @param options   juce::PopupMenu::Options configuring target, position, etc.
		 * @param callback  Optional callback lambda receiving the selected Item ID (0 if dismissed)
		 */
		static void showPopupMenuAsync(juce::PopupMenu &menu, const juce::PopupMenu::Options &options,
									   std::function<void(int)> callback = nullptr) {
#if JUCE_VERSION >= 0x070000
			menu.showMenuAsync(options, [callback](int result) {
				if (callback != nullptr)
					callback(result);
			});
#else
			int result = menu.showWithOptionalTargetComponent(options.getTargetComponent());
			if (callback != nullptr)
				callback(result);
#endif
		}

		/**************************************************************************************************/
static bool showNativeDialogBox(Icon icon, const juce::String &title, const juce::String &bodyText,
								const juce::String &buttonText1, const juce::String &buttonText2, bool isOkCancel,
								std::function<void(bool)> completionCallback = nullptr) {
	juce::MessageBoxIconType juce8Icon;

	switch (icon) {
	case AW::Question:
		juce8Icon = juce::MessageBoxIconType::QuestionIcon;
		break;
	case AW::Warning:
		juce8Icon = juce::MessageBoxIconType::WarningIcon;
		break;
	case AW::Info:
		juce8Icon = juce::MessageBoxIconType::InfoIcon;
		break;
	case AW::NoIcon:
	default:
		juce8Icon = juce::MessageBoxIconType::NoIcon;
		break;
	}

#if JUCE_VERSION < 0x070000
			// --- Legacy JUCE 6 Path (Synchronous) ---
			if (isOkCancel) {
				bool result =
					juce::AlertWindow::showOkCancelBox(juceAlertIcon, title, bodyText, buttonText1, buttonText2);

				if (completionCallback != nullptr)
					completionCallback(result);

				return result;
			} else {
				juce::AlertWindow::showMessageBox(juceAlertIcon, title, bodyText, buttonText1);
				if (completionCallback != nullptr)
					completionCallback(true);
				return true;
			}
#else
			// --- Modern JUCE 7/8 Path (Asynchronous) ---
			auto juce8NativeIcon = (icon == Question)  ? juce::MessageBoxIconType::QuestionIcon
								   : (icon == Warning) ? juce::MessageBoxIconType::WarningIcon
								   : (icon == Info)	   ? juce::MessageBoxIconType::InfoIcon
													   : juce::MessageBoxIconType::NoIcon;

			if (isOkCancel) {
				juce::NativeMessageBox::showOkCancelBox(
					juce8NativeIcon, title, bodyText, nullptr,
					juce::ModalCallbackFunction::create([completionCallback](int result) {
						if (completionCallback != nullptr) {
							completionCallback(result == 1);
						}
					}));
			} else {
				juce::NativeMessageBox::showMessageBoxAsync(
					juce8NativeIcon, title, bodyText, nullptr,
					juce::ModalCallbackFunction::create([completionCallback](int) {
						if (completionCallback != nullptr) {
							completionCallback(true);
						}
					}));
			}
			return false;
#endif
}

		/**************************************************************************************************/
		static void showMessageBox(Icon icon, const juce::String &title, const juce::String &message,
								   const juce::String &buttonText = "OK", std::function<void()> callback = nullptr) {
#if JUCE_VERSION >= 0x070000
			auto juce8Icon = (icon == Question)	 ? juce::MessageBoxIconType::QuestionIcon
							 : (icon == Warning) ? juce::MessageBoxIconType::WarningIcon
							 : (icon == Info)	 ? juce::MessageBoxIconType::InfoIcon
							 : (icon == NoIcon)	 ? juce::MessageBoxIconType::NoIcon
												 : juce::MessageBoxIconType::NoIcon;

			juce::AlertWindow::showMessageBoxAsync(juce8Icon, title, message, buttonText, nullptr,
												   juce::ModalCallbackFunction::create([callback](int /*result*/) {
													   if (callback != nullptr)
														   callback();
												   }));
#else
			auto juce6Icon = (icon == Question)	 ? juce::AlertWindow::QuestionIcon
							 : (icon == Warning) ? juce::AlertWindow::WarningIcon
							 : (icon == Info)	 ? juce::AlertWindow::InfoIcon
												 : juce::AlertWindow::NoIcon;

			juce::AlertWindow::showMessageBox(juce6Icon, title, message, buttonText);

			if (callback != nullptr)
				callback();
#endif
		}

		/**************************************************************************************************/
		static void showOkCancelAsyncSafe(Icon icon, const juce::String &title, const juce::String &bodyText,
										  std::function<void(bool)> completionCallback,
										  const juce::String &button1Text = "Yes",
										  const juce::String &button2Text = "No") {
			showNativeDialogBox(icon, title, bodyText, button1Text, button2Text, true, completionCallback);
		}

		/**************************************************************************************************/
		static void showYesNoCancelBox(Icon icon, const juce::String &title, const juce::String &message,
									   const juce::String &button1Text, const juce::String &button2Text,
									   const juce::String &button3Text, std::function<void(int)> callback) {
			juce::MessageBoxIconType juceIcon = juce::MessageBoxIconType::NoIcon;
			switch (icon) {
			case Question:
				juceIcon = juce::MessageBoxIconType::QuestionIcon;
				break;
			case Warning:
				juceIcon = juce::MessageBoxIconType::WarningIcon;
				break;
			case Info:
				juceIcon = juce::MessageBoxIconType::InfoIcon;
				break;
			default:
				break;
			}

			juce::NativeMessageBox::showAsync(juce::MessageBoxOptions()
												  .withIconType(juceIcon)
												  .withTitle(title)
												  .withMessage(message)
												  .withButton(button1Text)
												  .withButton(button2Text)
												  .withButton(button3Text),
											  [callback](int result) {
												  if (callback) {
													  callback(result);
												  }
											  });
		}

		/**************************************************************************************************/
		static void showWarning(const juce::String &title, const juce::String &message,
								std::function<void(int)> callback = nullptr) {
			juce::NativeMessageBox::showAsync(juce::MessageBoxOptions()
												  .withIconType(juce::MessageBoxIconType::WarningIcon)
												  .withTitle(title)
												  .withMessage(message)
												  .withButton("OK"),
											  [callback](int result) {
												  if (callback) {
													  callback(result);
												  }
											  });
		}

		/**************************************************************************************************/
// 		static void runCustomAlertAsyncSafe(juce::AlertWindow *alert, std::function<void(int)> callback) {
// #if JUCE_VERSION >= 0x070000
// 			alert->enterModalState(
// 				true, juce::ModalCallbackFunction::create([alert, callback](int result) { callback(result); }), true);
// #else
// 			int result = alert->runModalLoop();
// 			callback(result);
// #endif
// 		}

		/**************************************************************************************************/
// 		static void layoutButtonsJUCE8(juce::AlertWindow *alert, const juce::String &okText,
// 									   const juce::String &cancelText, int bW = 100, int bH = 35) {
// #if JUCE_VERSION >= 0x070000
// 			if (auto *okBtn = alert->getButton(okText)) {
// 				if (auto *cancelBtn = alert->getButton(cancelText)) {
// 					const int gap = 15;
// 					const int totalWidth = (bW * 2) + gap;
// 					const int startX = (alert->getWidth() - totalWidth) / 2;
// 					const int yPos = alert->getHeight() - bH - 25;

// 					okBtn->setBounds(startX, yPos, bW, bH);
// 					cancelBtn->setBounds(startX + bW + gap, yPos, bW, bH);
// 				}
// 			}
// #endif
// 		}
};
/**************************************************************************************************/
/*
USAGE

FC::saveFileAsync(
	"Save Panel File",
	panelFile,
	"*.panel",
	useOSDialog,
	[this](const File& selectedFile) {
		if (selectedFile.existsAsFile()) {
			// Save logic here
		}
	}
);


*/

namespace FC {
/**
 * Unified cross-version helper for saving files.
 * Works seamlessly on JUCE 6, 7, and 8.
 */
/**************************************************************************************************/
/**
 * Unified cross-version helper for opening single files.
 */
/**************************************************************************************************/
/**
 * Cross-version helper for saving a file asynchronously.
 */
inline void saveFileAsync(const juce::String &dialogTitle, const juce::File &initialFileOrDirectory,
						  const juce::String &filePatternsAllowed, bool useNativeDialog,
						  std::function<void(const juce::File &)> callback) {
	int flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles |
				juce::FileBrowserComponent::warnAboutOverwriting;

#if JUCE_VERSION >= 0x070000
	auto chooser =
		std::make_shared<juce::FileChooser>(dialogTitle, initialFileOrDirectory, filePatternsAllowed, useNativeDialog);
	chooser->launchAsync(flags, [chooser, callback](const juce::FileChooser &fc) {
		if (callback) {
			callback(fc.getResult());
		}
	});
#else
	auto *chooser = new juce::FileChooser(dialogTitle, initialFileOrDirectory, filePatternsAllowed, useNativeDialog);
	chooser->launchAsync(flags, [chooser, callback](const juce::FileChooser &fc) {
		if (callback) {
			callback(fc.getResult());
		}
		delete chooser;
	});
#endif
}
/**************************************************************************************************/
inline void openFileAsync(const String &dialogTitle, const File &initialFileOrDirectory,
						  const String &filePatternsAllowed, bool useNativeDialog,
						  std::function<void(const File &)> callback) {
	int flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

#if JUCE_MAJOR_VERSION >= 7
	// --- JUCE 7 & 8 Path ---
	auto chooser =
		std::make_shared<juce::FileChooser>(dialogTitle, initialFileOrDirectory, filePatternsAllowed, useNativeDialog);

	chooser->launchAsync(flags, [chooser, callback](const juce::FileChooser &fc) {
		if (callback) {
			callback(fc.getResult());
		}
	});
#else
	// --- JUCE 6 Path ---
	auto *chooser = new juce::FileChooser(dialogTitle, initialFileOrDirectory, filePatternsAllowed, useNativeDialog);

	chooser->launchAsync(flags, [chooser, callback](const juce::FileChooser &fc) {
		if (callback) {
			callback(fc.getResult());
		}
		delete chooser;
	});
#endif
}
/**
 * Cross-version helper to open multiple files asynchronously.
 */
/**************************************************************************************************/
inline void openMultipleFilesAsync(const juce::String &dialogTitle, const juce::File &initialFileOrDirectory,
								   const juce::String &filePatternsAllowed, bool useNativeDialog,
								   std::function<void(const juce::Array<juce::File> &)> callback) {
	int flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles |
				juce::FileBrowserComponent::canSelectMultipleItems;

#if JUCE_VERSION >= 0x070000
	auto chooser =
		std::make_shared<juce::FileChooser>(dialogTitle, initialFileOrDirectory, filePatternsAllowed, useNativeDialog);
	chooser->launchAsync(flags, [chooser, callback](const juce::FileChooser &fc) {
		if (callback) {
			callback(fc.getResults());
		}
	});
#else
	auto *chooser = new juce::FileChooser(dialogTitle, initialFileOrDirectory, filePatternsAllowed, useNativeDialog);
	chooser->launchAsync(flags, [chooser, callback](const juce::FileChooser &fc) {
		if (callback) {
			callback(fc.getResults());
		}
		delete chooser;
	});
#endif
}
} // namespace FC
/**************************************************************************************************/

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
