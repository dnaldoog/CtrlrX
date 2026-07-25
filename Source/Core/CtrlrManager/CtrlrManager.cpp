#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrApplicationWindow/CtrlrEditor.h"
#include "CtrlrComponents/CtrlrComponent.h"
#include "CtrlrLog.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelCanvas.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "Plugin/CtrlrProcessor.h"
#include "stdafx.h"

CtrlrManager::CtrlrManager(CtrlrProcessor *_owner, CtrlrLog &_ctrlrLog)
	: managerTree(Ids::manager),
	  ctrlrLog(_ctrlrLog),
	  owner(_owner),
	  ctrlrWindowManager(*this),
	  ctrlrMidiDeviceManager(*this),
	  ctrlrDocumentPanel(nullptr),
	  ctrlrManagerVst(nullptr),
	  ctrlrNativeObject(nullptr),
	  audioThumbnailCache(256),
	  ctrlrPlayerInstanceMode(InstanceMulti),
	  ctrlrManagerRestoring(false),
	  invalidModulator(nullptr),
	  nullPanel(nullptr),
	  ctrlrFontManager(nullptr) {
	ctrlrManagerVst.reset(new CtrlrManagerVst(*this));

	commandManager.addListener(this);
	audioFormatManager.registerBasicFormats();

	ctrlrDocumentPanel.reset(new CtrlrDocumentPanel(*this));
	nullPanel = new CtrlrPanel(*this);
	nullModulator = new CtrlrModulator(*nullPanel);
	ctrlrFontManager.reset(new CtrlrFontManager(*this));
}

CtrlrManager::~CtrlrManager() {
	shuttingDown = true; // 1. Freeze tree updates instantly
	DBG("!!! DTOR for CtrlrManager");

	commandManager.removeListener(this);
	managerTree.removeListener(this);

	// 2. Clear out the global VST modulator cache allocations natively
	// by passing the active editors to the engine's built-in teardown method!
	for (int i = ctrlrPanels.size() - 1; i >= 0; --i) {
		if (auto *panel = ctrlrPanels[i]) {
			// Use the panel's active editor wrapper natively if it exists
			if (auto *editor = panel->getPanelEditor()) {
				removePanel(editor);
			} else {
				// Fallback cache strip if no editor view context is active
				for (int j = 0; j < panel->getModulators().size(); ++j) {
					ctrlrModulators.removeAllInstancesOf(panel->getModulators()[j]);
					if (ctrlrManagerVst != nullptr)
						ctrlrManagerVst->remove(panel->getModulators()[j]);
				}
			}
		}
	}

	// 3. Detach UI visibility cleanly so child frames unbind layout rules
	if (ctrlrDocumentPanel != nullptr) {
		ctrlrDocumentPanel->setVisible(false);
	}

	// 4. Clear the backend data models completely
	ctrlrPanels.clear();
	managerTree.removeAllChildren(0);

	// 5. Clean up dummy elements
	if (nullPanel)
		nullPanel->setLookAndFeel(nullptr);
	if (nullModulator)
		deleteAndZero(nullModulator);
	if (nullPanel)
		deleteAndZero(nullPanel);
}

//~~

void CtrlrManager::setManagerReady() {
	managerTree.addListener(this);
	managerTree.addChild(ctrlrMidiDeviceManager.getManagerTree(), -1, 0);
	managerTree.addChild(ctrlrWindowManager.getManagerTree(), -1, 0);

	if (ctrlrEditor) {
		ctrlrEditor->activeCtrlrChanged();
	}
}

void CtrlrManager::setDefaults() {
	if (ctrlrProperties != nullptr)
		delete (ctrlrProperties.release());

	ctrlrProperties.reset(new CtrlrProperties(*this));

	setProperty(Ids::ctrlrLogToFile, false);
	setProperty(Ids::ctrlrLuaDebug, false);
	setProperty(Ids::ctrlrVersionSeparator, "_");
	setProperty(Ids::ctrlrVersionCompressed, false);
	setProperty(Ids::ctrlrMidiMonInputBufferSize, 8192);
	setProperty(Ids::ctrlrMidiMonOutputBufferSize, 8192);
	setProperty(Ids::ctrlrLuaDisabled, false);
	setProperty(Ids::ctrlrOverwriteResources, true);
	if (JUCEApplication::isStandaloneApp()) // Added v5.6.35
	{
		setProperty(Ids::ctrlrAutoSave, true);
		setProperty(Ids::ctrlrAutoSaveInterval, 300);
	}
	// setProperty (Ids::ctrlrLogOptions, 32); // Updated v5.6.31. Value sets default properties as enabled
	setProperty(Ids::ctrlrLogOptions, 6014); // 6014 shows everything by default with MIDI messages in Hex
	// setProperty (Ids::ctrlrUseEditorWrapper, true); // Removed v5.6.34. Conditions hard coded for the wrapper with
	// Ableton Live on Windows
	setProperty(Ids::ctrlrPropertiesAreURLs, true);
	setProperty(Ids::ctrlrNativeAlerts, false);

	// Added v5.6.31. Prevents the native FileChooser hanging when calling for an instance export by requesting the
	// fallback ugly embedded JUCE FileChooser
	setProperty(Ids::ctrlrNativeFileDialogs, true);
	auto typeOS = juce::SystemStats::getOperatingSystemType();
	std::cout << "typeOS: " << typeOS << std::endl;
	if (typeOS == juce::SystemStats::OperatingSystemType::MacOSX_10_15 // For OSX Catalina
		|| typeOS == juce::SystemStats::OperatingSystemType::MacOS_11  //  For macOS BigSur
		|| typeOS == juce::SystemStats::OperatingSystemType::MacOS_12  //  For macOS Monterey
	) {
		setProperty(Ids::ctrlrNativeFileDialogs, false);
	} else {
		setProperty(Ids::ctrlrNativeFileDialogs, true);
	}

	// setProperty (Ids::ctrlrPrivateKey, ""); // Removed v5.6.31. Encryption key is implemented at core level in Native
	// CtrlrMac & CtrlrWindows
	setProperty(Ids::ctrlrFontSizeBaseValue, 14.0f);
	setProperty(Ids::ctrlrScrollbarThickness, 16.0f); // Was 18.0f
	// setProperty (Ids::ctrlrLookAndFeel, "V4"); // Removed v5.6.30
	// setProperty (Ids::ctrlrColourScheme, "Light"); // Removed v5.6.30
	setProperty(Ids::ctrlrMenuBarHeight, 24);
	setProperty(Ids::ctrlrTabBarDepth, 24);
	setProperty(Ids::ctrlrPropertyLineheightBaseValue, 36);		  // Added v5.6.33.
	setProperty(Ids::ctrlrPropertyLineImprovedLegibility, false); // Added v5.6.34.
	setProperty(Ids::uiLuaConsoleInputRemoveAfterRun, true);
	setProperty(Ids::luaCtrlrSaveState, COMBO_ITEM_NONE);
	setProperty(Ids::luaCtrlrRestoreState, COMBO_ITEM_NONE);
}

CtrlrManagerVst &CtrlrManager::getVstManager() { return (*ctrlrManagerVst); }

void CtrlrManager::addModulator(CtrlrModulator *modulatorToAdd) {
	_DBG("CtrlrManager::addModulator [PRE] vstIndex==" + modulatorToAdd->getProperty(Ids::vstIndex).toString());
	ctrlrManagerVst->set(modulatorToAdd);
	ctrlrModulators.addIfNotAlreadyThere(modulatorToAdd);
	// _DBG("CtrlrManager::addModulator [POST] vstIndex=="+modulatorToAdd->getProperty(Ids::vstIndex).toString());
}

void CtrlrManager::removeModulator(CtrlrModulator *modulatorToDelete) {
	ctrlrManagerVst->remove(modulatorToDelete);
	ctrlrModulators.removeAllInstancesOf(modulatorToDelete);
}

bool CtrlrManager::containsCtrlrComponent(CtrlrComponent *componentToLookFor) {
	if (componentToLookFor == 0)
		return (false);

	return (ctrlrModulators.contains(&componentToLookFor->getOwner()));
}

CtrlrModulator *CtrlrManager::getModulator(const String &name) const {
	for (int i = 0; i < ctrlrModulators.size(); ++i) {
		if (ctrlrModulators[i]->getName() == name) {
			return (ctrlrModulators[i]);
		}
	}

	return (0);
}

const Array<CtrlrModulator *> CtrlrManager::getModulatorsByMidiType(const CtrlrMidiMessageType typeToFilter) {
	Array<CtrlrModulator *> ret;
	for (int i = 0; i < ctrlrModulators.size(); i++) {
		if (ctrlrModulators[i]->getMidiMessage().getMidiMessageType() == typeToFilter) {
			ret.add(ctrlrModulators[i]);
		}
	}

	return (ret);
}

const Array<CtrlrModulator *> CtrlrManager::getModulatorsByUIType(const Identifier &typeToFilter) {
	Array<CtrlrModulator *> ret;
	for (int i = 0; i < ctrlrModulators.size(); i++) {
		if (ctrlrModulators[i]->getComponentType() == typeToFilter) {
			ret.add(ctrlrModulators[i]);
		}
	}

	return (ret);
}

void CtrlrManager::allPanelsInitialized() {
	listeners.call(&CtrlrManager::Listener::managerStateChanged, PanelsLoaded);

	for (int i = 0; i < ctrlrModulators.size(); i++) {
		ctrlrModulators[i]->allModulatorsInitialized();
	}
}

CtrlrPanel *CtrlrManager::addPanel(const ValueTree &savedState, const bool showUI) {
    CtrlrPanel *panel = new CtrlrPanel(*this, getUniquePanelName("Ctrlr Panel"), ctrlrPanels.size());

    ctrlrPanels.add(panel);

    // Deep copy state restore
    panel->restoreState(savedState.createCopy());

    managerTree.addChild(panel->getPanelTree(), -1, 0);

    if (showUI) {
        if (auto* editor = panel->getEditor(true)) {
            // Store the Session ID directly on the Editor component
            // editor->getProperties().set("panelSessionId", panel->getSessionId().toString());
			// DBG("!!! set panelId to " <<  panel->getSessionId().toString());
			// moved to CtrlrPanelEditor::CtrlrPanelEditor
            addPanel(editor);
        }
    }

    organizePanels();

    return (panel);
}

void CtrlrManager::addPanel(CtrlrPanelEditor *panelToAdd) {
	// This override handles visual UI window docking, no change needed here
	ctrlrDocumentPanel->addDocument((Component *)panelToAdd, Colours::lightgrey, true);
}

Result CtrlrManager::addInstancePanel() {
	if (ctrlrPlayerInstanceTree.isValid()) {
		CtrlrPanel *panel = new CtrlrPanel(*this, getInstanceName(), ctrlrPanels.size());
		ctrlrPanels.add(panel);

		// ADDED DEEP COPY HERE: Breaks the shared memory link for standalone player instances
		Result restoreResult = panel->restoreState(ctrlrPlayerInstanceTree.createCopy());

		if (!restoreResult.wasOk()) {
			WARN("AddInstancePanel failed to restore the state of the panel");
			ctrlrPanels.clear(true);
			return (restoreResult);
		}

		managerTree.addChild(panel->getPanelTree(), -1, 0);
		addPanel(panel->getEditor(true));

		organizePanels();

		return (Result::ok());
	} else {
		return (Result::fail("AddInstancePanel failed, the decoded instance tree is invalid"));
	}
}

void CtrlrManager::restoreState(const ValueTree &savedTree) {
	_DBG("CtrlrManager::restoreState (ValueTree) enter");

// --- Start: Conditional MessageManagerLock for Thread Safety ---
#if JucePlugin_Build_AAX
	// For AAX builds, the lock is bypassed to prevent deadlock.
	// Any UI-touching code below MUST be marshalled to the Message Thread via callAsync.
	_DBG("CtrlrManager::restoreState: MessageManagerLock skipped (AAX build).");
#else
	// This lock is often necessary for thread safety in other plugin formats (VST/AU/Standalone)
	// when setStateInformation might involve direct UI updates or MessageManager interactions.
	// It's REMOVED for AAX builds because AAX calls setStateInformation on a host thread
	// where acquiring this lock directly can cause deadlocks in newer JUCE versions (v4+).
	MessageManagerLock mmlock;
	_DBG("CtrlrManager::restoreState: MessageManagerLock acquired (non-AAX build).");
#endif
	// --- End: Conditional MessageManagerLock ---

	if (savedTree.isValid()) {
		_DBG("CtrlrManager::restoreState: savedTree is valid. Proceeding with state restoration.");

		ctrlrManagerRestoring = true;

		_DBG("CtrlrManager::restoreState: Calling restoreProperties.");
		restoreProperties(savedTree, managerTree);
		_DBG("CtrlrManager::restoreState: restoreProperties returned.");

		if (owner->getOverrides().isValid()) {
			_DBG("CtrlrManager::restoreState: Overrides found, setting properties.");
			for (int i = 0; i < owner->getOverrides().getNumProperties(); i++) {
				setProperty(owner->getOverrides().getPropertyName(i),
							owner->getOverrides().getPropertyAsValue(owner->getOverrides().getPropertyName(i), 0));
			}
			_DBG("CtrlrManager::restoreState: Overrides processed.");
		}

		_DBG("CtrlrManager::restoreState: Clearing and re-adding managerTree children.");
		managerTree.removeAllChildren(0);
		managerTree.addChild(ctrlrMidiDeviceManager.getManagerTree(), -1, 0);
		managerTree.addChild(ctrlrWindowManager.getManagerTree(), -1,
							 0); // Potentially UI-touching via ctrlrWindowManager
		_DBG("CtrlrManager::restoreState: ManagerTree children updated.");

		if (savedTree.getChildWithName(Ids::midiDeviceManager).isValid()) {
			_DBG("CtrlrManager::restoreState: Restoring MIDI device manager state.");
			ctrlrMidiDeviceManager.restoreState(savedTree.getChildWithName(Ids::midiDeviceManager));
			_DBG("CtrlrManager::restoreState: MIDI device manager state restored.");
		}

		if (savedTree.getChildWithName(Ids::uiWindowManager).isValid()) {
			// IMPORTANT: If ctrlrWindowManager.restoreState() creates/manages actual UI windows or components,
			// for AAX, this might also need to be wrapped in a MessageManager::callAsync.
			_DBG("CtrlrManager::restoreState: Restoring UI Window manager state.");
			ctrlrWindowManager.restoreState(savedTree.getChildWithName(Ids::uiWindowManager));
			_DBG("CtrlrManager::restoreState: UI Window manager state restored.");
		}

		if (getInstanceMode() != InstanceMulti && savedTree.hasType(Ids::panelState)) {
			_DBG("CtrlrManager::restoreState: Instance not multi, restoring instance state.");
			restoreInstanceState(savedTree); // This might also involve UI, check its implementation
			_DBG("CtrlrManager::restoreState: restoreInstanceState returned.");
			// Ensure ctrlrManagerRestoring is set to false even on early return
			ctrlrManagerRestoring = false;
			_DBG("CtrlrManager::restoreState (ValueTree) exit - early return for single instance.");
			return;
		}

		_DBG("CtrlrManager::restoreState: Looping through savedTree children for panels.");
		for (int i = 0; i < savedTree.getNumChildren(); i++) {
			if (savedTree.getChild(i).hasType(Ids::panel)) {
				// IMPORTANT: addPanel likely creates/configures UI elements.
				// For AAX, this might also need to be wrapped in a MessageManager::callAsync.
				_DBG("CtrlrManager::restoreState: Adding panel from child " + String(i));
				addPanel(savedTree.getChild(i));
				_DBG("CtrlrManager::restoreState: Panel added.");
			}
		}

// --- CRITICAL SECTION FOR AAX UI-RELATED WORK ---
// If restoreEditorState() (or anything it calls) involves creating/modifying JUCE UI components,
// it MUST be explicitly marshalled to the JUCE Message Manager thread for AAX builds.
#if JucePlugin_Build_AAX
		_DBG("CtrlrManager::restoreState: AAX build - scheduling restoreEditorState on Message Thread.");
		juce::MessageManager::callAsync([this]() {
			// This lambda (the code inside {}) will be executed on the JUCE Message Manager thread.
			// It is safe to perform UI operations here.
			_DBG("CtrlrManager::restoreState: Executing restoreEditorState on Message Thread (AAX).");
			if (ctrlrEditor) {
				restoreEditorState();
			}
			_DBG("CtrlrManager::restoreState: restoreEditorState execution on Message Thread complete (AAX).");
		});
#else
		// For other plugin types (VST/AU/Standalone), it's generally safe to call directly here
		// if the original MessageManagerLock covered this scope.
		_DBG("CtrlrManager::restoreState: Non-AAX build - calling restoreEditorState directly.");
		if (ctrlrEditor) {
			restoreEditorState();
		}
#endif
		// --- END CRITICAL SECTION ---

		ctrlrManagerRestoring = false;
	} else // if savedTree is not valid
	{
		_DBG("CtrlrManager::restoreState: savedTree is NOT valid. No state to restore.");
	}

	_DBG("CtrlrManager::restoreState (ValueTree) exit");
}

void CtrlrManager::restoreState(const XmlElement &savedState) { restoreState(ValueTree::fromXml(savedState)); }

const String CtrlrManager::getUniquePanelName(const String &proposedName) {
	String n = proposedName;
	uint32 i = ctrlrPanels.size();

	while (getPanel(n)) {
		n = proposedName + String(++i);
	}
	return (n);
}

CtrlrPanel *CtrlrManager::getPanel(const String &panelName) {
	for (int i = 0; i < ctrlrPanels.size(); i++) {
		CtrlrPanel *ctrlrPanel = ctrlrPanels[i];
		if (ctrlrPanel != 0) {
			if (ctrlrPanel->getProperty(Ids::name) == panelName) {
				return (ctrlrPanel);
			}
		}
	}
	return (0);
}
#if JUCE_VERSION >= 0x070000
void CtrlrManager::canCloseWindow(std::function<void(bool)> completionCallback) {
	// Helper lambda to process panels sequentially
	auto checkNextPanel = [this, completionCallback](auto self, int index) -> void {
		// Base case: tested all panels successfully
		if (index >= getNumPanels()) {
			if (completionCallback)
				completionCallback(true);
			return;
		}

		CtrlrPanel *panel = getPanel(index);
		if (panel != nullptr) {
			// Check current panel asynchronously
			panel->canClose(false, [self, index, completionCallback](bool canCloseThisOne) {
				if (!canCloseThisOne) {
					// User canceled or panel cannot close -> abort the whole close process
					if (completionCallback)
						completionCallback(false);
					return;
				}

				// Current panel passed, check the next panel in line
				self(self, index + 1);
			});
		} else {
			// Skip invalid panel pointer and move to next
			self(self, index + 1);
		}
	};

	// Start checking from the first panel (index 0)
	checkNextPanel(checkNextPanel, 0);
}

#else
bool CtrlrManager::canCloseWindow() {
	for (int i = 0; i < getNumPanels(); i++) {
		CtrlrPanel *panel = getPanel(i);
		if (panel != nullptr) {
			if (!panel->canClose(false)) {
				return false;
			}
		}
	}
	return true;
}
#endif

void CtrlrManager::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) {
	if (property == Ids::ctrlrAutoSave) {
		if ((bool)getProperty(property) == true) {
			startTimer(TIMER_AUTO_SAVE, (int)getProperty(Ids::ctrlrAutoSaveInterval) * 1000);
		} else {
			stopTimer(TIMER_AUTO_SAVE);
		}
	} else if (property == Ids::ctrlrAutoSaveInterval) {
		stopTimer(TIMER_AUTO_SAVE);

		if ((bool)getProperty(Ids::ctrlrAutoSave) == true) {
			startTimer(TIMER_AUTO_SAVE, (int)getProperty(Ids::ctrlrAutoSaveInterval) * 1000);
		}
	} else if (property == Ids::ctrlrLogToFile) {
		ctrlrLog.setLogToFile((bool)getProperty(property));
	} else if (property == Ids::ctrlrLuaDebug) {
		for (int i = 0; i < ctrlrPanels.size(); i++) {
			ctrlrPanels[i]->setLuaDebug((bool)getProperty(property));
		}
	} else if (property == Ids::ctrlrLogOptions) {
		ctrlrLog.setMidiLogOptions(getProperty(property));
	} else if (property == Ids::uiPanelLookAndFeel) // Or whichever ID tracks your global look and feel style sheet
	{
		if (auto *editor = getEditor()) {
			// Call the public bridge method to safely swap the tooltip engine tracking state
			editor->recreateTooltipEngine();

			// Force the rest of the layout tree to cascade update
			editor->lookAndFeelChanged();
			editor->repaint();
		}
	} else if (property == Ids::ctrlrNativeAlerts) {
		if (getEditor())
			getEditor()->getLookAndFeel().setUsingNativeAlertWindows((bool)getProperty(property));
	}
}

bool CtrlrManager::isValidComponentName(const String &name) {
	if (name.length() < 3 || name.length() > 256)
		return (false);

	return (true);
}

CtrlrPanel *CtrlrManager::getActivePanel() {
	// 1. Guard against null or missing document panel
	if (ctrlrDocumentPanel == nullptr)
		return nullptr;

	// 2. Ensure documents actually exist before querying active document
	if (ctrlrDocumentPanel->getNumDocuments() <= 0)
		return nullptr;

	// 3. Safely cast active document
	if (auto *activeDoc = ctrlrDocumentPanel->getActiveDocument()) {
		if (auto *ed = dynamic_cast<CtrlrPanelEditor *>(activeDoc)) {
			return &(ed->getOwner());
		}
	}

	return nullptr;
}
void CtrlrManager::removePanel(CtrlrPanelEditor *editor) {
	if (editor == nullptr)
		return;

	// Guard: ensure the editor component is still valid
	juce::Component::SafePointer<CtrlrPanelEditor> safeEditor(editor);
	if (safeEditor == nullptr)
		return;

	CtrlrPanel *panel = &editor->getOwner();

	// 1. Unregister modulators from manager arrays safely
	if (panel != nullptr) {
		for (int i = 0; i < panel->getModulators().size(); i++) {
			ctrlrModulators.removeAllInstancesOf(panel->getModulators()[i]);
			if (ctrlrManagerVst != nullptr) {
				ctrlrManagerVst->remove(panel->getModulators()[i]);
			}
		}
	}

	// 2. Remove document tab safely
	if (ctrlrDocumentPanel != nullptr && safeEditor != nullptr) {
		// DO NOT call setActiveDocument(nullptr)!
		// Check if the document panel still contains this component
		bool containsDoc = false;
		for (int i = 0; i < ctrlrDocumentPanel->getNumDocuments(); ++i) {
			if (ctrlrDocumentPanel->getDocument(i) == safeEditor.getComponent()) {
				containsDoc = true;
				break;
			}
		}

		if (containsDoc) {
			// JUCE will automatically select a remaining active document
			// and delete/detach safeEditor asynchronously.
			ctrlrDocumentPanel->closeDocumentAsync(safeEditor.getComponent(), false, nullptr);
		}
	}

	// 3. Remove panel object from owned array
	if (panel != nullptr) {
		// Pass false soJUCE MultiDocumentPanel's automatic deletion
		// doesn't cause a double-free on the panel/editor.
		ctrlrPanels.removeObject(panel, false);
	}

	// 4. Re-organize layout
	organizePanels();
}
void CtrlrManager::restoreEditorState() {
	if (getProperty(Ids::ctrlrEditorBounds).toString() == "") {
		if (getInstanceMode() == InstanceSingle || getInstanceMode() == InstanceSingleRestricted) {
			Rectangle<int> r(32, 32, 800, 600);

			if (getActivePanel() && getActivePanel()->getEditor()) {
				r = VAR2RECT(getActivePanel()->getEditor()->getProperty(Ids::uiPanelCanvasRectangle));
			}

			ctrlrEditor->setSize(r.getWidth(), r.getHeight());
			return;
		}
	}
	// ctrlrEditor->centreWithSize(800, 600);
}

void CtrlrManager::setEditor(CtrlrEditor *editorToSet) {
	ctrlrEditor = editorToSet;

	restoreEditorState();
}

int CtrlrManager::getModulatorVstIndexByName(const String &modulatorName, CtrlrPanel *sourcePanel) {
	// 1. If no panel context was passed, fall back to whatever panel is active
	if (sourcePanel == nullptr) {
		sourcePanel = getActivePanel();
	}

	// 2. Ask the specific panel to find the modulator inside itself (NOT globally via 'this')
	if (sourcePanel != nullptr) {
		// We call getModulator on the sourcePanel object, passing the 'true' boolean
		// to satisfy the CtrlrPanel function signature we just verified in CtrlrPanel.h!
		CtrlrModulator *m = sourcePanel->getModulator(modulatorName, true);
		if (m) {
			return (m->getVstIndex());
		}
	}

	return (-1);
}

int CtrlrManager::compareElements(CtrlrModulator *first, CtrlrModulator *second) {
	if (first->getVstIndex() < second->getVstIndex())
		return (-1);
	else if (first->getVstIndex() == second->getVstIndex())
		return (0);
	else if (first->getVstIndex() > second->getVstIndex())
		return (1);
	else
		return (0);
}

int CtrlrManager::compareElements(CtrlrPanel *first, CtrlrPanel *second) {
	if (first->getPanelIndex() < second->getPanelIndex())
		return (-1);
	else if (first->getPanelIndex() == second->getPanelIndex())
		return (0);
	else if (first->getPanelIndex() > second->getPanelIndex())
		return (1);
	else
		return (0);
}

void CtrlrManager::organizePanels() {
	ctrlrPanels.sort(*this);
	for (int i = 0; i < ctrlrPanels.size(); i++) {
		ctrlrPanels[i]->setProperty(Ids::panelIndex, i);
	}
}

void CtrlrManager::handleAsyncUpdate() {}

int CtrlrManager::getPanelForModulator(const int modulatorIndex) {
	if (ctrlrModulators[modulatorIndex]) {
		return (ctrlrModulators[modulatorIndex]->getOwnerPanel().getPanelIndex());
	}
	return (-1);
}

int CtrlrManager::getNextVstIndex() { return (ctrlrManagerVst->getFirstFree()); }

void CtrlrManager::openPanelFromFile(Component *componentToAttachMenu) {
#if JUCE_VERSION >= 0x070000
    // 1. Set the specific file types Ctrlr expects, matching your legacy code
    String wildcards = "*.panel;*.panelz;*.bpanel;*.bpanelz;*.*";
    bool useNativeDialog = (bool)getProperty(Ids::ctrlrNativeFileDialogs);
    
    // Make sure fileChooser is declared as a std::unique_ptr<juce::FileChooser> 
    // either in your CtrlrManager class header file, or as a static/tracked instance.
    fileChooser = std::make_unique<FileChooser>(
        "Open panel", 
        File(getProperty(Ids::ctrlrLastBrowsedFileDirectory)),
        wildcards, 
        useNativeDialog
    );

    // 2. Launch the dialog box asynchronously
    fileChooser->launchAsync(
        FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
        [this](const FileChooser &chooser) {
            File result = chooser.getResult();

            // 3. Process the file using the original Ctrlr engine methods
            if (result.existsAsFile()) {
                openPanelInternal(result);
                panelFileOpened(result);
            }
        }
    );
#else
    // --- Legacy JUCE 6 Synchronous Execution ---
    FileChooser fc("Open panel", File(getProperty(Ids::ctrlrLastBrowsedFileDirectory)),
                   "*.panel;*.panelz;*.bpanel;*.bpanelz;*.*", (bool)getProperty(Ids::ctrlrNativeFileDialogs));
    
    if (fc.browseForFileToOpen()) {
        openPanelInternal(fc.getResult());
        panelFileOpened(fc.getResult());
    }
#endif
}

void CtrlrManager::panelFileOpened(const File &panelFile) {
	StringArray recentFiles;
	recentFiles.addTokens(getProperty(Ids::ctrlrRecenetOpenedPanelFiles).toString(), ";", "\"'");
	recentFiles.insert(0, panelFile.getFullPathName());
	recentFiles.removeRange(9, recentFiles.size() - 10);
	setProperty(Ids::ctrlrRecenetOpenedPanelFiles, recentFiles.joinIntoString(";"));

	setProperty(Ids::ctrlrLastBrowsedFileDirectory, panelFile.getFullPathName());
}

void CtrlrManager::openPanelInternal(const File &fileToOpen) {
	setProperty(Ids::panelLastSaveDir, fileToOpen.getParentDirectory().getFullPathName());

	ValueTree panelTree;

	if (fileToOpen.getFileExtension().startsWith(".b")) {
		panelTree = CtrlrPanel::openBinPanel(fileToOpen);
	} else {
		panelTree = CtrlrPanel::openXmlPanel(fileToOpen);
	}
	if (panelTree.isValid()) {
		// Patch panelFilePath property to match the actual file
		panelTree.setProperty(Ids::panelFilePath, fileToOpen.getFullPathName(), nullptr);
		addPanel(panelTree, true);
	}
}

void CtrlrManager::openPanelInternal(const ValueTree &panelTree) {
	if (panelTree.isValid()) {
		addPanel(panelTree, true);
	}
}

void CtrlrManager::changeListenerCallback(ChangeBroadcaster *source) {}

CtrlrPanel *CtrlrManager::getPanelByUid(const String &uid) {
	for (int i = 0; i < ctrlrPanels.size(); i++) {
		if (ctrlrPanels[i]->getProperty(Ids::panelUID) == uid) {
			return (ctrlrPanels[i]);
		}
	}

	return (0);
}

CtrlrPanel *CtrlrManager::getPanel(const int panelIndex) {
	if (panelIndex < ctrlrPanels.size()) {
		return (ctrlrPanels[panelIndex]);
	}
	return (0);
}

CtrlrPanel *
CtrlrManager::getPanelForEditor(CtrlrPanelEditor *editorToFind) // Added v5.6.34. To assign the right panel index from
																// the tab index. Panel tab close button.
{
	// Iterate through the 'ctrlrPanels' array
	for (int i = 0; i < ctrlrPanels.size(); ++i) {
		CtrlrPanel *panel = ctrlrPanels.getUnchecked(i); // Access the panel from the internal array

		// Check if this panel exists and its editor matches the one we're looking for
		// CtrlrPanel::getEditor(false) returns the editor for that panel
		if (panel != nullptr && panel->getEditor(false) == editorToFind) {
			return panel; // Found the correct CtrlrPanel for the given editor
		}
	}
	return nullptr; // No matching CtrlrPanel found for the given editor
}

int CtrlrManager::getNumPanels() { return (ctrlrPanels.size()); }

CtrlrModulator *CtrlrManager::getModulatorByVstIndex(const int index) {
	if (ctrlrManagerVst)
		return (ctrlrManagerVst->get(index));
	else
		return (nullptr);
}

int CtrlrManager::getNumModulators(const bool onlyVstParameters) {
	if (onlyVstParameters) {
		if (ctrlrManagerVst.get())
			return (ctrlrManagerVst->getLargestIndex() + 1);
		else
			return (CTRLR_DEFAULT_PARAMETER_COUNT);
	} else {
		return (ctrlrModulators.size() + 1);
	}
}

void CtrlrManager::saveStateToDisk() {
	if (JUCEApplication::isStandaloneApp()) {
		sendActionMessage("save");
	}
}

void CtrlrManager::timerCallback(int timerId) {
	if (timerId == TIMER_AUTO_SAVE) {
		saveStateToDisk();
	}
}

const File CtrlrManager::getCtrlrPropertiesDirectory() {
	return (getCtrlrProperties().getProperties().getUserSettings()->getFile().getParentDirectory());
}

CtrlrPanel* CtrlrManager::getPanelBySessionId(const String& sessionId)
{
    for (auto* panel : ctrlrPanels)
    {
        if (panel != nullptr && panel->getSessionId().toString() == sessionId)
        {
            return panel;
        }
    }
    return nullptr;
}

CtrlrProperties &CtrlrManager::getCtrlrProperties() { return (*ctrlrProperties); }

ApplicationProperties *CtrlrManager::getApplicationProperties() {
	if (ctrlrProperties) {
		return (&ctrlrProperties->getProperties());
	} else {
		return (nullptr);
	}
}

void CtrlrManager::applicationCommandInvoked(const ApplicationCommandTarget::InvocationInfo &info) {}

void CtrlrManager::applicationCommandListChanged() {}
