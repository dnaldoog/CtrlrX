#include "CtrlrSlider.h"
#include "../CtrlrComponentTypeManager.h"
#include "CtrlrLuaManager.h"
#include "CtrlrModulator/CtrlrModulator.h"
#include "CtrlrPanel/CtrlrPanelComponentProperties.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrPanel/CtrlrPanelProperties.h"
#include "CtrlrProcessor.h"
#include "Lua/JuceClasses/LLookAndFeel.h"
#include "stdafx.h"

CtrlrSlider::CtrlrSlider(CtrlrModulator &owner) : CtrlrComponent(owner), ctrlrSlider(*this) {
	setColour(TooltipWindow::textColourId, findColour(Label::textColourId));
	setColour(TooltipWindow::backgroundColourId, findColour(TooltipWindow::backgroundColourId));
	setColour(TooltipWindow::outlineColourId, findColour(TooltipWindow::outlineColourId));

	addAndMakeVisible(&ctrlrSlider);

	// Hardcode a safe internal baseline range & style so JUCE can boot without invariant errors
	ctrlrSlider.setRange(0, 127, 1);
	ctrlrSlider.setSliderStyle(Slider::RotaryVerticalDrag);
	ctrlrSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 64, 12);

	ctrlrSlider.addListener(this);

	// 1. Initialize core slider properties
	setProperty(Ids::uiSliderMin, 0);
	setProperty(Ids::uiSliderMax, 127);
	setProperty(Ids::uiSliderInterval, 1);
	setProperty(Ids::uiSliderDecimalPlaces, 0);
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

	setProperty(Ids::uiSliderLookAndFeel, "Default");

	setProperty(Ids::uiSliderPopupBubble, false);
	setProperty(Ids::uiSliderStyle, "RotaryVerticalDrag");

	// 2. Fetch look and feel setting safely from the panel editor
	String panelLnF = "V3";
	if (auto *editor = owner.getOwnerPanel().getEditor()) {
		panelLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
	}

	applyCentralLookAndFeel(&ctrlrSlider, panelLnF);

	// 3. Configure dimensional footprints based on the styling type
	if (panelLnF == "V3" || panelLnF == "V2" || panelLnF == "V1") {
		setSize(64, 64);
		setProperty(Ids::uiSliderRotaryOutlineColour, "0xff0000ff");
		setProperty(Ids::uiSliderRotaryFillColour, "0xff0000ff");
		setProperty(Ids::uiSliderThumbColour, "0xffff0000");
		setProperty(Ids::uiSliderTrackColour, "0xff0f0f0f");

		// Keep bounds safe from juce_MathsFunctions line 288 range exceptions
		setProperty(Ids::uiSliderValueWidth, 54);
		setProperty(Ids::uiSliderValueHeight, 12);
	} else {
		// V4 / Central palette initialization
		setSize(72, 96);
		setProperty(Ids::uiSliderRotaryOutlineColour,
					(String)findColour(Slider::rotarySliderOutlineColourId).toString());
		setProperty(Ids::uiSliderRotaryFillColour, (String)findColour(Slider::rotarySliderFillColourId).toString());
		setProperty(Ids::uiSliderThumbColour, (String)findColour(Slider::thumbColourId).toString());
		setProperty(Ids::uiSliderTrackColour, (String)findColour(Slider::rotarySliderFillColourId).toString());

		setProperty(Ids::uiSliderValueWidth, 64);
		setProperty(Ids::uiSliderValueHeight, 14);
	}

	setProperty(Ids::uiSliderIncDecButtonColour, (String)findColour(Slider::backgroundColourId).toString());
	setProperty(Ids::uiSliderIncDecTextColour, (String)findColour(Label::textColourId).toString());

	setProperty(Ids::uiSliderTrackCornerSize, 5);
	setProperty(Ids::uiSliderThumbCornerSize, 3);
	setProperty(Ids::uiSliderThumbWidth, 0);
	setProperty(Ids::uiSliderThumbHeight, 0);
	setProperty(Ids::uiSliderThumbFlatOnLeft, false);
	setProperty(Ids::uiSliderThumbFlatOnRight, false);
	setProperty(Ids::uiSliderThumbFlatOnTop, false);
	setProperty(Ids::uiSliderThumbFlatOnBottom, false);

	setProperty(Ids::uiSliderValuePosition, (int)Slider::TextBoxBelow);
	setProperty(Ids::uiSliderValueTextJustification, "centred");
	setProperty(Ids::uiSliderValueFont, FONT2STR(Font(12)));
	setProperty(Ids::uiSliderValueTextColour, (String)findColour(Slider::textBoxTextColourId).toString());
	setProperty(Ids::uiSliderValueHighlightColour, (String)findColour(Slider::textBoxHighlightColourId).toString());
	setProperty(Ids::uiSliderValueBgColour, "0x00ffffff");
	setProperty(Ids::uiSliderValueOutlineColour, "0x00ffffff");

	setProperty(Ids::uiSliderLookAndFeelIsCustom, true);

	// 4. Attach listener LAST so initial property assignments do not trigger false valueTreePropertyChanged events
	componentTree.addListener(this);
}

CtrlrSlider::~CtrlrSlider() {
	componentTree.removeListener(this);
	DBG("CtrlrSlider::~CtrlrSlider() called");
	ctrlrSlider.setLookAndFeel(nullptr);
	this->setLookAndFeel(nullptr);
	// Nothing to delete — LookAndFeel is owned by the editor, not by this slider
}

void CtrlrSlider::resized() {
	if (restoreStateInProgress)
		return;
	ctrlrSlider.setBounds(getUsableRect());
}

void CtrlrSlider::sliderValueChanged(Slider *sliderThatWasMoved) {
	if (sliderThatWasMoved == &ctrlrSlider) {
		if ((bool)owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelEditMode) == true)
			return;

		setComponentValue(ctrlrSlider.getValue(), true);
	}
}

void CtrlrSlider::mouseUp(const MouseEvent &e) {
	if (mouseUpCbk && !mouseUpCbk.wasObjectDeleted()) {
		if (mouseUpCbk->isValid()) {
			owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().call(mouseUpCbk, this, e);
		}
	}
	if ((bool)getProperty(Ids::uiSliderSpringMode) == true) {
		ctrlrSlider.setValue((double)getProperty(Ids::uiSliderSpringValue), sendNotificationSync);
	}
}

double CtrlrSlider::getComponentValue() {
	return (ctrlrSlider.getValue());
}

int CtrlrSlider::getComponentMidiValue() {
	return ((int)ctrlrSlider.getValue());
}

double CtrlrSlider::getComponentMaxValue() {
	return (ctrlrSlider.getMaximum());
}

void CtrlrSlider::setComponentValue(const double newValue, const bool sendChangeMessage) {
	ctrlrSlider.setValue(newValue, dontSendNotification);
	if (sendChangeMessage) {
		// DBG("Sending MIDI from uiSlider okay!");
		owner.getProcessor().setValueGeneric(CtrlrModulatorValue(newValue, CtrlrModulatorValue::changedByGUI));
	}
}

const Array<Font> CtrlrSlider::getFontList() {
	Array<Font> ret;
	Font f = STR2FONT(getProperty(Ids::uiSliderValueFont));
	if (f.getTypefaceName() != Font::getDefaultSerifFontName() &&
		f.getTypefaceName() != Font::getDefaultSansSerifFontName() &&
		f.getTypefaceName() != Font::getDefaultMonospacedFontName() && f.getTypefaceName() != "<Sans-Serif>") {
		ret.add(f);
	}
	return (ret);
}
void CtrlrSlider::updateComponentColors() {
	LNF::applyLookAndFeelState(ctrlrSlider, getComponentTree(), Ids::uiSliderLookAndFeelIsCustom,
							   {{Ids::uiSliderValueTextColour, juce::Slider::textBoxTextColourId},
								{Ids::uiSliderValueBgColour, juce::Slider::textBoxBackgroundColourId},
								{Ids::uiSliderRotaryOutlineColour, juce::Slider::rotarySliderOutlineColourId},
								{Ids::uiSliderRotaryFillColour, juce::Slider::rotarySliderFillColourId},
								{Ids::uiSliderThumbColour, juce::Slider::thumbColourId},
								{Ids::uiSliderValueHighlightColour, juce::Slider::textBoxHighlightColourId},
								{Ids::uiSliderValueOutlineColour, juce::Slider::textBoxOutlineColourId},
								{Ids::uiSliderTrackColour, juce::Slider::trackColourId}});

	// Force internal sub-components (like Label text boxes inside the Slider) to update theme colors
	ctrlrSlider.lookAndFeelChanged();
	ctrlrSlider.repaint();
}
void CtrlrSlider::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) {
	DBG("Value Tree Property Changed ::" << property);

	if (property == Ids::uiSliderStyle) {
		ctrlrSlider.setSliderStyle(
			(Slider::SliderStyle)CtrlrComponentTypeManager::sliderStringToStyle(getProperty(Ids::uiSliderStyle)));
	} else if (property == Ids::uiSliderSpringMode) {
		if ((bool)getProperty(property) == true) {
			ctrlrSlider.setValue(getProperty(Ids::uiSliderSpringValue), dontSendNotification);
		}
	} else if (property == Ids::uiSliderSpringValue) {
		ctrlrSlider.setValue(getProperty(property), dontSendNotification);
	} else if (property == Ids::uiSliderPopupBubble) {
		ctrlrSlider.setPopupDisplayEnabled((bool)getProperty(property), (bool)getProperty(property),
										   owner.getOwnerPanel().getEditor());
	} else if (property == Ids::uiSliderLookAndFeelIsCustom) {
		updateComponentColors();
	} else if (property == Ids::uiPanelLookAndFeel || property == Ids::uiSliderLookAndFeel) {

		String activeLnF = getProperty(Ids::uiSliderLookAndFeel).toString();
		DBG("!!! Look and feel changed for uiSlider : " << activeLnF);
		// 1. Fallback to panel editor's global theme if component is set to "Default"
		if (activeLnF.isEmpty() || activeLnF == "Default") {
			if (auto *editor = owner.getOwnerPanel().getEditor()) {
				activeLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
			}
		}

		// 2. Clear any local raw pointer assignment so JUCE doesn't crash on dangling pointers
		ctrlrSlider.setLookAndFeel(nullptr);

		// 3. Let Ctrlr's central engine apply the persistent theme instance
		applyCentralLookAndFeel(&ctrlrSlider, activeLnF);

		// 4. Update component colors (strips local hexes if in LNF mode, or re-applies if in User Mode)
		updateComponentColors();

		// 5. Force JUCE to invalidate its color cache and redraw
		ctrlrSlider.sendLookAndFeelChange();
		ctrlrSlider.repaint();
		repaint();

	} else if (property == Ids::uiSliderRotaryFillColour || property == Ids::uiSliderRotaryOutlineColour ||
			   property == Ids::uiSliderTrackColour || property == Ids::uiSliderThumbColour ||
			   property == Ids::uiSliderValueHighlightColour || property == Ids::uiSliderValueBgColour ||
			   property == Ids::uiSliderValueOutlineColour) {
		if (!restoreStateInProgress) {
			// Set custom mode to active (0 = User Mode)
			setProperty(Ids::uiSliderLookAndFeelIsCustom, 0);
		}
		updateComponentColors();
	} else if (property == Ids::uiSliderValueTextColour) {
		if (!restoreStateInProgress) {
			const Colour c = VAR2COLOUR(getProperty(Ids::uiSliderValueTextColour));
			for (int i = 0; i < ctrlrSlider.getNumChildComponents(); ++i) {
				if (auto *lb = dynamic_cast<Label *>(ctrlrSlider.getChildComponent(i))) {
					lb->setColour(Label::textColourId, c);
					lb->setColour(Label::textWhenEditingColourId, c);
				}
			}
			setProperty(Ids::uiSliderLookAndFeelIsCustom, 0);
		}
		updateComponentColors();
	}
	// --- SLIDER RANGE & METRICS ---
	else if (property == Ids::uiSliderInterval || property == Ids::uiSliderMax || property == Ids::uiSliderMin) {
		double max = getProperty(Ids::uiSliderMax);
		double min = getProperty(Ids::uiSliderMin);
		double interval = getProperty(Ids::uiSliderInterval);

		if (interval == 0)
			interval = std::abs(max - min) + 1;

		if (max <= min) {
			max = min + interval * 0.66;
		}

		ctrlrSlider.setRange(min, max, interval);
		owner.setProperty(Ids::modulatorMin, ctrlrSlider.getMinimum());
		owner.setProperty(Ids::modulatorMax, ctrlrSlider.getMaximum());
		lookAndFeelChanged();
	} else if (property == Ids::uiSliderDecimalPlaces) {
		ctrlrSlider.setNumDecimalPlacesToDisplay((int)getProperty(Ids::uiSliderDecimalPlaces));
		ctrlrSlider.lookAndFeelChanged();
	} else if (property == Ids::uiSliderValueSuffix) {
		ctrlrSlider.setTextValueSuffix(getProperty(Ids::uiSliderValueSuffix).toString());
		ctrlrSlider.lookAndFeelChanged();
	} else if (property == Ids::uiSliderValuePosition || property == Ids::uiSliderValueHeight ||
			   property == Ids::uiSliderValueWidth) {
		ctrlrSlider.setTextBoxStyle((Slider::TextEntryBoxPosition)(int)getProperty(Ids::uiSliderValuePosition), false,
									getProperty(Ids::uiSliderValueWidth, 64),
									getProperty(Ids::uiSliderValueHeight, 12));

		ctrlrSlider.lookAndFeelChanged();
	} else if (property == Ids::uiSliderSetNotificationOnlyOnRelease) {
		ctrlrSlider.setChangeNotificationOnlyOnRelease((bool)getProperty(Ids::uiSliderSetNotificationOnlyOnRelease));
	} else if (property == Ids::uiSliderVelocityMode || property == Ids::uiSliderVelocityModeKeyTrigger ||
			   property == Ids::uiSliderVelocitySensitivity || property == Ids::uiSliderVelocityThreshold ||
			   property == Ids::uiSliderVelocityOffset) {
		ctrlrSlider.setVelocityBasedMode((bool)getProperty(Ids::uiSliderVelocityMode));
		ctrlrSlider.setVelocityModeParameters(
			(double)getProperty(Ids::uiSliderVelocitySensitivity), (int)getProperty(Ids::uiSliderVelocityThreshold),
			(double)getProperty(Ids::uiSliderVelocityOffset), (bool)getProperty(Ids::uiSliderVelocityModeKeyTrigger));
	} else if (property == Ids::uiSliderDoubleClickValue || property == Ids::uiSliderDoubleClickEnabled) {
		ctrlrSlider.setDoubleClickReturnValue((bool)getProperty(Ids::uiSliderDoubleClickEnabled),
											  getProperty(Ids::uiSliderDoubleClickValue));
	} else {
		CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
	}

	if (restoreStateInProgress == false) {
		resized();
	}
}

const String CtrlrSlider::getComponentText() {
	return (String(getComponentValue()));
}

void CtrlrSlider::customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel) {
	if (customLookAndFeel != nullptr) {
		// Apply the active custom LookAndFeel instance directly to the child slider
		ctrlrSlider.setLookAndFeel(customLookAndFeel);
	} else {
		// Fall back to null so it inherits from parent, or fetch panel central theme
		ctrlrSlider.setLookAndFeel(nullptr);

		String panelLnF = getProperty(Ids::uiSliderLookAndFeel).toString();
		if (panelLnF.isEmpty() || panelLnF == "Default") {
			if (auto *editor = owner.getOwnerPanel().getEditor()) {
				panelLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
			}
		}
		applyCentralLookAndFeel(&ctrlrSlider, panelLnF);
	}

	// Force JUCE to invalidate cached draw calls and repaint
	ctrlrSlider.sendLookAndFeelChange();
	ctrlrSlider.repaint();
}

const String CtrlrSlider::getCurrentLF() {
	return getProperty(Ids::uiSliderLookAndFeel);
}

std::unique_ptr<LookAndFeel>
CtrlrSlider::getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty) {
	if (lookAndFeelComponentProperty == "Default") {
		return nullptr;
	}
	return gui::createLookAndFeelFromDescription(lookAndFeelComponentProperty, false);
}

void CtrlrSlider::resetLookAndFeelOverrides() {
	if (restoreStateInProgress == false) {
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
		setProperty(Ids::uiSliderValueBgColour, "0x00ffffff");
		setProperty(Ids::uiSliderValueOutlineColour, "0x00ffffff");

		setProperty(Ids::uiSliderLookAndFeelIsCustom, false);

		updatePropertiesPanel();
	}
}

void CtrlrSlider::updatePropertiesPanel() {
	CtrlrPanelProperties *props = owner.getCtrlrManagerOwner().getActivePanel()->getEditor(false)->getPropertiesPanel();
	if (props) {
		props->refreshAll();
	}
}