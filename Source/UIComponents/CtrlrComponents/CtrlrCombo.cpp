#include "CtrlrCombo.h"
#include "CtrlrFontManager.h"
#include "CtrlrIDs.h"
#include "CtrlrModulator/CtrlrModulator.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrUtilitiesGUI.h"
#include "CtrlrValueMap.h"
#include "JuceClasses/LLookAndFeel.h"
#include "stdafx.h"
#include <algorithm>
#include <rapidfuzz/fuzz.hpp> // Added v5.6.35. Support for rapidfuzz

CtrlrCombo::CtrlrCombo(CtrlrModulator &owner)
	: CtrlrComponent(owner), lf(*this), ctrlrCombo(nullptr), isUpdating(false), isSearching(false) {

	valueMap = std::make_unique<CtrlrValueMap>();
	ctrlrCombo = std::make_unique<juce::ComboBox>("ctrlrCombo");
	addAndMakeVisible(ctrlrCombo.get());

	ctrlrCombo->setEditableText(false);
	ctrlrCombo->setJustificationType(Justification::centred);
	ctrlrCombo->addListener(this);

	// 1. Default component tree properties
	setProperty(Ids::uiComboSearch, false);
	setProperty(Ids::uiComboButtonWidthOverride, false);
	setProperty(Ids::uiComboButtonWidth, 16);
	setProperty(Ids::uiComboDynamicContent, 0);
	setProperty(Ids::uiComboSelectedId, -1);
	setProperty(Ids::uiComboSelectedIndex, -1);
	setProperty(Ids::uiComboContent, "");

	setProperty(Ids::uiButtonLookAndFeelIsCustom, false);

	// Keep custom double-arrow renderer attached
	ctrlrCombo->setLookAndFeel(&lf);

	// Set consistent footprint
	setSize(120, 40);

	// 2. Determine LookAndFeel style
	String comboStyle = getProperty(Ids::uiButtonLookAndFeel).toString();
	if (comboStyle.isEmpty()) {
		comboStyle = "V3";
		setProperty(Ids::uiButtonLookAndFeel, "V3");
	}
	if (comboStyle == "Default") { // this ensures the theme changes with panel theme unless set to a specific style
		comboStyle = owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel).toString();
	}
	if (comboStyle == "V3" || comboStyle == "V2" || comboStyle == "V1") {
		setProperty(Ids::uiComboArrowColour, "0xff000000");
		setProperty(Ids::uiComboOutlineColour, "0xff808080");
		setProperty(Ids::uiComboTextJustification, "centred");
		setProperty(Ids::uiComboFont, FONT2STR(Font(14)));
		setProperty(Ids::uiComboTextColour, "0xff000000");
		setProperty(Ids::uiComboMenuFont, FONT2STR(Font(16)));
		setProperty(Ids::uiComboMenuFontColour, "0xff000000");
		setProperty(Ids::uiComboButtonColour, "0xffe0e0e0"); // Soft light gray instead of bright blue
		setProperty(Ids::uiComboBgColour, "0xffffffff");
		setProperty(Ids::uiComboMenuBackgroundColour, "0xfff0f0f0");
		setProperty(Ids::uiComboMenuHighlightColour, Colours::lightblue.toString());
		setProperty(Ids::uiComboMenuFontHighlightedColour, "0xff232323");
		setProperty(Ids::uiComboMenuBackgroundRibbed, true);
	} else {
		// V4 Panel Themes (LexiBlue, YamDX, Dark, Light, etc.)
		String panelLnF = owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel).toString();
		if (panelLnF.isEmpty())
			panelLnF = comboStyle;

		applyCentralLookAndFeel(ctrlrCombo.get(), panelLnF);

		setProperty(Ids::uiComboArrowColour, (String)ctrlrCombo->findColour(ComboBox::arrowColourId).toString());
		setProperty(Ids::uiComboOutlineColour, (String)ctrlrCombo->findColour(ComboBox::outlineColourId).toString());
		setProperty(Ids::uiComboTextJustification, "centred");
		setProperty(Ids::uiComboFont, FONT2STR(Font(14)));
		setProperty(Ids::uiComboTextColour, (String)ctrlrCombo->findColour(ComboBox::textColourId).toString());
		setProperty(Ids::uiComboMenuFont, FONT2STR(Font(16)));
		setProperty(Ids::uiComboMenuFontColour, (String)ctrlrCombo->findColour(ComboBox::textColourId).toString());
		setProperty(Ids::uiComboButtonColour, (String)ctrlrCombo->findColour(ComboBox::buttonColourId).toString());
		setProperty(Ids::uiComboBgColour, (String)ctrlrCombo->findColour(ComboBox::backgroundColourId).toString());
		setProperty(Ids::uiComboMenuBackgroundColour,
					(String)ctrlrCombo->findColour(ComboBox::backgroundColourId).toString());
		setProperty(Ids::uiComboMenuHighlightColour,
					(String)ctrlrCombo->findColour(PopupMenu::highlightedBackgroundColourId).toString());
		setProperty(Ids::uiComboMenuFontHighlightedColour,
					(String)ctrlrCombo->findColour(PopupMenu::highlightedTextColourId).toString());
		setProperty(Ids::uiComboMenuBackgroundRibbed, false);
	}

	setProperty(Ids::uiComboButtonGradient, false); // Disable force-gradient to keep theme button color clean

	// 3. Sync colors onto internal ComboBox
	ctrlrCombo->setColour(ComboBox::backgroundColourId, VAR2COLOUR(getProperty(Ids::uiComboBgColour)));
	ctrlrCombo->setColour(ComboBox::textColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
	ctrlrCombo->setColour(ComboBox::buttonColourId, VAR2COLOUR(getProperty(Ids::uiComboButtonColour)));
	ctrlrCombo->setColour(ComboBox::outlineColourId, VAR2COLOUR(getProperty(Ids::uiComboOutlineColour)));
	ctrlrCombo->setColour(ComboBox::arrowColourId, VAR2COLOUR(getProperty(Ids::uiComboArrowColour)));

	componentTree.addListener(this);
}
CtrlrCombo::~CtrlrCombo() {
	// Don't leave a live popup pointing back at a CtrlrCombo that's being destroyed.
	closeFuzzySearchPopupIfOpen();

	if (ctrlrCombo != nullptr) {
		// Unlink from the shared look and feel pipeline before tearing down components
		ctrlrCombo->setLookAndFeel(nullptr);
	}
}

void CtrlrCombo::resized() {
	if (restoreStateInProgress)
		return;

	ctrlrCombo->setBounds(getUsableRect());
}

void CtrlrCombo::mouseDown(const MouseEvent &e) {
	if (canPerformFuzzySearch()) {
		openFuzzySearchPopup();
		return;
	}
	CtrlrComponent::mouseDown(e);
}

bool CtrlrCombo::keyPressed(const KeyPress &key) {
	// Fuzzy search has its own self-contained popup (FuzzySearchPanel) with its
	// own key handling now, so there's nothing search-specific to do here any more.
	// Fallback to the Legacy Canvas behavior
	if (getParentComponent()) {
		if (auto *canvas = dynamic_cast<CtrlrPanelCanvas *>(getParentComponent())) {
			return canvas->keyPressed(key, this);
		}
	}

	return false;
}

void CtrlrCombo::visibilityChanged() {
	if (ctrlrCombo) {
		_DBG("GUI_TRACE [" + owner.getName() + "] visibilityChanged ENTER | isVisible: " + String((int)isVisible()) +
			 " | UI Index: " + String(ctrlrCombo->getSelectedItemIndex()) + " | UI Text: '" + ctrlrCombo->getText() +
			 "'");
	}

	if (isVisible() && (bool)getProperty(Ids::uiComboSearch)) {
		_DBG("LIFECYCLE: Component visible. Starting 250ms safety timer...");
		startTimer(250);
	}
}

void CtrlrCombo::focusLost(FocusChangeType cause) {
	if (ctrlrCombo) {
		ctrlrCombo->hidePopup();
	}
}

void CtrlrCombo::lookAndFeelChanged() {
	if (ctrlrCombo) {
		_DBG("GUI_TRACE [" + owner.getName() + "] lookAndFeelChanged ENTER | Current Index: " +
			 String(ctrlrCombo->getSelectedItemIndex()) + " | Text: '" + ctrlrCombo->getText() + "'");
	}

	CtrlrComponent::lookAndFeelChanged();

	if (getProperty(Ids::uiComboSearch)) {
		_DBG("LIFECYCLE: LookAndFeel changed. This usually wipes the Label state. Re-attaching...");
		startTimer(100);
	}

	if (ctrlrCombo) {
		_DBG("GUI_TRACE [" + owner.getName() + "] lookAndFeelChanged EXIT | Current Index: " +
			 String(ctrlrCombo->getSelectedItemIndex()) + " | Text: '" + ctrlrCombo->getText() + "'");
	}
}

void CtrlrCombo::parentHierarchyChanged() {
	if (ctrlrCombo) {
		_DBG("GUI_TRACE [" + owner.getName() + "] parentHierarchyChanged ENTER | UI Index: " +
			 String(ctrlrCombo->getSelectedItemIndex()) + " | UI Text: '" + ctrlrCombo->getText() + "'");
	}

	if (ctrlrCombo) {
		_DBG("GUI_TRACE [" + owner.getName() + "] parentHierarchyChanged EXIT | UI Index: " +
			 String(ctrlrCombo->getSelectedItemIndex()) + " | UI Text: '" + ctrlrCombo->getText() + "'");
	}

	if (isVisible() && getWidth() > 0 && getHeight() > 0) {
		startTimer(50);
	}
}

void CtrlrCombo::timerCallback() {
	stopTimer();

	if (ctrlrCombo) {
		_DBG("GUI_TRACE [" + owner.getName() + "] timerCallback FIRED | UI Index: " +
			 String(ctrlrCombo->getSelectedItemIndex()) + " | UI Text: '" + ctrlrCombo->getText() + "'");
	}

	const bool isInEditMode = owner.getOwnerPanel().getEditMode();

	if (isInEditMode) {
		_DBG("LIFECYCLE: Cleaning up Search for Edit Mode...");
		closeFuzzySearchPopupIfOpen();
	} else {
		// 1. ALWAYS restore the complete list when in user mode,
		// regardless of whether search is enabled/disabled or was filtered previously.
		if (ctrlrCombo != nullptr && valueMap != nullptr) {
			_DBG("GUI_TRACE [" + owner.getName() + "] timerCallback | Refilling Combo list");
			valueMap->fillCombo(*ctrlrCombo, true);
		}

		// Fuzzy search no longer relies on ctrlrCombo's own editable-text/Label -
		// it's a self-contained FuzzySearchPanel opened on demand from mouseDown(),
		// so there's nothing to re-attach here.
	}

	if (ctrlrCombo != nullptr && !isInEditMode) {
		const double modulatorValue = owner.getProcessor().getValue();
		_DBG("GUI_SYNC [" + owner.getName() + "] Re-applying processor value: " + String(modulatorValue));
		ctrlrCombo->setSelectedId(modulatorValue + 1, dontSendNotification);
	}

	if (ctrlrCombo) {
		_DBG("GUI_TRACE [" + owner.getName() + "] timerCallback EXIT | Final UI Index: " +
			 String(ctrlrCombo->getSelectedItemIndex()) + " | Final UI Text: '" + ctrlrCombo->getText() + "'");
	}
}

void CtrlrCombo::comboBoxChanged(ComboBox *comboBoxThatHasChanged) {
	if (isUpdating)
		return;
	if (comboBoxThatHasChanged == ctrlrCombo.get()) {
		const int id = ctrlrCombo->getSelectedId();
		if (id > 0) {
			isSearching = false;
			setComponentValue(id - 1, true);
			ctrlrCombo->setEditableText(false);
		}
	}
}

double CtrlrCombo::getComponentMaxValue() {
	return (valueMap->getNonMappedMax());
}

double CtrlrCombo::getComponentValue() {
	return (ctrlrCombo->getSelectedId() - 1);
}

int CtrlrCombo::getComponentMidiValue() {
	return (valueMap->getMappedValue(ctrlrCombo->getSelectedId() - 1));
}

const String CtrlrCombo::getComponentText() {
	return (ctrlrCombo->getText());
}

void CtrlrCombo::setComponentValue(const double newValue, const bool sendChangeMessage) {
	_DBG("GUI_TRACE [" + owner.getName() + "] setComponentValue START | newValue: " + String(newValue));

	if (ctrlrCombo) {
		_DBG("   -> Current Item Count in List: " + String(ctrlrCombo->getNumItems()));
		ctrlrCombo->setSelectedId(newValue + 1, sendChangeMessage ? sendNotificationSync : dontSendNotification);

		_DBG("GUI_TRACE [" + owner.getName() + "] Post-Selection | UI Index: " +
			 String(ctrlrCombo->getSelectedItemIndex()) + " | UI Text: '" + ctrlrCombo->getText() + "'");

		if (ctrlrCombo->getSelectedItemIndex() == -1 && newValue != -1) {
			_DBG("GUI_TRACE [" + owner.getName() + "] !! WARNING: ComboBox rejected Value " + String(newValue));
		}
	}

	if (sendChangeMessage) {
		owner.getProcessor().setValueGeneric(CtrlrModulatorValue(newValue, CtrlrModulatorValue::changedByGUI),
											 sendChangeMessage);
	}

	_DBG("GUI_TRACE [" + owner.getName() + "] setComponentValue END");
}

void CtrlrCombo::comboContentChanged() {
	if ((int)getProperty(Ids::uiComboDynamicContent) > 0)
		return;

	valueMap->copyFrom(owner.getProcessor().setValueMap(getProperty(Ids::uiComboContent)));
	valueMap->fillCombo(*ctrlrCombo, true);
}

void CtrlrCombo::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) {
	_DBG("PROP CHANGE: " + property.toString() + " = " + getProperty(property).toString());

	if (property == Ids::uiComboSelectedIndex || property == Ids::uiComboSelectedId) {
		_DBG("GUI_TRACE [" + owner.getName() + "] ValueTree Property Change: " + property.toString() +
			 " is now: " + treeWhosePropertyHasChanged.getProperty(property).toString());
	}

	if (property == Ids::uiComboContent) {
		comboContentChanged();
	} else if (property == Ids::uiComboSearch) {
		_DBG("PROP: uiComboSearch changed - updating mouse interception");
		if (ctrlrCombo != nullptr) {
			const bool allowComboClicks = !canPerformFuzzySearch();
			ctrlrCombo->setInterceptsMouseClicks(allowComboClicks, allowComboClicks);
		}
		startTimer(250);
	} else if (property == Ids::uiButtonLookAndFeel) {
		String comboStyle = getProperty(Ids::uiButtonLookAndFeel).toString();

		if (comboStyle == "V3" || comboStyle == "V2" || comboStyle == "V1") {
			ctrlrCombo->setLookAndFeel(&lf);
		} else {
			// Revert to central Panel LookAndFeel (V4 variants)
			ctrlrCombo->setLookAndFeel(nullptr);
		}

		// Force re-application of custom property colors over the new LookAndFeel defaults
		ctrlrCombo->setColour(ComboBox::backgroundColourId, VAR2COLOUR(getProperty(Ids::uiComboBgColour)));
		ctrlrCombo->setColour(ComboBox::textColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
		ctrlrCombo->setColour(ComboBox::buttonColourId, VAR2COLOUR(getProperty(Ids::uiComboButtonColour)));
		ctrlrCombo->setColour(ComboBox::outlineColourId, VAR2COLOUR(getProperty(Ids::uiComboOutlineColour)));
		ctrlrCombo->setColour(ComboBox::arrowColourId, VAR2COLOUR(getProperty(Ids::uiComboArrowColour)));

		updateInternalComponentStyles();
		ctrlrCombo->lookAndFeelChanged();
		ctrlrCombo->repaint();

	} else if (property == Ids::uiComboBgColour) {
		Colour c = VAR2COLOUR(getProperty(Ids::uiComboBgColour));
		ctrlrCombo->setColour(ComboBox::backgroundColourId, c);
		ctrlrCombo->setColour(TextEditor::backgroundColourId, c);
		ctrlrCombo->setColour(Label::backgroundColourId, c);

		updateInternalComponentStyles();
		ctrlrCombo->repaint();
	} else if (property == Ids::uiComboTextColour) {
		Colour c = VAR2COLOUR(getProperty(Ids::uiComboTextColour));
		ctrlrCombo->setColour(ComboBox::textColourId, c);
		ctrlrCombo->setColour(TextEditor::textColourId, c);
		ctrlrCombo->setColour(Label::textColourId, c);

		updateInternalComponentStyles();
		ctrlrCombo->repaint();
	} else if (property == Ids::uiComboButtonColour) {
		ctrlrCombo->setColour(ComboBox::buttonColourId, VAR2COLOUR(getProperty(Ids::uiComboButtonColour)));
		ctrlrCombo->repaint();
	} else if (property == Ids::uiComboTextColour) {
		ctrlrCombo->setColour(ComboBox::textColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
		ctrlrCombo->setColour(TextEditor::textColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
		ctrlrCombo->setColour(TextEditor::highlightedTextColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
		ctrlrCombo->setColour(Label::textColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
		ctrlrCombo->setColour(Label::textWhenEditingColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));

		updateInternalComponentStyles();
		ctrlrCombo->repaint();
	} else if (property == Ids::uiComboOutlineColour) {
		ctrlrCombo->setColour(ComboBox::outlineColourId, VAR2COLOUR(getProperty(Ids::uiComboOutlineColour)));
		ctrlrCombo->repaint();
	} else if (property == Ids::uiComboArrowColour) {
		ctrlrCombo->setColour(ComboBox::arrowColourId, VAR2COLOUR(getProperty(Ids::uiComboArrowColour)));
		ctrlrCombo->repaint();
	} else if (property == Ids::uiComboTextJustification) {
		ctrlrCombo->setJustificationType(justificationFromProperty(getProperty(property)));
		ctrlrCombo->repaint();
	} else if (property == Ids::uiComboButtonWidthOverride || property == Ids::uiComboButtonWidth) {
		ctrlrCombo->resized();
		ctrlrCombo->repaint();
	} else if (property == Ids::uiComboFont || property == Ids::uiComboMenuBackgroundColour ||
			   property == Ids::uiComboMenuFont || property == Ids::uiComboMenuFontColour ||
			   property == Ids::uiComboMenuHighlightColour || property == Ids::uiComboMenuFontHighlightedColour ||
			   property == Ids::uiComboButtonGradientColour1 || property == Ids::uiComboButtonGradientColour2) {

		if (property == Ids::uiComboFont) {
			if (auto *label = dynamic_cast<juce::Label *>(ctrlrCombo->findChildWithID("label"))) {
				Font f = owner.getOwnerPanel().getCtrlrManagerOwner().getFontManager().getFontFromString(
					getProperty(Ids::uiComboFont));
				label->setFont(f);
			}
		}
		updateInternalComponentStyles();
		ctrlrCombo->repaint();
		repaint();
	} else if (property == Ids::uiComboDynamicContent) {
		fillContent(getProperty(property));
	} else if (property == Ids::uiComboSelectedId) {
		if ((int)getProperty(property) != -1) {
			ctrlrCombo->setSelectedId(getProperty(property), sendNotificationSync);
		}
	} else if (property == Ids::uiComboSelectedIndex) {
		if ((int)getProperty(property) != -1) {
			ctrlrCombo->setSelectedItemIndex(getProperty(property), sendNotificationSync);
		}
	} else if (property.toString().startsWith("uiCombo")) {
		if (ctrlrCombo && (bool)getProperty(Ids::uiComboSearch)) {
			_DBG("STYLE_CHANGE: " + property.toString() + " - Resetting engine.");
			startTimer(250);
		} else {
			updateInternalComponentStyles();
			ctrlrCombo->repaint();
		}
	} else {
		CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
	}

	if (!restoreStateInProgress) {
		resized();
	}
}

void CtrlrCombo::setComponentText(const String &componentText) {
	ctrlrCombo->setText(componentText);
}

void CtrlrCombo::fillContent(const int contentType) {
	Array<File> files;
	const String prewviousContent = ctrlrCombo->getText();

	switch (contentType) {
	case 1:
		for (int i = 0; i < owner.getOwnerPanel().getModulators().size(); i++) {
			valueMap->setPair(i, i, owner.getOwnerPanel().getModulatorByIndex(i)->getName());
		}
		owner.getProcessor().setValueMap(*valueMap);
		valueMap->fillCombo(*ctrlrCombo, true);
		ctrlrCombo->setText(prewviousContent, dontSendNotification);
		break;

	case 2:
		File::findFileSystemRoots(files);
		for (int i = 0; i < files.size(); i++) {
			valueMap->setPair(i, i, files[i].getFullPathName());
		}
		owner.getProcessor().setValueMap(*valueMap);
		valueMap->fillCombo(*ctrlrCombo, true);
		ctrlrCombo->setText(prewviousContent, dontSendNotification);
		break;
	default:
		comboContentChanged();
		break;
	}
}
void CtrlrCombo::panelEditModeChanged(const bool isInEditMode) {
	_DBG("!!!! Combo Edit Mode: " + String(isInEditMode ? "ON" : "OFF"));

	if (ctrlrCombo != nullptr && ctrlrCombo->isPopupActive()) {
		ctrlrCombo->hidePopup();
	}

	if (isInEditMode)
		closeFuzzySearchPopupIfOpen();

	if (ctrlrCombo != nullptr) {
		// IF fuzzy search is enabled in user mode, pass clicks THROUGH to CtrlrCombo
		const bool allowComboClicks = !isInEditMode && !canPerformFuzzySearch();
		ctrlrCombo->setInterceptsMouseClicks(allowComboClicks, allowComboClicks);
	}

	startTimer(isInEditMode ? 50 : 200);

	if ((bool)owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelDisabledOnEdit)) {
		if (ctrlrCombo != nullptr)
			ctrlrCombo->setEnabled(!isInEditMode);
	}

	resized();
}

int CtrlrCombo::getSelectedId() {
	return (ctrlrCombo->getSelectedId());
}

int CtrlrCombo::getSelectedItemIndex() {
	return (ctrlrCombo->getSelectedItemIndex());
}

void CtrlrCombo::setSelectedId(const int id, const bool dontNotify) {
	ctrlrCombo->setSelectedId(id, dontNotify ? dontSendNotification : sendNotificationSync);
}

void CtrlrCombo::setSelectedItemIndex(const int index, const bool dontNotify) {
	ctrlrCombo->setSelectedItemIndex(index, dontNotify ? dontSendNotification : sendNotificationSync);
}

const String CtrlrCombo::getText() {
	return (ctrlrCombo->getText());
}

void CtrlrCombo::setText(const String &text, const bool dontNotify) {
	return (ctrlrCombo->setText(text, dontNotify ? dontSendNotification : sendNotificationSync));
}

std::unique_ptr<juce::LookAndFeel>
CtrlrCombo::getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty) {
	if (lookAndFeelComponentProperty == "Default") {
		return nullptr;
	}

	return gui::createLookAndFeelFromDescription(lookAndFeelComponentProperty, false);
}

void CtrlrCombo::resetLookAndFeelOverrides() {
	_DBG("RESET_LF: Start - restoreStateInProgress = " + String(restoreStateInProgress ? "TRUE" : "FALSE"));

	if (restoreStateInProgress == false) {
		_DBG("RESET_LF: Overriding UI properties from LookAndFeel defaults");
		_DBG("RESET_LF: Current Text Colour: " + findColour(ComboBox::textColourId).toDisplayString(true));

		setProperty(Ids::componentLabelColour, (String)findColour(Label::textColourId).toString());

		setProperty(Ids::uiComboArrowColour, (String)findColour(ComboBox::arrowColourId).toString());
		setProperty(Ids::uiComboOutlineColour, (String)findColour(ComboBox::outlineColourId).darker(0.5f).toString());

		setProperty(Ids::uiComboTextColour, (String)findColour(ComboBox::textColourId).toString());
		setProperty(Ids::uiComboMenuFontColour, (String)findColour(ComboBox::textColourId).toString());

		setProperty(Ids::uiComboButtonColour, (String)findColour(ComboBox::buttonColourId).toString());
		setProperty(Ids::uiComboBgColour, (String)findColour(ComboBox::backgroundColourId).toString());

		setProperty(Ids::uiComboMenuBackgroundColour, (String)findColour(ComboBox::backgroundColourId).toString());

		setProperty(Ids::uiComboMenuHighlightColour, (String)findColour(TextEditor::highlightColourId).toString());
		setProperty(Ids::uiComboMenuFontHighlightedColour,
					(String)findColour(TextEditor::highlightedTextColourId).toString());

		setProperty(Ids::uiComboButtonGradientColour1, (String)findColour(TextButton::buttonColourId).toString());
		setProperty(Ids::uiComboButtonGradientColour2,
					(String)findColour(TextButton::buttonColourId).darker(0.2f).toString());

		setProperty(Ids::uiComboArrowColour, (String)findColour(ComboBox::arrowColourId).toString());
		setProperty(Ids::uiComboLookAndFeelIsCustom, false);

		_DBG("RESET_LF: End");

		updatePropertiesPanel();
	}
}

void CtrlrCombo::updatePropertiesPanel() {
	CtrlrPanelProperties *props = owner.getCtrlrManagerOwner().getActivePanel()->getEditor(false)->getPropertiesPanel();
	if (props) {
		props->refreshAll();
	}
}

void CtrlrCombo::updateInternalComponentStyles() {
	_DBG("--- updateInternalComponentStyles ---");
	if (ctrlrCombo == nullptr)
		return;

	String comboStyle = getProperty(Ids::uiButtonLookAndFeel).toString();

	if (comboStyle.isEmpty() || comboStyle == "Default") {
		comboStyle = owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel).toString();
	}

	// Always enforce the local LookAndFeel object so double-arrow drawing is preserved
	ctrlrCombo->setLookAndFeel(&lf);

	// if (comboStyle != "V3" && comboStyle != "V2" && comboStyle != "V1") {
	// 	applyCentralLookAndFeel(ctrlrCombo.get(), comboStyle);
	// 	return;
	// }
	/* Here we can apply LNF colouring to uiCombo*/
	if (comboStyle != "V3" && comboStyle != "V2" && comboStyle != "V1") {
		applyCentralLookAndFeel(ctrlrCombo.get(), comboStyle);
		auto scheme = gui::colourSchemeFromProperty(comboStyle);

		Colour highlight = scheme.getUIColour(LookAndFeel_V4::ColourScheme::UIColour::highlightedFill);
		Colour highlightText = highlight.getPerceivedBrightness() < 0.5f ? Colours::white : Colours::black;

		Colour menuBackground = scheme.getUIColour(LookAndFeel_V4::ColourScheme::UIColour::menuBackground);
		Colour menuText = menuBackground.getPerceivedBrightness() < 0.5f ? Colours::white : Colours::black;

		setProperty(Ids::uiComboMenuHighlightColour, highlight.toString());
		setProperty(Ids::uiComboMenuFontHighlightedColour, highlightText.toString());
		setProperty(Ids::uiComboMenuBackgroundColour, menuBackground.toString());
		setProperty(Ids::uiComboMenuFontColour, menuText.toString());
		return;
	}
	// Standard fallback logic for manual V1/V2/V3 color properties
	const Colour bg = VAR2COLOUR(getProperty(Ids::uiComboBgColour));
	const Colour txt = VAR2COLOUR(getProperty(Ids::uiComboTextColour));

	ctrlrCombo->setColour(ComboBox::backgroundColourId, bg);
	ctrlrCombo->setColour(ComboBox::textColourId, txt);
	ctrlrCombo->setColour(ComboBox::buttonColourId, VAR2COLOUR(getProperty(Ids::uiComboButtonColour)));

	for (int i = 0; i < ctrlrCombo->getNumChildComponents(); ++i) {
		auto *child = ctrlrCombo->getChildComponent(i);

		if (auto *lb = dynamic_cast<juce::Label *>(child)) {
			lb->setColour(Label::backgroundColourId, Colours::transparentBlack);
			lb->setColour(Label::textColourId, txt);
		}

		if (auto *ed = dynamic_cast<juce::TextEditor *>(child)) {
			ed->setColour(TextEditor::backgroundColourId, bg);
			ed->setColour(TextEditor::textColourId, txt);
			ed->setColour(TextEditor::highlightColourId, txt.withAlpha(0.2f));
		}
	}

	ctrlrCombo->repaint();
}

//==============================================================================
// CtrlrCombo - fuzzy search popup open/close
//==============================================================================
void CtrlrCombo::openFuzzySearchPopup() {
	if (ctrlrCombo == nullptr || valueMap == nullptr)
		return;
	if (activeSearchPanel != nullptr) // already open
		return;

	_DBG("FUZZY: Opening search popup");

	auto panel = std::make_unique<FuzzySearchPanel>(*this);
	activeSearchPanel = panel.get();

	auto &box = juce::CallOutBox::launchAsynchronously(std::move(panel), ctrlrCombo->getScreenBounds(), nullptr);
	box.setDismissalMouseClicksAreAlwaysConsumed(true);

	isSearching = true;

	if (activeSearchPanel != nullptr)
		activeSearchPanel->focusSearchField();
}

void CtrlrCombo::closeFuzzySearchPopupIfOpen() {
	if (activeSearchPanel == nullptr)
		return;

	if (auto *box = activeSearchPanel->findParentComponentOfClass<juce::CallOutBox>())
		box->dismiss();

	isSearching = false;

	// Search only ever opens in normal (non-edit) mode, so it's always correct
	// to hand normal click-handling back to the ComboBox here.
	if (ctrlrCombo != nullptr)
		ctrlrCombo->setInterceptsMouseClicks(true, true);
}

//==============================================================================
// CtrlrCombo::FuzzySearchPanel
//==============================================================================
CtrlrCombo::FuzzySearchPanel::FuzzySearchPanel(CtrlrCombo &ownerCombo) : owner(ownerCombo) {
	const Colour bg = VAR2COLOUR(owner.getProperty(Ids::uiComboBgColour));
	const Colour txt = VAR2COLOUR(owner.getProperty(Ids::uiComboTextColour));

	addAndMakeVisible(searchBox);
	searchBox.setMultiLine(false);
	searchBox.setReturnKeyStartsNewLine(false);
	searchBox.setSelectAllWhenFocused(true);
	searchBox.setColour(juce::TextEditor::backgroundColourId, bg);
	searchBox.setColour(juce::TextEditor::textColourId, txt);
	searchBox.setColour(juce::TextEditor::highlightColourId, txt.withAlpha(0.25f));
	searchBox.setColour(juce::TextEditor::highlightedTextColourId, txt);
	searchBox.setFont(owner.getOwner().getOwnerPanel().getOwner().getFontManager().getFontFromString(
		owner.getProperty(Ids::uiComboFont)));
	searchBox.addListener(this);
	searchBox.addKeyListener(this);

	addAndMakeVisible(resultsList);
	resultsList.setModel(this);
	resultsList.setColour(juce::ListBox::backgroundColourId,
						  VAR2COLOUR(owner.getProperty(Ids::uiComboMenuBackgroundColour)));
	resultsList.setRowHeight(22);
	resultsList.setMultipleSelectionEnabled(false);
	resultsList.setWantsKeyboardFocus(false);
	resultsList.setMouseClickGrabsKeyboardFocus(false);

	const int width = jmax(220, owner.getOwnedComboBox()->getWidth());
	setSize(width, 60);

	refreshMatches();
}

CtrlrCombo::FuzzySearchPanel::~FuzzySearchPanel() {
	owner.activeSearchPanel = nullptr;
	owner.isSearching = false;

	if (owner.ctrlrCombo != nullptr)
		owner.ctrlrCombo->setInterceptsMouseClicks(true, true);
}

void CtrlrCombo::FuzzySearchPanel::resized() {
	auto area = getLocalBounds().reduced(4);
	searchBox.setBounds(area.removeFromTop(26));
	area.removeFromTop(4);
	resultsList.setBounds(area);
}

void CtrlrCombo::FuzzySearchPanel::visibilityChanged() {
	if (isVisible())
		focusSearchField();
}

void CtrlrCombo::FuzzySearchPanel::focusSearchField() {
	searchBox.grabKeyboardFocus();
	searchBox.moveCaretToEnd();
}

void CtrlrCombo::FuzzySearchPanel::paintOverChildren(juce::Graphics &g) {
	if (matches.empty() && searchBox.getText().isNotEmpty()) {
		g.setColour(VAR2COLOUR(owner.getProperty(Ids::uiComboMenuFontColour)).withAlpha(0.6f));
		g.setFont(searchBox.getFont());
		g.drawFittedText("(no matches)", resultsList.getBounds(), juce::Justification::centred, 1);
	}
}

void CtrlrCombo::FuzzySearchPanel::textEditorTextChanged(juce::TextEditor &) {
	refreshMatches();
}

void CtrlrCombo::FuzzySearchPanel::textEditorReturnKeyPressed(juce::TextEditor &) {
	int row = resultsList.getSelectedRow();
	if (row < 0 && !matches.empty())
		row = 0;
	commitRow(row);
}

void CtrlrCombo::FuzzySearchPanel::textEditorEscapeKeyPressed(juce::TextEditor &) {
	closePopup();
}

int CtrlrCombo::FuzzySearchPanel::getNumRows() {
	return (int)matches.size();
}

void CtrlrCombo::FuzzySearchPanel::paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height,
													bool rowIsSelected) {
	if (!isPositiveAndBelow(rowNumber, (int)matches.size()))
		return;

	const Colour bgColour = VAR2COLOUR(owner.getProperty(Ids::uiComboMenuBackgroundColour));
	const Colour textColour = VAR2COLOUR(owner.getProperty(Ids::uiComboMenuFontColour));
	const Colour hiColour = VAR2COLOUR(owner.getProperty(Ids::uiComboMenuHighlightColour));
	const Colour hiTextColour = VAR2COLOUR(owner.getProperty(Ids::uiComboMenuFontHighlightedColour));

	g.fillAll(rowIsSelected ? hiColour : bgColour);
	g.setColour(rowIsSelected ? hiTextColour : textColour);
	g.setFont(owner.getOwner().getOwnerPanel().getOwner().getFontManager().getFontFromString(
		owner.getProperty(Ids::uiComboMenuFont)));
	g.drawFittedText(matches[(size_t)rowNumber].text, 6, 0, width - 12, height, juce::Justification::centredLeft, 1);
}

void CtrlrCombo::FuzzySearchPanel::listBoxItemClicked(int row, const juce::MouseEvent &) {
	commitRow(row);
}

bool CtrlrCombo::FuzzySearchPanel::keyPressed(const juce::KeyPress &key, juce::Component *originatingComponent) {
	const int numRows = (int)matches.size();

	if (key == juce::KeyPress::downKey) {
		if (numRows > 0)
			resultsList.selectRow(jlimit(0, numRows - 1, resultsList.getSelectedRow() + 1));
		return true;
	}
	if (key == juce::KeyPress::upKey) {
		if (numRows > 0)
			resultsList.selectRow(jlimit(0, numRows - 1, resultsList.getSelectedRow() - 1));
		return true;
	}

	// Everything else (letters, Backspace, Delete, arrow-left/right, Home/End...) falls
	// straight through to the TextEditor untouched - this is the whole point of the
	// rewrite: the editor is never destroyed/recreated mid-keystroke.
	return false;
}

void CtrlrCombo::FuzzySearchPanel::refreshMatches() {
	matches.clear();

	auto &valueMap = owner.getValueMap();
	const String query = searchBox.getText();

	if (query.isEmpty()) {
		for (int i = 0; i < valueMap.getNumValues(); ++i)
			matches.push_back({i + 1, valueMap.getTextForIndex(i), 0.0});
	} else {
		const std::string queryStd = query.toLowerCase().toStdString();

		for (int i = 0; i < valueMap.getNumValues(); ++i) {
			const String itemText = valueMap.getTextForIndex(i);
			const double score = rapidfuzz::fuzz::WRatio(queryStd, itemText.toLowerCase().toStdString());

			if (score > 40.0) // cutoff - drop weak/noise matches
				matches.push_back({i + 1, itemText, score});
		}

		std::stable_sort(matches.begin(), matches.end(),
						 [](const Match &a, const Match &b) { return a.score > b.score; });
	}

	resultsList.updateContent();
	resultsList.selectRow(matches.empty() ? -1 : 0, false, true);

	const int rowsShown = jlimit(1, 8, jmax(1, (int)matches.size()));
	setSize(getWidth(), 34 + rowsShown * 22);

	repaint();
}

void CtrlrCombo::FuzzySearchPanel::commitRow(int row) {
	if (!isPositiveAndBelow(row, (int)matches.size()))
		return;

	owner.setSelectedId(matches[(size_t)row].id, false);
	closePopup();
}

void CtrlrCombo::FuzzySearchPanel::closePopup() {
	if (auto *box = findParentComponentOfClass<juce::CallOutBox>())
		box->dismiss();
}

void CtrlrCombo::CtrlrComboLF::drawPopupMenuBackground(Graphics &g, int width, int height) {
	const Colour background = VAR2COLOUR(owner.getProperty(Ids::uiComboMenuBackgroundColour));

	g.fillAll(background);

	if (owner.getProperty(Ids::uiComboMenuBackgroundRibbed)) {
		g.setColour(background.overlaidWith(Colour(0x2badd8e6)));

		for (int i = 0; i < height; i += 3)
			g.fillRect(0, i, width, 1);

#if !JUCE_MAC
		g.setColour(findColour(PopupMenu::textColourId).withAlpha(0.6f));
		g.drawRect(0, 0, width, height);
#endif
	}
}

void CtrlrCombo::CtrlrComboLF::drawPopupMenuItem(Graphics &g, const Rectangle<int> &area, bool isSeparator,
												 bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
												 const String &text, const String &shortcutKeyText,
												 const Drawable *icon, const Colour *textColourToUse) {
	const int width = area.getWidth();
	const int height = area.getHeight();

	const float halfH = height * 0.5f;

	if (isSeparator) {
		const float separatorIndent = 5.5f;

		g.setColour(findColour(ComboBox::textColourId).withAlpha(0.3f));
		g.drawLine(separatorIndent, halfH, width - separatorIndent, halfH);

		g.setColour(findColour(ComboBox::textColourId).withAlpha(0.6f));
		g.drawLine(separatorIndent, halfH + 1.0f, width - separatorIndent, halfH + 1.0f);
	} else {
		Colour textColour = VAR2COLOUR(owner.getProperty(Ids::uiComboMenuFontColour));

		if (textColourToUse != nullptr) {
			_DBG("Using passed in colour: " + textColourToUse->toString());
			textColour = *textColourToUse;
		}

		if (isHighlighted) {
			g.setColour(VAR2COLOUR(owner.getProperty(Ids::uiComboMenuHighlightColour)));
			g.fillRect(1, 1, width - 2, height - 2);

			g.setColour(VAR2COLOUR(owner.getProperty(Ids::uiComboMenuFontHighlightedColour)));
		} else {
			g.setColour(textColour);
		}

		if (!isActive)
			g.setOpacity(0.3f);

		Font font = owner.getOwner().getOwnerPanel().getCtrlrManagerOwner().getFontManager().getFontFromString(
			owner.getProperty(Ids::uiComboMenuFont));

		if (font.getHeight() > height / 1.3f)
			font.setHeight(height / 1.3f);

		g.setFont(font);

		const int leftBorder = (height * 5) / 4;
		const int rightBorder = 4;

		if (icon != nullptr) {
			icon->drawWithin(g, Rectangle<float>(2, 1, leftBorder - 4, height - 2),
							 RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
		} else if (isTicked) {
			const Path tick(getTickShape(1.0f));
			const float th = font.getAscent();
			const float ty = halfH - th * 0.5f;

			g.fillPath(tick, tick.getTransformToScaleToFit(2.0f, ty, (float)(leftBorder - 4), th, true));
		}

		g.drawFittedText(text, leftBorder, 0, width - (leftBorder + rightBorder), height, Justification::centredLeft,
						 1);

		if (shortcutKeyText.isNotEmpty()) {
			Font f2(font);
			f2.setHeight(f2.getHeight() * 0.75f);
			f2.setHorizontalScale(0.95f);
			g.setFont(f2);

			g.drawText(shortcutKeyText, leftBorder, 0, width - (leftBorder + rightBorder + 4), height,
					   Justification::centredRight, true);
		}

		if (hasSubMenu) {
			const float arrowH = 0.6f * getPopupMenuFont().getAscent();
			const float x = width - height * 0.6f;

			Path p;
			p.addTriangle(x, halfH - arrowH * 0.5f, x, halfH + arrowH * 0.5f, x + arrowH * 0.6f, halfH);

			g.fillPath(p);
		}
	}
}

void CtrlrCombo::applyComboLookAndFeel(const String &panelLnF) {
	ctrlrCombo->setLookAndFeel(&lf);

	if (panelLnF != "V3" && panelLnF != "V2" && panelLnF != "V1") {
		applyCentralLookAndFeel(ctrlrCombo.get(), panelLnF);
	}

	ctrlrCombo->lookAndFeelChanged();
	repaint();
}

void CtrlrCombo::CtrlrComboLF::drawComboBox(Graphics &g, int width, int height, bool isButtonDown, int buttonX,
											int buttonY, int buttonW, int buttonH, ComboBox &box) {
	// 1. Fetch the active panel theme name

	String comboStyle = owner.getProperty(Ids::uiButtonLookAndFeel).toString();
	if (comboStyle.isEmpty() || comboStyle == "Default") {
		comboStyle = owner.getOwner().getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel).toString();
	}

	// 2. If the active theme is "V4 Modern" / "Modern", fall back to JUCE's native V4 single-chevron renderer!
	if (comboStyle == "V4 Modern" || comboStyle == "Modern") {
		juce::LookAndFeel_V4::drawComboBox(g, width, height, isButtonDown, buttonX, buttonY, buttonW, buttonH, box);
		return;
	}

	// 3. Otherwise, draw the classic sharp Ctrlr box with double-arrows:
	_DBG("!!!! CtrlrComboLF::drawComboBox (Custom Double Arrows)");
	int bw = buttonW;
	int bx = buttonX;

	if ((bool)owner.getProperty(Ids::uiComboButtonWidthOverride) == true) {
		bw = owner.getProperty(Ids::uiComboButtonWidth);
		bx = width - bw;
	} else {
		// Default (non-override) button is square and scales with the box,
		// matching the legacy appearance shown in the old version.
		bw = buttonH;
		bx = width - bw;
	}
	const float outlineThickness = isButtonDown ? 1.2f : 0.5f;

	g.fillAll(box.findColour(ComboBox::backgroundColourId));
	g.setColour(box.findColour(ComboBox::outlineColourId));
	g.drawRect(0, 0, width, height);

	if ((bool)owner.getProperty(Ids::uiComboButtonWidthOverride) == true) {
		bw = jmax(1, (int)owner.getProperty(Ids::uiComboButtonWidth));
		bx = width - bw;
	}

	const float fillWidth = bw - outlineThickness * 2.0f;
	const float fillHeight = buttonH - outlineThickness * 2.0f;

	if (fillWidth > 0.0f && fillHeight > 0.0f) {
		const Colour baseColour(
			createBaseColour(box.findColour(ComboBox::buttonColourId), box.hasKeyboardFocus(true), false, isButtonDown)
				.withMultipliedAlpha(1.0f));

		if ((bool)owner.getProperty(Ids::uiComboButtonGradient) == true) {
			g.setGradientFill(ColourGradient(
				VAR2COLOUR(owner.getProperty(Ids::uiComboButtonGradientColour1)), buttonX + outlineThickness,
				buttonY + outlineThickness, VAR2COLOUR(owner.getProperty(Ids::uiComboButtonGradientColour2)),
				buttonX + outlineThickness, (buttonY + outlineThickness) + fillHeight, false));

			g.fillRect(buttonX + outlineThickness, buttonY + outlineThickness, fillWidth, fillHeight);
		} else {
			drawGlassLozenge(g, bx + outlineThickness, buttonY + outlineThickness, fillWidth, fillHeight, baseColour,
							 outlineThickness, -1.0f, true, true, true, true);
		}
	}

	const float arrowX = 0.3f;
	const float arrowH = 0.2f;

	// Use a single square region so the chevrons keep their proportions
	// regardless of the button's actual width/height aspect ratio.
	const float size = jmin((float)bw, (float)buttonH);
	const float offsetX = bx + (bw - size) * 0.5f;
	const float offsetY = buttonY + (buttonH - size) * 0.5f;

	Path p;
	p.addTriangle(offsetX + size * 0.5f, offsetY + size * (0.45f - arrowH), offsetX + size * (1.0f - arrowX),
				  offsetY + size * 0.45f, offsetX + size * arrowX, offsetY + size * 0.45f);

	p.addTriangle(offsetX + size * 0.5f, offsetY + size * (0.55f + arrowH), offsetX + size * (1.0f - arrowX),
				  offsetY + size * 0.55f, offsetX + size * arrowX, offsetY + size * 0.55f);

	g.setColour(box.findColour(ComboBox::arrowColourId));
	g.fillPath(p);
}

// --- CtrlrComboLF Implementations ---

const Colour CtrlrCombo::CtrlrComboLF::createBaseColour(const Colour &buttonColour, bool hasFocus, bool isMouseOver,
														bool isButtonDown) {
	if (isButtonDown)
		return buttonColour.darker(0.2f);
	if (isMouseOver)
		return buttonColour.brighter(0.1f);
	return buttonColour;
}

void CtrlrCombo::CtrlrComboLF::positionComboBoxText(juce::ComboBox &box, juce::Label &label) {
	int buttonWidth = box.getHeight(); // matches drawComboBox's default (square button)

	if ((bool)owner.getProperty(Ids::uiComboButtonWidthOverride) == true) {
		buttonWidth = owner.getProperty(Ids::uiComboButtonWidth);
	}

	label.setBounds(1, 1, box.getWidth() - buttonWidth - 2, box.getHeight() - 2);
	int justFlags = owner.getProperty(Ids::uiComboTextJustification);
	label.setJustificationType(juce::Justification(justFlags));
}

void CtrlrCombo::customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel) {
	// If you need custom LookAndFeel update logic when themes change, put it here.
	// E.g., redrawing or re-applying properties.
}

juce::Font CtrlrCombo::CtrlrComboLF::getComboBoxFont(juce::ComboBox &box) {
	return owner.getOwner().getOwnerPanel().getOwner().getFontManager().getFontFromString(
		owner.getProperty(Ids::uiComboFont));
}
juce::Font CtrlrCombo::CtrlrComboLF::getLabelFont(juce::Label &label) {
	return owner.getOwner().getOwnerPanel().getOwner().getFontManager().getFontFromString(
		owner.getProperty(Ids::uiComboFont));
}

juce::Font CtrlrCombo::CtrlrComboLF::getPopupMenuFont() {
	return owner.getOwner().getOwnerPanel().getOwner().getFontManager().getFontFromString(
		owner.getProperty(Ids::uiComboMenuFont));
}

bool CtrlrCombo::canPerformFuzzySearch() const {
	const bool isEnabledInInspector = (bool)getProperty(Ids::uiComboSearch);
	const bool isEditingLayout = owner.getOwnerPanel().getEditor()->getMode();

	// Search ONLY works if enabled in inspector AND NOT in edit mode
	return isEnabledInInspector && !isEditingLayout;
}