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
	setProperty(Ids::uiSliderLookAndFeelIsCustom, true);
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

void CtrlrSlider::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) {
	DBG("Value Tree Property Changed ::" << property);

	if (property == Ids::uiSliderStyle) {
		ctrlrSlider.setSliderStyle(
			(Slider::SliderStyle)CtrlrComponentTypeManager::sliderStringToStyle(getProperty(Ids::uiSliderStyle)));
		updateComponentColors();
		resized();
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
		/*
		 *
		 *  uiPanelLookAndFeel and uiSliderLookAndFeel are handled together in the same block below, because they are
		 * interdependent. The slider's LNF can be set to "Default" to inherit the panel's LNF, or it can be set to a
		 * specific LNF string to override the panel's LNF. When either property changes, we need to re-evaluate the
		 * effective LNF for the slider and apply it accordingly.
		 *
		 */
	} else if (property == Ids::uiPanelLookAndFeel || property == Ids::uiSliderLookAndFeel) {
		String activeLnF = getProperty(Ids::uiSliderLookAndFeel).toString();
		if (activeLnF.isEmpty() || activeLnF == "Default") {
			if (auto *editor = owner.getOwnerPanel().getEditor())
				activeLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
		}

		// 1. Detach EVERY pointer to the current LookAndFeel object first —
		//    this must happen before customLF is reset/reassigned below.
		ctrlrSlider.setLookAndFeel(nullptr);
		setLookAndFeel(nullptr);

		// 2. Only now is it safe to destroy/replace the old customLF
		if (activeLnF == "V1" || activeLnF == "V2" || activeLnF == "V3" || activeLnF.isEmpty() ||
			activeLnF == "Default") {
			customLF.reset();
			applyCentralLookAndFeel(this, activeLnF);
		} else {
			customLF = std::move(getLookAndFeelFromComponentProperty(activeLnF));
			if (customLF != nullptr)
				setLookAndFeel(customLF.get());
		}

		// 3. Re-attach the internal slider to whatever 'this' resolved to
		ctrlrSlider.setLookAndFeel(&getLookAndFeel());

		updateComponentColors();
		ctrlrSlider.sendLookAndFeelChange();
		ctrlrSlider.repaint();
		repaint();
	} else if (property == Ids::uiSliderRotaryFillColour || property == Ids::uiSliderRotaryOutlineColour ||
			   property == Ids::uiSliderTrackColour || property == Ids::uiSliderThumbColour ||
			   property == Ids::uiSliderValueHighlightColour || property == Ids::uiSliderValueBgColour ||
			   property == Ids::uiSliderValueOutlineColour || property == Ids::uiSliderIncDecButtonColour ||
			   property == Ids::uiSliderIncDecTextColour) {
		if (!restoreStateInProgress) {
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
void CtrlrSlider::lookAndFeelChanged() {
	DBG("CtrlrSlider::lookAndFeelChanged() called");
	// 1. Sync internal slider pointer with the wrapper's LookAndFeel
	ctrlrSlider.setLookAndFeel(&getLookAndFeel());

	// 2. Clear local hex overrides if in LNF mode, or re-apply if in User mode
	updateComponentColors();

	// 3. Notify internal JUCE child components (labels, textboxes)
	ctrlrSlider.sendLookAndFeelChange();

	// 4. Call base class implementation
	CtrlrComponent::lookAndFeelChanged();
}
void CtrlrSlider::customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel) {
	DBG("Custom look and feel changed for CtrlrSlider: ");
	if (customLookAndFeel != nullptr) {
		// Direct custom LookAndFeel assignment
		ctrlrSlider.setLookAndFeel(customLookAndFeel);
	} else {
		// 1. Clear internal slider pointer so it can accept parent theme
		ctrlrSlider.setLookAndFeel(nullptr);

		// 2. Fetch active theme string
		String panelLnF = getProperty(Ids::uiSliderLookAndFeel).toString();
		if (panelLnF.isEmpty() || panelLnF == "Default") {
			if (auto *editor = owner.getOwnerPanel().getEditor()) {
				panelLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
			}
		}

		// 3. Apply theme string to 'this' (the CtrlrComponent wrapper!), NOT &ctrlrSlider
		applyCentralLookAndFeel(this, panelLnF);

		// 4. Bind the wrapper's newly resolved LookAndFeel object to the internal member slider
		ctrlrSlider.setLookAndFeel(&getLookAndFeel());
	}

	// 5. Refresh colors and force redraw
	updateComponentColors();
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
void CtrlrSlider::updateComponentColors() {
	auto& lf = ctrlrSlider.getLookAndFeel();
	const bool isCustom = (bool)getProperty(Ids::uiSliderLookAndFeelIsCustom);

	LNF::applyLookAndFeelState(ctrlrSlider, getComponentTree(), Ids::uiSliderLookAndFeelIsCustom,
							   {{Ids::uiSliderValueTextColour, juce::Slider::textBoxTextColourId},
								{Ids::uiSliderValueBgColour, juce::Slider::textBoxBackgroundColourId},
								{Ids::uiSliderRotaryOutlineColour, juce::Slider::rotarySliderOutlineColourId},
								{Ids::uiSliderRotaryFillColour, juce::Slider::rotarySliderFillColourId},
								{Ids::uiSliderThumbColour, juce::Slider::thumbColourId},
								{Ids::uiSliderValueHighlightColour, juce::Slider::textBoxHighlightColourId},
								{Ids::uiSliderValueOutlineColour, juce::Slider::textBoxOutlineColourId},
								{Ids::uiSliderTrackColour, juce::Slider::trackColourId}});

	if (isCustom) {
		// USER MODE: Apply custom user-chosen property colors
		const Colour btnBg = VAR2COLOUR(getProperty(Ids::uiSliderIncDecButtonColour));
		const Colour btnText = VAR2COLOUR(getProperty(Ids::uiSliderIncDecTextColour));

		lf.setColour(juce::TextButton::buttonColourId, btnBg);
		lf.setColour(juce::TextButton::buttonOnColourId, btnBg);
		lf.setColour(juce::TextButton::textColourOffId, btnText);
		lf.setColour(juce::TextButton::textColourOnId, btnText);
	} else {
		// LNF MODE: Fetch panel theme colors to restore theme defaults
		String panelLnF = "V3";
		if (auto *editor = owner.getOwnerPanel().getEditor()) {
			panelLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
		}

		// Pull default theme colors from the central L&F system
		const Colour defaultBg = findColour(Slider::backgroundColourId);
		const Colour defaultText = findColour(Label::textColourId);

		lf.setColour(juce::TextButton::buttonColourId, defaultBg);
		lf.setColour(juce::TextButton::buttonOnColourId, defaultBg);
		lf.setColour(juce::TextButton::textColourOffId, defaultText);
		lf.setColour(juce::TextButton::textColourOnId, defaultText);
	}

	ctrlrSlider.lookAndFeelChanged();
	ctrlrSlider.repaint();
}

