#pragma once

#include "CtrlrMacros.h"
#include "CtrlrChildWindow.h"

class CtrlrManager;
class CtrlrChildWindow;

class CtrlrFloatingWindow : public DocumentWindow
{
    public:
        CtrlrFloatingWindow(const String &title, Component *content) 
            : DocumentWindow (title, Colours::white, DocumentWindow::allButtons, true)
        {
            setUsingNativeTitleBar (true);
            setContentNonOwned (content, true);
            setVisible (true);
            centreWithSize (getWidth(), getHeight());
        }

        virtual ~CtrlrFloatingWindow()
        {
            // Clean up content linkages cleanly
            clearContentComponent();
        }

        void closeButtonPressed()
        {
            // --- FIX: Never use raw 'delete this;' on managed layouts ---
            // Instead, tell JUCE to visibility-hide the component and queue its 
            // container memory for a clean, synchronous sweep on the next loop frame.
            setVisible (false);
            
            // This safely signals the parent array to release its tracking hooks
            // without corrupting the raw memory heap layout mid-flight.
            juce::MessageManager::callAsync ([this]()
            {
                // If it is inside a window manager, it is safer to let the manager close it,
                // otherwise we use the safe async deletion utility:
                juce::Component::SafePointer<CtrlrFloatingWindow> safePtr (this);
                if (safePtr != nullptr)
                {
                    // If you have a window manager hook, trigger it here, 
                    // otherwise, let JUCE handle the desktop drop safely.
                    removeFromDesktop();
                }
            });
        }
};

class CtrlrManagerWindowManager : public CtrlrWindowManager
{
	public:
		enum WindowType
		{
			Repository,
			MidiMonWindow,
			LogViewer,
			MIDICalculator,
			GlobalSettings, /*to avoid wayland crash*/
    		AboutWindow /*to avoid wayland crash*/
		};

		CtrlrManagerWindowManager(CtrlrManager &_owner);
		virtual ~CtrlrManagerWindowManager();
		void clearAllWindows();
		CtrlrManager &getOwner();

		const String getWindowName(const CtrlrManagerWindowManager::WindowType window);
		CtrlrManagerWindowManager::WindowType getWindowType(const String &windowName);
		void show (const CtrlrManagerWindowManager::WindowType window);
		void hide (const CtrlrManagerWindowManager::WindowType window);
		void toggle(const CtrlrManagerWindowManager::WindowType window, const bool makeVisible=true);
		bool isCreated(const CtrlrManagerWindowManager::WindowType window);
		Component *getContent(const CtrlrManagerWindowManager::WindowType window);

		void windowChanged(CtrlrChildWindow *windowThatChanged);
		void windowClosedButtonPressed(CtrlrChildWindow *windowThatChanged);

		void restoreState (const ValueTree &savedState);
		static void showModalDialog(const String &title, Component *content, const bool resizable=false, Component *parent=0);
		ValueTree &getManagerTree();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrManagerWindowManager)

	private:
		CtrlrChildWindow *getWindow(const CtrlrManagerWindowManager::WindowType window, const bool createIfNeeded=true);
		CtrlrChildWindow *createWindow(const CtrlrManagerWindowManager::WindowType window);
		void create(const CtrlrManagerWindowManager::WindowType window, const String &lastWindowState="");
		void create(const ValueTree &windowState);
		CtrlrManager &owner;
		OwnedArray <CtrlrChildWindow> windows;
		ValueTree managerTree;
};
