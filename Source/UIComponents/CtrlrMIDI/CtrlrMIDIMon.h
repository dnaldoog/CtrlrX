#ifndef __CTRLR_MIDI_MON__
#define __CTRLR_MIDI_MON__

#include "CtrlrMacros.h"
#include "CtrlrLog.h"
#include "CtrlrWindowManagers/CtrlrChildWindowContent.h"
#include "CtrlrWindowManagers/CtrlrManagerWindowManager.h" // Ensure this include is present
class CtrlrManager;

class CtrlrMIDIMon  : public CtrlrChildWindowContent,
                      public CtrlrLog::Listener
{

	public:
		CtrlrMIDIMon (CtrlrManager &_owner);
		~CtrlrMIDIMon();
		void messageLogged (CtrlrLog::CtrlrLogMessage _message);
		String getContentName()					{ return ("MIDI Monitor"); }
		uint8 getType() override { return static_cast<uint8>(CtrlrManagerWindowManager::WindowType::MidiMonWindow); }

		void paint (Graphics& g);
		void resized();

		StringArray getMenuBarNames();
		PopupMenu getMenuForIndex(int topLevelMenuIndex, const String &menuName);
		void menuItemSelected(int menuItemID, int topLevelMenuIndex);
		bool shouldFilterMessage(const MidiMessage& m, int filterMask);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrMIDIMon)

	private:
		enum MenuItemIDs
		{
			// File menu
			CloseWindow = 1,
			ClearInputLog = 2,
			ClearOutputLog = 3,

			// View menu base
			ViewMenuBase = 10,

			// Filter menu
			FilterMenuBase = 10000,
			ClearAllFilters = 99999,
			SelectAllFilters = 99998
			
		};
		CtrlrManager &owner;
		CodeDocument docOut, docIn;
		StretchableLayoutManager layoutManager;
		bool logIn, logOut;
		CodeDocument outputDocument, inputDocument;
		StretchableLayoutResizerBar* resizer;
		CodeEditorComponent* outMon;
		CodeEditorComponent* inMon;
		Label* outLabel;
		Label* inLabel;
};


#endif
