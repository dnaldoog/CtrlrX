#pragma once

#include "CtrlrMacros.h"
class CtrlrPanel;
class CtrlrLuaMethod;
class CtrlrPanelMIDISnapshot : public AsyncUpdater, public Thread, public Timer
{
	public:
		CtrlrPanelMIDISnapshot(CtrlrPanel &_owner);
		~CtrlrPanelMIDISnapshot();
		void sendSnapshot();
		void saveSnapshotToFile();
		void loadSnapshotFromFile();
		void handleAsyncUpdate() override;
		void gatherSnapshotData();
		void run() override;
		void setDelay(const int _snapshotDelay);
		void setStatusMessage (const String& newStatusMessage);
		void setPreLuaCallback(CtrlrLuaMethod *method);
		void setPostLuaCallback(CtrlrLuaMethod *method);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CtrlrPanelMIDISnapshot)

	private:
	    void timerCallback() override;
        double progress;
        AlertWindow *alertWindow;
        bool wasCancelledByUser;
        String message;
        CriticalSection messageLock;
        bool showDialog;
		CtrlrPanel &owner;
		MidiBuffer buffer;
		int snapshotDelay;
		WeakReference <CtrlrLuaMethod> luaPanelMidiSnapshotPostCbk, luaPanelMidiSnapshotPreCbk;
		JUCE_DECLARE_WEAK_REFERENCEABLE(CtrlrPanelMIDISnapshot)
};
