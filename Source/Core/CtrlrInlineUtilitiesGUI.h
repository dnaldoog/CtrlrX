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

	alert->enterModalState(
		true, juce::ModalCallbackFunction::create([alert, callback](int result) { callback(result); }), true);
}

// 1. AlertWindow version
static inline void layoutButtonsJUCE8(juce::AlertWindow *alert, const juce::String &okText,
									  const juce::String &cancelText, int bW = 100, int bH = 35) {

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
}

// 2. Component* overload (calls the AlertWindow version above)
static inline void layoutButtonsJUCE8(juce::Component *component, const juce::String &okText,
									  const juce::String &cancelText, int bW = 100, int bH = 35) {
	if (auto *alert = dynamic_cast<juce::AlertWindow *>(component)) {
		layoutButtonsJUCE8(alert, okText, cancelText, bW, bH);
	}
}

} // namespace AW

//--------------------------------------------------------------------------------------

class PU {
	public:
		// =========================================================================
		// SYNCHRONOUS HELPERS (Return selected item ID integer to Lua)
		// =========================================================================

		/** Displays a menu synchronously with layout constraints */
		static int showMenuSync(juce::PopupMenu &menu, int itemIDThatMustBeVisible = 0, int minimumWidth = 0,
								int maximumNumColumns = 0, int standardItemHeight = 0) {
			auto options = juce::PopupMenu::Options()
							   .withMinimumWidth(minimumWidth)
							   .withMaximumNumColumns(maximumNumColumns)
							   .withStandardItemHeight(standardItemHeight);

			if (itemIDThatMustBeVisible > 0)
				options = options.withItemThatMustBeVisible(itemIDThatMustBeVisible);

			return showSyncWithOptions(menu, options);
		}

		/** Displays a menu synchronously attached to a target Component */
		static int showMenuSyncAtComponent(juce::PopupMenu &menu, juce::Component *componentToAttachTo,
										   int standardItemHeight = 0) {
			if (componentToAttachTo == nullptr)
				return showMenuSync(menu, 0, 0, 0, standardItemHeight);

			auto options = juce::PopupMenu::Options()
							   .withTargetComponent(componentToAttachTo)
							   .withStandardItemHeight(standardItemHeight);

			return showSyncWithOptions(menu, options);
		}

		/** Displays a menu synchronously attached to a Screen Area */
		static int showMenuSyncAtArea(juce::PopupMenu &menu, const juce::Rectangle<int> &areaToAttachTo,
									  int standardItemHeight = 0) {
			auto options = juce::PopupMenu::Options()
							   .withTargetScreenArea(areaToAttachTo)
							   .withStandardItemHeight(standardItemHeight);

			return showSyncWithOptions(menu, options);
		}

		/** Runs the JUCE 8 async menu synchronously via local message pumping */
		static int showSyncWithOptions(juce::PopupMenu &menu, const juce::PopupMenu::Options &options) {
			auto *mm = juce::MessageManager::getInstance();

			if (!mm->isThisTheMessageThread()) {
				jassertfalse; // Popup menus must be shown from the Message Thread
				return 0;
			}

			bool finished = false;
			int selectedResult = 0;

			menu.showMenuAsync(options, [&finished, &selectedResult](int result) {
				selectedResult = result;
				finished = true;
			});

			// Pump the queue in short slices, checking our OWN local flag —
			// never touch stopDispatchLoop()/the global quit state.
			while (!finished)
				mm->runDispatchLoopUntil(20);

			return selectedResult;
		}
		static void showMenuAsyncAtArea(juce::PopupMenu &menuToDisplay, const juce::Rectangle<int> &screenArea,
										std::function<void(int)> callback) {
			auto options = juce::PopupMenu::Options().withTargetScreenArea(screenArea);
			menuToDisplay.showMenuAsync(options, [callback](int result) {
				if (callback)
					callback(result);
			});
		}
		static void showMenuAsyncAtArea(juce::PopupMenu &menuToDisplay, const juce::Rectangle<int> &screenArea,
										juce::Component *targetComponent, std::function<void(int)> callback) {
			// Adding 2-4 pixels to the Y position moves the anchor slightly below the cursor
			juce::Rectangle<int> adjustedArea = screenArea.translated(0, 4);

			auto options = juce::PopupMenu::Options().withTargetScreenArea(adjustedArea);

			if (targetComponent != nullptr) {
				juce::Component::SafePointer<juce::Component> safeTarget(targetComponent);
				if (safeTarget != nullptr) {
					options = options.withParentComponent(safeTarget.getComponent());
				}
			}

			menuToDisplay.showMenuAsync(options, [callback](int result) {
				if (callback)
					callback(result);
			});
		}
		// =========================================================================
		// ASYNCHRONOUS HELPERS (For modern C++ / Lua callbacks)
		// =========================================================================

		/** Safely shows a popup menu asynchronously */
		static void showMenuAsyncSafe(juce::PopupMenu &menuToDisplay, juce::Component *targetComponent,
									  std::function<void(int)> callback) {
			auto options = juce::PopupMenu::Options();
			if (targetComponent != nullptr) {
				juce::Component::SafePointer<juce::Component> safeTarget(targetComponent);
				options = options.withTargetComponent(safeTarget.getComponent());
			}

			menuToDisplay.showMenuAsync(options, [callback](int result) {
				if (callback)
					callback(result);
			});
		}
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
	menu.showMenuAsync(options, [callback](int result) {
		if (callback != nullptr)
			callback(result);
	});
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

	// --- Modern JUCE 7/8 Path (Asynchronous) ---
	auto juce8NativeIcon = (icon == Question)  ? juce::MessageBoxIconType::QuestionIcon
						   : (icon == Warning) ? juce::MessageBoxIconType::WarningIcon
						   : (icon == Info)	   ? juce::MessageBoxIconType::InfoIcon
											   : juce::MessageBoxIconType::NoIcon;

	if (isOkCancel) {
		juce::NativeMessageBox::showOkCancelBox(juce8NativeIcon, title, bodyText, nullptr,
												juce::ModalCallbackFunction::create([completionCallback](int result) {
													if (completionCallback != nullptr) {
														completionCallback(result == 1);
													}
												}));
	} else {
		juce::NativeMessageBox::showMessageBoxAsync(juce8NativeIcon, title, bodyText, nullptr,
													juce::ModalCallbackFunction::create([completionCallback](int) {
														if (completionCallback != nullptr) {
															completionCallback(true);
														}
													}));
	}
	return false;
}

/**************************************************************************************************/
static void showMessageBox(Icon icon, const juce::String &title, const juce::String &message,
						   const juce::String &buttonText = "OK", std::function<void()> callback = nullptr) {

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
}

/**************************************************************************************************/
static void showOkCancelAsyncSafe(Icon icon, const juce::String &title, const juce::String &bodyText,
								  std::function<void(bool)> completionCallback, const juce::String &button1Text = "Yes",
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

}; // namespace AW
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

	auto chooser =
		std::make_shared<juce::FileChooser>(dialogTitle, initialFileOrDirectory, filePatternsAllowed, useNativeDialog);
	chooser->launchAsync(flags, [chooser, callback](const juce::FileChooser &fc) {
		if (callback) {
			callback(fc.getResult());
		}
	});
}
/**************************************************************************************************/
inline void openFileAsync(const String &dialogTitle, const File &initialFileOrDirectory,
						  const String &filePatternsAllowed, bool useNativeDialog,
						  std::function<void(const File &)> callback) {
	int flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

	auto chooser =
		std::make_shared<juce::FileChooser>(dialogTitle, initialFileOrDirectory, filePatternsAllowed, useNativeDialog);

	chooser->launchAsync(flags, [chooser, callback](const juce::FileChooser &fc) {
		if (callback) {
			callback(fc.getResult());
		}
	});
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

	auto chooser =
		std::make_shared<juce::FileChooser>(dialogTitle, initialFileOrDirectory, filePatternsAllowed, useNativeDialog);
	chooser->launchAsync(flags, [chooser, callback](const juce::FileChooser &fc) {
		if (callback) {
			callback(fc.getResults());
		}
	});
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
} // namespace gui

namespace LNF {

struct ColourMapping {
		juce::Identifier treePropId;
		int juceColourId;
};

// --- Overload 1: Flexible version using initializer list (Great for Sliders with many colors) ---
inline void applyLookAndFeelState(juce::Component &targetComp, juce::ValueTree &ownerTree,
								  const juce::Identifier &customFlagId, std::initializer_list<ColourMapping> mappings) {
	const bool useUserSettings = !(bool)ownerTree.getProperty(customFlagId);

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
		targetComp.sendLookAndFeelChange();
	}

	targetComp.repaint();
}

// --- Overload 2: Positional 11-argument version (Fixes CtrlrButton compilation error) ---
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

#endif