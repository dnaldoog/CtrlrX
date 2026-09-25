#ifndef CTRLRSETTINGS_H
#define CTRLRSETTINGS_H

#include "CtrlrManager/CtrlrManager.h"
#include "stdafx.h"
#include <juce_gui_basics/juce_gui_basics.h>

class CtrlrSettings : public juce::Component, public juce::ValueTree::Listener {
	public:
		CtrlrSettings(CtrlrManager &_owner);
		~CtrlrSettings() override;

		void paint(juce::Graphics &g) override;
		void resized() override;
		void restart();

		// --- ValueTree::Listener callbacks ---
		void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
									  const juce::Identifier &property) override;

		void valueTreeChildAdded(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenAdded) override {}
		void valueTreeChildRemoved(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenRemoved,
								   int indexFromWhichItWasRemoved) override {}
		void valueTreeChildOrderChanged(juce::ValueTree &parentTree, int oldIndex, int newIndex) override {}
		void valueTreeParentChanged(juce::ValueTree &treeWhoseParentHasChanged) override {}

	private:
		CtrlrManager &owner;
		juce::PropertyPanel *propertyPanel = nullptr;
		bool settingsWereModified = false; // Flag to track changes

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrSettings)CtrlrPanelCanvas
};

#endif