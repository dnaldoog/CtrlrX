#include "stdafx.h"
#include "CtrlrCombo.h"
#include "CtrlrValueMap.h"
#include "CtrlrIDs.h"
#include "CtrlrFontManager.h"
#include "CtrlrModulator/CtrlrModulator.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrUtilitiesGUI.h"
#include "JuceClasses/LLookAndFeel.h"
#include <rapidfuzz/fuzz.hpp> // Added v5.6.35. Support for rapidfuzz


CtrlrCombo::CtrlrCombo (CtrlrModulator &owner)
    : CtrlrComponent(owner),
      lf(*this),
      ctrlrCombo (nullptr)
{
	valueMap = new CtrlrValueMap();
    
    // 1. Attach our fuzzy listener (The code we added earlier)
    addAndMakeVisible (ctrlrCombo = new ComboBox (L"ctrlrCombo"));
    
    // Force default editableText to false on construction like in 5.6.34-
    ctrlrCombo->setEditableText (false);
    ctrlrCombo->setJustificationType (Justification::centred);
    
    ctrlrCombo->addListener (this);
	ctrlrCombo->setLookAndFeel (&lf);
	componentTree.addListener (this);

	setProperty (Ids::uiComboSearch, false);
    setProperty (Ids::uiComboButtonWidthOverride, false);
    setProperty (Ids::uiComboButtonWidth, 16);
    setProperty (Ids::uiComboDynamicContent, 0);
    setProperty (Ids::uiComboSelectedId, -1);
    setProperty (Ids::uiComboSelectedIndex, -1);
    
    setProperty (Ids::uiComboContent, "");
    
    setProperty (Ids::uiButtonLookAndFeel, "Default");
    setProperty (Ids::uiButtonLookAndFeelIsCustom, false);
    
    if ( owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel) == "V3"
        || owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel) == "V2"
        || owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel) == "V1" )
    {
        setProperty (Ids::uiComboArrowColour, "0xff0000ff");
        setProperty (Ids::uiComboOutlineColour, "0xff0000ff");
        
        setProperty (Ids::uiComboTextJustification, "centred");
        setProperty (Ids::uiComboFont, FONT2STR (Font(14)));
        setProperty (Ids::uiComboTextColour, "0xff000000");
        
        setProperty (Ids::uiComboMenuFont, FONT2STR (Font(16)));
        setProperty (Ids::uiComboMenuFontColour, "0xff000000");
        
        setProperty (Ids::uiComboButtonColour, "0xff0000ff");
        setProperty (Ids::uiComboBgColour, "0xffffffff");
        
        setProperty (Ids::uiComboMenuBackgroundColour, "0xfff0f0f0");
        
        setProperty (Ids::uiComboMenuHighlightColour, Colours::lightblue.toString());
        setProperty (Ids::uiComboMenuFontHighlightedColour, "0xff232323");
        
        setProperty (Ids::uiComboMenuBackgroundRibbed, true);
        
        setSize (88, 32);
    }
    else
    {
        setProperty (Ids::uiComboArrowColour, (String)findColour(ComboBox::arrowColourId).toString());
        setProperty (Ids::uiComboOutlineColour, (String)findColour(ComboBox::outlineColourId).darker(0.5f).toString());
        
        setProperty (Ids::uiComboTextJustification, "centred");
        setProperty (Ids::uiComboFont, FONT2STR (Font(14)));
        setProperty (Ids::uiComboTextColour, (String)findColour(ComboBox::textColourId).toString());
        
        setProperty (Ids::uiComboMenuFont, FONT2STR (Font(16)));
        setProperty (Ids::uiComboMenuFontColour, (String)findColour(ComboBox::textColourId).toString());
        
        setProperty (Ids::uiComboButtonColour, (String)findColour(ComboBox::buttonColourId).toString());
        setProperty (Ids::uiComboBgColour, (String)findColour(ComboBox::backgroundColourId).toString());
        
        setProperty (Ids::uiComboMenuBackgroundColour, (String)findColour(ComboBox::backgroundColourId).toString());
        
        setProperty (Ids::uiComboMenuHighlightColour, (String)findColour(TextEditor::highlightColourId).toString());
        setProperty (Ids::uiComboMenuFontHighlightedColour, (String)findColour(TextEditor::highlightedTextColourId).toString());
        
        setProperty (Ids::uiComboMenuBackgroundRibbed, false);
        
        setSize (120, 40);
    }
    
    setProperty (Ids::uiComboButtonGradient, true);
    setProperty (Ids::uiComboButtonGradientColour1, (String)findColour(TextButton::buttonColourId).toString());
    setProperty (Ids::uiComboButtonGradientColour2, (String)findColour(TextButton::buttonColourId).darker(0.2f).toString());

    setProperty (Ids::uiButtonLookAndFeelIsCustom, false);
    
    // Explicitly set the TextEditor colors to match the Combo background/text
    // ComboBox level
    ctrlrCombo->setColour (ComboBox::textColourId, findColour(ComboBox::textColourId));
    ctrlrCombo->setColour (ComboBox::backgroundColourId, findColour(ComboBox::backgroundColourId));
    
    // TextEditor level (for active typing)
    ctrlrCombo->setColour (TextEditor::textColourId, findColour(TextEditor::textColourId));
    ctrlrCombo->setColour (TextEditor::highlightColourId, findColour(TextEditor::highlightColourId));
    ctrlrCombo->setColour (TextEditor::highlightedTextColourId, findColour(TextEditor::highlightedTextColourId));
    ctrlrCombo->setColour (TextEditor::outlineColourId, Colours::transparentBlack);
    
    // Label level (the bridge between Combo and Editor)
    ctrlrCombo->setColour (Label::textColourId, findColour(Label::textColourId));
    ctrlrCombo->setColour (Label::backgroundColourId, Colours::transparentBlack); // Keep transparent to see the Combo bg
    ctrlrCombo->setColour (Label::textWhenEditingColourId, findColour(Label::textWhenEditingColourId));
    ctrlrCombo->setColour (Label::backgroundWhenEditingColourId, findColour(Label::backgroundWhenEditingColourId));
    
	_DBG("--- CONSTRUCTION PHASE END ---");
}

CtrlrCombo::~CtrlrCombo()
{
    // 1. IMPORTANT: Reach into the combo and kill the callback
    // before 'this' becomes invalid.
    if (ctrlrCombo != nullptr)
    {
        // Reach in and kill the lambda before 'this' disappears
        for (int i = 0; i < ctrlrCombo->getNumChildComponents(); ++i)
        {
            if (auto* lb = dynamic_cast<juce::Label*> (ctrlrCombo->getChildComponent(i)))
            {
                lb->onEditorShow = nullptr; // <--- CRITICAL
                if (searchListener != nullptr)
                    lb->removeListener (searchListener.get());
            }
        }
    }

    // 2. Existing cleanup
    if (searchListener != nullptr)
    {
        searchListener.reset();
    }
    
    // 3. Delete the UI component and null the pointer.
    // deleteAndZero is a JUCE macro that calls 'delete' and sets the pointer to nullptr.
    deleteAndZero (ctrlrCombo);
}

void CtrlrCombo::resized()
{
    //ctrlrCombo->setBounds (2, 2, getWidth() - 4, getHeight() - 4);
	if (restoreStateInProgress)
		return;

	ctrlrCombo->setBounds (getUsableRect());
}

void CtrlrCombo::mouseDown (const MouseEvent& e)
{
    if (getProperty(Ids::uiComboSearch))
    {
        _DBG("UX RESET: Restoring full list for browser mode");
        
        // Reset the ComboBox to show all items from the valueMap
        valueMap->fillCombo (*ctrlrCombo, true);
        
        // We do NOT clear the text here so it "stays" as requested,
        // but the internal list is now complete.
    }

    CtrlrComponent::mouseDown (e);
}

bool CtrlrCombo::keyPressed (const KeyPress& key) // Updated v5.6.35. Combined methods
{
    // 1. Handle the Fuzzy Search UI (New Logic)
    if (key == KeyPress::returnKey)
    {
        // If the menu is open and has items, select the first one
        if (ctrlrCombo->isPopupActive())
        {
            ctrlrCombo->setSelectedItemIndex(0, true);
            ctrlrCombo->hidePopup();
            return true;
        }
    }
    else if (key == KeyPress::escapeKey)
    {
        // Cancel search: restore full list and clear text
        valueMap->fillCombo (*ctrlrCombo, true);
        ctrlrCombo->hidePopup();
        return true;
    }
    
    // 2. Fallback to the Legacy Canvas behavior
    // We don't need 'originatingComponent' because 'this' is the source.
    if (getParentComponent())
    {
        if (auto *canvas = dynamic_cast<CtrlrPanelCanvas*>(getParentComponent()))
        {
            return canvas->keyPressed (key, this);
        }
    }
    
    return false;  // Return true if your handler uses this key event, or false to allow it to be passed-on.
}

void CtrlrCombo::visibilityChanged()
{
    if (isVisible() && (bool)getProperty(Ids::uiComboSearch))
    {
        _DBG("LIFECYCLE: Component visible. Starting 250ms safety timer...");
        // This is now a valid call
        startTimer(250);
    }
}

void CtrlrCombo::lookAndFeelChanged()
{
    // Call the base class first
    CtrlrComponent::lookAndFeelChanged();

    if (getProperty(Ids::uiComboSearch))
    {
        _DBG("LIFECYCLE: LookAndFeel changed. This usually wipes the Label state. Re-attaching...");
        
        // We use a slight delay because JUCE's lookAndFeelChanged
        // often happens before the internal Label is re-configured.
        // Use the safe class timer instead of callAfterDelay
        startTimer(100);
    }
}

void CtrlrCombo::parentHierarchyChanged()
{
    if (getParentComponent() != nullptr)
    {
        _DBG("LIFECYCLE: Component attached to parent. Refreshing Search state.");
        triggerAsyncUpdate();
    }
}

void CtrlrCombo::timerCallback()
{
    stopTimer();
    const bool isInEditMode = owner.getOwnerPanel().getEditMode();
    
    if (isInEditMode)
    {
        _DBG("LIFECYCLE: Cleaning up Search for Edit Mode...");
        if (ctrlrCombo != nullptr)
        {
            ctrlrCombo->setEditableText(false);
            // Clear callbacks to prevent the 0x1000000000 crash
            for (int i = 0; i < ctrlrCombo->getNumChildComponents(); ++i)
            {
                if (auto* lb = dynamic_cast<juce::Label*> (ctrlrCombo->getChildComponent(i)))
                {
                    lb->onEditorShow = nullptr;
                    if (searchListener != nullptr)
                        lb->removeListener (searchListener.get());
                }
            }
        }
        if (searchListener != nullptr) searchListener.reset();
    }
    else
    {
        if (ctrlrCombo != nullptr && (bool)getProperty(Ids::uiComboSearch))
        {
            _DBG("LIFECYCLE: Restoring Fuzzy Search for User Mode...");
            
            // Re-enable editability so findAndAttach sees the Label
            ctrlrCombo->setEditableText(true);
            
            findAndAttach(ctrlrCombo);
            
            // Ensure the list is fresh
            if (valueMap != nullptr)
                valueMap->fillCombo(*ctrlrCombo, true);
        }
    }
}

void CtrlrCombo::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == ctrlrCombo)
    {
        //[UserComboBoxCode_ctrlrCombo] -- add your combo box handling code here..
        _DBG("CtrlrCombo::comboBoxChanged");
        
        // setComponentValue (ctrlrCombo->getSelectedItemIndex(), true);
        
        // Use getSelectedId() - 1 to map back to the original valueMap index
        // because we added +1 during the addItem loop in updateFuzzySearch
        int originalIndex = ctrlrCombo->getSelectedId() - 1;
        
        if (originalIndex >= 0)
        {
            setComponentValue (originalIndex, true);
        }
    }
}

double CtrlrCombo::getComponentMaxValue()
{
	return (valueMap->getNonMappedMax());
}

double CtrlrCombo::getComponentValue()
{
	return (ctrlrCombo->getSelectedId()-1);
}

int CtrlrCombo::getComponentMidiValue()
{
	return (valueMap->getMappedValue(ctrlrCombo->getSelectedId() - 1));
}

const String CtrlrCombo::getComponentText()
{
	return (ctrlrCombo->getText());
}

void CtrlrCombo::setComponentValue (const double newValue, const bool sendChangeMessage)
{
	ctrlrCombo->setSelectedId (newValue+1, sendChangeMessage ? sendNotificationSync : dontSendNotification);

	if (sendChangeMessage)
	{
		owner.getProcessor().setValueGeneric (CtrlrModulatorValue(newValue,CtrlrModulatorValue::changedByGUI), sendChangeMessage);
	}
}

void CtrlrCombo::comboContentChanged()
{
	if ((int)getProperty(Ids::uiComboDynamicContent) > 0)
		return;

	valueMap->copyFrom (owner.getProcessor().setValueMap (getProperty(Ids::uiComboContent)));
	valueMap->fillCombo (*ctrlrCombo, true);
}

void CtrlrCombo::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
	// Log EVERY property change during load
    _DBG("PROP CHANGE: " + property.toString() + " = " + getProperty(property).toString());
	
	if (property == Ids::uiComboContent)
	{
		comboContentChanged();
	}
	// PATH 1: User toggles search ON/OFF
	if (property == Ids::uiComboSearch)
	{
		_DBG("PROP: uiComboSearch changed - starting safety timer");
		startTimer(250);
	}
	// PATH 2: Background/Font color changes
	else if (property.toString().startsWith("uiCombo") && property != Ids::uiComboSearch)
	{
		if (ctrlrCombo && (bool)getProperty(Ids::uiComboSearch))
		{
			_DBG("STYLE_CHANGE: " + property.toString() + " - Resetting engine.");
			
			// We set it false here immediately to clear the old state
			ctrlrCombo->setEditableText(false);
			
			// Then let the timer handle the "Turn back ON"
			startTimer(250);
		}
		else {
			updateInternalComponentStyles();
		}
	}
	else if (property == Ids::uiButtonLookAndFeel)
	{
		String LookAndFeelType = getProperty(property);
        setLookAndFeel(getLookAndFeelFromComponentProperty(LookAndFeelType)); // Updates the current component LookAndFeel
        
        if (LookAndFeelType == "Default")
        {
            setProperty(Ids::uiButtonLookAndFeelIsCustom, false); // Resets the Customized Flag to False to allow Global L&F to apply
        }
        
        if (!getProperty(Ids::uiButtonLookAndFeelIsCustom))
        {
            resetLookAndFeelOverrides(); // Retrieves LookAndFeel colours from selected ColourScheme
        }
    }
	else if (property == Ids::uiComboBgColour)
	{
		ctrlrCombo->setColour (ComboBox::backgroundColourId, VAR2COLOUR(getProperty(Ids::uiComboBgColour)));
        
        ctrlrCombo->setColour (TextEditor::backgroundColourId, VAR2COLOUR(getProperty(Ids::uiComboBgColour)));
        ctrlrCombo->setColour (TextEditor::highlightColourId, VAR2COLOUR(getProperty(Ids::uiComboBgColour)));
        
        ctrlrCombo->setColour (Label::backgroundColourId, VAR2COLOUR(getProperty(Ids::uiComboBgColour)));
        ctrlrCombo->setColour (Label::backgroundWhenEditingColourId, VAR2COLOUR(getProperty(Ids::uiComboBgColour)));
        
        updateInternalComponentStyles();
        
        setProperty(Ids::uiButtonLookAndFeelIsCustom, true); // Locks the component custom colourScheme
	}
	else if (property == Ids::uiComboButtonColour)
	{
		ctrlrCombo->setColour (ComboBox::buttonColourId, VAR2COLOUR(getProperty(Ids::uiComboButtonColour)));
        setProperty(Ids::uiButtonLookAndFeelIsCustom, true); // Locks the component custom colourScheme
	}
	else if (property == Ids::uiComboTextColour)
	{
		ctrlrCombo->setColour (ComboBox::textColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
        
        ctrlrCombo->setColour (TextEditor::textColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
        ctrlrCombo->setColour (TextEditor::highlightedTextColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
        
        ctrlrCombo->setColour (Label::textColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
        ctrlrCombo->setColour (Label::textWhenEditingColourId, VAR2COLOUR(getProperty(Ids::uiComboTextColour)));
        
        updateInternalComponentStyles();
        
        setProperty(Ids::uiButtonLookAndFeelIsCustom, true); // Locks the component custom colourScheme
	}
	else if (property == Ids::uiComboOutlineColour)
	{
		ctrlrCombo->setColour (ComboBox::outlineColourId, VAR2COLOUR(getProperty(Ids::uiComboOutlineColour)));
        setProperty(Ids::uiButtonLookAndFeelIsCustom, true); // Locks the component custom colourScheme
	}
	else if (property == Ids::uiComboArrowColour)
	{
		ctrlrCombo->setColour (ComboBox::arrowColourId, VAR2COLOUR(getProperty(Ids::uiComboArrowColour)));
        setProperty(Ids::uiButtonLookAndFeelIsCustom, true); // Locks the component custom colourScheme
	}
	else if (property == Ids::uiComboTextJustification)
	{
		ctrlrCombo->setJustificationType (justificationFromProperty(getProperty(property)));
	}
	// SHOULD WE PUT ALL COMPONENTS COLOUR PROPERTIES IN THE FOLLOWING CONDITION ??? >>> uiButtonLookAndFeelIsCustom true
	else if (property == Ids::uiComboFont
             || property == Ids::uiComboTextColour // Added v5.6.35
             || property == Ids::uiComboBgColour // Added v5.6.35
             || property == Ids::uiComboOutlineColour // Added v5.6.35
             || property == Ids::uiComboMenuBackgroundColour
             || property == Ids::uiComboMenuFont
             || property == Ids::uiComboMenuFontColour
             || property == Ids::uiComboMenuHighlightColour
             || property == Ids::uiComboMenuFontHighlightedColour
             || property == Ids::uiComboButtonWidthOverride
             || property == Ids::uiComboButtonWidth
             || property == Ids::uiComboButtonColour // Added v5.6.35
             || property == Ids::uiComboArrowColour // Added v5.6.35. Not sure if needed?
             || property == Ids::uiComboButtonGradientColour1 // Added v5.6.35. Not sure if needed?
             || property == Ids::uiComboButtonGradientColour2 // Added v5.6.35. Not sure if needed?
             )
	{
		ctrlrCombo->setLookAndFeel(0);
		ctrlrCombo->setLookAndFeel(&lf);
        setProperty(Ids::uiButtonLookAndFeelIsCustom, true); // Locks the component custom colourScheme
	}
	else if (property == Ids::uiComboDynamicContent)
	{
		fillContent(getProperty(property));
	}
	else if (property == Ids::uiComboSelectedId )
    {
        if ((int)getProperty(property) != -1)
        {
            ctrlrCombo->setSelectedId (getProperty(property), sendNotificationSync);
        }
    }
    else if (property == Ids::uiComboSelectedIndex)
    {
        if ((int)getProperty(property) != -1)
        {
            ctrlrCombo->setSelectedItemIndex (getProperty(property), sendNotificationSync);
        }
    }
    else
    {
        CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
    }

	if (restoreStateInProgress == false)
	{
        resized();
	}
}

void CtrlrCombo::handleAsyncUpdate()
{
    const bool shouldBeEditable = getProperty(Ids::uiComboSearch);
    _DBG("--- ASYNC UPDATE START ---");
    _DBG("Target Search State: " + String(shouldBeEditable ? "ON" : "OFF"));
    
    ctrlrCombo->setEditableText (shouldBeEditable);
    
    if (shouldBeEditable)
    {
        MessageManager::callAsync([this]() {
            if (ctrlrCombo)
            {
                _DBG("Async: Attempting findAndAttach...");
                findAndAttach (ctrlrCombo);
                updateInternalComponentStyles();
            }
            else {
                _DBG("Async ERROR: ctrlrCombo is NULL");
            }
        });
    }
    else
    {
        _DBG("Async: Resetting search listener");
        searchListener.reset();
        comboContentChanged();
        updateInternalComponentStyles();
    }
    
    repaint();
}

void CtrlrCombo::findAndAttach (juce::ComboBox* combo)
{
    if (combo == nullptr) {
        _DBG("findAndAttach: Aborted (Combo is null)");
        return;
    }

    bool labelFound = false;
    for (int i = 0; i < combo->getNumChildComponents(); ++i)
    {
        if (auto* lb = dynamic_cast<juce::Label*> (combo->getChildComponent(i)))
        {
            _DBG("findAndAttach: Attached to Label at address: " + String::toHexString((int64)lb));
            
            labelFound = true;
            _DBG("findAndAttach: Found Label child. Attaching listener.");
            
            if (searchListener == nullptr)
                searchListener = std::make_unique<SearchListener>(*this);
            
            lb->removeListener (searchListener.get());
            lb->addListener (searchListener.get());

            // --- SAFETY WRAPPERS ---
            // These automatically become null if the objects are deleted
            // during the Edit Mode transition.
            juce::Component::SafePointer<juce::Label> safeLabel (lb);
            juce::Component::SafePointer<CtrlrCombo> safeThis (this);

            lb->onEditorShow = [safeThis, safeLabel] {
                _DBG("UI EVENT: onEditorShow triggered");
                
                // If the panel or label is dying/in edit mode, ABORT.
                if (safeThis == nullptr || safeLabel == nullptr || safeThis->getOwner().getOwnerPanel().getEditMode())
                    return;
                
                juce::MessageManager::callAsync([safeThis, safeLabel]() {
                    // Double check: Did the objects survive the async delay?
                    if (safeThis == nullptr || safeLabel == nullptr)
                        return;

                    if (auto* ed = safeLabel->getCurrentTextEditor())
                    {
                        _DBG("UI EVENT: Applying colors to active TextEditor");
                        
                        ed->setColour (juce::TextEditor::backgroundColourId, VAR2COLOUR(safeThis->getProperty(Ids::uiComboBgColour)));
                        ed->setColour (juce::TextEditor::textColourId, VAR2COLOUR(safeThis->getProperty(Ids::uiComboTextColour)));
                        ed->setColour (juce::TextEditor::highlightColourId, VAR2COLOUR(safeThis->getProperty(Ids::uiComboTextColour)).withAlpha(0.3f));
                        
                        // Capture safeThis to prevent 'this' dangling inside text change
                        ed->onTextChange = [safeThis, ed] {
                            if (safeThis != nullptr) {
                                _DBG("FUZZY: Text changed to: " + ed->getText());
                                safeThis->updateFuzzySearch (ed->getText());
                            }
                        };
                    }
                });
                
                if (safeThis->ctrlrCombo && (bool)safeThis->getProperty(Ids::uiComboSearch)) {
                    safeThis->valueMap->fillCombo (*(safeThis->ctrlrCombo), true);
                    safeThis->ctrlrCombo->showPopup();
                }
            };
            break;
        }
    }
    
    if (!labelFound) {
        _DBG("findAndAttach WARNING: No Label component found! ComboBox might not be initialized.");
    }
}

void CtrlrCombo::setComponentText (const String &componentText)
{
	ctrlrCombo->setText (componentText);
}

void CtrlrCombo::fillContent(const int contentType)
{
	Array<File> files;
	const String prewviousContent = ctrlrCombo->getText();

	switch (contentType)
	{
		case 1:
			for (int i=0; i<owner.getOwnerPanel().getModulators().size(); i++)
			{
				valueMap->setPair (i, i, owner.getOwnerPanel().getModulatorByIndex(i)->getName());
			}
			owner.getProcessor().setValueMap (*valueMap);
			valueMap->fillCombo (*ctrlrCombo, true);
			ctrlrCombo->setText (prewviousContent, dontSendNotification);
			break;

		case 2:
			File::findFileSystemRoots(files);
			for (int i=0; i<files.size(); i++)
			{
				valueMap->setPair (i, i, files[i].getFullPathName());
			}
			owner.getProcessor().setValueMap (*valueMap);
			valueMap->fillCombo (*ctrlrCombo, true);
			ctrlrCombo->setText (prewviousContent, dontSendNotification);
			break;
		default:
			comboContentChanged();
			break;
	}
}

void CtrlrCombo::panelEditModeChanged(const bool isInEditMode)
{
    _DBG("Combo Edit Mode: " + String(isInEditMode ? "ON" : "OFF"));
    
    // We always use the timer to decouple from the synchronous mode change.
    // 50ms is enough for 'Entering', 200ms is safer for 'Exiting' (rebuilding UI).
    startTimer (isInEditMode ? 50 : 200);

    // Standard Ctrlr enablement logic
    if ((bool)owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelDisabledOnEdit))
    {
        if (ctrlrCombo != nullptr)
            ctrlrCombo->setEnabled (!isInEditMode);
    }

    resized();
}

int CtrlrCombo::getSelectedId()
{
	return (ctrlrCombo->getSelectedId());
}

int CtrlrCombo::getSelectedItemIndex()
{
	return (ctrlrCombo->getSelectedItemIndex());
}

void CtrlrCombo::setSelectedId(const int id, const bool dontNotify)
{
	ctrlrCombo->setSelectedId (id, dontNotify ? dontSendNotification : sendNotificationSync);
}

void CtrlrCombo::setSelectedItemIndex(const int index, const bool dontNotify)
{
	ctrlrCombo->setSelectedItemIndex (index, dontNotify ? dontSendNotification : sendNotificationSync);
}

const String CtrlrCombo::getText()
{
	return (ctrlrCombo->getText());
}

void CtrlrCombo::setText(const String &text, const bool dontNotify)
{
	return (ctrlrCombo->setText(text, dontNotify ? dontSendNotification : sendNotificationSync));
}

void CtrlrCombo::customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel)
{
    if (customLookAndFeel == nullptr)
        ctrlrCombo->setLookAndFeel (&lf);
    else
        ctrlrCombo->setLookAndFeel (customLookAndFeel);
}

LookAndFeel *CtrlrCombo::getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty) // Updated v5.6.34
{
    if (lookAndFeelComponentProperty == "Default")
    {
        // This case still means "use the default LookAndFeel (which might be the global one)"
        // so returning nullptr is appropriate if that's the desired behavior.
        return nullptr;
    }

    // Call your new generic factory function
    // We pass 'false' for the second argument here, as 'Default' is handled separately
    // and an unknown string should likely result in nullptr to fall back to the global L&F.
    return gui::createLookAndFeelFromDescription(lookAndFeelComponentProperty, false);
}

void CtrlrCombo::resetLookAndFeelOverrides()
{
    _DBG("RESET_LF: Start - restoreStateInProgress = " + String(restoreStateInProgress ? "TRUE" : "FALSE"));
	
    if (restoreStateInProgress == false)
    {
        // 1. Log the current LookAndFeel state
        _DBG("RESET_LF: Overriding UI properties from LookAndFeel defaults");
		
        // Let's see if findColour is actually returning what we expect
        _DBG("RESET_LF: Current Text Colour: " + findColour(ComboBox::textColourId).toDisplayString(true));
		
        setProperty (Ids::componentLabelColour, (String)findColour(Label::textColourId).toString());
        
        setProperty (Ids::uiComboArrowColour, (String)findColour(ComboBox::arrowColourId).toString());
        setProperty (Ids::uiComboOutlineColour, (String)findColour(ComboBox::outlineColourId).darker(0.5f).toString());
        
        setProperty (Ids::uiComboTextColour, (String)findColour(ComboBox::textColourId).toString());
        
        setProperty (Ids::uiComboMenuFontColour, (String)findColour(ComboBox::textColourId).toString());
        
        setProperty (Ids::uiComboButtonColour, (String)findColour(ComboBox::buttonColourId).toString());
        setProperty (Ids::uiComboBgColour, (String)findColour(ComboBox::backgroundColourId).toString());
        
        setProperty (Ids::uiComboMenuBackgroundColour, (String)findColour(ComboBox::backgroundColourId).toString());
        
        setProperty (Ids::uiComboMenuHighlightColour, (String)findColour(TextEditor::highlightColourId).toString());
        setProperty (Ids::uiComboMenuFontHighlightedColour, (String)findColour(TextEditor::highlightedTextColourId).toString());
        
        setProperty (Ids::uiComboButtonGradientColour1, (String)findColour(TextButton::buttonColourId).toString());
        setProperty (Ids::uiComboButtonGradientColour2, (String)findColour(TextButton::buttonColourId).darker(0.2f).toString());
        
        setProperty (Ids::uiComboArrowColour, (String)findColour(ComboBox::arrowColourId).toString());
        
        setProperty (Ids::uiButtonLookAndFeelIsCustom, false); // Resets the component colourScheme if a new default colourScheme is selected from the menu
        
		_DBG("RESET_LF: End");
		
        updatePropertiesPanel(); // Refreshes property pane
    }
}

void CtrlrCombo::updatePropertiesPanel()
{
    CtrlrPanelProperties *props = owner.getCtrlrManagerOwner().getActivePanel()->getEditor(false)->getPropertiesPanel();
    if (props)
    {
        props->refreshAll(); // Needs extra code to prevent scrolling back to top on refresh
    }
}

void CtrlrCombo::updateInternalComponentStyles()
{
    _DBG("--- updateInternalComponentStyles ---");
	if (ctrlrCombo == nullptr) return;

    const Colour bg = VAR2COLOUR(getProperty(Ids::uiComboBgColour));
    const Colour txt = VAR2COLOUR(getProperty(Ids::uiComboTextColour));

    // 1. Force the ComboBox itself to update its internal color IDs
    ctrlrCombo->setColour(ComboBox::backgroundColourId, bg);
    ctrlrCombo->setColour(ComboBox::textColourId, txt);
    
    // For good measure, sync the button color too if you use it
    ctrlrCombo->setColour(ComboBox::buttonColourId, VAR2COLOUR(getProperty(Ids::uiComboButtonColour)));

    // 2. Iterate through children to force the change on existing sub-components
    for (int i = 0; i < ctrlrCombo->getNumChildComponents(); ++i)
    {
        auto* child = ctrlrCombo->getChildComponent(i);
        
        // If the Label exists (idle state), force its color
        if (auto* lb = dynamic_cast<juce::Label*>(child))
        {
            lb->setColour(Label::backgroundColourId, Colours::transparentBlack);
            lb->setColour(Label::textColourId, txt);
        }
        
        // If the TextEditor exists (active search state), force its colors
        if (auto* ed = dynamic_cast<juce::TextEditor*>(child))
        {
            ed->setColour(TextEditor::backgroundColourId, bg);
            ed->setColour(TextEditor::textColourId, txt);
            ed->setColour(TextEditor::highlightColourId, txt.withAlpha(0.2f));
        }
    }
    
    ctrlrCombo->repaint();
}

void CtrlrCombo::updateFuzzySearch(const String& searchText)
{
    _DBG("STAGE 1: updateFuzzySearch() triggered for: " + searchText);

    // 1. Find the active TextEditor to maintain focus later
    juce::TextEditor* ed = nullptr;
    std::function<void(juce::Component*)> findEd = [&](juce::Component* c) {
        if (ed != nullptr || c == nullptr) return;
        for (int i = 0; i < c->getNumChildComponents(); ++i) {
            auto* child = c->getChildComponent(i);
            if (auto* found = dynamic_cast<juce::TextEditor*>(child)) { ed = found; return; }
            findEd(child);
        }
    };
    findEd(ctrlrCombo);

    // 2. Force the old popup to close first to prevent stacking
    if (ctrlrCombo->isPopupActive()) {
        ctrlrCombo->hidePopup();
    }

    if (searchText.isEmpty()) {
        _DBG("STAGE 2: Text empty, resetting list");
        valueMap->fillCombo (*ctrlrCombo, true);
        ctrlrCombo->showPopup();
        return;
    }
    
    _DBG("STAGE 3: Filtering for string: " + searchText);

    // 3. Filter items
    ctrlrCombo->clear (juce::dontSendNotification);
    int resultsFound = 0;

    const int numValues = valueMap->getNumValues();
    for (int i = 0; i < numValues; ++i) {
        const String itemText = valueMap->getTextForIndex (i);
        std::string searchStr = searchText.toLowerCase().toStdString();
        std::string targetStr = itemText.toLowerCase().toStdString();
        
        // Rapidfuzz Score
        double score = rapidfuzz::fuzz::partial_ratio (searchStr, targetStr);
        if (itemText.startsWithIgnoreCase (searchText)) score = 100.0;
        
        double threshold = (searchText.length() < 3) ? 90.0 : 65.0;
        
        if (score >= threshold) {
            ctrlrCombo->addItem (itemText, i + 1);
            resultsFound++;
        }
    }
    
    _DBG("STAGE 4: Matches found: " + String(resultsFound));
    
    // 4. Display the updated results
    if (resultsFound > 0) {
        ctrlrCombo->showPopup();
    }
    else {
        ctrlrCombo->addItem ("(no matches found)", -1);
        ctrlrCombo->setItemEnabled (-1, false);
        ctrlrCombo->showPopup();
    }
    
    // 5. CRITICAL: Re-grab focus immediately so typing is not interrupted
    if (ed != nullptr) {
        ed->grabKeyboardFocus();
    }
}

void CtrlrCombo::CtrlrComboLF::drawPopupMenuBackground (Graphics &g, int width, int height)
{
	const Colour background = VAR2COLOUR(owner.getProperty(Ids::uiComboMenuBackgroundColour));

    g.fillAll (background);

	if (owner.getProperty (Ids::uiComboMenuBackgroundRibbed))
	{
    g.setColour (background.overlaidWith (Colour (0x2badd8e6)));

    for (int i = 0; i < height; i += 3)
        g.fillRect (0, i, width, 1);

#if ! JUCE_MAC
    g.setColour (findColour (PopupMenu::textColourId).withAlpha (0.6f));
    g.drawRect (0, 0, width, height);
#endif
	}
}

// Updated v5.6.35. Custom LookAndFeel Class and methods moved at the bottom for clarity
void CtrlrCombo::CtrlrComboLF::drawPopupMenuItem (Graphics &g, const Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                            const String& text, const String& shortcutKeyText,
                            const Drawable* icon, const Colour* textColourToUse)
{
	const int width = area.getWidth();
	const int height = area.getHeight();

	const float halfH = height * 0.5f;

    if (isSeparator)
    {
        const float separatorIndent = 5.5f;

        g.setColour (findColour(ComboBox::textColourId).withAlpha(0.3f));// Colour (0x33000000));
        g.drawLine (separatorIndent, halfH, width - separatorIndent, halfH);

        g.setColour (findColour(ComboBox::textColourId).withAlpha(0.6f)); // Colour (0x66ffffff));
        g.drawLine (separatorIndent, halfH + 1.0f, width - separatorIndent, halfH + 1.0f);
    }
    else
    {
		Colour textColour = VAR2COLOUR(owner.getProperty(Ids::uiComboMenuFontColour));

        if (textColourToUse != nullptr)
		{
			_DBG("Using passed in colour: "+textColourToUse->toString());
            textColour = *textColourToUse;
		}

        if (isHighlighted)
        {
			g.setColour (VAR2COLOUR(owner.getProperty(Ids::uiComboMenuHighlightColour)));
            g.fillRect (1, 1, width - 2, height - 2);

            g.setColour (VAR2COLOUR(owner.getProperty(Ids::uiComboMenuFontHighlightedColour)));
        }
        else
        {
            g.setColour (textColour);
        }

        if (! isActive)
            g.setOpacity (0.3f);

		Font font = owner.getOwner().getOwnerPanel().getCtrlrManagerOwner().getFontManager().getFontFromString (owner.getProperty(Ids::uiComboMenuFont));

        if (font.getHeight() > height / 1.3f)
            font.setHeight (height / 1.3f);

        g.setFont (font);

        const int leftBorder = (height * 5) / 4;
        const int rightBorder = 4;

        if (icon != nullptr)
        {
			icon->drawWithin (g, Rectangle<float>(2, 1, leftBorder - 4, height - 2), RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
        }
        else if (isTicked)
        {
            const Path tick (getTickShape (1.0f));
            const float th = font.getAscent();
            const float ty = halfH - th * 0.5f;

            g.fillPath (tick, tick.getTransformToScaleToFit (2.0f, ty, (float) (leftBorder - 4),
                                                             th, true));
        }

        g.drawFittedText (text,
                          leftBorder, 0,
                          width - (leftBorder + rightBorder), height,
                          Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            Font f2 (font);
            f2.setHeight (f2.getHeight() * 0.75f);
            f2.setHorizontalScale (0.95f);
            g.setFont (f2);

            g.drawText (shortcutKeyText,
                        leftBorder,
                        0,
                        width - (leftBorder + rightBorder + 4),
                        height,
                        Justification::centredRight,
                        true);
        }

        if (hasSubMenu)
        {
            const float arrowH = 0.6f * getPopupMenuFont().getAscent();
            const float x = width - height * 0.6f;

            Path p;
            p.addTriangle (x, halfH - arrowH * 0.5f,
                           x, halfH + arrowH * 0.5f,
                           x + arrowH * 0.6f, halfH);

            g.fillPath (p);
        }
    }
}

void CtrlrCombo::CtrlrComboLF::drawComboBox (Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box)
{
	int bw=buttonW;
	int bx=buttonX;
	const float outlineThickness = isButtonDown ? 1.2f : 0.5f;

	g.fillAll (box.findColour (ComboBox::backgroundColourId));
    g.setColour (box.findColour (ComboBox::outlineColourId));
    g.drawRect (0, 0, width, height);

	if ((bool)owner.getProperty (Ids::uiComboButtonWidthOverride) == true)
	{
		bw = owner.getProperty (Ids::uiComboButtonWidth);
		bx = width - bw;
	}

    const Colour baseColour (createBaseColour (box.findColour (ComboBox::buttonColourId),
                                                                   box.hasKeyboardFocus (true),
                                                                   false, isButtonDown)
                                .withMultipliedAlpha (1.0f));

	if ((bool)owner.getProperty (Ids::uiComboButtonGradient) == true)
	{
		g.setGradientFill (
							ColourGradient (
												VAR2COLOUR(owner.getProperty(Ids::uiComboButtonGradientColour1)),
												buttonX + outlineThickness,
												buttonY + outlineThickness,
												VAR2COLOUR(owner.getProperty(Ids::uiComboButtonGradientColour2)),
												buttonX + outlineThickness,
												(buttonY + outlineThickness) + (buttonH - outlineThickness * 2.0f),
												false
											)
						);

		g.fillRect (buttonX + outlineThickness, buttonY + outlineThickness, buttonW - outlineThickness * 2.0f, buttonH - outlineThickness * 2.0f);
	}
	else
	{
		drawGlassLozenge (g,
							bx + outlineThickness,
							buttonY + outlineThickness,
							bw - outlineThickness * 2.0f,
							buttonH - outlineThickness * 2.0f,
							baseColour,
							outlineThickness, -1.0f,
							true,
							true,
							true,
							true);
	}

	const float arrowX = 0.3f;
    const float arrowH = 0.2f;

    Path p;
    p.addTriangle (	bx + bw * 0.5f,
					buttonY + buttonH * (0.45f - arrowH),
					bx + bw * (1.0f - arrowX), buttonY + buttonH * 0.45f,
                    bx + bw * arrowX,          buttonY + buttonH * 0.45f);

    p.addTriangle (	bx + bw * 0.5f,
					buttonY + buttonH * (0.55f + arrowH),
                    bx + bw * (1.0f - arrowX), buttonY + buttonH * 0.55f,
                    bx + bw * arrowX,          buttonY + buttonH * 0.55f);

    g.setColour (box.findColour (ComboBox::arrowColourId));
    g.fillPath (p);
}

const Colour CtrlrCombo::CtrlrComboLF::createBaseColour (const Colour& buttonColour, const bool hasKeyboardFocus, const bool isMouseOverButton, const bool isButtonDown)
{
	const float sat = hasKeyboardFocus ? 1.3f : 0.9f;
	const Colour baseColour (buttonColour.withMultipliedSaturation (sat));

	if (isButtonDown)
		return baseColour.contrasting (0.2f);
	else if (isMouseOverButton)
		return baseColour.contrasting (0.1f);

	return baseColour;
}

void CtrlrCombo::CtrlrComboLF::positionComboBoxText (ComboBox& box, Label& label)
{
	int bw			= owner.getProperty (Ids::uiComboButtonWidth);

	if ((bool)owner.getProperty (Ids::uiComboButtonWidthOverride) == true)
	{
		label.setBounds (1, 1, box.getWidth() - bw, box.getHeight() - 2);
	}
	else
	{
		label.setBounds (1, 1, box.getWidth() + 3 - box.getHeight(), box.getHeight() - 2);
	}

    label.setFont (getComboBoxFont (box));
}

void CtrlrCombo::CtrlrComboLF::fillLabelTextEditorBackground (Graphics& g, TextEditor& editor)
{
     // Force the background to match your ComboBox background property
     g.fillAll (VAR2COLOUR(owner.getProperty(Ids::uiComboBgColour)));
    
     // Draw an outline if you want one
     g.setColour (VAR2COLOUR(owner.getProperty(Ids::uiComboOutlineColour)));
     g.drawRect (0, 0, editor.getWidth(), editor.getHeight());
}

Font CtrlrCombo::CtrlrComboLF::getComboBoxFont (ComboBox &box)
{
    return (owner.getOwner().getOwnerPanel().getCtrlrManagerOwner().getFontManager().getFontFromString (owner.getProperty(Ids::uiComboFont)));
}

Font CtrlrCombo::CtrlrComboLF::getPopupMenuFont ()
{
    return (owner.getOwner().getOwnerPanel().getCtrlrManagerOwner().getFontManager().getFontFromString (owner.getProperty(Ids::uiComboMenuFont)));
}

Font CtrlrCombo::CtrlrComboLF::getLabelFont (Label& label) // Added v5.6.35. Font styling sans Colour
{
    return owner.getOwner().getOwnerPanel().getCtrlrManagerOwner().getFontManager().getFontFromString (owner.getProperty(Ids::uiComboFont));
}
