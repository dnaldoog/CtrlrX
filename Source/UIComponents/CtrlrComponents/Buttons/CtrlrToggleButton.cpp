#include "CtrlrToggleButton.h"
#include "CtrlrIDs.h"
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrModulator/CtrlrModulator.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrValueMap.h"
#include "stdafx.h"

CtrlrToggleButton::CtrlrToggleButton(CtrlrModulator &owner) : CtrlrComponent(owner) {
	valueMap = std::make_unique<CtrlrValueMap>();
	ctrlrButton = std::make_unique<ToggleButton>("ctrlrButton");
	addAndMakeVisible(ctrlrButton.get());
	ctrlrButton->setButtonText("Button");
	ctrlrButton->addListener(this);

	//[UserPreSize]
	ctrlrButton->setBufferedToImage(true);
	setProperty(Ids::uiToggleButtonText, "Button");
	setProperty(Ids::uiButtonTrueValue, 1);
	setProperty(Ids::uiButtonFalseValue, 0);
	owner.setProperty(Ids::modulatorMax, 1);
	owner.setProperty(Ids::modulatorMin, 0);
	setProperty(Ids::uiButtonIsRadioButton, false);
	setProperty(Ids::uiButtonLookAndFeel, "Default");
	setProperty(Ids::uiButtonLookAndFeelIsCustom, true);

	//[/UserPreSize]

	if (owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel) == "V3" ||
		owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel) == "V2" ||
		owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel) == "V1") {
		setProperty(Ids::uiButtonTextColourOn, "0xff000000");
		setProperty(Ids::uiButtonColourOff, "0xffff0000");
		setProperty(Ids::uiToggleButtonFocusOutline, "0xff0000ff");
		setProperty(Ids::uiToggleButtontickColour, "0xff0000ff");
	} else if (owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel) ==
			   "V4 Light") // Because V4 Light scheme returns white on white colours
	{
		setProperty(Ids::uiButtonTextColourOn, "0xff000000");		// Text colour
		setProperty(Ids::uiButtonColourOff, "0x80000000");			// V2 specific Colour
		setProperty(Ids::uiToggleButtonFocusOutline, "0x60000000"); // Tick box colour
		setProperty(Ids::uiToggleButtontickColour, "0xff000000");	// Tick colour
	} else {
		setProperty(Ids::uiButtonTextColourOn,
					(String)LookAndFeel::findColour(ToggleButton::textColourId).toString()); // Text colour
		setProperty(Ids::uiButtonColourOff, (String)LookAndFeel::findColour(ToggleButton::textColourId)
												.withAlpha(0.7f)
												.toString()); // V2 specific Colour
		setProperty(Ids::uiToggleButtonFocusOutline, (String)LookAndFeel::findColour(ToggleButton::tickDisabledColourId)
														 .withAlpha(0.5f)
														 .toString()); // Tick colour
		setProperty(Ids::uiToggleButtontickColour,
					(String)LookAndFeel::findColour(ToggleButton::tickColourId).toString()); // Tick colour
	}
	setSize(88, 48);

	setProperty(Ids::uiButtonLookAndFeelIsCustom,
				false); // Resets the component colourScheme if a new default colourScheme is selected from the menu

	//[Constructor] You can add your own custom stuff here..
	// owner.getProcessor().setValueFromGUI (0, true);
	//[/Constructor]
}

CtrlrToggleButton::~CtrlrToggleButton() {

	customLF.reset(); // Safely delete the custom LookAndFeel object if it exists
}

//==============================================================================
void CtrlrToggleButton::paint(Graphics &g) {
	//[UserPrePaint] Add your own custom painting code here..
	//[/UserPrePaint]

	//[UserPaint] Add your own custom painting code here..
	//[/UserPaint]
}

void CtrlrToggleButton::resized() {
	ctrlrButton->setBounds(0, 0, getWidth() - 0, getHeight() - 0);
	//[UserResized] Add your own custom resize handling here..
	//[/UserResized]
}

void CtrlrToggleButton::buttonClicked(Button *buttonThatWasClicked) {
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
		setComponentValue(ctrlrButton->getToggleState(), true);
		//[/UserButtonCode_ctrlrButton]
	}

	//[UserbuttonClicked_Post]
	//[/UserbuttonClicked_Post]
}

void CtrlrToggleButton::mouseDown(const MouseEvent &e) {
	//[UserCode_mouseDown] -- Add your code here...
	CtrlrComponent::mouseDown(e);
	//[/UserCode_mouseDown]
}

//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void CtrlrToggleButton::setComponentValue(const double newValue, const bool sendChangeMessage) {
	if (!owner.getOwnerPanel().checkRadioGroup(this, ctrlrButton->getToggleState()))
		return;

	if (ctrlrButton->getClickingTogglesState()) {
		ctrlrButton->setToggleState(newValue ? true : false, dontSendNotification);
	}

	if (sendChangeMessage) {
		owner.getProcessor().setValueGeneric(CtrlrModulatorValue(newValue, CtrlrModulatorValue::changedByGUI), true);
	}
}

void CtrlrToggleButton::setComponentMidiValue(const int newValue, const bool sendChangeMessage) {
	if (!owner.getOwnerPanel().checkRadioGroup(this, ctrlrButton->getToggleState()))
		return;

	if (ctrlrButton->getClickingTogglesState()) {
		ctrlrButton->setToggleState(valueMap->getIndexForValue(newValue) ? true : false, dontSendNotification);
	}

	if (sendChangeMessage) {
		owner.getProcessor().setValueGeneric(CtrlrModulatorValue(newValue, CtrlrModulatorValue::changedByGUI), true);
	}
}

double CtrlrToggleButton::getComponentMaxValue() {
	return (1);
}

bool CtrlrToggleButton::getToggleState() {
	return (ctrlrButton->getToggleState());
}

double CtrlrToggleButton::getComponentValue() {
	if (ctrlrButton->getToggleState()) {
		return (1);
	} else {
		return (0);
	}
}

int CtrlrToggleButton::getComponentMidiValue() {
	return (valueMap->getMappedValue(ctrlrButton->getToggleState()));
}

void CtrlrToggleButton::updateComponentColors() {
	if (ctrlrButton == nullptr)
		return;

	const bool isRadioButton = (bool)getProperty(Ids::uiButtonIsRadioButton);

	// 1. Re-enforce LookAndFeel pointer state based on Radio mode
	if (isRadioButton) {
		// ALWAYS re-apply custom radio LNF so panel theme switches don't override it to square
		ctrlrButton->setLookAndFeel(&owner.getOwnerPanel().getCustomRadioLNF());
	} else {
		// If a component-level custom LNF string is defined, use it; otherwise clear to inherit panel LNF
		String lookType = getProperty(Ids::uiButtonLookAndFeel);
		if (lookType != "Default" && customLF != nullptr) {
			ctrlrButton->setLookAndFeel(customLF.get());
		} else {
			ctrlrButton->setLookAndFeel(nullptr);
		}
	}

	// 2. Apply Custom vs. Theme colors using the exact helper signature
	if (isRadioButton) {
		LNF::applyLookAndFeelState(*ctrlrButton, getComponentTree(), Ids::uiButtonLookAndFeelIsCustom,
								   {{Ids::uiButtonTextColourOn, juce::ToggleButton::textColourId},
									{Ids::uiButtonColourOff, juce::ResizableWindow::backgroundColourId},
									{Ids::uiToggleButtontickColour, juce::ToggleButton::tickColourId},
									{Ids::uiToggleButtonFocusOutline, juce::ToggleButton::tickDisabledColourId}});
	} else {
		LNF::applyLookAndFeelState(*ctrlrButton, getComponentTree(), Ids::uiButtonLookAndFeelIsCustom,
								   {{Ids::uiButtonTextColourOn, juce::ToggleButton::textColourId},
									{Ids::uiButtonColourOff, juce::ToggleButton::tickDisabledColourId},
									{Ids::uiToggleButtontickColour, juce::ToggleButton::tickColourId}});
	}

	//   ctrlrButton->repaint(); stop changing property jusmps to top of editor panel
}

void CtrlrToggleButton::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) {
	if (property == Ids::componentRadioGroupId) {
		ctrlrButton->setToggleState(false, dontSendNotification);

		// getOwnerPanel() returns CtrlrPanel& — direct reference call
		owner.getOwnerPanel().setRadioGroupId(this, getProperty(Ids::componentRadioGroupId));
	} else if (property == Ids::uiButtonIsRadioButton) {
		// Toggle custom circular radio button LookAndFeel vs default square checkbox
		if ((bool)getProperty(Ids::uiButtonIsRadioButton)) {
			ctrlrButton->setLookAndFeel(&owner.getOwnerPanel().getCustomRadioLNF());
		} else {
			// Revert to default square toggle rendering
			ctrlrButton->setLookAndFeel(nullptr);

			// Re-apply component-level custom LookAndFeel if a specific one was selected
			String LookAndFeelType = getProperty(Ids::uiButtonLookAndFeel);
			if (LookAndFeelType != "Default" && customLF != nullptr) {
				ctrlrButton->setLookAndFeel(customLF.get());
			}
		}

		Component::SafePointer<CtrlrToggleButton> safeThis(this);
		MessageManager::callAsync([safeThis]() {
			if (safeThis == nullptr)
				return;

			ValueTree modulatorTree = safeThis->owner.getModulatorTree(); // confirm real accessor name

			if (auto *props = safeThis->owner.getOwnerPanel().getEditor()->getPropertiesPanel())
				props->refreshIfEditing(modulatorTree);
		});
	} else if (property == Ids::uiButtonLookAndFeel) {
		String LookAndFeelType = getProperty(property);

		// 1. Explicitly clear LNF pointers from BOTH parent and child component FIRST
		setLookAndFeel(nullptr);
		if (ctrlrButton != nullptr)
			ctrlrButton->setLookAndFeel(nullptr);

		// 2. Now safe to reset or assign the unique_ptr
		if (LookAndFeelType == "Default") {
			customLF.reset();
			if (ctrlrButton != nullptr && (bool)getProperty(Ids::uiButtonIsRadioButton))
				ctrlrButton->setLookAndFeel(&owner.getOwnerPanel().getCustomRadioLNF());
		} else {
			customLF = std::move(CtrlrToggleButton::getLookAndFeelFromComponentProperty(LookAndFeelType));
			if (customLF != nullptr) {
				setLookAndFeel(customLF.get());

				if (ctrlrButton != nullptr) {
					if ((bool)getProperty(Ids::uiButtonIsRadioButton))
						ctrlrButton->setLookAndFeel(&owner.getOwnerPanel().getCustomRadioLNF());
					else
						ctrlrButton->setLookAndFeel(customLF.get());
				}
			}
		}

		if (!getProperty(Ids::uiButtonLookAndFeelIsCustom) && !restoreStateInProgress) {
			resetLookAndFeelOverrides();
		}

		updateComponentColors();

	} else if (property == Ids::uiButtonLookAndFeelIsCustom) {
		if (!getProperty(Ids::uiButtonLookAndFeelIsCustom) && !restoreStateInProgress) {
			resetLookAndFeelOverrides();
		}
		updateComponentColors();
	} else if (property == Ids::uiButtonTextColourOn || property == Ids::uiButtonColourOff ||
			   property == Ids::uiToggleButtontickColour || property == Ids::uiToggleButtonFocusOutline) {
		if (!restoreStateInProgress)
			setProperty(Ids::uiButtonLookAndFeelIsCustom, true);
		updateComponentColors();
	} else if (property == Ids::uiToggleButtonText) {
		ctrlrButton->setButtonText(getProperty(Ids::uiToggleButtonText));
	} else if (property == Ids::uiButtonTrueValue || property == Ids::uiButtonFalseValue) {
		valueMap->setPair(0, getProperty(Ids::uiButtonFalseValue), "");
		valueMap->setPair(1, getProperty(Ids::uiButtonTrueValue), "");
		owner.getProcessor().setValueMap(*valueMap);
	} else {
		CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
	}

	if (restoreStateInProgress == false) {
		resized();
	}
}

void CtrlrToggleButton::click()
{
    ctrlrButton->triggerClick();
}

bool CtrlrToggleButton::isToggleButton()
{
	//    return (true);
	return (ctrlrButton->getClickingTogglesState());
}

void CtrlrToggleButton::setToggleState(const bool toggleState, const bool sendChangeMessage)
{
    ctrlrButton->setToggleState(toggleState, sendChangeMessage ? sendNotification : dontSendNotification);
}

std::unique_ptr<juce::LookAndFeel> CtrlrToggleButton::getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty) // Updated v5.6.34
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

void CtrlrToggleButton::resetLookAndFeelOverrides()
{
    if (restoreStateInProgress == false) // To prevent the props lines position stacking up to top and keep their original position
    {
        setProperty(Ids::componentLabelColour, (String)LookAndFeel::findColour(Label::textColourId).toString());

        if (getProperty(Ids::uiButtonLookAndFeel) == "V4 Light") // Because V4 Light scheme returns white on white colours
        {
			DBG("Resetting LookAndFeel overrides for V4 Light scheme");
			setProperty(Ids::uiButtonTextColourOn, "0xff000000");       // Text colour
            setProperty(Ids::uiButtonColourOff, "0x80000000");          // V2 specific Colour
            setProperty(Ids::uiToggleButtonFocusOutline, "0x60000000"); // Tick box colour
            setProperty(Ids::uiToggleButtontickColour, "0xff000000");   // Tick colour
		}
		/*      else
			  {
				  setProperty(Ids::uiButtonTextColourOn,
		   (String)LookAndFeel::findColour(ToggleButton::textColourId).toString());                               //
		   Text colour setProperty(Ids::uiButtonColourOff,
		   (String)LookAndFeel::findColour(ToggleButton::textColourId).withAlpha(0.7f).toString());                  //
		   V2 specific Colour setProperty(Ids::uiToggleButtonFocusOutline,
		   (String)LookAndFeel::findColour(ToggleButton::tickDisabledColourId).withAlpha(0.5f).toString()); // Tick
		   colour setProperty(Ids::uiToggleButtontickColour,
		   (String)LookAndFeel::findColour(ToggleButton::tickColourId).toString());                           // Tick
		   colour
			  }
	  */
	}
	setProperty(Ids::uiButtonLookAndFeelIsCustom,
				false); // Resets the component colourScheme if a new default colourScheme is selected from the menu

	// updatePropertiesPanel(); // I commented out to stop jump to top of page
}

void CtrlrToggleButton::updatePropertiesPanel()
{
	if (restoreStateInProgress)
		return;

	if (auto *panel = owner.getOwnerPanel().getEditor(false)) {
		if (auto *props = panel->getPropertiesPanel()) {
			// Refreshes values inside existing rows without rebuilding the panel layout/scroll position
			props->refreshIfEditing(owner.getModulatorTree());
		}
	}
} // bool isRadio = (bool)getProperty(Ids::uiButtonIsRadioButton);

// CtrlrPanelProperties does not expose a direct relabel API, so refresh the
// inspector labels by forcing a refresh of the property panel instead.
//[/MiscUserCode]

//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CtrlrToggleButton" componentName=""
                 parentClasses="public CtrlrComponent" constructorParams="CtrlrModulator &amp;owner"
                 variableInitialisers="CtrlrComponent(owner)" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330000013" fixedSize="1" initialWidth="88"
                 initialHeight="32">
  <METHODS>
    <METHOD name="mouseDown (const MouseEvent&amp; e)"/>
  </METHODS>
  <BACKGROUND backgroundColour="ffffff"/>
  <TOGGLEBUTTON name="ctrlrButton" id="ece5e33c201d706e" memberName="ctrlrButton"
                virtualName="" explicitFocusOrder="0" pos="0 0 0M 0M" buttonText="Button"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
