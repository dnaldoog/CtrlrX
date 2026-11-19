#pragma once

#include "CtrlrMethodEditorTabCloseButton.h"

class CtrlrLuaMethodEditor;
class CtrlrLuaMethod;

class CtrlrLuaMethodEditorTabs : public TabbedComponent
{
	public:
		CtrlrLuaMethodEditorTabs(CtrlrLuaMethodEditor &_owner);
		~CtrlrLuaMethodEditorTabs();
		TabBarButton *createTabButton (const String &tabName, int tabIndex) override;
		void currentTabChanged (int newCurrentTabIndex, const String &newCurrentTabName) override;
		CtrlrLuaMethod *getMethodForTab(int tabIndex) const;
		void updateAllTooltips();
		CtrlrLuaMethodEditor &getMethodEditor() {
			return owner;
		}
		const CtrlrLuaMethodEditor &getMethodEditor() const {
			return owner;
		}
		JUCE_LEAK_DETECTOR(CtrlrLuaMethodEditorTabs)

	private:
		CtrlrLuaMethodEditor &owner;
};
