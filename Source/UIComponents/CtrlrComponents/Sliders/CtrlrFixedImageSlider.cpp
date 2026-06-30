#include "CtrlrFixedImageSlider.h"
#include "CtrlrComponents/CtrlrFilmStripPainter.h"
#include "CtrlrLuaManager.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrPanel/CtrlrPanelResource.h"
#include "CtrlrSliderInternal.h"
#include "stdafx.h"

CtrlrFixedImageSlider::CtrlrFixedImageSlider(CtrlrModulator &owner) : CtrlrComponent(owner), ctrlrSlider(nullptr) {
	// 1. Initialize modern safe unique ownership containers
	valueMap = std::make_unique<CtrlrValueMap>();
	lf = std::make_unique<CtrlrImageSliderLF>(*this);

	/** Tooltip properties */
	setColour(juce::TooltipWindow::textColourId, findColour(juce::Label::textColourId));
	setColour(juce::TooltipWindow::backgroundColourId, findColour(juce::TooltipWindow::backgroundColourId));
	setColour(juce::TooltipWindow::outlineColourId, findColour(juce::TooltipWindow::outlineColourId));

	// 2. Wrap the child component allocation and attach its raw address to JUCE
	ctrlrSlider = std::make_unique<CtrlrSliderInternal>(*this);
	addAndMakeVisible(ctrlrSlider.get());

	ctrlrSlider->setName("ctrlrSlider");
	ctrlrSlider->addListener(this);

	// 3. Extract the raw pointer of your custom look-and-feel tracker via .get()
	ctrlrSlider->setLookAndFeel(lf.get());

	componentTree.addListener(this);

	setProperty(Ids::uiSliderMin, 0);
	setProperty(Ids::uiSliderMax, 1);
	// setProperty (Ids::uiSliderInterval, 1); // Removed v5.6.32. Useless since indexes are stepped
	// 1 by 1
	setProperty(Ids::uiSliderValueSuffix, ""); // Added v5.6.32
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
	setProperty(Ids::uiSliderLookAndFeelIsCustom, false);

	setProperty(Ids::uiSliderPopupBubble, false);

	setProperty(Ids::uiSliderStyle, "RotaryVerticalDrag");

	setProperty(Ids::uiImageSliderResource, COMBO_ITEM_NONE);
	setProperty(Ids::resourceImageWidth, 32);
	setProperty(Ids::resourceImageHeight, 32);
	setProperty(Ids::resourceImagePaintMode, 36);
	setProperty(Ids::resourceImageOrientation, 1);

	setProperty(Ids::uiSliderIncDecTextColour, (String)findColour(Label::textColourId).toString());

	setProperty(Ids::uiSliderValuePosition, (int)Slider::TextBoxBelow);
	setProperty(Ids::uiSliderValueWidth, 64);
	setProperty(Ids::uiSliderValueHeight, 12);
	setProperty(Ids::uiSliderValueTextJustification, "centred");
	setProperty(Ids::uiSliderValueFont, FONT2STR(Font(12)));
	setProperty(Ids::uiSliderValueTextColour, (String)findColour(Slider::textBoxTextColourId).toString());
	setProperty(Ids::uiSliderValueHighlightColour, (String)findColour(Slider::textBoxHighlightColourId).toString());
	setProperty(Ids::uiSliderValueBgColour,
				"0x00ffffff"); // (String)findColour (Slider::textBoxBackgroundColourId).toString());
	setProperty(Ids::uiSliderValueOutlineColour,
				"0x00ffffff"); //(String)findColour (Slider::textBoxOutlineColourId).toString());

	setProperty(Ids::uiSliderLookAndFeelIsCustom, false);

	setSize(64, 90);
}

CtrlrFixedImageSlider::~CtrlrFixedImageSlider() {
		customLF.reset();
}

void CtrlrFixedImageSlider::paint(Graphics &g) {}

void CtrlrFixedImageSlider::resized() {
	if (restoreStateInProgress)
		return;
	ctrlrSlider->setBounds(getUsableRect());
}

void CtrlrFixedImageSlider::mouseUp(const MouseEvent &e) {
	if (mouseUpCbk && !mouseUpCbk.wasObjectDeleted()) {
		if (mouseUpCbk->isValid()) {
			owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().call(mouseUpCbk, this, e);
		}
	}
	if ((bool)getProperty(Ids::uiSliderSpringMode) == true) {
		ctrlrSlider->setValue((double)getProperty(Ids::uiSliderSpringValue), sendNotificationSync);
	}
}

double CtrlrFixedImageSlider::getComponentMaxValue() { return (valueMap->getNonMappedMax()); }

double CtrlrFixedImageSlider::getComponentValue() { return ((int)ctrlrSlider->getValue()); }

int CtrlrFixedImageSlider::getComponentMidiValue() { return (valueMap->getMappedValue(ctrlrSlider->getValue())); }
const String CtrlrFixedImageSlider::getComponentText() { return (valueMap->getTextForIndex(ctrlrSlider->getValue())); }

void CtrlrFixedImageSlider::setComponentValue(const double newValue, const bool sendChangeMessage) {
	ctrlrSlider->setValue(newValue, dontSendNotification);
	if (sendChangeMessage) {
		owner.getProcessor().setValueGeneric(CtrlrModulatorValue(newValue, CtrlrModulatorValue::changedByGUI));
	}
}

void CtrlrFixedImageSlider::sliderContentChanged() {
	// Fix multi-parse duplication from legacy copy-paste line
	valueMap->copyFrom(owner.getProcessor().setValueMap(getProperty(Ids::uiFixedSliderContent)));

	double min = valueMap->getNonMappedMin();
	double max = valueMap->getNonMappedMax();
	const double interval = 1.0;

	// Strict JUCE Guard: Ensure range width is at least 1 full step wide
	if (max <= min) {
		max = min + interval;
	}
	if ((max - min) < interval) {
		max = min + interval;
	}

	ctrlrSlider->setRange(min, max, interval);
}

void CtrlrFixedImageSlider::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged,
													 const Identifier &property) {
	if (property == Ids::resourceImagePaintMode) {
		lf->setPaintMode((RectanglePlacement)(int)getProperty(Ids::resourceImagePaintMode));
	} else if (property == Ids::uiSliderLookAndFeel) {
		String LookAndFeelType = getProperty(property);
		customLF = getLookAndFeelFromComponentProperty(LookAndFeelType);
		if (customLF != nullptr) {
			setLookAndFeel(customLF.get()); // Updates the current component LookAndFeel
		}

		if (LookAndFeelType == "Default") {
			setProperty(Ids::uiSliderLookAndFeelIsCustom,
						false); // Resets the Customized Flag to False to allow Global L&F to apply
		}

		if (!getProperty(Ids::uiSliderLookAndFeelIsCustom)) {
			resetLookAndFeelOverrides(); // Retrieves LookAndFeel colours from selected ColourScheme
		}
	} else if (property == Ids::resourceImageWidth) {
		lf->setImage(filmStripImage, (int)getProperty(Ids::resourceImageWidth),
					 (int)getProperty(Ids::resourceImageHeight));
	} else if (property == Ids::resourceImageHeight) {
		lf->setImage(filmStripImage, (int)getProperty(Ids::resourceImageWidth),
					 (int)getProperty(Ids::resourceImageHeight));
	} else if (property == Ids::resourceImageOrientation) {
		lf->setOrientation((bool)getProperty(Ids::resourceImageOrientation));
	} else if (property == Ids::uiImageSliderResource) {
		setResource();
	} else if (property == Ids::uiSliderStyle) {
		ctrlrSlider->setSliderStyle(
			(Slider::SliderStyle)CtrlrComponentTypeManager::sliderStringToStyle(getProperty(Ids::uiSliderStyle)));
	} else if (property == Ids::uiSliderValueTextColour) {
		ctrlrSlider->setColour(Slider::textBoxTextColourId, VAR2COLOUR(getProperty(Ids::uiSliderValueTextColour)));
		setProperty(Ids::uiSliderLookAndFeelIsCustom,
					true); // Locks the component custom colourScheme
	} else if (property == Ids::uiSliderValueHighlightColour) {
		ctrlrSlider->setColour(Slider::textBoxHighlightColourId,
							   VAR2COLOUR(getProperty(Ids::uiSliderValueHighlightColour)));
		setProperty(Ids::uiSliderLookAndFeelIsCustom,
					true); // Locks the component custom colourScheme
	} else if (property == Ids::uiSliderValueBgColour) {
		ctrlrSlider->setColour(Slider::textBoxBackgroundColourId, VAR2COLOUR(getProperty(Ids::uiSliderValueBgColour)));
		setProperty(Ids::uiSliderLookAndFeelIsCustom,
					true); // Locks the component custom colourScheme
	} else if (property == Ids::uiSliderValueOutlineColour) {
		ctrlrSlider->setColour(Slider::textBoxOutlineColourId,
							   VAR2COLOUR(getProperty(Ids::uiSliderValueOutlineColour)));
		setProperty(Ids::uiSliderLookAndFeelIsCustom,
					true); // Locks the component custom colourScheme
	} else if (property == Ids::uiFixedSliderContent) {
		sliderContentChanged();
	} else if (property == Ids::uiSliderValueSuffix) // Added v5.6.32
	{
		ctrlrSlider->setTextValueSuffix(getProperty(Ids::uiSliderValueSuffix).toString());
		ctrlrSlider->lookAndFeelChanged();
	} else if (property == Ids::uiSliderValuePosition || property == Ids::uiSliderValueHeight ||
			   property == Ids::uiSliderValueWidth) {
		ctrlrSlider->setTextBoxStyle((Slider::TextEntryBoxPosition)(int)getProperty(Ids::uiSliderValuePosition), false,
									 getProperty(Ids::uiSliderValueWidth, 64),
									 getProperty(Ids::uiSliderValueHeight, 12));
	} else if (property == Ids::uiSliderSetNotificationOnlyOnRelease) {
		ctrlrSlider->setChangeNotificationOnlyOnRelease((bool)getProperty(Ids::uiSliderSetNotificationOnlyOnRelease));
	} else if (property == Ids::uiSliderValueFont || property == Ids::uiSliderValueTextJustification) {
		filmStripImage =
			owner.getOwnerPanel().getResourceManager().getResourceAsImage(getProperty(Ids::uiImageSliderResource));
		lf->setImage(filmStripImage, (int)getProperty(Ids::resourceImageWidth),
					 (int)getProperty(Ids::resourceImageHeight));
		lf->setPaintMode((RectanglePlacement)(int)getProperty(Ids::resourceImagePaintMode));
		ctrlrSlider->setLookAndFeel(0);
		ctrlrSlider->setLookAndFeel(lf.get());
	}

	else if (property == Ids::uiSliderVelocityMode || property == Ids::uiSliderVelocityModeKeyTrigger ||
			 property == Ids::uiSliderVelocitySensitivity || property == Ids::uiSliderVelocityThreshold ||
			 property == Ids::uiSliderVelocityOffset) {
		ctrlrSlider->setVelocityBasedMode((bool)getProperty(Ids::uiSliderVelocityMode));
		ctrlrSlider->setVelocityModeParameters(
			(double)getProperty(Ids::uiSliderVelocitySensitivity), (int)getProperty(Ids::uiSliderVelocityThreshold),
			(double)getProperty(Ids::uiSliderVelocityOffset), (bool)getProperty(Ids::uiSliderVelocityModeKeyTrigger));
	}

	else if (property == Ids::uiSliderSpringValue) {
		ctrlrSlider->setValue(getProperty(property), dontSendNotification);
	} else if (property == Ids::uiSliderDoubleClickValue || property == Ids::uiSliderDoubleClickEnabled) {
		ctrlrSlider->setDoubleClickReturnValue((bool)getProperty(Ids::uiSliderDoubleClickEnabled),
											   getProperty(Ids::uiSliderDoubleClickValue));
	} else if (property == Ids::uiSliderSpringMode) {
		if ((bool)getProperty(property) == true) {
			ctrlrSlider->setValue(getProperty(Ids::uiSliderSpringValue), dontSendNotification);
		}
	}

	else if (property == Ids::uiSliderPopupBubble) {
		ctrlrSlider->setPopupDisplayEnabled((bool)getProperty(property), (bool)getProperty(property),
											owner.getOwnerPanel().getEditor());
	} else {
		CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
	}

	if (restoreStateInProgress == false) {
		resized();
	}
}

const String CtrlrFixedImageSlider::getTextForValue(const double value) { return (valueMap->getTextForIndex(value)); }

void CtrlrFixedImageSlider::sliderValueChanged(Slider *sliderThatWasMoved) {
	setComponentValue(ctrlrSlider->getValue(), true);
}

void CtrlrFixedImageSlider::setResource() {
	filmStripImage =
		owner.getOwnerPanel().getResourceManager().getResourceAsImage(getProperty(Ids::uiImageSliderResource));
	lf->setImage(filmStripImage, (int)getProperty(Ids::resourceImageWidth), (int)getProperty(Ids::resourceImageHeight));
	lookAndFeelChanged();
	repaint();
	resized();
}

void CtrlrFixedImageSlider::reloadResources(Array<CtrlrPanelResource *> resourcesThatChanged) {
	for (int i = 0; i < resourcesThatChanged.size(); i++) {
		if (resourcesThatChanged[i]->getName() == getProperty(Ids::uiImageSliderResource).toString()) {
			setResource();
		}
	}
}

Slider *CtrlrFixedImageSlider::getOwnedSlider() { return (ctrlrSlider.get()); }

std::unique_ptr<LookAndFeel> CtrlrFixedImageSlider::getLookAndFeelFromComponentProperty(
	const String &lookAndFeelComponentProperty) // Updated v5.6.34
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

void CtrlrFixedImageSlider::resetLookAndFeelOverrides() {
	if (restoreStateInProgress ==
		false) // To prevent the prop lines stacking up from top and keeping their original position
	{
		setProperty(Ids::componentLabelColour, (String)findColour(Label::textColourId).toString());

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

void CtrlrFixedImageSlider::updatePropertiesPanel() {
	CtrlrPanelProperties *props = owner.getCtrlrManagerOwner().getActivePanel()->getEditor(false)->getPropertiesPanel();
	if (props) {
		props->refreshAll(); // Needs extra code to prevent scrolling back to top on refresh
	}
}
