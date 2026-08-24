#include "CtrlrGroup.h"
#include "CtrlrIDs.h"
#include "CtrlrModulator/CtrlrModulator.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrPanel/CtrlrPanelResource.h"
#include "CtrlrUtilitiesGUI.h"
#include "stdafx.h"

CtrlrGroupContentComponent::CtrlrGroupContentComponent(CtrlrGroup &_owner) : owner(_owner) {
	setColour(GroupComponent::outlineColourId, Colours::transparentBlack);
}

CtrlrGroupContentComponent::~CtrlrGroupContentComponent() {}

void CtrlrGroupContentComponent::customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel) {
	for (int i = 0; i < getNumChildComponents(); i++) {
		CtrlrComponent *c = dynamic_cast<CtrlrComponent *>(getChildComponent(i));

		if (c != nullptr) {
			c->setCustomLookAndFeel(customLookAndFeel);
		}
	}
}
//[/MiscUserDefs]

//==============================================================================
CtrlrGroup::CtrlrGroup(CtrlrModulator &owner) : CtrlrComponent(owner), content(*this) {
	label = std::make_unique<Label>("label");
	addAndMakeVisible(label.get());
	label->setFont(Font(14.0000f, Font::plain));
	label->setJustificationType(Justification::centred);
	label->setEditable(false, false, false);
	label->setColour(TextEditor::textColourId, findColour(Label::textColourId));
	label->setColour(TextEditor::backgroundColourId, Colour(0x0));

	addAndMakeVisible(&content);
	componentTree.addListener(this);

	owner.setProperty(Ids::modulatorIsStatic, true);
	owner.setProperty(Ids::modulatorVstExported, false);

	// Ensure initial dimensions are set so Ctrlr can place multiple instances
	setSize(120, 100);

	// Only seed initial values for NEW components (when property is missing from XML/ValueTree)
	if (!componentTree.hasProperty(Ids::uiGroupText))
		setProperty(Ids::uiGroupText, "Group Text");

	// 2. Fetch look and feel setting safely from the panel editor
	String panelLnF = "V3";
	if (auto *editor = owner.getOwnerPanel().getEditor()) {
		panelLnF = editor->getProperty(Ids::uiPanelLookAndFeel).toString();
	}

	if (!componentTree.hasProperty(Ids::uiGroupTextFont))
		setProperty(Ids::uiGroupTextFont, FONT2STR(Font(14)));

	if (!componentTree.hasProperty(Ids::uiGroupTextMargin))
		setProperty(Ids::uiGroupTextMargin, 18);

	if (!componentTree.hasProperty(Ids::componentLabelVisible))
		setProperty(Ids::componentLabelVisible, true);

	if (!componentTree.hasProperty(Ids::uiGroupLookAndFeel))
		setProperty(Ids::uiGroupLookAndFeel, "Default");

	if (!componentTree.hasProperty(Ids::uiGroupLookAndFeelIsCustom))
		setProperty(Ids::uiGroupLookAndFeelIsCustom, true);

	if (!componentTree.hasProperty(Ids::uiGroupOutlineThickness))
		setProperty(Ids::uiGroupOutlineThickness, 2.0);

	if (!componentTree.hasProperty(Ids::uiGroupOutlineRoundAngle))
		setProperty(Ids::uiGroupOutlineRoundAngle, 5.0);

	if (!componentTree.hasProperty(Ids::uiGroupBackgroundImage))
		setProperty(Ids::uiGroupBackgroundImage, "");

	if (!componentTree.hasProperty(Ids::uiGroupBackgroundImageLayout))
		setProperty(Ids::uiGroupBackgroundImageLayout, 36);

	if (!componentTree.hasProperty(Ids::uiGroupBackgroundImageAlpha))
		setProperty(Ids::uiGroupBackgroundImageAlpha, 255);

	if (!componentTree.hasProperty(Ids::uiGroupBackgroundGradientType))
		setProperty(Ids::uiGroupBackgroundGradientType, 1);

	// Apply default scheme only if no colors are saved in the tree
	if (!componentTree.hasProperty(Ids::uiGroupBackgroundColour1)) {
		if (owner.getOwnerPanel().getEditor() &&
			(owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel) == "V3" ||
			 owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel) == "V2" ||
			 owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel) == "V1")) {
			setProperty(Ids::uiGroupTextColour, "0xff000000");
			setProperty(Ids::uiGroupBackgroundColour1, "0xffa3a3a3");
			setProperty(Ids::uiGroupBackgroundColour2, "0xffffffff");
			setProperty(Ids::uiGroupOutlineGradientType, "Vertical");
			setProperty(Ids::uiGroupOutlineColour1, "0xffa3a3a3");
			setProperty(Ids::uiGroupOutlineColour2, "0xffffffff");
		} else {
			setProperty(Ids::uiGroupTextColour, (String)findColour(Label::textColourId).toString());
			setProperty(Ids::uiGroupBackgroundColour1,
						(String)findColour(DocumentWindow::backgroundColourId).darker(0.1f).toString());
			setProperty(Ids::uiGroupBackgroundColour2,
						(String)findColour(DocumentWindow::backgroundColourId).toString());
			setProperty(Ids::uiGroupOutlineGradientType, "SolidColour");
			setProperty(Ids::uiGroupOutlineColour1,
						(String)findColour(DocumentWindow::textColourId).darker(0.2f).toString());
			setProperty(Ids::uiGroupOutlineColour2, (String)findColour(DocumentWindow::textColourId).toString());
		}
	}
	applyLabelProperties();
	restoreStateInProgress = false;
}

CtrlrGroup::~CtrlrGroup() {
	owner.getModulatorTree().removeListener(this);
	componentTree.removeListener(this);
	// deleteAndZero (label); // Removed v5.6.34. Thanks to @dnaldoog
}

//==============================================================================
void CtrlrGroup::paint(Graphics &g) {
	const bool isCustom = (bool)getProperty(Ids::uiGroupLookAndFeelIsCustom);

	Rectangle<int> r = getUsableRect();

	if (isCustom || customLF == nullptr) {
		// USER COLOURS

		gradientFromProperty(g, getBounds(), getObjectTree(), Ids::uiGroupOutlineGradientType,
							 Ids::uiGroupOutlineColour1, Ids::uiGroupOutlineColour2);

		g.drawRoundedRectangle(r.toFloat().reduced((float)getProperty(Ids::uiGroupOutlineThickness) / 2.0f),
							   getProperty(Ids::uiGroupOutlineRoundAngle), getProperty(Ids::uiGroupOutlineThickness));

		gradientFromProperty(g, getBounds(), getObjectTree(), Ids::uiGroupBackgroundGradientType,
							 Ids::uiGroupBackgroundColour1, Ids::uiGroupBackgroundColour2);

		g.fillRoundedRectangle(r.toFloat().reduced((float)getProperty(Ids::uiGroupOutlineThickness) / 2.0f),
							   getProperty(Ids::uiGroupOutlineRoundAngle));
	} else {
		// LNF COLOURS

		const Colour background = customLF->findColour(ResizableWindow::backgroundColourId);

		const Colour outline = customLF->findColour(GroupComponent::outlineColourId);

		const Colour text = customLF->findColour(GroupComponent::textColourId);

		g.setColour(outline);

		g.drawRoundedRectangle(r.toFloat().reduced((float)getProperty(Ids::uiGroupOutlineThickness) / 2.0f),
							   getProperty(Ids::uiGroupOutlineRoundAngle), getProperty(Ids::uiGroupOutlineThickness));

		g.setColour(background);

		g.fillRoundedRectangle(r.toFloat().reduced((float)getProperty(Ids::uiGroupOutlineThickness) / 2.0f),
							   getProperty(Ids::uiGroupOutlineRoundAngle));
	}

	// background image remains as it is
	if (groupBackgroundImage.isValid()) {
		if ((int)getProperty(Ids::uiGroupBackgroundImageLayout) == 8192) {
			g.setTiledImageFill(groupBackgroundImage, 0, 0,
								(float)getProperty(Ids::uiGroupBackgroundImageAlpha) / 255.0f);
			g.fillRect(r);
		} else {
			g.setColour(Colours::black.withAlpha((float)getProperty(Ids::uiGroupBackgroundImageAlpha) / 255.0f));
			g.drawImageWithin(groupBackgroundImage, r.getX(), r.getY(), r.getWidth(), r.getHeight(),
							  RectanglePlacement(getProperty(Ids::uiGroupBackgroundImageLayout)));
		}
	}
}
// void CtrlrGroup::paint(Graphics &g) {
// 	//[UserPrePaint] Add your own custom painting code here..
// 	//[/UserPrePaint]

// 	//[UserPaint] Add your own custom painting code here..
// 	Rectangle<int> r = getUsableRect();

// 	gradientFromProperty(g, getBounds(), getObjectTree(), Ids::uiGroupOutlineGradientType,
// Ids::uiGroupOutlineColour1, 						 Ids::uiGroupOutlineColour2);
// 	g.drawRoundedRectangle(r.toFloat().reduced((float)getProperty(Ids::uiGroupOutlineThickness) / 2.0f),
// 						   getProperty(Ids::uiGroupOutlineRoundAngle), getProperty(Ids::uiGroupOutlineThickness));

// 	gradientFromProperty(g, getBounds(), getObjectTree(), Ids::uiGroupBackgroundGradientType,
// 						 Ids::uiGroupBackgroundColour1, Ids::uiGroupBackgroundColour2);
// 	g.fillRoundedRectangle(r.toFloat().reduced((float)getProperty(Ids::uiGroupOutlineThickness) / 2.0f),
// 						   getProperty(Ids::uiGroupOutlineRoundAngle));

// 	if (groupBackgroundImage.isValid()) {
// 		if ((int)getProperty(Ids::uiGroupBackgroundImageLayout) == 8192) {
// 			g.setTiledImageFill(groupBackgroundImage, 0, 0,
// 								(float)getProperty(Ids::uiGroupBackgroundImageAlpha) / 255.0f);
// 			g.fillRect(r);
// 		} else {
// 			g.setColour(Colours::black.withAlpha((float)getProperty(Ids::uiGroupBackgroundImageAlpha) / 255.0f));
// 			g.drawImageWithin(groupBackgroundImage, r.getX(), r.getY(), r.getWidth(), r.getHeight(),
// 							  RectanglePlacement(getProperty(Ids::uiGroupBackgroundImageLayout)));
// 		}
// 	}
// 	//[/UserPaint]
// }
void CtrlrGroup::resized() {
	// label->setBounds (0, 0, getWidth() - 0, getHeight() - 0);
	//[UserResized] Add your own custom resize handling here..
	label->setBounds(textMargin, textMargin, getWidth() - (textMargin * 2), getHeight() - (textMargin * 2));
	content.setBounds(getUsableRect());
	//[/UserResized]
}

	//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
	void CtrlrGroup::setComponentValue(const double newValue, const bool sendChangeMessage) {}

	const Array<Font> CtrlrGroup::getFontList() {
		Array<Font> ret;
		Font f = STR2FONT(getProperty(Ids::uiGroupTextFont));
		if (f.getTypefaceName() != Font::getDefaultSerifFontName() &&
			f.getTypefaceName() != Font::getDefaultSansSerifFontName() &&
			f.getTypefaceName() != Font::getDefaultMonospacedFontName() && f.getTypefaceName() != "<Sans-Serif>") {
			ret.add(f);
		}
		return (ret);
	}

	const String CtrlrGroup::getComponentText() {
		return (label->getText());
	}

	void CtrlrGroup::setComponentText(const String &componentText) {
		setProperty(Ids::uiGroupText, componentText, true);
	}

	double CtrlrGroup::getComponentMaxValue() {
		return (1);
	}

	double CtrlrGroup::getComponentValue() {
		return (1);
	}

	int CtrlrGroup::getComponentMidiValue() {
		return (1);
	}

	void CtrlrGroup::updateComponentColors() {
		const bool isCustom = (bool)getProperty(Ids::uiGroupLookAndFeelIsCustom);

		if (isCustom) {
			label->setColour(Label::textColourId, VAR2COLOUR(getProperty(Ids::uiGroupTextColour)));
		} else if (customLF != nullptr) {
			label->setColour(Label::textColourId, customLF->findColour(GroupComponent::textColourId));
		}

		label->repaint();
		repaint();
	}

	void CtrlrGroup::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) {
		if (restoreStateInProgress) {
			if (property == Ids::uiGroupText || property == Ids::uiGroupTextFont ||
				property == Ids::uiGroupTextColour || property == Ids::uiGroupTextPlacement ||
				property == Ids::uiGroupTextMargin || property == Ids::componentLabelVisible) {
				// Allow through
			} else {
				return;
			}
		}

	if (property == Ids::uiGroupOutlineColour1 || property == Ids::uiGroupOutlineColour2 ||
		property == Ids::uiGroupBackgroundColour1 || property == Ids::uiGroupBackgroundColour2 ||
		property == Ids::uiGroupBackgroundGradientType || property == Ids::uiGroupOutlineGradientType ||
		property == Ids::uiGroupOutlineRoundAngle || property == Ids::uiGroupOutlineThickness) {
		setProperty(Ids::uiGroupLookAndFeelIsCustom, true);
		repaint();
	} else if (property == Ids::uiGroupLookAndFeel) {
		String LookAndFeelType = getProperty(property);

		setLookAndFeel(nullptr);

		if (LookAndFeelType == "Default") {
			customLF.reset();
		} else {
			customLF = getLookAndFeelFromComponentProperty(LookAndFeelType);

			if (customLF != nullptr) {
				setLookAndFeel(customLF.get());
			}
		}

		if (!getProperty(Ids::uiGroupLookAndFeelIsCustom)) {
			resetLookAndFeelOverrides();
		}

		updateComponentColors();
	} else if (property == Ids::uiGroupLookAndFeelIsCustom) {
		if (!getProperty(Ids::uiGroupLookAndFeelIsCustom)) {
			resetLookAndFeelOverrides();
		}
		updateComponentColors();
	} else if (property == Ids::uiGroupTextColour) {
		setProperty(Ids::uiGroupLookAndFeelIsCustom, true);
		label->setColour(Label::textColourId, VAR2COLOUR(getProperty(Ids::uiGroupTextColour)));
		updateComponentColors();
	} else if (property == Ids::uiGroupText) {
		// FORCE LABEL UPDATE AND REPAINT
		label->setText(getProperty(Ids::uiGroupText).toString(), dontSendNotification);
		label->repaint();
	} else if (property == Ids::uiGroupTextFont) {
		label->setFont(STR2FONT(getProperty(Ids::uiGroupTextFont)));
		label->repaint();
	} else if (property == Ids::uiGroupTextPlacement) {
		label->setJustificationType(justificationFromProperty(getProperty(Ids::uiGroupTextPlacement)));
	} else if (property == Ids::uiGroupTextMargin) {
		textMargin = getProperty(Ids::uiGroupTextMargin);
		resized();
	} else if (property == Ids::uiGroupBackgroundImage || property == Ids::uiGroupBackgroundImageAlpha ||
			   property == Ids::uiGroupBackgroundImageLayout) {
		setResource();
	} else {
		CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
	}
	}

	const CtrlrGroup::GradientType CtrlrGroup::gradientFromString(const String &str) {
		if (str == "None")
			return CtrlrGroup::None;

		if (str == "Vertical")
			return CtrlrGroup::Vertical;

		if (str == "Horizontal")
			return CtrlrGroup::Horizontal;

		if (str == "Radial")
			return CtrlrGroup::Radial;

		return CtrlrGroup::None;
	}

	bool CtrlrGroup::isOwned(CtrlrComponent *componentToCheck) {
		for (int i = 0; i < content.getNumChildComponents(); i++) {
			CtrlrComponent *c = dynamic_cast<CtrlrComponent *>(content.getChildComponent(i));
			if (c != nullptr) {
				if (c == componentToCheck)
					return (true);
			}
		}

		return (false);
	}

	Array<CtrlrComponent *> CtrlrGroup::getOwnedChildren() {
		Array<CtrlrComponent *> ret;

		for (int i = 0; i < content.getNumChildComponents(); i++) {
			CtrlrComponent *c = dynamic_cast<CtrlrComponent *>(content.getChildComponent(i));
			if (c != nullptr) {
				ret.add(c);
			}
		}

		return (ret);
	}

	void CtrlrGroup::setOwned(CtrlrComponent *componentToOwn, const int subIndexInGroup,
							  const bool shouldOwnThisComponent) {
		if (componentToOwn == nullptr) // Updated v5.6.36
			return;

		if (shouldOwnThisComponent) {
			content.addAndMakeVisible(componentToOwn);
			componentToOwn->setProperty(Ids::componentGroupName, owner.getName(), true);
			componentToOwn->setProperty(Ids::componentGroupped, true, true);

			// When moving into a group, we should also clear any old Tab associations
			if (auto *dragContainer = DragAndDropContainer::findParentDragContainerFor(this)) {
				if (dragContainer->isDragAndDropActive()) {
					componentToOwn->setProperty(Ids::componentTabName, String(), true);
				}
			}
		} else {
			owner.getOwnerPanel().getEditor()->getCanvas()->addAndMakeVisibleNg(componentToOwn);
			componentToOwn->setProperty(Ids::componentGroupped, false, true);

			// NEW FIX: Clear the group name when dragged out to the canvas
			if (auto *dragContainer = DragAndDropContainer::findParentDragContainerFor(this)) {
				if (dragContainer->isDragAndDropActive()) {
					componentToOwn->setProperty(Ids::componentGroupName, String(), true);
				}
			}
		}
	}

	void CtrlrGroup::canvasStateRestored() {
		// applyRestoredProperties();
		// DBG("CtrlrGroup::canvasStateRestored() called for group: " + owner.getName());
		Array<CtrlrModulator *> children =
			owner.getOwnerPanel().getModulatorsWithProperty(Ids::componentGroupName, owner.getName());

		for (int i = 0; i < children.size(); i++) {
			if (children[i]->getComponent()) {
				if (owner.getOwnerPanel().isSchemeAtLeast(1)) {
					if (children[i]->getComponent()->getProperty(Ids::componentGroupped)) {
						setOwned(children[i]->getComponent(), 0, true);

						if (children[i]->getComponent()->getProperty(Ids::componentSentBack)) {
							children[i]->getComponent()->toBack();
						}
					}
				} else {
					setOwned(children[i]->getComponent(), 0, true);

					if (children[i]->getComponent()->getProperty(Ids::componentSentBack)) {
						children[i]->getComponent()->toBack();
					}
				}
			}
		}
	}

	void CtrlrGroup::modulatorNameChanged(const String &newName) {
		for (int i = 0; i < content.getNumChildComponents(); i++) {
			CtrlrComponent *c = dynamic_cast<CtrlrComponent *>(content.getChildComponent(i));
			if (c != 0) {
				c->setProperty(Ids::componentGroupName, newName, true);
			}
		}
	}

	bool CtrlrGroup::isInterestedInDragSource(const SourceDetails &dragSourceDetails) {
		if (dragSourceDetails.description == "__ctrlr_component_selection") {
			return (true);
		}

		return (false);
	}

	void CtrlrGroup::itemDropped(const SourceDetails &dragSourceDetails) {
		if (dragSourceDetails.description == "__ctrlr_component_selection") {
			if (owner.getOwnerPanel().getEditor() && owner.getOwnerPanel().getEditor()->getSelection()) {
				AffineTransform trans = owner.getOwnerPanel().getEditor()->moveSelectionToPosition(
					dragSourceDetails.localPosition.getX(), dragSourceDetails.localPosition.getY());

				for (int i = 0; i < owner.getOwnerPanel().getEditor()->getSelection()->getNumSelected(); i++) {
					CtrlrComponent *c = owner.getOwnerPanel().getEditor()->getSelection()->getSelectedItem(i);

					if (c == this || isOwned(c) || (bool)c->getProperty(Ids::componentIsLocked) == true)
						continue;

					setOwned(c, 0, true);

					c->setBounds(c->getBounds().transformedBy(trans));
				}
			}
		}
	}

	void CtrlrGroup::itemDragExit(const SourceDetails &dragSourceDetails) {}

	void CtrlrGroup::itemDragEnter(const SourceDetails &dragSourceDetails) {}

	void CtrlrGroup::setResource() {
		groupBackgroundImage =
			owner.getOwnerPanel().getResourceManager().getResourceAsImage(getProperty(Ids::uiGroupBackgroundImage));
		repaint();
		resized();
	}

	void CtrlrGroup::reloadResources(Array<CtrlrPanelResource *> resourcesThatChanged) {
		for (int i = 0; i < resourcesThatChanged.size(); i++) {
			if (resourcesThatChanged[i]->getName() == getProperty(Ids::uiGroupBackgroundImage).toString()) {
				setResource();
			}
		}
	}

	void CtrlrGroup::customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel) {
		content.customLookAndFeelChanged(customLookAndFeel);
	}

	std::unique_ptr<juce::LookAndFeel>
	CtrlrGroup::getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty) {
		if (lookAndFeelComponentProperty == "Default") {
			return nullptr;
		}

		return gui::createLookAndFeelFromDescription(lookAndFeelComponentProperty, false);
	}

	void CtrlrGroup::resetLookAndFeelOverrides() {
		// 	if (restoreStateInProgress ==
		// 		false) // To prevent the props lines position stacking up to top and keep their original position
		// 	{
		// 		// Do not wipe out custom colors if the component is set to Custom mode
		// 		// if (getProperty(Ids::uiGroupLookAndFeelIsCustom) || restoreStateInProgress)
		// 		// 	return;

		// 		restoreStateInProgress = true; // Lock property callbacks temporarily

		// 		String activeLnF = getProperty(Ids::uiGroupLookAndFeel);

		// 		if (activeLnF == "V1" || activeLnF == "V2" || activeLnF == "V3") {
		// 			setProperty(Ids::uiGroupTextColour, "0xff000000");
		// 			setProperty(Ids::uiGroupBackgroundGradientType, 1);
		// 			setProperty(Ids::uiGroupBackgroundColour1, "0xffa3a3a3");
		// 			setProperty(Ids::uiGroupBackgroundColour2, "0xffffffff");
		// 			setProperty(Ids::uiGroupOutlineGradientType, "Vertical");
		// 			setProperty(Ids::uiGroupOutlineColour1, "0xffa3a3a3");
		// 			setProperty(Ids::uiGroupOutlineColour2, "0xffffffff");
		// 		} else {
		// 			setProperty(Ids::uiGroupTextColour, (String)findColour(Label::textColourId).toString());
		// 			setProperty(Ids::uiGroupBackgroundGradientType, 0);
		// 			setProperty(Ids::uiGroupBackgroundColour1,
		// 						(String)findColour(DocumentWindow::backgroundColourId).darker(0.1f).toString());
		// 			setProperty(Ids::uiGroupBackgroundColour2,
		// 						(String)findColour(DocumentWindow::backgroundColourId).toString());
		// 			setProperty(Ids::uiGroupOutlineGradientType, "SolidColour");
		// 			setProperty(Ids::uiGroupOutlineColour1,
		// 						(String)findColour(DocumentWindow::textColourId).darker(0.2f).toString());
		// 			setProperty(Ids::uiGroupOutlineColour2,
		// (String)findColour(DocumentWindow::textColourId).toString());
		// 		}

		// 		restoreStateInProgress = false; // Unlock callbacks
		// 	}
	}

	void CtrlrGroup::updatePropertiesPanel() {
		CtrlrPanelProperties *props =
			owner.getCtrlrManagerOwner().getActivePanel()->getEditor(false)->getPropertiesPanel();
		if (props) {
			props->refreshAll(); // Needs extra code to prevent scrolling back to top on refresh
		}
	}
	void CtrlrGroup::applyLabelProperties() {
		if (label == nullptr)
			return;

		if (componentTree.hasProperty(Ids::uiGroupText))
			label->setText(getProperty(Ids::uiGroupText), dontSendNotification);

		if (componentTree.hasProperty(Ids::uiGroupTextFont))
			label->setFont(STR2FONT(getProperty(Ids::uiGroupTextFont)));

		if (componentTree.hasProperty(Ids::uiGroupTextColour))
			label->setColour(Label::textColourId, VAR2COLOUR(getProperty(Ids::uiGroupTextColour)));

		if (componentTree.hasProperty(Ids::uiGroupTextPlacement))
			label->setJustificationType(justificationFromProperty(getProperty(Ids::uiGroupTextPlacement)));

		if (componentTree.hasProperty(Ids::uiGroupTextMargin))
			textMargin = getProperty(Ids::uiGroupTextMargin);

		if (componentTree.hasProperty(Ids::componentLabelVisible))
			label->setVisible((bool)getProperty(Ids::componentLabelVisible));

		resized();
	}
	// void CtrlrGroup::applyRestoredProperties() {
	// 	if (label == nullptr)
	// 		return;

	// 	label->setText(getProperty(Ids::uiGroupText), dontSendNotification);

	// 	label->setFont(STR2FONT(getProperty(Ids::uiGroupTextFont)));

	// 	label->setJustificationType(justificationFromProperty(getProperty(Ids::uiGroupTextPlacement)));

	// 	textMargin = getProperty(Ids::uiGroupTextMargin);

	// 	label->setVisible((bool)getProperty(Ids::componentLabelVisible));

	// 	updateComponentColors();

	// 	resized();
	// }
	//[/MiscUserCode]

	//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CtrlrGroup" componentName=""
                 parentClasses="public CtrlrComponent" constructorParams="CtrlrModulator &amp;owner"
                 variableInitialisers="CtrlrComponent(owner), content(*this)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="128" initialHeight="128">
  <BACKGROUND backgroundColour="ffffff"/>
  <LABEL name="new label" id="20a4cb0ec13b8efc" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0 0 0M 0M" edTextCol="ff000000" edBkgCol="0"
         labelText="Group Text" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="14"
         bold="1" italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

	//[/MiscUserCode]

	//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CtrlrGroup" componentName=""
                 parentClasses="public CtrlrComponent" constructorParams="CtrlrModulator &amp;owner"
                 variableInitialisers="CtrlrComponent(owner), content(*this)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="128" initialHeight="128">
  <BACKGROUND backgroundColour="ffffff"/>
  <LABEL name="new label" id="20a4cb0ec13b8efc" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0 0 0M 0M" edTextCol="ff000000" edBkgCol="0"
         labelText="Group Text" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="14"
         bold="1" italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
