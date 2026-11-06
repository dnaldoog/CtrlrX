#include "stdafx.h"
#include "CtrlrLuaMethodEditorTabs.h"
#include "CtrlrLuaMethodEditor.h"
#include "CtrlrLuaMethodCodeEditor.h"
#include "Core/CtrlrPanel/CtrlrPanel.h"

// Tabbed component
CtrlrLuaMethodEditorTabs::CtrlrLuaMethodEditorTabs(CtrlrLuaMethodEditor &_owner)
	:	owner(_owner),
		TabbedComponent(TabbedButtonBar::TabsAtTop)
{
	setTabBarDepth(owner.getOwner().getOwner().getProperty(Ids::ctrlrTabBarDepth)); // Tab height for horizontal bar
    getTabbedButtonBar().setMinimumTabScaleFactor(1.0); // Min tab width ratio, when total tabBar width exceeds the window width, a + sign is displayed
}

CtrlrLuaMethodEditorTabs::~CtrlrLuaMethodEditorTabs()
{
}

TabBarButton *CtrlrLuaMethodEditorTabs::createTabButton (const String &tabName, int tabIndex)
{
	TabBarButton *btn = new TabBarButton (tabName, getTabbedButtonBar());
    
    CtrlrMethodEditorTabCloseButton *close = new CtrlrMethodEditorTabCloseButton(owner, tabIndex); // Added v5.6.30. Creates extra component *close
    close->setMouseCursor (MouseCursor::PointingHandCursor);
    close->setSize (22, 20); // Added v5.6.30
    btn->setExtraComponent (close, TabBarButton::afterText); // Added v5.6.30. Defines *close as extraComponent and sets its position

	return (btn);
}

void CtrlrLuaMethodEditorTabs::currentTabChanged(int newCurrentTabIndex, const String &newCurrentTabName)
{
	updateAllTooltips();
	CtrlrLuaMethodCodeEditor * codeEditor = dynamic_cast<CtrlrLuaMethodCodeEditor*> (getCurrentContentComponent());
	if (owner.findDialogActive == true)
	{
		if (codeEditor)
			codeEditor->getCodeComponent()->showFindPanel(false);
	}
	else
	{
		if (codeEditor)
			codeEditor->getCodeComponent()->hideFindPanel();
	}
}
// Helper function to extract path
static String getMethodVirtualFolderPath(CtrlrLuaMethod* method)
{
    if (method == nullptr)
        return "/";

    ValueTree parent = method->getMethodTree().getParent();
    StringArray pathParts;

    while (parent.isValid())
    {
        // Reached the panel itself: stop, so its name never ends up in the path
        if (parent.hasType(Ids::panel))
            break;

        if (parent.hasProperty(Ids::name))
        {
            const String name = parent.getProperty(Ids::name).toString();
            if (name.isNotEmpty() && name != "Lua Methods" && name != "methods")
                pathParts.insert(0, name);
        }
        parent = parent.getParent();
    }

    return "/" + pathParts.joinIntoString("/");   // "/COMPONENT/TVF", or "/" for top level
}

void CtrlrLuaMethodEditorTabs::updateAllTooltips()
{
    TabbedButtonBar& bar = getTabbedButtonBar();
    
    for (int i = 0; i < getNumTabs(); ++i)
    {
        auto* codeEditor = dynamic_cast<CtrlrLuaMethodCodeEditor*>(getTabContentComponent(i));
        if (codeEditor != nullptr)
        {
            CtrlrLuaMethod* method = codeEditor->getMethod();
            if (method != nullptr)
            {
                String folderPath = getMethodVirtualFolderPath(method);
                if (auto* tabButton = bar.getTabButton(i))
                {
                    tabButton->setTooltip(folderPath);
                }
            }
        }
    }
}