#include "CtrlrStandaloneWindow.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrProcessor.h"
#include "stdafx.h"

extern AudioProcessor *JUCE_CALLTYPE createPluginFilter();

CtrlrStandaloneWindow::CtrlrStandaloneWindow(const String &title, const Colour &backgroundColour)
	: DocumentWindow(title, backgroundColour, DocumentWindow::allButtons, true),
	  ctrlrProcessor(nullptr),
	  filter(nullptr),
	  appProperties(nullptr),
	  restoreState(true) {
	filter = createPluginFilter();
	setTitleBarButtonsRequired(DocumentWindow::allButtons, false);
	setUsingNativeTitleBar(true);
	setResizable(true, true); // default. Set to false, false to lock, hide the corner resizer
	centreWithSize(800, 600); // set Size of the whole app with title bar included H22px and borders 2x1px

	if (filter != 0) {
		ctrlrProcessor = dynamic_cast<CtrlrProcessor *>(filter);

		if (ctrlrProcessor == nullptr) {
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "CTRLR",
											 "The filter object is not a valid Ctrlr Processor");
			return;
		}

		/* set some default audio stuff so the filter works without the audio card */
		// ctrlrProcessor->setPlayConfigDetails (0, 0, SAMPLERATE, 512);
		ctrlrProcessor->setRateAndBufferSizeDetails(SAMPLERATE, 512);
		addKeyListener(ctrlrProcessor->getManager().getCommandManager().getKeyMappings());

		/* we want to listen too manager actions */
		ctrlrProcessor->getManager().addActionListener(this);

		// We want to be notified by CtrlrProcessor when the active panel changes to update the
		// title bar
		ctrlrProcessor->addChangeListener(this);

		/* get the properties pointer from the manager */
		appProperties = ctrlrProcessor->getManager().getApplicationProperties();

		if (appProperties != nullptr) {
			_DBG("appProperties != nullptr");
			auto xml = appProperties->getUserSettings()->getXmlValue(CTRLR_PROPERTIES_FILTER_STATE);

			if (xml != nullptr) {
				_DBG("xml != nullptr");
				ctrlrProcessor->setStateInformation(xml.get());
			}

			AudioProcessorEditor *editor = ctrlrProcessor->createEditorIfNeeded();
			setName(ctrlrProcessor->getManager().getInstanceName());

			if (appProperties->getUserSettings()->getValue(CTRLR_PROPERTIES_WINDOW_STATE, "") != "") {
				_DBG("CTRLR_PROPERTIES_WINDOW_STATE != null");
				restoreWindowStateFromString(appProperties->getUserSettings()->getValue(CTRLR_PROPERTIES_WINDOW_STATE));
			} else {
				_DBG("CTRLR_PROPERTIES_WINDOW_STATE == null");
				if (ctrlrProcessor->getManager().getInstanceTree().getChildWithName(Ids::uiPanelEditor).isValid()) {
					_DBG("uiPanelEditor isValid");
					ValueTree ed = ctrlrProcessor->getManager().getInstanceTree().getChildWithName(Ids::uiPanelEditor);
					Rectangle<int> r = VAR2RECT(ed.getProperty(Ids::uiPanelCanvasRectangle, "0 0 800 600"));
					int menuBarHeight = (int)ctrlrProcessor->getManager().getProperty(Ids::ctrlrMenuBarHeight);
					if (menuBarHeight <= 0)
						menuBarHeight = 24;

					bool menuBarVisible = ed.getProperty(Ids::uiPanelMenuBarVisible,
														 true); // default to true if missing
					centreWithSize(r.getWidth() <= 0 ? 800 : r.getWidth(),
								   (r.getHeight() <= 0 ? 600 : r.getHeight()) + (menuBarVisible ? menuBarHeight : 0));
					// centreWithSize (r.getWidth(), r.getHeight() +
					// ((bool)ed.getProperty(Ids::uiPanelMenuBarVisible) ?
					// (int)ctrlrProcessor->getManager().getProperty(Ids::ctrlrMenuBarHeight) : 0));
				}
			}

			setContentOwned(editor, false);
		} else {
			_DBG("No appProperties");

			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "CTRLR",
											 "Can't find any application properties");
		}
	}

	ValueTree ed = ctrlrProcessor->getManager().getInstanceTree().getChildWithName(Ids::uiPanelEditor);
	Rectangle<int> r = VAR2RECT(ed.getProperty(Ids::uiPanelCanvasRectangle));
	panelCanvasWidth = r.getWidth();
	panelCanvasHeight = r.getHeight();

	vpResizable = ed.getProperty(Ids::uiViewPortResizable, true);
	vpEnableFixedAspectRatio = ed.getProperty(Ids::uiViewPortEnableFixedAspectRatio, false);

	vpOsFrameTop = getPeer()->getFrameSize().getTop();	   // OS Native Title Bar Height
	vpOsFrameBtm = getPeer()->getFrameSize().getBottom();  // OS Native Window Border Bottom thickness
	vpOsFrameLeft = getPeer()->getFrameSize().getLeft();   // OS Native Window Border Left thickness
	vpOsFrameRight = getPeer()->getFrameSize().getRight(); // OS Native Window Border Right thickness
	vpOsWindowWidth = getPeer()->getBounds().getWidth();   // OS Native Window Height incl borders
	vpOsWindowHeight = getPeer()->getBounds().getHeight(); // OS Native Window Height incl borders
	// Native Window size is stored in the OS.
	// If the APP has value off the target, it's because the same UID was used previously with
	// different sizes & ratio OR width and Height require to be set from inner content + Borders

	// vpStandaloneAspectRatio = double(vpOsWindowWidth) / double(vpOsWindowHeight); // Requires
	// Native Title bar Height and borders
	vpStandaloneAspectRatio = double(panelCanvasWidth + vpOsFrameLeft + vpOsFrameRight) /
							  double(panelCanvasHeight + vpOsFrameTop + vpOsFrameBtm);

	vpEnableResizableLimits = ed.getProperty(Ids::uiViewPortEnableResizeLimits);
	vpMinWidth = ed.getProperty(Ids::uiViewPortMinWidth);
	vpMinHeight = ed.getProperty(Ids::uiViewPortMinHeight);
	vpMaxWidth = ed.getProperty(Ids::uiViewPortMaxWidth);
	vpMaxHeight = ed.getProperty(Ids::uiViewPortMaxHeight);

	if (ctrlrProcessor->getManager().getInstanceMode() ==
		InstanceSingleRestricted) // restricted instances check flag to be resizable
	{
		_DBG("Restricted Instance Mode");
		setResizable(vpResizable, true);

		if (auto *constrainer = getConstrainer()) {
			if (vpEnableFixedAspectRatio == true) {
				constrainer->setFixedAspectRatio(vpStandaloneAspectRatio); // set window aspect ratio

				if (vpEnableResizableLimits == true) {
					if (vpMinWidth != 0 && vpMaxWidth != 0) {
						setResizeLimits(vpMinWidth, round(vpMinWidth / vpStandaloneAspectRatio), vpMaxWidth,
										round(vpMaxWidth / vpStandaloneAspectRatio));
					} else if (vpMinWidth != 0 && vpMinHeight != 0 && vpMaxWidth != 0 && vpMaxHeight != 0) {
						setResizeLimits(
							vpMinWidth + vpOsFrameLeft + vpOsFrameRight, vpMinHeight + vpOsFrameTop + vpOsFrameBtm,
							vpMaxWidth + vpOsFrameLeft + vpOsFrameRight, vpMaxHeight + vpOsFrameTop + vpOsFrameBtm);
					} else {
						constrainer->setMinimumSize(panelCanvasWidth, panelCanvasHeight + vpOsFrameTop + vpOsFrameBtm);
					}
				}
			} else if (vpEnableResizableLimits == true && vpMinWidth != 0 && vpMinHeight != 0 && vpMaxWidth != 0 &&
					   vpMaxHeight != 0) {
				setResizeLimits(vpMinWidth + vpOsFrameLeft + vpOsFrameRight, vpMinHeight + vpOsFrameTop + vpOsFrameBtm,
								vpMaxWidth + vpOsFrameLeft + vpOsFrameRight, vpMaxHeight + vpOsFrameTop + vpOsFrameBtm);
			}
		}
	} else {
		setResizable(true, true);
	}

	restoreState = false;
	setVisible(true);
}

CtrlrStandaloneWindow::~CtrlrStandaloneWindow()
{
	DBG("(A) CtrlrDocumentPanel~CtrlrStandaloneWindow: Destructor called");
	ctrlrProcessor->removeChangeListener(this);
    ctrlrProcessor->getManager().removeActionListener (this);
    saveStateNow();
    deleteFilter();
}



// CtrlrStandaloneWindow::~CtrlrStandaloneWindow() {
// 	// 1. Only clean up via the processor if it actually still exists!
// 	DBG("(A) CtrlrDocumentPanel~CtrlrStandaloneWindow: Destructor called");
//     if (auto* docPanel = dynamic_cast<CtrlrDocumentPanel*>(getContentComponent()))
//     {
//         docPanel->closeAllPanelsAndDetach();
//     }
// 	if (ctrlrProcessor != nullptr) {
// 		ctrlrProcessor->removeChangeListener(this);

// 		// Use a defensive check for the manager as well
// 		try {
// 			ctrlrProcessor->getManager().removeActionListener(this);
// 		} catch (...) {
// 			_DBG("~CtrlrStandaloneWindow: Failed to remove action listener (Manager already dead)");
// 		}

// 		// 2. Save state while the processor is still guaranteed alive
// 		// saveStateNow();
// 		/*let ~CtrlrStandaloneWindow() handle it exclusively (since ~CtrlrStandaloneWindow()
// 		 * already checks if
// 		 * ctrlrProcessor is alive and deletes the filter afterwards):*/
// 		// 3. Delete the processor owned by this window
// 		deleteFilter();
// 	}
// }

void CtrlrStandaloneWindow::actionListenerCallback(const String &message) {
	if (message == "save") {
		saveStateNow();
	}
}

void CtrlrStandaloneWindow::changeListenerCallback(ChangeBroadcaster *source) { // Check for window title modification
	CtrlrPanel *panel = ctrlrProcessor->getManager().getActivePanel();
	String windowTitle = ctrlrProcessor->getManager().getInstanceName();
	if (panel && !ctrlrProcessor->getManager().isSingleInstance()) {
		windowTitle += " - " + panel->getPanelWindowTitle();
	}
	setName(windowTitle);
}

void CtrlrStandaloneWindow::saveStateNow() {
    _DBG("CtrlrStandaloneWindow::saveStateNow");
	if (auto *manager = getManager()) {
		// If the manager is already in the middle of running its destructor,
		// instantly break out so we don't spin up phantom UI updates or leaks!
		if (manager->isShuttingDown())
			return;
	}
	    // if (ctrlrProcessor != nullptr && ctrlrProcessor->getManager().isShuttingDown())
        // return;
	// REMOVED: manager->isShuttingDown() guard block!
	// We WANT to save state specifically when shutting down.

	if (ctrlrProcessor != nullptr && appProperties != nullptr) {
		appProperties->getUserSettings()->setValue(CTRLR_PROPERTIES_WINDOW_STATE, getWindowStateAsString());

		MemoryBlock data;
		ctrlrProcessor->getStateInformation(data);

		if (data.getSize() > 0) {
			std::unique_ptr<XmlElement> xml(CtrlrProcessor::getXmlFromBinary(data.getData(), (int)data.getSize()));

			if (xml) {
				appProperties->getUserSettings()->setValue(CTRLR_PROPERTIES_FILTER_STATE, xml.get());
			}
		}

		// --- CRITICAL JUCE 8 FIX: Force flush to disk! ---
		appProperties->getUserSettings()->saveIfNeeded();
	}
}
void CtrlrStandaloneWindow::deleteFilter() {
	if (filter != 0 && getContentComponent() != 0) {
		filter->editorBeingDeleted(dynamic_cast<AudioProcessorEditor *>(getContentComponent()));
		clearContentComponent();
	}

	deleteAndZero(filter);
}

PropertySet *CtrlrStandaloneWindow::getGlobalSettings() {
	return ctrlrProcessor->getManager().getCtrlrProperties().getProperties().getUserSettings();
}

void CtrlrStandaloneWindow::closeButtonPressed() {
	if (ctrlrProcessor == nullptr) {
		JUCEApplication::quit();
		return;
	}

	// Pass a callback function that runs after the window close check finishes
	ctrlrProcessor->getManager().canCloseWindow([this](bool canClose) {
		if (canClose) {
			JUCEApplication::quit();
		}
	});
}

void CtrlrStandaloneWindow::clearProcessorPointer() // Added JUCE 8
{
	ctrlrProcessor = nullptr;
}

void CtrlrStandaloneWindow::resized() {
	DocumentWindow::resized();

	if (appProperties != nullptr && !restoreState) {
		appProperties->getUserSettings()->setValue(CTRLR_PROPERTIES_WINDOW_STATE, getWindowStateAsString());
	}
}

void CtrlrStandaloneWindow::moved() {
	DocumentWindow::moved();

	if (appProperties != nullptr) {
		appProperties->getUserSettings()->setValue(CTRLR_PROPERTIES_WINDOW_STATE, getWindowStateAsString());
	}
}

AudioProcessor *CtrlrStandaloneWindow::getFilter() { return (filter); }

void CtrlrStandaloneWindow::openFileFromCli(const File &file) {
	if (ctrlrProcessor) {
		ctrlrProcessor->openFileFromCli(file);
	}
}

CtrlrManager *CtrlrStandaloneWindow::getManager() {
	if (ctrlrProcessor != nullptr) {
		// Wrap the manager reference inside an address-of operator (&) to return a raw pointer
		return &(ctrlrProcessor->getManager());
	}
	return nullptr;
}

void CtrlrStandaloneWindow::closeAllPanelsEarly() {
	if (auto *manager = getManager()) {
		// Safely loop backward through the active panel count to avoid index shifting bugs
		for (int i = manager->getNumPanels() - 1; i >= 0; --i) {
			if (auto *panel = manager->getPanel(i)) {
				if (auto *editor = panel->getEditor()) {
					// If removePanel expects the editor pointer, remove the UI view context
					manager->removePanel(editor);
				}
			}
		}
	}
}
