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
	if (ctrlrCombo != nullptr) {
		// Unlink from the shared look and feel pipeline before tearing down components
		ctrlrCombo->setLookAndFeel(nullptr);

		for (int i = 0; i < ctrlrCombo->getNumChildComponents(); ++i) {
			if (auto *lb = dynamic_cast<juce::Label *>(ctrlrCombo->getChildComponent(i))) {
				lb->onEditorShow = nullptr;
				if (searchListener != nullptr)
					lb->removeListener(searchListener.get());
			}
		}
	}

	if (searchListener != nullptr) {
		searchListener.reset();
	}
}

void CtrlrCombo::resized() {
	if (restoreStateInProgress)
		return;

	ctrlrCombo->setBounds(getUsableRect());
}

void CtrlrCombo::mouseDown(const MouseEvent &e) {
	if (getProperty(Ids::uiComboSearch)) {
		if (!isSearching) {
			_DBG("STARTING SEARCH MODE: Showing list + forcing caret");

			isUpdating = true;
			valueMap->fillCombo(*ctrlrCombo, true);
			isUpdating = false;

			// 1. Enable editing mode
			ctrlrCombo->setEditableText(true);

			// 2. Show the popup
			ctrlrCombo->showPopup();

			// 3. THE FIX: Find the internal editor and force it to show up NOW
			for (int i = 0; i < ctrlrCombo->getNumChildComponents(); ++i) {
				if (auto *lb = dynamic_cast<juce::Label *>(ctrlrCombo->getChildComponent(i))) {
					lb->showEditor(); // This creates the TextEditor component

					if (auto *ed = lb->getCurrentTextEditor()) {
						ed->grabKeyboardFocus();
						ed->setCaretVisible(true);
						ed->moveCaretToEnd();
					}
					break;
				}
			}
			return;
		}
	}
	CtrlrComponent::mouseDown(e);
}

bool CtrlrCombo::keyPressed(const KeyPress &key) {
	// 1. Handle the Fuzzy Search UI (New Logic)
	if (key == KeyPress::returnKey) {
		// If the menu is open and has items, select the first one
		if (ctrlrCombo->isPopupActive()) {
			ctrlrCombo->setSelectedItemIndex(0, true);
			ctrlrCombo->hidePopup();
			return true;
		}
	} else if (key == KeyPress::escapeKey) {
		// Cancel search: restore full list and clear text
		valueMap->fillCombo(*ctrlrCombo, true);
		ctrlrCombo->hidePopup();
		return true;
	}

	// 2. Fallback to the Legacy Canvas behavior
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

		if (getProperty(Ids::uiComboSearch)) {
			isSearching = false;
			ctrlrCombo->setEditableText(false);
		}
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

	if (getParentComponent() != nullptr && isSearching) {
		_DBG("LIFECYCLE: Component re-attached during active search. Refreshing search results.");
		triggerAsyncUpdate();
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
		if (ctrlrCombo != nullptr) {
			ctrlrCombo->setEditableText(false);
			for (int i = 0; i < ctrlrCombo->getNumChildComponents(); ++i) {
				if (auto *lb = dynamic_cast<juce::Label *>(ctrlrCombo->getChildComponent(i))) {
					lb->onEditorShow = nullptr;
					if (searchListener != nullptr)
						lb->removeListener(searchListener.get());
				}
			}
		}
		if (searchListener != nullptr)
			searchListener.reset();
	} else {
		if (ctrlrCombo != nullptr && (bool)getProperty(Ids::uiComboSearch)) {
			_DBG("LIFECYCLE: Restoring Fuzzy Search for User Mode...");

			ctrlrCombo->setEditableText(true);
			findAndAttach(ctrlrCombo.get());

			if (valueMap != nullptr) {
				_DBG("GUI_TRACE [" + owner.getName() + "] timerCallback | Refilling Combo list");
				valueMap->fillCombo(*ctrlrCombo, true);
			}

			_DBG("GUI_TRACE [" + owner.getName() + "] timerCallback | Post-Refill Index: " +
				 String(ctrlrCombo->getSelectedItemIndex()) + " | Text: '" + ctrlrCombo->getText() + "'");
		}

		if (getParentComponent() != nullptr) {
			_DBG("LIFECYCLE: Component re-attached during active search. Refreshing search results.");
			triggerAsyncUpdate();
		}
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
		_DBG("PROP: uiComboSearch changed - starting safety timer");
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
			ctrlrCombo->setEditableText(false);
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

void CtrlrCombo::handleAsyncUpdate() {
	if (ctrlrCombo == nullptr || valueMap == nullptr)
		return;

	int caretPos = 0;
	String preservedText = ctrlrCombo->getText();

	for (int i = 0; i < ctrlrCombo->getNumChildComponents(); ++i) {
		if (auto *lb = dynamic_cast<juce::Label *>(ctrlrCombo->getChildComponent(i))) {
			if (auto *ed = lb->getCurrentTextEditor()) {
				caretPos = ed->getCaretPosition();
				break;
			}
		}
	}

	isUpdating = true;

	StringArray matches;
	Array<int> ids;
	for (int i = 0; i < valueMap->getNumValues(); ++i) {
		String item = valueMap->getTextForIndex(i);
		if (item.containsIgnoreCase(lastSearchText)) {
			matches.add(item);
			ids.add(i + 1);
		}
	}

	ctrlrCombo->clear(juce::dontSendNotification);

	for (int i = 0; i < matches.size(); ++i)
		ctrlrCombo->addItem(matches[i], ids[i]);

	if (preservedText.isNotEmpty())
		ctrlrCombo->setText(preservedText, juce::dontSendNotification);

	if (isSearching) {
		juce::PopupMenu::dismissAllActiveMenus();

		juce::MessageManager::callAsync([this, caretPos, preservedText]() {
			if (ctrlrCombo == nullptr)
				return;

			ctrlrCombo->showPopup();

			for (int i = 0; i < ctrlrCombo->getNumChildComponents(); ++i) {
				if (auto *lb = dynamic_cast<juce::Label *>(ctrlrCombo->getChildComponent(i))) {
					if (lb->getCurrentTextEditor() == nullptr && lb->isEditable()) {
						lb->showEditor();
					}

					if (auto *currentEd = lb->getCurrentTextEditor()) {
						currentEd->grabKeyboardFocus();
						currentEd->setHighlightedRegion(juce::Range<int>(0, 0));
						currentEd->setCaretPosition(caretPos);
					}
				}
			}
		});
	}

	isUpdating = false;
}

void CtrlrCombo::findAndAttach(juce::ComboBox *combo) {
	if (combo == nullptr)
		return;

	for (int i = 0; i < combo->getNumChildComponents(); ++i) {
		if (auto *lb = dynamic_cast<juce::Label *>(combo->getChildComponent(i))) {
			if (searchListener == nullptr)
				searchListener = std::make_unique<SearchListener>(*this);

			lb->removeListener(searchListener.get());
			lb->addListener(searchListener.get());

			juce::Component::SafePointer<juce::Label> safeLabel(lb);
			juce::Component::SafePointer<CtrlrCombo> safeThis(this);

			lb->onEditorShow = [safeThis, safeLabel] {
				if (safeThis == nullptr || safeLabel == nullptr)
					return;

				if (auto *ed = safeLabel->getCurrentTextEditor()) {
					ed->setSelectAllWhenFocused(false);
					ed->moveCaretToEnd();

					_DBG("UI EVENT: Applying colors to active TextEditor");

					ed->setColour(juce::TextEditor::backgroundColourId,
								  VAR2COLOUR(safeThis->getProperty(Ids::uiComboBgColour)));
					ed->setColour(juce::TextEditor::textColourId,
								  VAR2COLOUR(safeThis->getProperty(Ids::uiComboTextColour)));
					ed->setColour(juce::TextEditor::highlightColourId,
								  VAR2COLOUR(safeThis->getProperty(Ids::uiComboTextColour)).withAlpha(0.3f));

					ed->onTextChange = [safeThis, ed] {
						if (safeThis != nullptr)
							safeThis->updateFuzzySearch(ed->getText());
					};
				}
			};
			break;
		}
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

	if (isInEditMode) {
		// 1. Cache the actual user setting before changing it
		savedFuzzySearchState = (bool)getProperty(Ids::uiComboSearch);

		// 2. Explicitly disable fuzzy search during edit mode
		setProperty(Ids::uiComboSearch, false);

		// 3. Close active popup if open
		if (ctrlrCombo != nullptr && ctrlrCombo->isPopupActive()) {
			ctrlrCombo->hidePopup();
		}
	} else {
		// 4. Exiting edit mode: Restore original fuzzy search state
		setProperty(Ids::uiComboSearch, savedFuzzySearchState);
	}

	if (ctrlrCombo != nullptr) {
		// Pass mouse interaction to CtrlrComponent so resize handles work in edit mode
		ctrlrCombo->setInterceptsMouseClicks(!isInEditMode, !isInEditMode);
		ctrlrCombo->setEditableText(false);
	}

	// We always use the timer to decouple from the synchronous mode change.
	// 50ms is enough for 'Entering', 200ms is safer for 'Exiting' (rebuilding UI).
	startTimer(isInEditMode ? 50 : 200);

	// Standard Ctrlr enablement logic
	if ((bool)owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelDisabledOnEdit)) {
		if (ctrlrCombo != nullptr)
			ctrlrCombo->setEnabled(!isInEditMode);
	}

	resized();
}
// void CtrlrCombo::panelEditModeChanged(const bool isInEditMode) {
// 	_DBG("Combo Edit Mode: " + String(isInEditMode ? "ON" : "OFF"));

// 	startTimer(isInEditMode ? 50 : 200);

// 	if ((bool)owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelDisabledOnEdit)) {
// 		if (ctrlrCombo != nullptr)
// 			ctrlrCombo->setEnabled(!isInEditMode);
// 	}

// 	resized();
// }

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

void CtrlrCombo::customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel) {
	if (customLookAndFeel == nullptr) {
		ctrlrCombo->setLookAndFeel(nullptr);

		String panelLnF = owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel);
		// applyCentralLookAndFeel(ctrlrCombo.get(), panelLnF.isNotEmpty() ? panelLnF : "V3");
		// applyCentralLookAndFeel(ctrlrCombo.get(), "V3");
		applyComboLookAndFeel(panelLnF);

		repaint();
	} else {
		ctrlrCombo->setLookAndFeel(customLookAndFeel);
	}
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

	if (comboStyle != "V3" && comboStyle != "V2" && comboStyle != "V1") {
		applyCentralLookAndFeel(ctrlrCombo.get(), comboStyle);
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

void CtrlrCombo::updateFuzzySearch(const String &searchText) {
	_DBG("FUZZY_STEP 1: Enter updateFuzzySearch with '" + searchText + "'");

	if (ctrlrCombo == nullptr || valueMap == nullptr)
		return;
	if (isUpdating)
		return;

	isUpdating = true;
	lastSearchText = searchText;
	isSearching = searchText.isNotEmpty();

	juce::TextEditor *ed = nullptr;
	for (int i = 0; i < ctrlrCombo->getNumChildComponents(); ++i) {
		if (auto *lb = dynamic_cast<juce::Label *>(ctrlrCombo->getChildComponent(i))) {
			ed = lb->getCurrentTextEditor();
			if (ed != nullptr)
				break;
		}
	}
	const int caretPos = (ed != nullptr) ? ed->getCaretPosition() : 0;

	ctrlrCombo->clear(juce::dontSendNotification);

	if (isSearching) {
		int matchCount = 0;
		for (int i = 0; i < valueMap->getNumValues(); ++i) {
			String item = valueMap->getTextForIndex(i);
			if (item.containsIgnoreCase(searchText)) {
				ctrlrCombo->addItem(item, i + 1);
				matchCount++;
			}
		}
		_DBG("FUZZY_STEP 3: Found " + String(matchCount) + " matches");

		if (matchCount == 0)
			ctrlrCombo->addItem("(no matches)", -1);

		juce::PopupMenu::dismissAllActiveMenus();

		juce::MessageManager::callAsync([this, ed, caretPos]() {
			if (ctrlrCombo == nullptr)
				return;
			ctrlrCombo->showPopup();
			if (ed != nullptr) {
				ed->grabKeyboardFocus();
				ed->setCaretPosition(caretPos);
			}
		});
	} else {
		_DBG("FUZZY: Search cleared, showing full list");
		valueMap->fillCombo(*ctrlrCombo, true);

		if (!ctrlrCombo->isPopupActive())
			ctrlrCombo->showPopup();

		if (ed != nullptr) {
			ed->grabKeyboardFocus();
			ed->setCaretPosition(0);
		}
	}

	isUpdating = false;
	_DBG("FUZZY_STEP 7: Logic Finished");
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

void CtrlrCombo::CtrlrComboLF::drawComboBox(Graphics &g, int width, int height, bool isButtonDown, int buttonX,
											int buttonY, int buttonW, int buttonH, ComboBox &box) {
	// 1. Fetch the active panel theme name
	String comboStyle = owner.getProperty(Ids::uiButtonLookAndFeel).toString();
	if (comboStyle.isEmpty() || comboStyle == "Default") {
		comboStyle = owner.getProperty(Ids::uiPanelLookAndFeel).toString();
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

	Path p;
	p.addTriangle(bx + bw * 0.5f, buttonY + buttonH * (0.45f - arrowH), bx + bw * (1.0f - arrowX),
				  buttonY + buttonH * 0.45f, bx + bw * arrowX, buttonY + buttonH * 0.45f);

	p.addTriangle(bx + bw * 0.5f, buttonY + buttonH * (0.55f + arrowH), bx + bw * (1.0f - arrowX),
				  buttonY + buttonH * 0.55f, bx + bw * arrowX, buttonY + buttonH * 0.55f);

	g.setColour(box.findColour(ComboBox::arrowColourId));
	g.fillPath(p);
}