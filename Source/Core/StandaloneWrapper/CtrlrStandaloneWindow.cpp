#include "CtrlrStandaloneWindow.h"
#include "CtrlrInlineUtilitiesGUI.h"
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

#if JUCE_LINUX
    if (auto svgXml = juce::XmlDocument::parse(juce::String::createStringFromData(
            BinaryData::ctrlrx_logo_svg,
            BinaryData::ctrlr_logo_svgSize)))
    {
        if (auto drawable = juce::Drawable::createFromSVG(*svgXml))
        {
            juce::Image iconImage(juce::Image::ARGB, 64, 64, true);
            juce::Graphics g(iconImage);
            drawable->drawWithin(g, juce::Rectangle<float>(0, 0, 64, 64), juce::RectanglePlacement::centred, 1.0f);
            if (iconImage.isValid())
                setIcon(iconImage);
        }
    }
#endif

	setResizable(true, true);
	centreWithSize(800, 600);

	if (filter == nullptr)
		return;

	ctrlrProcessor = dynamic_cast<CtrlrProcessor *>(filter);

	if (ctrlrProcessor == nullptr) {
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "CTRLR",
										 "The filter object is not a valid Ctrlr Processor");
		return;
	}

	ctrlrProcessor->setRateAndBufferSizeDetails(SAMPLERATE, 512);
	addKeyListener(ctrlrProcessor->getManager().getCommandManager().getKeyMappings());
	ctrlrProcessor->getManager().addActionListener(this);
	ctrlrProcessor->addChangeListener(this);

	appProperties = ctrlrProcessor->getManager().getApplicationProperties();

	if (appProperties == nullptr) {
		_DBG("No appProperties");
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "CTRLR", "Can't find any application properties");
		return;
	}

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

			int menuBarHeight = static_cast<int>(ctrlrProcessor->getManager().getProperty(Ids::ctrlrMenuBarHeight));
			if (menuBarHeight <= 0)
				menuBarHeight = 24;

			bool menuBarVisible = ed.getProperty(Ids::uiPanelMenuBarVisible, true);

			centreWithSize(r.getWidth() <= 0 ? 800 : r.getWidth(),
						   (r.getHeight() <= 0 ? 600 : r.getHeight()) + (menuBarVisible ? menuBarHeight : 0));
		}
	}

	setContentOwned(editor, false);

	/* Fetch Viewport Configuration & Panel Bounds */
	ValueTree ed = ctrlrProcessor->getManager().getInstanceTree().getChildWithName(Ids::uiPanelEditor);
	Rectangle<int> r = VAR2RECT(ed.getProperty(Ids::uiPanelCanvasRectangle, "0 0 800 600"));
	panelCanvasWidth = r.getWidth() <= 0 ? 800 : r.getWidth();
	panelCanvasHeight = r.getHeight() <= 0 ? 600 : r.getHeight();

	const auto vpMode = gui::viewPortModeFromString(ed.getProperty(Ids::uiViewPortMode, "Scrollable").toString());

	if (auto *peer = getPeer()) {
		vpOsFrameTop = peer->getFrameSize().getTop();
		vpOsFrameBtm = peer->getFrameSize().getBottom();
		vpOsFrameLeft = peer->getFrameSize().getLeft();
		vpOsFrameRight = peer->getFrameSize().getRight();
		vpOsWindowWidth = peer->getBounds().getWidth();
		vpOsWindowHeight = peer->getBounds().getHeight();
	}

	vpStandaloneAspectRatio = double(panelCanvasWidth + vpOsFrameLeft + vpOsFrameRight) /
							  double(panelCanvasHeight + vpOsFrameTop + vpOsFrameBtm);

	vpEnableResizableLimits = ed.getProperty(Ids::uiViewPortEnableResizeLimits, false);
	vpMinWidth = ed.getProperty(Ids::uiViewPortMinWidth, 0);
	vpMinHeight = ed.getProperty(Ids::uiViewPortMinHeight, 0);
	vpMaxWidth = ed.getProperty(Ids::uiViewPortMaxWidth, 0);
	vpMaxHeight = ed.getProperty(Ids::uiViewPortMaxHeight, 0);

	/* Configure Native Window Constraints according to ViewPortMode */
	if (ctrlrProcessor->getManager().getInstanceMode() == InstanceSingleRestricted) {
		_DBG("Restricted Instance Mode");

		if (vpMode == gui::ViewPortMode::Fixed) {
			setResizable(false, false);
		} else {
			setResizable(true, true);

if (auto *constrainer = getConstrainer()) {
    resizeGrip = std::make_unique<juce::ResizableCornerComponent>(this, constrainer);
    addAndMakeVisible(resizeGrip.get());

    if (vpMode == gui::ViewPortMode::Scaled) {
        constrainer->setFixedAspectRatio(vpStandaloneAspectRatio);

        if (vpEnableResizableLimits && vpMinWidth > 0 && vpMaxWidth > 0) {
            setResizeLimits(vpMinWidth, round(vpMinWidth / vpStandaloneAspectRatio), vpMaxWidth,
                             round(vpMaxWidth / vpStandaloneAspectRatio));
        } else if (vpEnableResizableLimits && vpMinWidth > 0 && vpMinHeight > 0 && vpMaxWidth > 0 &&
                   vpMaxHeight > 0) {
            setResizeLimits(
                vpMinWidth + vpOsFrameLeft + vpOsFrameRight, vpMinHeight + vpOsFrameTop + vpOsFrameBtm,
                vpMaxWidth + vpOsFrameLeft + vpOsFrameRight, vpMaxHeight + vpOsFrameTop + vpOsFrameBtm);
        } else {
            constrainer->setMinimumSize(panelCanvasWidth, panelCanvasHeight + vpOsFrameTop + vpOsFrameBtm);
        }
    } else { // Scrollable
        constrainer->setFixedAspectRatio(0.0);

        if (vpEnableResizableLimits && vpMinWidth > 0 && vpMinHeight > 0 && vpMaxWidth > 0 && vpMaxHeight > 0) {
            setResizeLimits(
                vpMinWidth + vpOsFrameLeft + vpOsFrameRight, vpMinHeight + vpOsFrameTop + vpOsFrameBtm,
                vpMaxWidth + vpOsFrameLeft + vpOsFrameRight, vpMaxHeight + vpOsFrameTop + vpOsFrameBtm);
        }

        // Author-chosen initial size: 0, or anything larger than the canvas, means "panel size"
        int initW = (int)ed.getProperty(Ids::uiViewPortWidth, 0);
        int initH = (int)ed.getProperty(Ids::uiViewPortHeight, 0);
        if (initW <= 0 || initW > panelCanvasWidth)  initW = panelCanvasWidth;
        if (initH <= 0 || initH > panelCanvasHeight) initH = panelCanvasHeight;

        int menuBarHeight = static_cast<int>(ctrlrProcessor->getManager().getProperty(Ids::ctrlrMenuBarHeight));
        if (menuBarHeight <= 0)
            menuBarHeight = 24;
        const bool menuBarVisible = ed.getProperty(Ids::uiPanelMenuBarVisible, true);

        centreWithSize(initW, initH + (menuBarVisible ? menuBarHeight : 0));
    }
}
		}
	}

	restoreState = false;
	setVisible(true);
}

CtrlrStandaloneWindow::~CtrlrStandaloneWindow()
{
	DBG("(A) CtrlrDocumentPanel~CtrlrStandaloneWindow: Destructor called");
	ctrlrProcessor->removeChangeListener(this);
	ctrlrProcessor->getManager().removeActionListener(this);
	saveStateNow();
    deleteFilter();
}

void CtrlrStandaloneWindow::actionListenerCallback(const String &message) {
	if (message == "save") {
		saveStateNow();
	}
}

void CtrlrStandaloneWindow::changeListenerCallback(ChangeBroadcaster *source) {
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
		if (manager->isShuttingDown())
			return;
	}

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

	ctrlrProcessor->getManager().canCloseWindow([this](bool canClose) {
		if (canClose) {
			JUCEApplication::quit();
		}
	});
}

void CtrlrStandaloneWindow::clearProcessorPointer() {
	ctrlrProcessor = nullptr;
}

void CtrlrStandaloneWindow::resized() {
	DocumentWindow::resized();
	if (resizeGrip)
		resizeGrip->setBounds(getWidth() - 16, getHeight() - 16, 16, 16);

	if (appProperties != nullptr && !restoreState) {
		appProperties->getUserSettings()->setValue(CTRLR_PROPERTIES_WINDOW_STATE, getWindowStateAsString());
	}
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
		return &(ctrlrProcessor->getManager());
	}
	return nullptr;
}

void CtrlrStandaloneWindow::closeAllPanelsEarly() {
	if (auto *manager = getManager()) {
		for (int i = manager->getNumPanels() - 1; i >= 0; --i) {
			if (auto *panel = manager->getPanel(i)) {
				if (auto *editor = panel->getEditor()) {
					manager->removePanel(editor);
				}
			}
		}
	}
}