#ifndef __CTRLR_SETTINGS__
#define __CTRLR_SETTINGS__

#include "CtrlrManager/CtrlrManager.h"
#include "stdafx.h"
#include <juce_gui_basics/juce_gui_basics.h>

class CtrlrSettings : public juce::Component, public juce::ValueTree::Listener {
	public:
		CtrlrSettings (CtrlrManager &_owner);
		~CtrlrSettings() override;
		void paint (Graphics& g);
		void resized();
        void restart();
		// --- ValueTree::Listener callbacks ---
		void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
									  const juce::Identifier &property) override;

		// Optional virtual methods from ValueTree::Listener (if overriding):
		void valueTreeChildAdded(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenAdded) override {}
		void valueTreeChildRemoved(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenRemoved,
								   int indexFromWhichItWasRemoved) override {}
		void valueTreeChildOrderChanged(juce::ValueTree &parentTree, int oldIndex, int newIndex) override {}
		void valueTreeParentChanged(juce::ValueTree &treeWhoseParentHasChanged) override {}
		void valueTreeRedirected(juce::ValueTree &treeWhichHasBeenRedirected) override {}

	private:
		CtrlrManager &owner;
		PropertyPanel* propertyPanel;
		bool settingsWereModified = false; // Flag to track changes

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrSettings)CtrlrPanelCanvas
};

#endif
