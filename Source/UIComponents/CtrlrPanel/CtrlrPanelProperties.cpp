#include "stdafx.h"
#include "CtrlrPanelEditor.h"
#include "CtrlrPanelUtilities.h"
#include "CtrlrLog.h"
#include "CtrlrPanelComponentProperties.h"
#include "CtrlrPanelResourceEditor.h"


#include "CtrlrPanelProperties.h"

CtrlrPanelProperties::CtrlrPanelProperties(CtrlrPanelEditor& _owner)
    : Component(L"Properties"),
    owner(_owner),
    tabbedComponent(0)
{

    
    addAndMakeVisible(tabbedComponent = new TabbedComponent(TabbedButtonBar::TabsAtRight));

    tabbedComponent->setTabBarDepth(owner.getOwner().getOwner().getProperty(Ids::ctrlrTabBarDepth));
    tabbedComponent->setCurrentTabIndex(-1);
    tabbedComponent->setOutline(1);

	auto *props = new CtrlrPanelComponentProperties(owner);

	tabbedComponent->addTab("General", getLookAndFeel().findColour(TabbedComponent::backgroundColourId), props, true);

	tabbedComponent->addTab("Resources",
        getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
        new CtrlrPanelResourceEditor(owner),
        true
    );

    tabbedComponent->addTab("XML Viewer",
        getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
        new CtrlrPanelUtilities(owner),
        true
    );
    /*MOVED TO CtrlrEditor.h Propably needs a separate file h/cpp*/
    // NEW: Add Expressions tab
    // tabbedComponent->addTab("Expressions",
    //     getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
    //     new CtrlrExpressionsHelp(),
    //     true
    // );

    //tabbedComponent->addTab("MIDI Bulk Dump",
    //    getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
    //    new CtrlrTransferDumpHelp(),
    //    true
    //);

    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::frontTextColourId, findColour(Label::textColourId));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabTextColourId, findColour(Label::textColourId).withAlpha(0.6f));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabOutlineColourId, findColour(Slider::textBoxOutlineColourId));

	// ctrlrPanelFindProperty.reset(new CtrlrPanelFindProperty(owner, props));
	// addAndMakeVisible(ctrlrPanelFindProperty.get());

	ctrlrPanelFindProperty.reset(new CtrlrPanelFindProperty(owner, props));
	addAndMakeVisible(ctrlrPanelFindProperty.get());
	setSize(216, 364);
}

CtrlrPanelProperties::~CtrlrPanelProperties()
{
    if (tabbedComponent != nullptr)
    {
        // 1. Safely unhook listeners / lookAndFeels
        for (int i = 0; i < tabbedComponent->getNumTabs(); ++i)
        {
            if (auto* content = tabbedComponent->getTabContentComponent(i))
            {
                // Disable component callbacks before JUCE triggers visibilityChanged
                content->setComponentEffect(nullptr);
                content->setLookAndFeel(nullptr);
            }
        }

        // 2. CRITICAL: Clear tab content pointers BEFORE deleting tabbedComponent
        // This prevents ~TabbedComponent() -> clearTabs() -> visibilityChanged() from firing!
        while (tabbedComponent->getNumTabs() > 0)
        {
            // Remove component without triggering visibility refresh cascades
            tabbedComponent->removeTab(0);
        }

        // 3. Remove and delete tabbedComponent cleanly
        removeChildComponent(tabbedComponent);
        delete tabbedComponent;
        tabbedComponent = nullptr;
    }
}

/*
CtrlrPanelProperties::~CtrlrPanelProperties()
{
    if (tabbedComponent != nullptr)
    {
		// 1. Clean up listeners and LookAndFeel pointers while tabs are still valid
		for (int i = 0; i < tabbedComponent->getNumTabs(); ++i) {
			if (auto *content = tabbedComponent->getTabContentComponent(i)) {
				if (auto *p = dynamic_cast<CtrlrPanelComponentProperties *>(content)) {
					owner.getOwner().getCtrlrManagerOwner().removeListener(p);
				}

				content->setLookAndFeel(nullptr);
			}
		}

		// 2. Remove tabbedComponent from parent child list BEFORE deleting it
		removeChildComponent(tabbedComponent);

		// 3. Delete tabbedComponent (this automatically deletes all tabs because deleteComponentWhenNotNeeded == true)
		delete tabbedComponent;
		tabbedComponent = nullptr;
	}
}
*/
void CtrlrPanelProperties::paint(Graphics& g)
{
    g.fillAll(findColour(DocumentWindow::backgroundColourId));
	
//	  NOTE : The following code might seem to work, but it's a dangerous practice that can lead to performance issues and potential crashes.
//    While it might not immediately cause an infinite loop on all systems, it's an incorrect use of the JUCE API. The paint() function is for drawing, not for changing component properties.
//
//	  According to Gemini, this is why this Approach is not recommended :
//
//    The JUCE framework uses a "dirty rectangle" system for repainting. When a component's state changes (e.g., its color), it invalidates the area that needs to be redrawn, which then triggers a call to its paint() function.
//    When you call setTabBackgroundColour() inside paint():
//    State Change: The setTabBackgroundColour() function changes the tab's color property.
//    Repaint Trigger: This state change triggers a repaint of the component.
//    Recursive Call: The repaint call leads back to the paint() function, which then runs your loop again.
//
//    This creates a chain reaction that can be very inefficient. In many cases, it will cause a soft lock where your application's CPU usage spikes, leading to sluggish performance or a full crash on a busy message queue.
	
//	Moved to updateTabColors()
//	for (int i = 0; i < tabbedComponent->getNumTabs(); i++) // Added v5.6.34. Thanks to @dobo365
//    {
//        if (i == tabbedComponent->getTabbedButtonBar().getCurrentTabIndex())
//        {
//            // This is for v3 where the color is forced. Should add an If..then..else checking v4 LnF
//			if (owner.getProperty(Ids::uiPanelLookAndFeel) == "V3")
//			{
//				tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, Colour(0xffcccccc));   // Added v5.6.34. Set to light grey. ATTENTION: this is also changing the bottom part. By @dobo365 DB.
//			}
//			// This is for the other LnF versions, mostly v4.
//			else
//			{
//				tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, getLookAndFeel().findColour(TabbedComponent::backgroundColourId).contrasting(0.2f));
//			}
//        }
//        else
//        {
//            tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, getLookAndFeel().findColour(TabbedComponent::backgroundColourId));
//        }
//    }
}

void CtrlrPanelProperties::resized()
{
    ctrlrPanelFindProperty->setBounds(0, 0, getWidth() - (int)owner.getOwner().getOwner().getProperty(Ids::ctrlrTabBarDepth), 32);
    tabbedComponent->setBounds(0, 32, getWidth() - 0, getHeight() - 32);
    updateTabColours(); // Added v5.6.34
    repaint();
}

void CtrlrPanelProperties::updateTabColours() // Added v5.6.34
{
    for (int i = 0; i < tabbedComponent->getNumTabs(); i++) // Added v5.6.34. Thanks to @dobo365
    {
        if (i == tabbedComponent->getTabbedButtonBar().getCurrentTabIndex())
        {
            // This is for v3 where the color is forced. Should add an If..then..else checking v4 LnF
            if (owner.getProperty(Ids::uiPanelLookAndFeel) == "V3")
            {
                tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, Colour(0xffcccccc));   // Added v5.6.34. Set to light grey. ATTENTION: this is also changing the bottom part. By @dobo365 DB.
            }
            // This is for the other LnF versions, mostly v4.
            else
            {
                tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, getLookAndFeel().findColour(TabbedComponent::backgroundColourId).contrasting(0.2f));
            }
        }
        else
        {
            tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, getLookAndFeel().findColour(TabbedComponent::backgroundColourId));
        }
    }
}

void CtrlrPanelProperties::lookAndFeelChanged() // Added v5.6.31
{
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::frontTextColourId, findColour(Label::textColourId)); // Added v5.6.31
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabTextColourId, findColour(Label::textColourId).withAlpha(0.6f)); // Added v5.6.31
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabOutlineColourId, findColour(Slider::textBoxOutlineColourId)); // Added v5.6.31
    repaint(); // Added v5.6.31
}

void CtrlrPanelProperties::changeListenerCallback(ChangeBroadcaster* source)
{
}

void CtrlrPanelProperties::refreshAll()
{
    for (int i = 0; i < tabbedComponent->getNumTabs(); i++)
    {
        CtrlrPanelComponentProperties* cp = dynamic_cast<CtrlrPanelComponentProperties*>(tabbedComponent->getTabContentComponent(i));
        if (cp != 0)
        {
            cp->refreshAll();
        }
    }
}

void CtrlrPanelProperties::layoutChanged()
{
    if ((bool)owner.getProperty(Ids::uiPanelPropertiesOnRight) == true)
    {
        tabbedComponent->setOrientation(TabbedButtonBar::TabsAtLeft);
    }
    else
    {
        tabbedComponent->setOrientation(TabbedButtonBar::TabsAtRight);
    }
}
