/*
  ==============================================================================

    CtrlrLuaMethodEditorTabBarButton.h
    Created: 19 Nov 2026 7:50:49am
    Author:  zan64

  ==============================================================================
*/

#pragma once
#include "CtrlrLuaMethodEditorTabs.h"

class CtrlrLuaMethodEditorTabBarButton : public TabBarButton
{
public:
    CtrlrLuaMethodEditorTabBarButton (const String& name, TabbedButtonBar& ownerBar, CtrlrLuaMethodEditorTabs& ownerTabs, int tabIdx)
        : TabBarButton (name, ownerBar), ownerTabs (ownerTabs), initialIndex (tabIdx), isDraggingTab (false) {}

    // void mouseDown (const MouseEvent& e) override
    // {
    //     if (e.mods.isLeftButtonDown())
    //     {
    //         dragStartX = e.getScreenX();
    //         isDraggingTab = false;
    //     }
    //     TabBarButton::mouseDown (e);
    // }
void mouseDown (const MouseEvent& e) override
{
    if (e.mods.isPopupMenu())
    {
        const int tabIdx = getTabbedButtonBar().indexOfTabButton (this);
        if (tabIdx >= 0)
        {
            Component::SafePointer<CtrlrLuaMethodEditorTabBarButton> safeThis (this);

            PopupMenu menu;
            menu.addItem (1, "Reveal in Method Tree");

            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (this),
                                [safeThis, tabIdx] (int result)
                                {
                                    if (safeThis == nullptr || result != 1)
                                        return;

                                    if (auto *method = safeThis->ownerTabs.getMethodForTab (tabIdx))
                                    {
                                        safeThis->ownerTabs.getMethodEditor().revealMethodInTree (method);
                                    }
                                });
        }
        return; // Don't treat right-click as a left-drag start
    }

    if (e.mods.isLeftButtonDown())
    {
        dragStartX = e.getScreenX();
        isDraggingTab = false;
    }

    TabBarButton::mouseDown (e);
}
void mouseDrag (const MouseEvent& e) override
    {
        if (e.mods.isLeftButtonDown())
        {
            int dx = e.getScreenX() - dragStartX;
            
            if (!isDraggingTab && std::abs (dx) > 6)
            {
                isDraggingTab = true;
                toFront(true); 
                repaint();     
            }

            if (isDraggingTab)
            {
                int currentIndex = getTabbedButtonBar().indexOfTabButton (this);
                if (currentIndex == -1) return;

                Point<int> screenPos = e.getScreenPosition();
                TabbedButtonBar& bar = getTabbedButtonBar();

                if (auto* otherBtn = bar.getTabButton (currentIndex - 1))
                {
                    Rectangle<int> otherBounds = otherBtn->getScreenBounds();
                    // Require moving past the center PLUS a buffer (e.g., 25% into the left tab) 
                    // to prevent premature racing/swapping
                    if (screenPos.x < otherBounds.getRight() - (otherBounds.getWidth() * 0.25f))
                    {
                        ownerTabs.moveTab (currentIndex, currentIndex - 1, false);
                        return; // Process one swap per drag event to slow it down
                    }
                }

                if (auto* otherBtn = bar.getTabButton (currentIndex + 1))
                {
                    Rectangle<int> otherBounds = otherBtn->getScreenBounds();
                    // Require moving into the right tab by at least 25%
                    if (screenPos.x > otherBounds.getX() + (otherBounds.getWidth() * 0.25f))
                    {
                        ownerTabs.moveTab (currentIndex, currentIndex + 1, false);
                        return; // Process one swap per drag event to slow it down
                    }
                }
            }
        }
        TabBarButton::mouseDrag (e);
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (isDraggingTab)
        {
            isDraggingTab = false;
            repaint(); // Clear the drag indicator color
        }
        TabBarButton::mouseUp (e);
    }

    // Custom paint override to alter background color while dragging
void paint (Graphics& g) override
    {
        TabBarButton::paint (g);

        if (isDraggingTab)
        {
            // Draw a distinct light-green border or overlay tint on top of everything
            g.setColour (Colours::lightgreen.withAlpha (0.2f));
            g.fillRect (getLocalBounds());
        }
    }

private:
    CtrlrLuaMethodEditorTabs& ownerTabs;
    int initialIndex;
    int dragStartX = 0;
    bool isDraggingTab;
};