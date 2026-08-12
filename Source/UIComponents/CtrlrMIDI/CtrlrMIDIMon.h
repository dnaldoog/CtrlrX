#ifndef __CTRLR_MIDI_MON__
#define __CTRLR_MIDI_MON__

#include "CtrlrLog.h"
#include "CtrlrMacros.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrWindowManagers/CtrlrChildWindowContent.h"
#include "CtrlrWindowManagers/CtrlrManagerWindowManager.h"

class CtrlrManager;

class MidiMonitorEditor : public juce::CodeEditorComponent {
	public:
		enum CustomMenuIDs { ClearMonitorID = 1000 };

		MidiMonitorEditor(juce::CodeDocument &doc, juce::CodeTokeniser *tokeniser)
			: juce::CodeEditorComponent(doc, tokeniser) {}

		// Injects items into the standard right-click menu
		void addPopupMenuItems(juce::PopupMenu &menuToAddTo, const juce::MouseEvent *mouseClickEvent) override {
			// Add default Cut/Copy/Paste/Undo/Redo items
			juce::CodeEditorComponent::addPopupMenuItems(menuToAddTo, mouseClickEvent);

			// Append custom "Clear" option at the bottom
			menuToAddTo.addSeparator();
			menuToAddTo.addItem(ClearMonitorID, "Clear");
		}

		// Handles the selection
		void performPopupMenuAction(int menuItemID) override {
			if (menuItemID == ClearMonitorID) {
				getDocument().replaceAllContent(juce::String());
			} else {
				// Pass standard editing actions (Cut/Copy/Paste) back to JUCE
				juce::CodeEditorComponent::performPopupMenuAction(menuItemID);
			}
		}
};

class CtrlrMIDIMon : public CtrlrChildWindowContent, public CtrlrLog::Listener {

	public:
		CtrlrMIDIMon(CtrlrManager &_owner);
		~CtrlrMIDIMon();
		void messageLogged(CtrlrLog::CtrlrLogMessage _message);
		String getContentName() {
			return ("MIDI Monitor");
		}
		uint8 getType() {
			return static_cast<uint8>(CtrlrManagerWindowManager::MidiMonWindow);
		}

		void paint(Graphics &g);
		void resized();
		StringArray getMenuBarNames();
		PopupMenu getMenuForIndex(int topLevelMenuIndex, const String &menuName);
		void menuItemSelected(int menuItemID, int topLevelMenuIndex);
		void valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property);
		// void mouseDown(const juce::MouseEvent &e);

		bool shouldFilterMessage(const MidiMessage &m, int filterMask);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrMIDIMon)

	private:
		enum MenuItemIDs {
			// File menu
			CloseWindow = 1,
			ClearInputLog = 2,
			ClearOutputLog = 3,

			// View menu base
			ViewMenuBase = 10,

			// Filter menu
			FilterMenuBase = 10000,
			SelectAllFilters = 99998,
			ClearAllFilters = 99999
		};
		CtrlrManager &owner;
		CodeDocument docOut, docIn;
		StretchableLayoutManager layoutManager;
		bool logIn, logOut;
		CodeDocument outputDocument, inputDocument;
		StretchableLayoutResizerBar *resizer;
		void visibilityChanged();
		void focusGained(FocusChangeType cause);
		// CodeEditorComponent *outMon;
		// CodeEditorComponent *inMon;

		MidiMonitorEditor *outMon;
		MidiMonitorEditor *inMon;
		Label *outLabel;
		Label *inLabel;
		void updateDeviceLabels();
};

#endif
