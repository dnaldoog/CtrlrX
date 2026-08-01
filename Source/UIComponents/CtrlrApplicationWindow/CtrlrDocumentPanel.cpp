#include "CtrlrDocumentPanel.h"
#include "CtrlrEditor.h"
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrMacros.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrProcessor.h"
#include "stdafx.h"
#include <juce_gui_basics/juce_gui_basics.h>

CtrlrDocumentPanel::CtrlrDocumentPanel(CtrlrManager &_owner) : ctrlrEditor(0), owner(_owner) {
	/* Full screen mode is not completely implemented in JUCE 6
	   we get some assertions when adding the first Tab to an invisble
	   panel, as it wants to grep the focus, which is not possible at this stage.
	*/
	// useFullscreenWhenOneDocument (true);
	setSize(600, 400);
	setBackgroundColour(
		(Colours::lightgrey).darker(0.2f)); // Added v.6.30. Updated v5.6.31 for (0.2f). Sets background colour behind
											// main window by default on grey to please everyone :)
}
CtrlrDocumentPanel::~CtrlrDocumentPanel()
{
	DBG("(D) TRACKING: CtrlrDocumentPanel Destructor has been entered !!!");
}
/*
CtrlrDocumentPanel::~CtrlrDocumentPanel() {
	DBG("(D) TRACKING: CtrlrDocumentPanel Destructor has been entered !!!");

	// // 1. Manually force synchronous destruction of all open document components
	// for (int i = getNumDocuments() - 1; i >= 0; --i)
	// {
	//     if (auto* doc = getDocument(i))
	//     {
	//         // closeDocumentAsync takes the component pointer and a callback function
	//         closeDocumentAsync(doc, [](bool) {});
	//     }
	// }

	// // 2. Clear out any remaining children as a fallback
	// deleteAllChildren();
}
*/
CtrlrDocumentPanelCloseButton::CtrlrDocumentPanelCloseButton(const String &buttonName) // Added v5.6.30
	: Button("") {
	setSize(18, 18);
}

CtrlrDocumentPanelCloseButton::~CtrlrDocumentPanelCloseButton() // Added v5.6.30
{}

void CtrlrDocumentPanel::resized() { MultiDocumentPanel::resized(); }

bool CtrlrDocumentPanel::tryToCloseDocument(Component *component) { return (true); }

void CtrlrDocumentPanel::tryToCloseDocumentAsync(Component *component, std::function<void(bool)> callback) {
	callback(tryToCloseDocument(component));
}

void CtrlrDocumentPanel::activeDocumentChanged() {
	CtrlrEditor *ed = dynamic_cast<CtrlrEditor *>(getParentComponent());
	if (ed) {
		ed->activeCtrlrChanged();
	}

	if (getCurrentTabbedComponent()) {

		if (owner.getInstanceMode() ==
			InstanceSingleRestricted) // Added v5.6.31. Hides tabs on exported restricted instances
		{
			getCurrentTabbedComponent()->setTabBarDepth(0); // Tab height for horizontal bar
			getCurrentTabbedComponent()->getTabbedButtonBar().setMinimumTabScaleFactor(0); // Min tab width ratio
		} else {
			getCurrentTabbedComponent()->setTabBarDepth(
				owner.getProperty(Ids::ctrlrTabBarDepth)); // Tab height for horizontal bar
			getCurrentTabbedComponent()->getTabbedButtonBar().setMinimumTabScaleFactor(1.0); // Min tab width ratio
		}

		TabbedButtonBar &bar = getCurrentTabbedComponent()->getTabbedButtonBar();

		for (int i = 0; i < bar.getNumTabs(); i++) {
			TabBarButton *button = bar.getTabButton(i); // Gets the panel tab button(i)

			if (button) {
				CtrlrDocumentPanelCloseButton *closeTabButton = new CtrlrDocumentPanelCloseButton(
					"x"); // Added v5.6.30. Brings back the close button for panel tabs
				closeTabButton->addListener(this);
				closeTabButton->setSize(20, 20);
				closeTabButton->getProperties().set("index", bar.indexOfTabButton(button));
				closeTabButton->setMouseCursor(MouseCursor::PointingHandCursor);
				button->setExtraComponent(closeTabButton, TabBarButton::afterText);
			}
		}
	}
}

void CtrlrDocumentPanel::buttonClicked(Button *button) {
	if (button == nullptr)
		return;

	// 1. Find the TabBarButton hosting this close button
	if (auto *tabButton = button->findParentComponentOfClass<TabBarButton>()) {
		if (auto *tc = getCurrentTabbedComponent()) {
			int tabIndex = tc->getTabbedButtonBar().indexOfTabButton(tabButton);
			if (tabIndex >= 0) {
				if (auto *ed = dynamic_cast<CtrlrPanelEditor *>(tc->getTabContentComponent(tabIndex))) {
					if (auto *panelToClose = owner.getPanelForEditor(ed)) {
						panelToClose->canClose(true, [this, ed, panelToClose](bool canCloseNow) {
							if (canCloseNow && panelToClose != nullptr) {
								// Deselect any active UI items
								if (auto *selection = ed->getSelection())
									selection->deselectAll();

								// Tell CtrlrManager to destroy the panel object
								owner.removePanel(ed);

								// CRITICAL: Tell MultiDocumentPanel to synchronously remove the visual tab!
								closeDocumentAsync(ed, false, nullptr);
							}
						});
					}
				}
			}
		}
	}
}
void CtrlrDocumentPanel::closeAllPanelsAndDetach()
{
    // 1. Hide immediately to stop repaints
    setVisible(false);

    // 2. Notify editors and cleanly detach them from JUCE parent components
    for (int i = getNumDocuments() - 1; i >= 0; --i)
    {
        if (auto* docComponent = getDocument(i))
        {
            if (auto* editor = dynamic_cast<CtrlrPanelEditor*>(docComponent))
            {
                editor->panelWillClose();
            }

            // Remove from JUCE UI hierarchy without deleting the pointer
            if (auto* parent = docComponent->getParentComponent())
            {
                parent->removeChildComponent(docComponent);
            }
        }
    }

    // DO NOT call closeDocumentAsync here! 
    // Let CtrlrPanel's unique_ptr<CtrlrPanelEditor> handle single ownership deletion.
}
void CtrlrDocumentPanel::closeDocumentAsync(juce::Component *doc, bool checkItsOkToCloseFirst,
											std::function<void(bool)> callback) {
	DBG("===closeDocumentAsync() - I closed : " << doc->getName());
	if (doc == nullptr) {
		if (callback != nullptr)
			callback(false);
		return;
	}

	// 1. Clear active focus FIRST so active tab operations don't target a dying component
	// if (getActiveDocument() == doc) {
	// 	setActiveDocument(nullptr);
	// }

	// 2. Call JUCE's base async method
	juce::MultiDocumentPanel::closeDocumentAsync(doc, checkItsOkToCloseFirst, callback);
}

#if JUCE_MODAL_LOOPS_PERMITTED
//bool CtrlrDocumentPanel::closeDocument(juce::Component *doc, bool checkItsOkToCloseFirst) {
//	if (doc == nullptr)
//		return false;
//
//	if (getActiveDocument() == doc) {
//		setActiveDocument(nullptr);
//	}
//
//	return juce::MultiDocumentPanel::closeDocument(doc, checkItsOkToCloseFirst);
//}
#endif
// In CtrlrDocumentPanel.h

void CtrlrDocumentPanel::setEditor(CtrlrEditor *_editorToSet) { ctrlrEditor = _editorToSet; }

/** lookAndFeelChanged() override works great for a single panel instance but not when having multiple panels loaded in
   CtrlrX. Problem with LookAndFeelChanged() is that the whole APP lookandfeel is following the last global LnF
   selected. It won't allow multi panel to have their own colours pushed to the top level APP design elements while
   switching panels. */

// void CtrlrDocumentPanel::lookAndFeelChanged()
//{
//     setBackgroundColour(Component::findColour(DocumentWindow::backgroundColourId));
//     //setBackgroundColour((Colours::lightgrey).darker(0.2)); // Sets background colour behind main window by default
//     on grey to please everyone :)
//
//     if (getCurrentTabbedComponent()) {
//         getCurrentTabbedComponent()->setTabBarDepth(owner.getProperty(Ids::ctrlrTabBarDepth));
//         getCurrentTabbedComponent()->getTabbedButtonBar().setTabBackgroundColour(getCurrentTabbedComponent()->getTabbedButtonBar().getCurrentTabIndex(),
//         findColour(TextButton::buttonColourId)); // Tab colour background
//         getCurrentTabbedComponent()->getTabbedButtonBar().setColour(TabbedButtonBar::tabTextColourId,
//         findColour(TextButton::textColourOffId)); // Tab text colour
//         getCurrentTabbedComponent()->getTabbedButtonBar().setColour(TabbedButtonBar::tabOutlineColourId,
//         findColour(TabbedButtonBar::tabOutlineColourId));  // Tab outline colour
//         getCurrentTabbedComponent()->getTabbedButtonBar().setColour(TabbedButtonBar::frontTextColourId,
//         findColour(TextButton::textColourOffId));  // Selected tab text colour
//         getCurrentTabbedComponent()->getTabbedButtonBar().setColour(TabbedButtonBar::frontOutlineColourId,
//         findColour(TabbedButtonBar::frontOutlineColourId));  // Selected tab outline colour
//     }
// }

/** Added v5.6.30. Brings back the panel tab close button */
void CtrlrDocumentPanelCloseButton::resized() {
	/* static position */
	internalPath1.clear();
	internalPath1.startNewSubPath((float)(proportionOfWidth(0.7000f)), (float)(proportionOfHeight(0.2500f)));
	internalPath1.lineTo((float)(proportionOfWidth(0.7500f)), (float)(proportionOfHeight(0.3000f)));
	internalPath1.lineTo((float)(proportionOfWidth(0.3000f)), (float)(proportionOfHeight(0.7500f)));
	internalPath1.lineTo((float)(proportionOfWidth(0.2500f)), (float)(proportionOfHeight(0.7000f)));
	internalPath1.closeSubPath();

	internalPath2.clear();
	internalPath2.startNewSubPath((float)(proportionOfWidth(0.3000f)), (float)(proportionOfHeight(0.2500f)));
	internalPath2.lineTo((float)(proportionOfWidth(0.7500f)), (float)(proportionOfHeight(0.7000f)));
	internalPath2.lineTo((float)(proportionOfWidth(0.7000f)), (float)(proportionOfHeight(0.7500f)));
	internalPath2.lineTo((float)(proportionOfWidth(0.2500f)), (float)(proportionOfHeight(0.3000f)));
	internalPath2.closeSubPath();

	/* Mouse over Position */
	internalPath3.clear();
	internalPath3.startNewSubPath((float)(proportionOfWidth(0.7000f)), (float)(proportionOfHeight(0.2500f)));
	internalPath3.lineTo((float)(proportionOfWidth(0.7500f)), (float)(proportionOfHeight(0.3000f)));
	internalPath3.lineTo((float)(proportionOfWidth(0.3000f)), (float)(proportionOfHeight(0.7500f)));
	internalPath3.lineTo((float)(proportionOfWidth(0.2500f)), (float)(proportionOfHeight(0.7000f)));
	internalPath3.closeSubPath();

	internalPath4.clear();
	internalPath4.startNewSubPath((float)(proportionOfWidth(0.3000f)), (float)(proportionOfHeight(0.2500f)));
	internalPath4.lineTo((float)(proportionOfWidth(0.7500f)), (float)(proportionOfHeight(0.7000f)));
	internalPath4.lineTo((float)(proportionOfWidth(0.7000f)), (float)(proportionOfHeight(0.7500f)));
	internalPath4.lineTo((float)(proportionOfWidth(0.2500f)), (float)(proportionOfHeight(0.3000f)));
	internalPath4.closeSubPath();

	/* Mouse down Position */
	internalPath5.clear();
	internalPath5.startNewSubPath((float)(proportionOfWidth(0.7000f)), (float)(proportionOfHeight(0.2500f)));
	internalPath5.lineTo((float)(proportionOfWidth(0.7500f)), (float)(proportionOfHeight(0.3000f)));
	internalPath5.lineTo((float)(proportionOfWidth(0.3000f)), (float)(proportionOfHeight(0.7500f)));
	internalPath5.lineTo((float)(proportionOfWidth(0.2500f)), (float)(proportionOfHeight(0.7000f)));
	internalPath5.closeSubPath();

	internalPath6.clear();
	internalPath6.startNewSubPath((float)(proportionOfWidth(0.3000f)), (float)(proportionOfHeight(0.2500f)));
	internalPath6.lineTo((float)(proportionOfWidth(0.7500f)), (float)(proportionOfHeight(0.7000f)));
	internalPath6.lineTo((float)(proportionOfWidth(0.7000f)), (float)(proportionOfHeight(0.7500f)));
	internalPath6.lineTo((float)(proportionOfWidth(0.2500f)), (float)(proportionOfHeight(0.3000f)));
	internalPath6.closeSubPath();
}

void CtrlrDocumentPanelCloseButton::paintButton(Graphics &g, bool isMouseOverButton, bool isButtonDown) {
	if (isButtonDown) {
		// g.setColour (Colour(findColour(TextButton::buttonOnColourId)).brighter(0.6));
		g.setColour(Colour(0xdfe7e7e8));

		g.fillRoundedRectangle((float)(proportionOfWidth(0.0500f)), (float)(proportionOfHeight(0.0500f)),
							   (float)(proportionOfWidth(0.9000f)), (float)(proportionOfHeight(0.9000f)),
							   (float)(proportionOfWidth(0.1f)));

		g.setColour(Colour(0xdf3f3e45));
		g.fillPath(internalPath5);

		g.setColour(Colour(0xdf3f3e45));
		g.fillPath(internalPath6);
	}

	else if (isMouseOverButton) {
		// g.setColour (Colour(findColour(TextButton::buttonOnColourId)).brighter(0.4)); // v.5.2.198
		// g.setColour (Colour (0xdfe7e7e8)); // Added v5.6.30
		// g.setColour (Colours::red); // Added v5.6.31 by GoodWeather
		g.setColour(findColour(TextButton::buttonOnColourId)); // Added v5.6.31

		g.fillRoundedRectangle((float)(proportionOfWidth(0.0500f)), (float)(proportionOfHeight(0.0500f)),
							   (float)(proportionOfWidth(0.9000f)), (float)(proportionOfHeight(0.9000f)),
							   (float)(proportionOfWidth(0.1f)));

		// g.setColour (Colour (0xdf3f3e45)); // Added v5.6.30
		g.setColour(Colours::white); // Added v5.6.31
		g.fillPath(internalPath3);

		// g.setColour (Colour (0xdf3f3e45)); // Added v5.6.30
		g.setColour(Colours::white); // Added v5.6.31
		g.fillPath(internalPath4);
	}

	else {
		g.setColour(Colour(0xdf2e2d32));
		g.fillPath(internalPath1);

		g.setColour(Colour(0xdf2e2d32));
		g.fillPath(internalPath2);
	}
}
