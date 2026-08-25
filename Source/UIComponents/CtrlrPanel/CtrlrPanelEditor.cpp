#include "CtrlrPanelEditor.h"
#include "CtrlrComponents/CtrlrCombo.h"
#include "CtrlrComponents/CtrlrComponent.h"
#include "CtrlrComponents/CtrlrComponentTypeManager.h"
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLog.h"
#include "CtrlrLuaManager.h"
#include "CtrlrMIDI/CtrlrMIDISettingsDialog.h"
#include "CtrlrMacros.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrModulator/CtrlrModulator.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelCanvas.h"
#include "CtrlrPanel/CtrlrPanelResource.h"
#include "CtrlrPanel/CtrlrPanelViewport.h"
#include "CtrlrProcessor.h"
#include "CtrlrUtilities.h"
#include "JuceClasses/LMemoryBlock.h"
#include "stdafx.h"

//--------------------------------------------------------------------------------------------------
// CtrlrPanelNotifier
//--------------------------------------------------------------------------------------------------

CtrlrPanelNotifier::CtrlrPanelNotifier(
	CtrlrPanelEditor &_owner) // Added back v5.6.31 for file management bottom notification bar
	: owner(_owner), background(Colours::lightgrey) {
	text = std::make_unique<Label>();
	addAndMakeVisible(text.get());
	text->addMouseListener(this, true);
	text->setColour(Label::backgroundColourId, Colours::transparentBlack);
	text->setColour(Label::textColourId, Colours::white.withAlpha(0.85f));
	text->setColour(Label::outlineColourId, Colours::transparentBlack);
	text->setFont(Font(12.0f, Font::bold));
	text->setText("", dontSendNotification); // Default text required
}
CtrlrPanelNotifier::~CtrlrPanelNotifier() // Added v5.6.34. Thanks to @dnaldoog
{
	DBG("(F) ~CtrlrPanelNotifier()");
	// The ScopedPointer 'text' will be automatically cleaned up.
	// No manual cleanup is needed. But...
	text = nullptr; // Force ScopedPointer cleanup
}
void CtrlrPanelNotifier::paint(Graphics &g) // Added back v5.6.31 for file management bottom notification bar
{
	gui::drawSelectionRectangle(g, getWidth(), getHeight(), background); // Updated v5.6.31 (link to GUI class)
}

void CtrlrPanelNotifier::resized() // Added back v5.6.31 for file management bottom notification bar
{
	text->setBounds(0, 0, getWidth(), getHeight());
}

void CtrlrPanelNotifier::setNotification(
	const String &notification,
	const CtrlrNotificationType ctrlrNotificationType) // Added back v5.6.31 for file management bottom notification bar
{
	background = getBackgroundColourForNotification(ctrlrNotificationType);
	text->setText(notification, dontSendNotification);
}
// Call this explicitly BEFORE CtrlrManager or CtrlrDocumentPanel deletes the panel/editor:
void CtrlrPanelNotifier::panelWillClose() {
	if (text != nullptr) {
		text->removeMouseListener(this);
		removeChildComponent(text.get());
		text.reset(); // Destroy juce::Label NOW while ValueTree memory is valid
	}
}
void CtrlrPanelNotifier::mouseDown(const MouseEvent &e) {
	owner.notificationClicked(e);
}

Colour CtrlrPanelNotifier::getBackgroundColourForNotification(
	const CtrlrNotificationType ctrlrNotificationType) // Added back v5.6.31 for file management bottom notification bar
{
	switch (ctrlrNotificationType) {
	case NotifySuccess:
		return (Colours::green.brighter(0.2f));
	case NotifyFailure:
		return (Colours::red.brighter(0.2f));
	case NotifyWarning:
		return (Colours::yellow.darker(0.5f));
	case NotifyInformation:
		return (Colours::grey);
	}

	return (Colours::lightgrey);
}

//--------------------------------------------------------------------------------------------------
// CtrlrPanelEditor
//--------------------------------------------------------------------------------------------------
/*
In JUCE 7/8, MultiDocumentPanel refactored its tab component integration.
It grabs component->getName() once at the exact moment addDocument() is called to build the tab title.
If the component's name hasn't been updated before it gets registered to the panel,
JUCE 8 caches "Ctrlr Panel Editor" and won't re-query getName() when the XML finishes parsing later.
i.e. use panelname not
CtrlrPanelEditor::CtrlrPanelEditor(CtrlrPanel &_owner, CtrlrManager &_ctrlrManager, const juce::String &panelName)
	: juce::Component(L"Ctrlr Panel Editor"),
*/
CtrlrPanelEditor::CtrlrPanelEditor(CtrlrPanel &_owner, CtrlrManager &_ctrlrManager, const juce::String &panelName)
	: juce::Component(panelName),
	  lastEditMode(true),
	  ctrlrManager(_ctrlrManager),
	  owner(_owner),
	  panelEditorTree(Ids::uiPanelEditor),
	  currentRestoreState(true),
	  canvasWidth(0),
	  canvasHeight(0) {

	ctrlrComponentSelection.reset(new CtrlrComponentSelection(*this));
	ctrlrPanelViewport = std::make_unique<CtrlrPanelViewport>(*this);
	ctrlrPanelProperties = std::make_unique<CtrlrPanelProperties>(*this);
	spacerComponent = std::make_unique<juce::StretchableLayoutResizerBar>(&layoutManager, 1, true);

	addAndMakeVisible(ctrlrPanelViewport.get());
	addAndMakeVisible(ctrlrPanelProperties.get());
	addAndMakeVisible(spacerComponent.get());

	// Added back v5.6.31 for file management bottom notification bar
	ctrlrPanelNotifier = std::make_unique<CtrlrPanelNotifier>(*this);
	addAndMakeVisible(ctrlrPanelNotifier.get());

	ctrlrPanelNotifier->setAlwaysOnTop(true); // Added back v5.6.31 for file management bottom notification bar
	ctrlrPanelNotifier->setVisible(false);	  // Added back v5.6.31 for file management bottom notification bar
	// componentAnimator.addChangeListener (this); // Added back v5.6.31 not working

	spacerComponent->setName(L"spacerComponent");

	getPanelEditorTree().addListener(this);

	layoutManager.setItemLayout(0, -0.001, -1.0, -0.7);
	layoutManager.setItemLayout(1, 8, 8, 8);
	layoutManager.setItemLayout(2, -0.001, -1.0, -0.3);

	editorComponentsInEditMode[0] = ctrlrPanelViewport.get();
	editorComponentsInEditMode[1] = spacerComponent.get();
	editorComponentsInEditMode[2] = ctrlrPanelProperties.get();

	editorComponents[0] = ctrlrPanelViewport.get();

	setProperty(Ids::name, panelName);
	setProperty(Ids::uiPanelEditMode, true);
	setProperty(Ids::uiPanelLock, false);
	setProperty(Ids::uiPanelDisabledOnEdit, false);
	setProperty(Ids::uiPanelMenuBarVisible, true);
	setProperty(Ids::uiPanelMenuBarHideOnExport, false);
	setProperty(Ids::uiPanelProgramsMenuHideOnExport, false);
	setProperty(Ids::uiPanelMidiControllerMenuHideOnExport, false);
	setProperty(Ids::uiPanelMidiThruMenuHideOnExport, false);
	setProperty(Ids::uiPanelMidiChannelMenuHideOnExport, false);

	setProperty(Ids::uiPanelViewPortSize, 800);
	setProperty(Ids::uiPanelPropertiesSize, 300);

	setProperty(Ids::uiViewPortResizable, true);
	setProperty(Ids::uiViewPortShowScrollBars, true);
	setProperty(Ids::uiViewPortWidth, 400);
	setProperty(Ids::uiViewPortHeight, 400);
	setProperty(Ids::uiViewPortEnableResizeLimits, false);
	setProperty(Ids::uiViewPortMinWidth, 0);
	setProperty(Ids::uiViewPortMinHeight, 0);
	setProperty(Ids::uiViewPortMaxWidth, 0);
	setProperty(Ids::uiViewPortMaxHeight, 0);
	setProperty(Ids::uiViewPortEnableFixedAspectRatio, false);
	setProperty(Ids::uiViewPortFixedAspectRatio, 1.5);
	setProperty(Ids::uiPanelZoom, 1.0);

	setProperty(Ids::uiPanelViewPortBackgroundColour,
				(String)Component::findColour(ResizableWindow::backgroundColourId)
					.withAlpha(0.7f)
					.toString()); // ViewPort background color. was "transparentblack"
	setProperty(
		Ids::uiPanelBackgroundColour,
		(String)Component::findColour(ResizableWindow::backgroundColourId).toString()); // Canvas Colour 0xffffffff
	setProperty(
		Ids::uiPanelBackgroundColour1,
		(String)Component::findColour(ResizableWindow::backgroundColourId).toString()); // Canvas Colour1 if gradient
	setProperty(Ids::uiPanelBackgroundColour2, (String)Component::findColour(ResizableWindow::backgroundColourId)
												   .darker(0.2f)
												   .toString()); // Canvas Colour2 if gradient
	setProperty(Ids::uiPanelBackgroundGradientType, 0);			 // Default set to SolidColor [No background gradient]
	setProperty(Ids::uiPanelImageResource, COMBO_ITEM_NONE);
	setProperty(Ids::uiPanelImageAlpha, 255);
	setProperty(Ids::uiPanelImageLayout, 64);
	setProperty(Ids::uiPanelSnapActive, true);

	setProperty(Ids::uiPanelSnapSize, 8);
	setProperty(Ids::uiPanelPropertiesOnRight, false);

	setProperty(Ids::luaPanelPaintBackground, COMBO_ITEM_NONE);
	setProperty(Ids::luaViewPortResized, COMBO_ITEM_NONE);
	setProperty(Ids::luaPanelResized, COMBO_ITEM_NONE);
	setProperty(Ids::luaPanelFileDragDropHandler, COMBO_ITEM_NONE);
	setProperty(Ids::luaPanelFileDragEnterHandler, COMBO_ITEM_NONE);
	setProperty(Ids::luaPanelFileDragExitHandler, COMBO_ITEM_NONE);

	setProperty(Ids::uiPanelInvisibleComponentAlpha, 0.5);

	setProperty(Ids::uiPanelTooltipPlacement, BubbleComponent::below);
	setProperty(Ids::uiPanelTooltipFont, Font(12.0f, Font::plain).toString());
	setProperty(Ids::uiPanelTooltipColour, (String)Component::findColour(Label::textColourId).toString()); // 0xff000000
	setProperty(Ids::uiPanelTooltipBackgroundColour,
				(String)Component::findColour(BubbleComponent::backgroundColourId).toString()); // 0xffeeeebb
	setProperty(Ids::uiPanelTooltipOutlineColour,
				(String)Component::findColour(BubbleComponent::outlineColourId).toString()); // 0xff000000
	setProperty(Ids::uiPanelTooltipCornerRound, 1.0);

	ValueTree ed = owner.getCtrlrManagerOwner().getManagerTree();

	if (ed.getProperty(Ids::ctrlrLegacyMode) == "1" || ed.getProperty(Ids::ctrlrLookAndFeel) == "V3" ||
		ed.getProperty(Ids::ctrlrLookAndFeel) == "V2" || ed.getProperty(Ids::ctrlrLookAndFeel) == "V1") {
		setProperty(Ids::uiPanelLegacyMode, "1");
		setProperty(Ids::uiPanelLookAndFeel, "V3");
	} else {
		setProperty(Ids::uiPanelLegacyMode, false);
		setProperty(Ids::uiPanelLookAndFeel, "V4");

		// Requires passing the colourScheme to the property uiPanelLookAndFeel from ctrlrColourScheme
		// Updated v5.6.34. For a generic method schemeName Property--> schemeName. Get the current colour scheme name
		// from the property
		juce::String schemeName = ed.getProperty(Ids::ctrlrColourScheme).toString();

		// <fallback for empty instances without any colourscheme yet defined
		if (schemeName.isEmpty()) {
			schemeName = "Light";
		}

		// Determine the LookAndFeel description string
		juce::String lookAndFeelDesc;

		if (schemeName.startsWith("V4 ")) {
			// If it already has "V4 ", use it as is
			lookAndFeelDesc = schemeName;
		} else {
			// Otherwise, prepend "V4 " (e.g., "Light" becomes "V4 Light")
			lookAndFeelDesc = "V4 " + schemeName;
		}

		// Set the uiPanelLookAndFeel property with the determined string
		setProperty(Ids::uiPanelLookAndFeel, lookAndFeelDesc);
	}

#if JUCE_LINUX
    juce::Timer::callAfterDelay(1250, [this]()
    {
        DBG("!*! Linux Deferred MIDI Refresh Triggered !*!");

        // 1. Force ALSA/JACK device enumeration
        auto& devMgr = this->owner.getCtrlrManagerOwner().getCtrlrMidiDeviceManager();
        devMgr.refreshDevices();

        // 2. Fetch active saved device properties
        const String inDev   = this->owner.getProperty(Ids::panelMidiInputDevice).toString();
        const String outDev  = this->owner.getProperty(Ids::panelMidiOutputDevice).toString();
        const String ctrlDev = this->owner.getProperty(Ids::panelMidiControllerDevice).toString();

        // 3. Re-trigger properties to force open streams and re-bind device indices
        if (inDev.isNotEmpty())
            this->owner.setProperty(Ids::panelMidiInputDevice, inDev, true);

        if (outDev.isNotEmpty())
            this->owner.setProperty(Ids::panelMidiOutputDevice, outDev, true);

        if (ctrlDev.isNotEmpty())
            this->owner.setProperty(Ids::panelMidiControllerDevice, ctrlDev, true);

        // 4. Force property panel / menu bar UI components to refresh their dropdown models & checkmarks
        if (auto* props = this->getPropertiesPanel()) {
            props->refreshAll();
        }

        // Force editor menu bar redraw
        this->getOwner().getEditor()->repaint();
    });
#endif

	ctrlrComponentSelection->addChangeListener(ctrlrPanelProperties.get());

	setSize(600, 400);

	ctrlrComponentSelection->sendChangeMessage();
}


void CtrlrPanelEditor::timerCallback()
{
    // Stop the timer immediately so it acts as a single-shot trigger
#if JUCE_LINUX
    DBG("!*! Linux Deferred MIDI Refresh Triggered !*!");

    // 1. Force the global device scan once the window handle & event queue are fully active
    owner.getCtrlrManagerOwner().getCtrlrMidiDeviceManager().refreshDevices();

    // 2. Re-apply saved MIDI properties to trigger setMidiDeviceFromProperty and open streams
    const String inDev  = owner.getProperty(Ids::panelMidiInputDevice).toString();
    const String outDev = owner.getProperty(Ids::panelMidiOutputDevice).toString();
    const String ctrlDev = owner.getProperty(Ids::panelMidiControllerDevice).toString();

    if (inDev.isNotEmpty())
        owner.setProperty(Ids::panelMidiInputDevice, inDev, true);

    if (outDev.isNotEmpty())
        owner.setProperty(Ids::panelMidiOutputDevice, outDev, true);

    if (ctrlDev.isNotEmpty())
        owner.setProperty(Ids::panelMidiControllerDevice, ctrlDev, true);
#endif
}
void CtrlrPanelEditor::panelWillClose() {
	// 1. Detach and destroy notifier while manager/trees are 100% alive
	if (ctrlrPanelNotifier != nullptr) {
		ctrlrPanelNotifier->panelWillClose();

		// Remove from UI hierarchy and null out unique_ptr/pointer
		removeChildComponent(ctrlrPanelNotifier.get()); // or panelNotifier if raw pointer
		ctrlrPanelNotifier.reset();						// If panelNotifier is std::unique_ptr
	}

	// 2. Clear LookAndFeel bindings to avoid juce_LookAndFeel.cpp assertion
	setLookAndFeel(nullptr);
}
CtrlrPanelEditor::~CtrlrPanelEditor() {
	DBG("(E) ~CtrlrPanelEditor Destructor Called");

	// 1. Unhook listeners / callbacks first
	// (If CtrlrPanelEditor implements juce::ChangeListener, KeyListener, etc.)

	// 2. Safely detach all child components from JUCE's component tree
	// WITHOUT deleting them via JUCE
	for (int i = getNumChildComponents() - 1; i >= 0; --i) {
		if (auto *child = getChildComponent(i)) {
			removeChildComponent(child);
		}
	}

	// 3. Reset unique pointers explicitly in dependency order if they exist
	// (Replace or adjust these member names to match your CtrlrPanelEditor.h declarations)
	if (ctrlrPanelViewport != nullptr) {
		ctrlrPanelViewport.reset();
	}
}

void CtrlrPanelEditor::visibilityChanged() {}

void CtrlrPanelEditor::resized() {
	ctrlrPanelViewport->setBounds(0, 0, getWidth() - 608, getHeight());
	ctrlrPanelProperties->setBounds(getWidth() - 600, 32, 600, getHeight() - 32);
	spacerComponent->setBounds(getWidth(), 32, 8, getHeight() - 32);

	setProperty(Ids::uiViewPortWidth, getWidth());
	setProperty(Ids::uiViewPortHeight, getHeight());

	if (ctrlrPanelNotifier) {
		ctrlrPanelNotifier->setBounds(0, getHeight() - 28, getWidth() - 32, 20);
	}

	layoutItems();

	if (!getRestoreState()) {
		saveLayout();
	}

	if (resizedCbk && !resizedCbk.wasObjectDeleted()) {
		if (resizedCbk->isValid()) {
			owner.getCtrlrLuaManager().getMethodManager().call(resizedCbk, &owner);
		}
	}
}
CtrlrComponentSelection *CtrlrPanelEditor::getSelection() {
	return (ctrlrComponentSelection.get());
}

void CtrlrPanelEditor::layoutItems() {
	if (getProperty(Ids::uiPanelEditMode)) {
		if (getProperty(Ids::uiPanelPropertiesOnRight)) {
			Component *comps[] = {ctrlrPanelProperties.get(), spacerComponent.get(), ctrlrPanelViewport.get()};
			layoutManager.layOutComponents(comps, 3, 0, 0, getWidth(), getHeight(), false, true);
		} else {
			Component *comps[] = {ctrlrPanelViewport.get(), spacerComponent.get(), ctrlrPanelProperties.get()};
			layoutManager.layOutComponents(comps, 3, 0, 0, getWidth(), getHeight(), false, true);
		}
	} else {
		layoutManager.layOutComponents(editorComponents, 1, 0, 0, getWidth(), getHeight(), false, true);
	}
}

void CtrlrPanelEditor::saveLayout() {
	setProperty(Ids::uiPanelViewPortSize, layoutManager.getItemCurrentAbsoluteSize(0));
	setProperty(Ids::uiPanelPropertiesSize, layoutManager.getItemCurrentAbsoluteSize(2));
}

CtrlrPanelCanvas *CtrlrPanelEditor::getCanvas() {
	if (ctrlrPanelViewport != 0) {
		return (ctrlrPanelViewport->getCanvas());
	}

	return (0);
}
void CtrlrPanelEditor::editModeChanged() {
	const bool editMode = getProperty(Ids::uiPanelEditMode);
	owner.editModeChanged(editMode);

	if (editMode) {
		layoutManager.setItemLayout(0, -0.001, -1.0, getProperty(Ids::uiPanelViewPortSize, -0.7));
		layoutManager.setItemLayout(2, -0.001, -1.0, getProperty(Ids::uiPanelPropertiesSize, -0.3));
		spacerComponent->setVisible(true);
		ctrlrPanelProperties->setVisible(true);
		getCanvas()->getResizableBorder()->setVisible(true);

		// Notify the property inspector so it can display the current selection.
		// CtrlrPanelProperties does not expose a setTargetObject() method.
		if (getSelection() != nullptr)
			getSelection()->sendChangeMessage();

		if ((bool)getProperty(Ids::uiPanelDisableCombosOnEdit))
			setAllCombosDisabled();
		// if (editMode) {
		// 	layoutManager.setItemLayout(0, -0.001, -1.0, getProperty(Ids::uiPanelViewPortSize, -0.7));
		// 	layoutManager.setItemLayout(2, -0.001, -1.0, getProperty(Ids::uiPanelPropertiesSize, -0.3));
		// 	spacerComponent->setVisible(true);
		// 	ctrlrPanelProperties->setVisible(true);
		// 	getCanvas()->getResizableBorder()->setVisible(true);

		// 	if ((bool)getProperty(Ids::uiPanelDisableCombosOnEdit))
		// 		setAllCombosDisabled();
	} else {
		if (getSelection())
			getSelection()->deselectAll();
		spacerComponent->setVisible(false);
		ctrlrPanelProperties->setVisible(false);
		getCanvas()->getResizableBorder()->setVisible(false);

		if ((bool)getProperty(Ids::uiPanelDisableCombosOnEdit))
			setAllCombosEnabled();
	}

	resized();
}

void CtrlrPanelEditor::setAllCombosDisabled() {
	OwnedArray<CtrlrModulator, CriticalSection> &mods = owner.getModulators();
	for (int i = 0; i < mods.size(); i++) {
		CtrlrCombo *cc = dynamic_cast<CtrlrCombo *>(mods[i]->getComponent());
		if (cc != nullptr) {
			cc->setEnabled(false);
		}
	}
}

void CtrlrPanelEditor::setAllCombosEnabled() {
	OwnedArray<CtrlrModulator, CriticalSection> &mods = owner.getModulators();
	for (int i = 0; i < mods.size(); i++) {
		CtrlrCombo *cc = dynamic_cast<CtrlrCombo *>(mods[i]->getComponent());
		if (cc != nullptr) {
			cc->setEnabled(true);
		}
	}
}

void CtrlrPanelEditor::restoreState(const ValueTree &savedState) {
	setVisible(false);

	setRestoreState(true);

	restoreProperties(savedState.getChildWithName(Ids::uiPanelEditor), panelEditorTree, 0);

	bool IsNotLegacyMode = savedState.getChildWithName(Ids::uiPanelEditor)
							   .hasProperty(Ids::uiPanelLegacyMode); // Legacy mode check for version before 5.6.29
	if (!IsNotLegacyMode) {
		setProperty(Ids::uiPanelLegacyMode, true);
		setProperty(Ids::uiPanelLookAndFeel, "V3");
	}

	getCanvas()->restoreState(savedState);

	if (getSelection()) {
		getSelection()->sendChangeMessage();
	}

	if (owner.getCtrlrManagerOwner().getInstanceMode() == InstanceSingle ||
		owner.getCtrlrManagerOwner().getInstanceMode() == InstanceSingleRestricted) {
		initSingleInstance();
	}

	editModeChanged();
	setRestoreState(false);

	setVisible(true);
}

CtrlrComponent *CtrlrPanelEditor::getSelected(const Identifier &type) {
	if (getSelection() == nullptr)
		return (nullptr);

	if (getSelection()->getNumSelected() == 1) {
		if (CtrlrComponentTypeManager::findType(getSelection()->getSelectedItem(0)) == type) {
			return (getSelection()->getSelectedItem(0));
		}
	}

	return (0);
}

void CtrlrPanelEditor::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) {
	if (!isShowing() || owner.getOwner().isShuttingDown())
		return;
	juce::Component::SafePointer<CtrlrPanelEditor> safeThis(this);
	if (safeThis == nullptr)
		return;
	if (owner.getOwner().isShuttingDown())
		return;
	// 1. Guard against a null editor context
	if (this == nullptr)
		return;

	// 2. Safely grab the parent panel and check its address
	auto &panel = getOwner(); // Or getPanel() depending on your exact class getter
	if (&panel == nullptr)
		return;

	// 3. Guard against the Lua manager reference resolving to nullptr during destruction
	if (&panel.getCtrlrLuaManager() == nullptr) {
		return;
	}

	// 4. Safely check if the method manager has already been deallocated
	auto &luaManager = panel.getCtrlrLuaManager();
	if (&luaManager.getMethodManager() == nullptr) {
		return;
	}
	if (treeWhosePropertyHasChanged.hasType(Ids::uiPanelEditor)) {
		if (property == Ids::uiPanelEditMode) {
			editModeChanged();
		} else if (property == Ids::luaViewPortResized) {
			if (getProperty(property) == "")
				return;

			resizedCbk = owner.getCtrlrLuaManager().getMethodManager().getMethod(getProperty(property));
		} else if (property == Ids::uiPanelSnapSize) {
			repaint();
		} else if (property == Ids::name) {
			// Use getPanelWindowTitle() to get the "*" when the panel is dirty
			Component::setName(owner.getPanelWindowTitle());
		} else if (property == Ids::uiPanelPropertiesOnRight) {
			ctrlrPanelProperties->layoutChanged();

			if (!owner.getCtrlrManagerOwner().isRestoring()) {
				resized();
			}
		} else if (property == Ids::uiPanelCanvasRectangle) {
			getCanvas()->setBounds(
				VAR2RECT(getProperty(property))); // update canvas size if values in the property field are changed
			canvasHeight =
				getCanvas()->getHeight();		   // Updated v5.6.31 by GoodWeather. Removed type double(canvasHeight)
			canvasWidth = getCanvas()->getWidth(); // Updated v5.6.31 by GoodWeather. Removed type double(canvasWidth)
			canvasAspectRatio =
				canvasWidth / canvasHeight; // Updated v5.6.31 by GoodWeather. Removed type double(canvasAspectRatio) =
											// double(canvasWidth) / double(canvasHeight)
			setProperty(Ids::uiViewPortFixedAspectRatio,
						canvasAspectRatio); // update canvas aspect ratio if canvas is resized
			resized();
		} else if (property == Ids::uiViewPortResizable || property == Ids::uiViewPortShowScrollBars ||
				   property == Ids::uiViewPortEnableFixedAspectRatio || property == Ids::uiViewPortFixedAspectRatio ||
				   property == Ids::uiViewPortEnableResizeLimits || property == Ids::uiViewPortMinWidth ||
				   property == Ids::uiViewPortMinHeight || property == Ids::uiViewPortMaxWidth ||
				   property == Ids::uiViewPortMaxHeight || property == Ids::uiViewPortShowScrollBars) {
			resized();
		} else if (property == Ids::uiViewPortWidth || property == Ids::uiViewPortHeight) {
			resized();
		} else if (property == Ids::uiPanelDisableCombosOnEdit) {
			if ((bool)getProperty(property) && getMode()) {
				setAllCombosDisabled();
			} else {
				setAllCombosEnabled();
			}
		} else if (property == Ids::uiPanelZoom) {
			getPanelViewport()->setZoom(getProperty(property), getCanvas()->getBounds().getCentre().getX(),
										getCanvas()->getBounds().getCentre().getY());
		} else if (property == Ids::uiPanelMenuBarVisible) {
			if (owner.getCtrlrManagerOwner().getEditor()) {
				owner.getCtrlrManagerOwner().getEditor()->activeCtrlrChanged();
			}
		} else if (property == Ids::uiPanelBackgroundGradientType || property == Ids::uiPanelViewPortBackgroundColour ||
				   property == Ids::uiPanelBackgroundColour || property == Ids::uiPanelBackgroundColour1 ||
				   property == Ids::uiPanelBackgroundColour2) {
			resized();
		} else if (property == Ids::uiPanelUIColourWindowBackground ||
				   property == Ids::uiPanelUIColourWidgetBackground || property == Ids::uiPanelUIColourMenuBackground ||
				   property == Ids::uiPanelUIColourOutline || property == Ids::uiPanelUIColourDefaultText ||
				   property == Ids::uiPanelUIColourDefaultFill || property == Ids::uiPanelUIColourHighlightedText ||
				   property == Ids::uiPanelUIColourHighlightedFill || property == Ids::uiPanelUIColourMenuText) {
		} else if (property == Ids::uiPanelLookAndFeel) {
			static bool handlingLookAndFeelChange = false;
			if (handlingLookAndFeelChange)
				return;
			handlingLookAndFeelChange = true;
			struct Guard {
					~Guard() {
						handlingLookAndFeelChange = false;
					}
			} guard;

			auto newLookAndFeel =
				std::unique_ptr<juce::LookAndFeel>(getLookAndFeelFromDescription(getProperty(property)));

			if (newLookAndFeel == nullptr)
				return;

			// Detach only the components that explicitly held our old lookAndFeel pointer.
			// Do NOT use sendLookAndFeelChange() here — it clears all children's explicit
			// LookAndFeel assignments (e.g. slider's lfV3 from applyCentralLookAndFeel).
			setLookAndFeel(nullptr);
			if (getCanvas() != nullptr)
				getCanvas()->setLookAndFeel(nullptr);
			if (ctrlrPanelProperties != nullptr)
				ctrlrPanelProperties->setLookAndFeel(nullptr);

			// Now safe to destroy old lookAndFeel — WeakReferences released
			lookAndFeel = std::move(newLookAndFeel);

			// Reattach new lookAndFeel
			getCanvas()->setLookAndFeel(lookAndFeel.get());
			setLookAndFeel(lookAndFeel.get());
			if (ctrlrPanelProperties != nullptr)
				ctrlrPanelProperties->setLookAndFeel(lookAndFeel.get());

			// Propagates to all children via normal inheritance
			lookAndFeelChanged();
			if (!getProperty(Ids::uiPanelLegacyMode)) {
				setProperty(
					Ids::uiPanelViewPortBackgroundColour,
					(String)Component::findColour(ResizableWindow::backgroundColourId).withAlpha(0.7f).toString());
				setProperty(Ids::uiPanelBackgroundColour,
							(String)Component::findColour(ResizableWindow::backgroundColourId).toString());
				setProperty(Ids::uiPanelBackgroundColour1,
							(String)Component::findColour(ResizableWindow::backgroundColourId).toString());
				setProperty(Ids::uiPanelBackgroundColour2,
							(String)Component::findColour(ResizableWindow::backgroundColourId).darker(0.2f).toString());
				setProperty(Ids::uiPanelTooltipBackgroundColour,
							(String)Component::findColour(BubbleComponent::backgroundColourId).toString());
				setProperty(Ids::uiPanelTooltipOutlineColour,
							(String)Component::findColour(BubbleComponent::outlineColourId).toString());
				setProperty(Ids::uiPanelTooltipColour, (String)Component::findColour(Label::textColourId).toString());
			}

			if (owner.getCtrlrManagerOwner().getEditor())
				owner.getCtrlrManagerOwner().getEditor()->activeCtrlrChanged();

			ctrlrPanelProperties->refreshAll();
			if (getSelection())
				getSelection()->sendChangeMessage();

			// --- ADD THIS BLOCK TO FORCE ALL SLIDERS / COMPONENTS TO UPDATE ---
			for (int i = 0; i < owner.getModulators().size(); ++i) {
				if (auto *mod = owner.getModulatorByIndex(i)) {
					if (auto *comp = mod->getComponent()) {
						// Notify the component to refresh its style/colors against the new Panel LnF

						comp->valueTreePropertyChanged(comp->getComponentTree(), Ids::uiSliderStyle);
						comp->valueTreePropertyChanged(comp->getComponentTree(), Ids::uiButtonLookAndFeel);
						comp->lookAndFeelChanged();
						comp->repaint();
					}
				}
			}
			if (getSelection())
				getSelection()->sendChangeMessage();
		}
	}
}

std::unique_ptr<juce::LookAndFeel>
CtrlrPanelEditor::getLookAndFeelFromDescription(const juce::String &lookAndFeelDesc) // Added v5.6.34
{
	// If "Default" has a special meaning for CtrlrPanelEditor, handle it here.
	// Otherwise, you can just directly call the generic function.
	if (lookAndFeelDesc == "Default") {
		return nullptr; // Or whatever "Default" means for this specific component
						// e.g., return new juce::LookAndFeel_V4();
	}

	// Now, simply call your centralized function!
	// The second argument `true` means if the `lookAndFeelDesc` doesn't match
	// any known scheme, it will return a new LookAndFeel_V4 with the LightColourScheme.
	// If you prefer it to return nullptr in unknown cases, change it to `false`.
	return gui::createLookAndFeelFromDescription(lookAndFeelDesc, true);
}

const var &CtrlrPanelEditor::getProperty(const Identifier &name) const {
	return (panelEditorTree.getProperty(name));
}

const var CtrlrPanelEditor::getProperty(const Identifier &name, const var &defaultReturnValue) const {
	return (panelEditorTree.getProperty(name, defaultReturnValue));
}

void CtrlrPanelEditor::setProperty(const Identifier &name, const var &newValue, const bool isUndoable) {
	panelEditorTree.setProperty(name, newValue, 0);
}

const bool CtrlrPanelEditor::getMode() {
	return (getProperty(Ids::uiPanelEditMode));
}

AffineTransform CtrlrPanelEditor::moveSelectionToPosition(const int newX, const int newY) {
	if (getSelection() == nullptr)
		return (AffineTransform());

	RectangleList<int> rectangleList;

	for (int i = 0; i < getSelection()->getNumSelected(); i++) {
		CtrlrComponent *c = getSelection()->getSelectedItem(i);
		rectangleList.add(c->getBounds());
	}

	RectanglePlacement rp(RectanglePlacement::xLeft);
	return (rp.getTransformToFit(rectangleList.getBounds().toFloat(),
								 rectangleList.getBounds().withPosition(newX, newY).toFloat()));
}

void CtrlrPanelEditor::initSingleInstance() {
	owner.setProperty(Ids::uiPanelEditMode, false);
}

void CtrlrPanelEditor::notify(
	const String &notification, CtrlrNotificationCallback *callback,
	const CtrlrNotificationType ctrlrNotificationType) // Added back v5.6.31 for file management bottom notification bar
{
	if (ctrlrPanelNotifier) {
		if ((int)owner.getProperty(Ids::panelMessageTime) <= 0)
			return;

		notificationCallback = callback;

		componentAnimator.cancelAllAnimations(true);
		ctrlrPanelNotifier->setVisible(true);

		if (notificationCallback != nullptr) {
			ctrlrPanelNotifier->setMouseCursor(MouseCursor::PointingHandCursor);
		} else {
			ctrlrPanelNotifier->setMouseCursor(MouseCursor::NormalCursor);
		}
		ctrlrPanelNotifier->setBounds(0, getHeight() - 28, getWidth(), 20);
		ctrlrPanelNotifier->setAlpha(1.0f);

		ctrlrPanelNotifier->setNotification(notification, ctrlrNotificationType);

		componentAnimator.animateComponent(ctrlrPanelNotifier.get(), ctrlrPanelNotifier->getBounds(), 0.0f,
										   owner.getProperty(Ids::panelMessageTime), false, 1.0, 1.0);
	}
}

void CtrlrPanelEditor::notificationClicked(
	const MouseEvent e) // Added back v5.6.31 for file management bottom notification bar
{
	componentAnimator.cancelAllAnimations(true);

	if (!notificationCallback.wasObjectDeleted() && notificationCallback) {
		notificationCallback->notificationClicked(e);
	}
}

void CtrlrPanelEditor::changeListenerCallback(
	ChangeBroadcaster *source) // Added back v5.6.31 for file management bottom notification bar
{
	if (source == &componentAnimator) {
		if (!componentAnimator.isAnimating()) {
			ctrlrPanelNotifier->setVisible(false);
		}
	}
}

void CtrlrPanelEditor::reloadResources(Array<CtrlrPanelResource *> resourcesThatChanged) {
	for (int i = 0; i < owner.getNumModulators(); i++) {
		if (owner.getModulatorByIndex(i)->getComponent()) {
			owner.getModulatorByIndex(i)->getComponent()->reloadResources(resourcesThatChanged);
		}
	}

	resized();
}

void CtrlrPanelEditor::searchForProperty() {
	if (panelFindProperty == nullptr) {
		// Pass 'this' as owner, and nullptr for props
		panelFindProperty.reset(new CtrlrPanelFindProperty(*this, nullptr));

		if (panelFindProperty != nullptr) {
			addAndMakeVisible(panelFindProperty.get());
			panelFindProperty->toFront(true);
		}
	} else {
		panelFindProperty->toFront(true);
	}
}

bool CtrlrPanelEditor::luaEditorExistsAndIsFocused() // Added v5.6.34. Required to pass keypress to the LUA method
													 // manager for menu items. Handles the focus gain/loss.
{
	// We use the public getContent() method on the window manager.
	// This will return a pointer to the component inside the Lua editor window.
	juce::Component *luaEditorContent =
		owner.getPanelWindowManager().getContent(CtrlrPanelWindowManager::LuaMethodEditor);

	// Now we check if the content component exists and has keyboard focus.
	if (luaEditorContent != nullptr && luaEditorContent->hasKeyboardFocus(true)) {
		return true;
	}

	return false;
}