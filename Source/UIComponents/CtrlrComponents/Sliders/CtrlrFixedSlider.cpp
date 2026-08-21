#include "stdafx.h"

#include "CtrlrFixedSlider.h"
#include "CtrlrLuaManager.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "Lua/JuceClasses/LLookAndFeel.h"

CtrlrFixedSlider::CtrlrFixedSlider(CtrlrModulator &owner) : CtrlrComponent(owner) {
	valueMap = std::make_unique<CtrlrValueMap>();

	setColour(TooltipWindow::textColourId, findColour(Label::textColourId));
	setColour(TooltipWindow::backgroundColourId, findColour(TooltipWindow::backgroundColourId));
	setColour(TooltipWindow::outlineColourId, findColour(TooltipWindow::outlineColourId));

	ctrlrSlider = std::make_unique<CtrlrSliderInternal>(*this);
	addAndMakeVisible(ctrlrSlider.get());

	ctrlrSlider->setName("ctrlrSlider");
	ctrlrSlider->addListener(this);

	// Default slider properties...
	setProperty(Ids::uiSliderMin, 0);
	setProperty(Ids::uiSliderMax, 1);
	setProperty(Ids::uiSliderValueSuffix, "");
	setProperty(Ids::uiSliderSetNotificationOnlyOnRelease, false);
	setProperty(Ids::uiSliderDoubleClickEnabled, true);
	setProperty(Ids::uiSliderDoubleClickValue, 0);

	setProperty(Ids::uiSliderVelocitySensitivity, 1.0);
	setProperty(Ids::uiSliderVelocityThreshold, 1);
	setProperty(Ids::uiSliderVelocityOffset, 0.0);
	setProperty(Ids::uiSliderVelocityMode, false);
	setProperty(Ids::uiSliderVelocityModeKeyTrigger, true);

	setProperty(Ids::uiSliderSpringMode, false);
	setProperty(Ids::uiSliderSpringValue, 0);

	setProperty(Ids::uiSliderMouseWheelInterval, 1);
	setProperty(Ids::uiFixedSliderContent, "");

	setProperty(Ids::uiSliderLookAndFeel, "Default");
	setProperty(Ids::uiSliderLookAndFeelIsCustom, true);
	setProperty(Ids::uiSliderPopupBubble, false);
	setProperty(Ids::uiSliderStyle, "RotaryVerticalDrag");

	String panelLnF = "V3";
	if (auto *editor = owner.getOwnerPanel().getEditor()) {
		panelLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
	}

	applyCentralLookAndFeel(this, panelLnF);
	ctrlrSlider->setLookAndFeel(&getLookAndFeel());

	if (panelLnF == "V3" || panelLnF == "V2" || panelLnF == "V1") {
		setSize(64, 64);
		setProperty(Ids::uiSliderRotaryOutlineColour, "0xff0000ff");
		setProperty(Ids::uiSliderRotaryFillColour, "0xff0000ff");
		setProperty(Ids::uiSliderTrackColour, "0xff0f0f0f");
		setProperty(Ids::uiSliderThumbColour, "0xffff0000");
	} else {
		setSize(72, 96);
		setProperty(Ids::uiSliderRotaryOutlineColour,
					(String)findColour(Slider::rotarySliderOutlineColourId).toString());
		setProperty(Ids::uiSliderRotaryFillColour, (String)findColour(Slider::rotarySliderFillColourId).toString());
		setProperty(Ids::uiSliderTrackColour, (String)findColour(Slider::rotarySliderFillColourId).toString());
		setProperty(Ids::uiSliderThumbColour, (String)findColour(Slider::thumbColourId).toString());
	}

	setProperty(Ids::uiSliderIncDecButtonColour, (String)findColour(Slider::backgroundColourId).toString());
	setProperty(Ids::uiSliderIncDecTextColour, (String)findColour(Label::textColourId).toString());

	setProperty(Ids::uiSliderTrackCornerSize, 5);
	setProperty(Ids::uiSliderThumbCornerSize, 3);
	setProperty(Ids::uiSliderThumbWidth, 0);
	setProperty(Ids::uiSliderThumbHeight, 0);

	setProperty(Ids::uiSliderValuePosition, (int)Slider::TextBoxBelow);
	setProperty(Ids::uiSliderValueWidth, 64);
	setProperty(Ids::uiSliderValueHeight, 10);
	setProperty(Ids::uiSliderValueTextJustification, "centred");
	setProperty(Ids::uiSliderValueFont, FONT2STR(Font(12)));
	setProperty(Ids::uiSliderValueTextColour, (String)findColour(Slider::textBoxTextColourId).toString());
	setProperty(Ids::uiSliderValueBgColour, "0x00ffffff");
	setProperty(Ids::uiSliderValueHighlightColour, (String)findColour(Slider::textBoxHighlightColourId).toString());
	setProperty(Ids::uiSliderValueOutlineColour, "0x00ffffff");

	componentTree.addListener(this);
}

CtrlrFixedSlider::~CtrlrFixedSlider() {
	componentTree.removeListener(this);
	if (ctrlrSlider != nullptr) {
		ctrlrSlider->setLookAndFeel(nullptr);
	}
	setLookAndFeel(nullptr);
}
// CtrlrFixedSlider::~CtrlrFixedSlider() {
// 	// 2. Clear out the look and feel reference before deleting the child sub-component
// 	if (ctrlrSlider != nullptr) {
// 		ctrlrSlider->setLookAndFeel(nullptr);
// 		customLF.reset();
// 	}

// 	// deleteAndZero(ctrlrSlider);
// }

//==============================================================================
void CtrlrFixedSlider::updateComponentColors() {
	if (ctrlrSlider != nullptr) {
		LNF::applyLookAndFeelState(*ctrlrSlider, getComponentTree(), Ids::uiSliderLookAndFeelIsCustom,
								   {{Ids::uiSliderValueTextColour, juce::Slider::textBoxTextColourId},
									{Ids::uiSliderValueBgColour, juce::Slider::textBoxBackgroundColourId},
									{Ids::uiSliderRotaryOutlineColour, juce::Slider::rotarySliderOutlineColourId},
									{Ids::uiSliderRotaryFillColour, juce::Slider::rotarySliderFillColourId},
									{Ids::uiSliderThumbColour, juce::Slider::thumbColourId},
									{Ids::uiSliderValueHighlightColour, juce::Slider::textBoxHighlightColourId},
									{Ids::uiSliderValueOutlineColour, juce::Slider::textBoxOutlineColourId},
									{Ids::uiSliderTrackColour, juce::Slider::trackColourId}});

		ctrlrSlider->lookAndFeelChanged();
		ctrlrSlider->repaint();
	}
}
// void CtrlrFixedSlider::updateComponentFonts() {
// 	if (ctrlrSlider == nullptr)
// 		return;

// 	LNF::applyFontState(*ctrlrSlider, getComponentTree(), Ids::uiSliderLookAndFeelIsCustom,
// 						{{Ids::uiSliderValueFont, [](const juce::Font &) {}}});

// 	ctrlrSlider->repaint();
// }

void CtrlrFixedSlider::lookAndFeelChanged() {
	if (ctrlrSlider != nullptr) {
		ctrlrSlider->setLookAndFeel(&getLookAndFeel());
		updateComponentColors();
		ctrlrSlider->sendLookAndFeelChange();
	}
	CtrlrComponent::lookAndFeelChanged();
}

void CtrlrFixedSlider::customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel) {
	DBG("!*!*!*!* Hit CtrlrFixedSlider::customLookAndFeelChanged");
	if (ctrlrSlider == nullptr)
		return;

	if (customLookAndFeel != nullptr) {
		// Cast customLookAndFeel to juce::LookAndFeel*
		if (auto *juceLF = dynamic_cast<juce::LookAndFeel *>(customLookAndFeel)) {
			ctrlrSlider->setLookAndFeel(juceLF);
		} else {
			ctrlrSlider->setLookAndFeel(nullptr);
		}
	} else {
		ctrlrSlider->setLookAndFeel(nullptr);

		String panelLnF = getProperty(Ids::uiSliderLookAndFeel).toString();
		if (panelLnF.isEmpty() || panelLnF == "Default") {
			if (auto *editor = owner.getOwnerPanel().getEditor()) {
				panelLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
			}
		}

		applyCentralLookAndFeel(this, panelLnF);
		ctrlrSlider->setLookAndFeel(&getLookAndFeel());
	}

	updateComponentColors();
	ctrlrSlider->sendLookAndFeelChange();
	ctrlrSlider->repaint();
}

const String CtrlrFixedSlider::getCurrentLF() {
	return getProperty(Ids::uiSliderLookAndFeel);
}

void CtrlrFixedSlider::paint(Graphics &g) {
	//[UserPrePaint] Add your own custom painting code here..
	//[/UserPrePaint]

	//[UserPaint] Add your own custom painting code here..
	//[/UserPaint]
}

void CtrlrFixedSlider::resized() {
	// ctrlrSlider->setBounds (0, 0, getWidth() - 0, getHeight() - 0);
	//[UserResized] Add your own custom resize handling here..
	if (restoreStateInProgress)
		return;
	ctrlrSlider->setBounds(getUsableRect());
	//[/UserResized]
}

void CtrlrFixedSlider::mouseUp(const MouseEvent &e) {
	//[UserCode_mouseUp] -- Add your code here...
	if (mouseUpCbk && !mouseUpCbk.wasObjectDeleted()) {
		if (mouseUpCbk->isValid()) {
			owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().call(mouseUpCbk, this, e);
		}
	}
	if ((bool)getProperty(Ids::uiSliderSpringMode) == true) {
		ctrlrSlider->setValue((double)getProperty(Ids::uiSliderSpringValue), sendNotificationSync);
	}
	//[/UserCode_mouseUp]
}

//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
double CtrlrFixedSlider::getComponentMaxValue() {
	return (valueMap->getNonMappedMax());
}

double CtrlrFixedSlider::getComponentValue() {
	return ((int)ctrlrSlider->getValue());
}

int CtrlrFixedSlider::getComponentMidiValue() {
	return (valueMap->getMappedValue(ctrlrSlider->getValue()));
}

const String CtrlrFixedSlider::getComponentText() {
	return (valueMap->getTextForIndex(ctrlrSlider->getValue()));
}

void CtrlrFixedSlider::setComponentValue(const double newValue, const bool sendChangeMessage) {
	ctrlrSlider->setValue(newValue, dontSendNotification);
	if (sendChangeMessage) {
		owner.getProcessor().setValueGeneric(CtrlrModulatorValue(newValue, CtrlrModulatorValue::changedByGUI));
	}
}

void CtrlrFixedSlider::sliderContentChanged() {
	String values = getProperty(Ids::uiFixedSliderContent);
	if (values.isNotEmpty()) {
		valueMap->copyFrom(owner.getProcessor().setValueMap(values));
		double max = valueMap->getNonMappedMax();
		const double min = valueMap->getNonMappedMin();
		// For JUCE MAX must be >= min
		if (max <= min) {
			// samething between 0.5 and 1 times the interval
			// to avoid rounding errors
			max = min + 0.66;
		}
		double interval = 1.0;
		ctrlrSlider->setRange(min, max, interval);
	}
}

void CtrlrFixedSlider::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) {
	if (property == Ids::uiSliderStyle) {
		ctrlrSlider->setSliderStyle(
			(Slider::SliderStyle)CtrlrComponentTypeManager::sliderStringToStyle(getProperty(Ids::uiSliderStyle)));
	} else if (property == Ids::uiSliderLookAndFeelIsCustom) {
		updateComponentColors();
	} else if (property == Ids::uiPanelLookAndFeel || property == Ids::uiSliderLookAndFeel) {
		String activeLnF = getProperty(Ids::uiSliderLookAndFeel).toString();
		if (activeLnF.isEmpty() || activeLnF == "Default") {
			if (auto *editor = owner.getOwnerPanel().getEditor())
				activeLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
		}

		ctrlrSlider->setLookAndFeel(nullptr);
		setLookAndFeel(nullptr);

		if (activeLnF == "V1" || activeLnF == "V2" || activeLnF == "V3" || activeLnF.isEmpty() ||
			activeLnF == "Default") {
			customLF.reset();
			applyCentralLookAndFeel(this, activeLnF);
		} else {
			customLF = std::move(getLookAndFeelFromComponentProperty(activeLnF));
			if (customLF != nullptr)
				setLookAndFeel(customLF.get());
		}

		ctrlrSlider->setLookAndFeel(&getLookAndFeel());

		updateComponentColors();
		ctrlrSlider->sendLookAndFeelChange();
		ctrlrSlider->repaint();
		repaint();
	} else if (property == Ids::uiSliderRotaryFillColour || property == Ids::uiSliderRotaryOutlineColour ||
			   property == Ids::uiSliderTrackColour || property == Ids::uiSliderThumbColour ||
			   property == Ids::uiSliderValueHighlightColour || property == Ids::uiSliderValueBgColour ||
			   property == Ids::uiSliderValueOutlineColour || property == Ids::uiSliderValueTextColour) {
		if (!restoreStateInProgress) {
			setProperty(Ids::uiSliderLookAndFeelIsCustom, true);
		}
		updateComponentColors();
	} else if (property == Ids::uiFixedSliderContent) {
		sliderContentChanged();
	} else if (property == Ids::uiSliderValueSuffix) {
		ctrlrSlider->setTextValueSuffix(getProperty(Ids::uiSliderValueSuffix).toString());
		ctrlrSlider->lookAndFeelChanged();
	} else if (property == Ids::uiSliderValuePosition || property == Ids::uiSliderValueHeight ||
			   property == Ids::uiSliderValueWidth) {
		ctrlrSlider->setTextBoxStyle((Slider::TextEntryBoxPosition)(int)getProperty(Ids::uiSliderValuePosition), false,
									 getProperty(Ids::uiSliderValueWidth, 64),
									 getProperty(Ids::uiSliderValueHeight, 12));
	} else if (property == Ids::uiSliderSetNotificationOnlyOnRelease) {
		ctrlrSlider->setChangeNotificationOnlyOnRelease((bool)getProperty(Ids::uiSliderSetNotificationOnlyOnRelease));
	} else if (property == Ids::uiSliderIncDecButtonColour || property == Ids::uiSliderIncDecTextColour ||
			   property == Ids::uiSliderValueFont || property == Ids::uiSliderValueTextJustification) {
		String panelLnF = "V3";
		if (auto *editor = owner.getOwnerPanel().getEditor()) {
			panelLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
		}

		applyCentralLookAndFeel(this, panelLnF);
		ctrlrSlider->setLookAndFeel(&getLookAndFeel());
		setProperty(Ids::uiSliderLookAndFeelIsCustom, true);
		repaint();
	} else if (property == Ids::uiSliderVelocityMode || property == Ids::uiSliderVelocityModeKeyTrigger ||
			   property == Ids::uiSliderVelocitySensitivity || property == Ids::uiSliderVelocityThreshold ||
			   property == Ids::uiSliderVelocityOffset) {
		ctrlrSlider->setVelocityBasedMode((bool)getProperty(Ids::uiSliderVelocityMode));
		ctrlrSlider->setVelocityModeParameters(
			(double)getProperty(Ids::uiSliderVelocitySensitivity), (int)getProperty(Ids::uiSliderVelocityThreshold),
			(double)getProperty(Ids::uiSliderVelocityOffset), (bool)getProperty(Ids::uiSliderVelocityModeKeyTrigger));
	} else if (property == Ids::uiSliderSpringValue) {
		ctrlrSlider->setValue(getProperty(property), dontSendNotification);
	} else if (property == Ids::uiSliderSpringMode) {
		if ((bool)getProperty(property) == true) {
			ctrlrSlider->setValue(getProperty(Ids::uiSliderSpringValue), dontSendNotification);
		}
	} else if (property == Ids::uiSliderDoubleClickValue || property == Ids::uiSliderDoubleClickEnabled) {
		ctrlrSlider->setDoubleClickReturnValue((bool)getProperty(Ids::uiSliderDoubleClickEnabled),
											   getProperty(Ids::uiSliderDoubleClickValue));
	} else if (property == Ids::uiSliderPopupBubble) {
		ctrlrSlider->setPopupDisplayEnabled((bool)getProperty(property), (bool)getProperty(property),
											owner.getOwnerPanel().getEditor());
	} else {
		CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
	}

	if (!restoreStateInProgress) {
		resized();
	}
}
const String CtrlrFixedSlider::getTextForValue(const double value) {
	return (valueMap->getTextForIndex(value));
}

void CtrlrFixedSlider::sliderValueChanged(Slider *sliderThatWasMoved) {
	setComponentValue(ctrlrSlider->getValue(), true);
}

std::unique_ptr<LookAndFeel>
CtrlrFixedSlider::getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty) // Updated v5.6.34
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

void CtrlrFixedSlider::resetLookAndFeelOverrides() {
	if (restoreStateInProgress ==
		false) // To prevent the prop lines stacking up from top and keeping their original position
	{
		setProperty(Ids::componentLabelColour, (String)findColour(Label::textColourId).toString());

		setProperty(Ids::uiSliderRotaryOutlineColour,
					(String)findColour(Slider::rotarySliderOutlineColourId).toString());
		setProperty(Ids::uiSliderRotaryFillColour, (String)findColour(Slider::rotarySliderFillColourId).toString());
		setProperty(Ids::uiSliderThumbColour, (String)findColour(Slider::thumbColourId).toString());

		setProperty(Ids::uiSliderTrackColour, (String)findColour(Slider::rotarySliderFillColourId).toString());

		setProperty(Ids::uiSliderIncDecTextColour, (String)findColour(Slider::textBoxTextColourId).toString());
		setProperty(Ids::uiSliderIncDecButtonColour, (String)findColour(Slider::backgroundColourId).toString());

		setProperty(Ids::uiSliderValueTextColour, (String)findColour(Slider::textBoxTextColourId).toString());
		setProperty(Ids::uiSliderValueHighlightColour, (String)findColour(Slider::textBoxHighlightColourId).toString());
		setProperty(Ids::uiSliderValueBgColour,
					"0x00ffffff"); // (String)findColour (Slider::textBoxBackgroundColourId).toString());
		setProperty(Ids::uiSliderValueOutlineColour,
					"0x00ffffff"); //(String)findColour (Slider::textBoxOutlineColourId).toString());

		setProperty(Ids::uiSliderLookAndFeelIsCustom,
					false); // Resets the component colourScheme if a new default colourScheme is
							// selected from the menu

		updatePropertiesPanel(); // Refreshes property pane
	}
}

void CtrlrFixedSlider::updatePropertiesPanel() {
	CtrlrPanelProperties *props = owner.getCtrlrManagerOwner().getActivePanel()->getEditor(false)->getPropertiesPanel();
	if (props) {
		props->refreshAll(); // Needs extra code to prevent scrolling back to top on refresh
	}
}

//[/MiscUserCode]

//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CtrlrFixedSlider" componentName=""
                 parentClasses="public CtrlrComponent, public SettableTooltipClient, public Slider::Listener"
                 constructorParams="CtrlrModulator &amp;owner" variableInitialisers="CtrlrComponent(owner), lf(componentTree)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="64" initialHeight="64">
  <METHODS>
    <METHOD name="mouseUp (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDoubleClick (const MouseEvent&amp; e)"/>
  </METHODS>
  <BACKGROUND backgroundColour="0"/>
  <GENERICCOMPONENT name="ctrlrSlider" id="725ab5397cee0647" memberName="ctrlrSlider"
                    virtualName="" explicitFocusOrder="0" pos="0 0 0M 0M" class="CtrlrOwnSlider"
                    params="*this, &quot;ctrlrSlider&quot;"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
