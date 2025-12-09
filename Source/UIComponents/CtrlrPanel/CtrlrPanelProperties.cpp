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

    CtrlrPanelComponentProperties* props = new CtrlrPanelComponentProperties(owner);

    tabbedComponent->addTab("General",
        getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
        props,
        true
    );

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

    ctrlrPanelFindProperty.reset(new CtrlrPanelFindProperty(owner, props));
    addAndMakeVisible(ctrlrPanelFindProperty.get());
    setSize(216, 364);
}

CtrlrPanelProperties::~CtrlrPanelProperties()
{
    CtrlrPanelComponentProperties* p = dynamic_cast <CtrlrPanelComponentProperties*>(tabbedComponent->getTabContentComponent(0));
    if (p)
    {
        owner.getOwner().getCtrlrManagerOwner().removeListener(p);
    }
    deleteAndZero(tabbedComponent);
}

void CtrlrPanelProperties::paint(Graphics& g)
{
    g.fillAll(findColour(DocumentWindow::backgroundColourId));
}

void CtrlrPanelProperties::resized()
{
    ctrlrPanelFindProperty->setBounds(0, 0, getWidth() - (int)owner.getOwner().getOwner().getProperty(Ids::ctrlrTabBarDepth), 32);
    tabbedComponent->setBounds(0, 32, getWidth() - 0, getHeight() - 32);
    updateTabColours();
    repaint();
}

void CtrlrPanelProperties::updateTabColours()
{
    for (int i = 0; i < tabbedComponent->getNumTabs(); i++)
    {
        if (i == tabbedComponent->getTabbedButtonBar().getCurrentTabIndex())
        {
            if (owner.getProperty(Ids::uiPanelLookAndFeel) == "V3")
            {
                tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, Colour(0xffcccccc));
            }
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

void CtrlrPanelProperties::lookAndFeelChanged()
{
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::frontTextColourId, findColour(Label::textColourId));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabTextColourId, findColour(Label::textColourId).withAlpha(0.6f));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabOutlineColourId, findColour(Slider::textBoxOutlineColourId));
    repaint();
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