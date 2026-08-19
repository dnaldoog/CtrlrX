#include "CtrlrButton.h"
#include "CtrlrIDs.h"
#include "CtrlrValueMap.h"
#include "stdafx.h"

#include "CtrlrModulator/CtrlrModulator.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"

CtrlrButton::CtrlrButton(CtrlrModulator &owner) : CtrlrComponent(owner), ctrlrButton(nullptr) {
	valueMap = std::make_unique<CtrlrValueMap>();
	// 1. Allocate the TextButton inside the unique_ptr container using std::make_unique
	ctrlrButton = std::make_unique<TextButton>("ctrlrButton");

	// 2. Pass the underlying raw address to JUCE's UI tree via .get()
	addAndMakeVisible(ctrlrButton.get());
	ctrlrButton->addListener(this);

	setProperty(Ids::uiButtonLookAndFeel, "Default");
	setProperty(Ids::uiButtonLookAndFeelIsCustom, false);

	ctrlrButton->addMouseListener(this, true);
	ctrlrButton->setBufferedToImage(true);
	setProperty(Ids::uiButtonIsToggle, true);
	setProperty(Ids::uiButtonTrueValue, 1);
	setProperty(Ids::uiButtonFalseValue, 0);
	setProperty(Ids::uiButtonContent, "False\nTrue");

	setProperty(Ids::uiButtonRepeat, false);
	setProperty(Ids::uiButtonRepeatRate, 100);
	setProperty(Ids::uiButtonTriggerOnMouseDown, false);
	setProperty(Ids::componentInternalFunction, COMBO_ITEM_NONE);

	bool LegacyMode = owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLegacyMode);
	String panelLnF = owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel);
	// REPLACED THE LEAKY 'NEW' CALLS:
	// Safely apply look-and-feel style rules from shared pointers
	applyCentralLookAndFeel(ctrlrButton.get(), panelLnF);

	if (panelLnF == "V3" || panelLnF == "V2" || panelLnF == "V1") {
		setSize(88, 32);
		setProperty(Ids::uiButtonColourOn, "0xff0000ff");
		setProperty(Ids::uiButtonColourOff, "0xff4364ff");
		setProperty(Ids::uiButtonTextColourOn, "0xff000000");
		setProperty(Ids::uiButtonTextColourOff, "0xff454545");
	} else {
		setSize(88, 64);
		setProperty(Ids::uiButtonColourOn, (String)findColour(TextButton::buttonOnColourId).toString());
		setProperty(Ids::uiButtonColourOff, (String)findColour(TextButton::buttonColourId).toString());
		setProperty(Ids::uiButtonTextColourOn, (String)findColour(TextButton::textColourOnId).toString());
		setProperty(Ids::uiButtonTextColourOff, (String)findColour(TextButton::textColourOffId).toString());
	}

	setProperty(Ids::uiButtonConnectedLeft, false);
	setProperty(Ids::uiButtonConnectedRight, false);
	setProperty(Ids::uiButtonConnectedTop, false);
	setProperty(Ids::uiButtonConnectedBottom, false);

	setProperty(Ids::uiButtonLookAndFeel, "Default");
	setProperty(Ids::uiButtonLookAndFeelIsCustom, false); // Default to Use LNF Settings
	// DO NOT set explicit uiButtonColourOn / uiButtonColourOff properties here!
	// Instead, call updateComponentColors() at the end of constructor:
	updateComponentColors();
}

CtrlrButton::~CtrlrButton() {
	// Cleanly unbind the button from any shared look-and-feel instances
	// before deleting the pointer to ensure the weak reference count hits 0.
	if (ctrlrButton != nullptr) {
		ctrlrButton->setLookAndFeel(nullptr);
		customLF.reset(); // Safely delete the custom LookAndFeel object if it exists
	}

	// Now safely delete the UI component pointer completely out of memory
	// deleteAndZero (ctrlrButton);
	customLF.reset(); // Safely delete the custom LookAndFeel object if it exists
}

//==============================================================================
void CtrlrButton::paint(Graphics &g) {}

void CtrlrButton::resized() {
	ctrlrButton->setBounds(getUsableRect());
}

void CtrlrButton::buttonClicked(Button *buttonThatWasClicked) {
	//[UserbuttonClicked_Pre]
	if (isInternal()) {
		owner.getOwnerPanel().performInternalComponentFunction(this);
		return;
	}

	if (!owner.getOwnerPanel().checkRadioGroup(this, buttonThatWasClicked->getToggleState()))
		return;
	//[/UserbuttonClicked_Pre]

	if (buttonThatWasClicked == ctrlrButton.get()) {
		//[UserButtonCode_ctrlrButton] -- add your button handler code here..
		valueMap->increment();
		ctrlrButton->setButtonText(valueMap->getCurrentText());
		setComponentValue(valueMap->getCurrentNonMappedValue(), true);
		//[/UserButtonCode_ctrlrButton]
	}

	//[UserbuttonClicked_Post]
	//[/UserbuttonClicked_Post]
}

void CtrlrButton::mouseDown(const MouseEvent &e) {
	//[UserCode_mouseDown] -- Add your code here...
	if ((bool)getProperty(Ids::uiButtonTriggerOnMouseDown) == true) {
		if (e.eventComponent == ctrlrButton.get()) {
			if (!isTimerRunning() && (bool)getProperty(Ids::uiButtonRepeat)) {
				startTimer((int)getProperty(Ids::uiButtonRepeatRate));
			}

			if (getProperty(Ids::uiButtonTriggerOnMouseDown)) {
				ctrlrButton->triggerClick();
			}
		}
	}
	CtrlrComponent::mouseDown(e);
	//[/UserCode_mouseDown]
}

//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void CtrlrButton::timerCallback() {
	if (ctrlrButton->isMouseButtonDown()) {
		ctrlrButton->triggerClick();
	} else {
		stopTimer();
	}
}

void CtrlrButton::setComponentValue(const double newValue, const bool sendChangeMessage) {
	valueMap->setCurrentNonMappedValue(newValue);
	ctrlrButton->setButtonText(valueMap->getTextForIndex(newValue));

	if (ctrlrButton->getClickingTogglesState()) {
		if ((double)getProperty(Ids::uiButtonTrueValue) == newValue) {
			ctrlrButton->setToggleState(true, dontSendNotification);
			valueMap->setCurrentNonMappedValue(1);
		} else {
			ctrlrButton->setToggleState(false, dontSendNotification);
			valueMap->setCurrentNonMappedValue(0);
		}
	}

	if (sendChangeMessage) {
		owner.getProcessor().setValueGeneric(CtrlrModulatorValue(newValue, CtrlrModulatorValue::changedByGUI),
											 sendChangeMessage);
	}
}

double CtrlrButton::getComponentMaxValue() {
	return (valueMap->getNonMappedMax());
}

bool CtrlrButton::getToggleState() {
	return (ctrlrButton->getToggleState());
}

const String CtrlrButton::getComponentText() {
	return (ctrlrButton->getButtonText());
}

void CtrlrButton::setComponentText(const String &componentText) {
	setComponentValue(valueMap->getNonMappedValue(componentText));
}

double CtrlrButton::getComponentValue() {
	return (valueMap->getCurrentNonMappedValue());
}

int CtrlrButton::getComponentMidiValue() {
	return (valueMap->getCurrentMappedValue());
}

void CtrlrButton::buttonContentChanged() {
	valueMap->copyFrom(owner.getProcessor().setValueMap(getProperty(Ids::uiButtonContent)));
	setComponentValue(0, false);
}

void CtrlrButton::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) {
	DBG("!!!! valueTreePropertyChanged: " + property.toString());

	if (ctrlrButton == nullptr) {
		CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
		return;
	}

	if (property == Ids::uiButtonContent) {
		buttonContentChanged();
	} else if (property == Ids::uiButtonLookAndFeel || property == Ids::uiPanelLookAndFeel) {
		String localStyle = getProperty(Ids::uiButtonLookAndFeel).toString();

		// 1. Unlink existing LookAndFeel
		ctrlrButton->setLookAndFeel(nullptr);

		// 2. Assign the new LookAndFeel theme FIRST
		if (localStyle.isEmpty() || localStyle == "Default") {
			customLF.reset();

			String effectiveStyle = localStyle;
			if (auto *editor = owner.getOwnerPanel().getEditor()) {
				effectiveStyle = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
			}

			applyCentralLookAndFeel(ctrlrButton.get(), effectiveStyle);
		} else {
			customLF = std::move(CtrlrButton::getLookAndFeelFromComponentProperty(localStyle));

			if (customLF != nullptr) {
				ctrlrButton->setLookAndFeel(customLF.get());
			}
		}

		// 3. Notify JUCE that LookAndFeel changed
		ctrlrButton->lookAndFeelChanged();

		// 4. NOW apply/clear color overrides against the NEW LookAndFeel
		updateComponentColors();

		ctrlrButton->repaint();
		repaint();
	} else if (property == Ids::uiButtonLookAndFeelIsCustom) {
		// Mode toggle changed ("Use My Colours" <-> "Use LNF")
		updateComponentColors();
	} else if (property == Ids::uiButtonColourOff || property == Ids::uiButtonColourOn) {
		const String backupKey = property.toString() + "_UserBackup";

		// Check if this property change was caused by LNF sync vs manual user picker edit
		const bool isCustomMode = (bool)getProperty(Ids::uiButtonLookAndFeelIsCustom);

		if (isCustomMode) {
			// User explicitly edited a color picker while in "Use My Colours" mode
			ctrlrButton->setColour(TextButton::buttonOnColourId, VAR2COLOUR(getProperty(Ids::uiButtonColourOn)));
			ctrlrButton->setColour(TextButton::buttonColourId, VAR2COLOUR(getProperty(Ids::uiButtonColourOff)));
			ctrlrButton->repaint();
		}

	} else if (property == Ids::uiButtonLookAndFeelIsCustom) {
		// Toggled between "Use User Settings" and "Use LNF Settings"
		updateComponentColors();
	} else if (property == Ids::uiButtonColourOff || property == Ids::uiButtonColourOn) {
		const bool isCustomMode = (bool)getProperty(Ids::uiButtonLookAndFeelIsCustom);

		if (!isCustomMode) {
			// Scenario 3: User edited a color picker while in "LNF Settings" mode!
			// Freeze current LNF colors into user properties and switch mode to "Use User Settings"
			LNF::freezeLnfToUserSettings(*ctrlrButton, getComponentTree(), Ids::uiButtonLookAndFeelIsCustom,
										 Ids::uiButtonColourOn, Ids::uiButtonColourOff,
										 juce::TextButton::buttonOnColourId, juce::TextButton::buttonColourId);
		} else {
			// User explicitly edited color in Custom Mode
			updateComponentColors();
		}
	} else if (property == Ids::uiButtonTextColourOff || property == Ids::uiButtonTextColourOn) {
		ctrlrButton->setColour(TextButton::textColourOffId, VAR2COLOUR(getProperty(Ids::uiButtonTextColourOff)));
		ctrlrButton->setColour(TextButton::textColourOnId, VAR2COLOUR(getProperty(Ids::uiButtonTextColourOn)));
	} else if (property == Ids::uiButtonIsToggle) {
		ctrlrButton->setClickingTogglesState((bool)getProperty(property));
	} else if (property == Ids::uiButtonConnectedLeft || property == Ids::uiButtonConnectedRight ||
			   property == Ids::uiButtonConnectedTop || property == Ids::uiButtonConnectedBottom) {
		const int leftFlag = (bool)getProperty(Ids::uiButtonConnectedLeft) ? Button::ConnectedOnLeft : 0;
		const int rightFlag = (bool)getProperty(Ids::uiButtonConnectedRight) ? Button::ConnectedOnRight : 0;
		const int topFlag = (bool)getProperty(Ids::uiButtonConnectedTop) ? Button::ConnectedOnTop : 0;
		const int bottomFlag = (bool)getProperty(Ids::uiButtonConnectedBottom) ? Button::ConnectedOnBottom : 0;

		ctrlrButton->setConnectedEdges(leftFlag | rightFlag | topFlag | bottomFlag);
	} else if (property == Ids::uiButtonTrueValue) {
		owner.setProperty(Ids::modulatorMax, getProperty(property));
	} else if (property == Ids::uiButtonFalseValue) {
		owner.setProperty(Ids::modulatorMin, getProperty(property));
	} else if (property == Ids::uiButtonRepeat) {
		if ((bool)getProperty(property) == false) {
			stopTimer();
		}
	} else {
		CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
	}

	if (!restoreStateInProgress) {
		resized();
	}
}

void CtrlrButton::updateComponentColors() {
    if (ctrlrButton == nullptr)
        return;

    LNF::applyLookAndFeelState(*ctrlrButton, 
                               getComponentTree(), 
                               Ids::uiButtonLookAndFeelIsCustom,
                               Ids::uiButtonColourOn, 
                               Ids::uiButtonColourOff, 
                               juce::TextButton::buttonOnColourId, 
                               juce::TextButton::buttonColourId);

    // Refresh the Property Inspector so the colour pickers reflect the updated tree values!
    updatePropertiesPanel();
}

void CtrlrButton::click() {
	ctrlrButton->triggerClick();
}

bool CtrlrButton::isToggleButton() {
	return (ctrlrButton->getClickingTogglesState());
}

void CtrlrButton::setToggleState(const bool toggleState, const bool sendChangeMessage) {
	ctrlrButton->setToggleState(toggleState, sendChangeMessage ? sendNotification : dontSendNotification);
}

std::unique_ptr<juce::LookAndFeel>
CtrlrButton::getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty) // Updated v5.6.34
{
	if (lookAndFeelComponentProperty == "Default") {
		// This case still means "use the default LookAndFeel (which might be the global one)"
		// so returning nullptr is appropriate if that's the desired behavior.
		return nullptr;
	}

	// Call your new generic factory function
	// We pass 'false' for the second argument here, as 'Default' is handled separately
	// and an unknown string should likely result in nullptr to fall back to the global L&F.
	return gui::createLookAndFeelFromDescription(lookAndFeelComponentProperty, false);
}

void CtrlrButton::resetLookAndFeelOverrides() {
	if (restoreStateInProgress ==
		false) // To prevent the props lines position stacking up to top and keep their original position
	{
		setProperty(Ids::componentLabelColour, (String)findColour(Label::textColourId).toString());

		setProperty(Ids::uiButtonColourOn, (String)findColour(TextButton::buttonOnColourId).toString());
		setProperty(Ids::uiButtonColourOff, (String)findColour(TextButton::buttonColourId).toString());
		setProperty(Ids::uiButtonTextColourOn, (String)findColour(TextButton::textColourOnId).toString());
		setProperty(Ids::uiButtonTextColourOff, (String)findColour(TextButton::textColourOffId).toString());

		setProperty(Ids::uiButtonLookAndFeelIsCustom,
					false); // Resets the component colourScheme if a new default colourScheme is selected from the menu

		updatePropertiesPanel(); // Refreshes property pane
	}
}

void CtrlrButton::updatePropertiesPanel() {
    // 1. Don't trigger panel updates while restoring state or initializing
    if (restoreStateInProgress)
        return;

    // 2. Safely check every pointer in the chain
    if (auto* panel = owner.getOwnerPanel().getEditor(false)) {
        if (auto* props = panel->getPropertiesPanel()) {
            props->refreshAll();
        }
    }
}

// #endif

//[/MiscUserCode]

//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CtrlrButton" componentName=""
                 parentClasses="public CtrlrComponent" constructorParams="CtrlrModulator &amp;owner"
                 variableInitialisers="CtrlrComponent(owner)" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330000013" fixedSize="1" initialWidth="88"
                 initialHeight="32">
  <METHODS>
    <METHOD name="mouseDown (const MouseEvent&amp; e)"/>
  </METHODS>
  <BACKGROUND backgroundColour="ffffff"/>
  <TEXTBUTTON name="ctrlrButton" id="d906fca95b2d6ff7" memberName="ctrlrButton"
              virtualName="" explicitFocusOrder="0" pos="0 0 0M 0M" buttonText="Button"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
