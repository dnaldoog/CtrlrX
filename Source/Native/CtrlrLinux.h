#ifndef CTRLR_LINUX_H
#define CTRLR_LINUX_H

#include "CtrlrNative.h"

class CtrlrManager;

class CtrlrLinux : public CtrlrNative {
	public:
		CtrlrLinux(CtrlrManager &owner);
		~CtrlrLinux() override;

		// Modernized async panel export (matching CtrlrNative)
		void exportWithDefaultPanel(CtrlrPanel *panelToWrite, bool isRestricted, bool signPanel,
									std::function<void(juce::Result)> callback) override;

		// Fixed return types (removed legacy 'const' qualifiers & added 'override')
		juce::Result getDefaultPanel(juce::MemoryBlock &dataToWrite) override;
		juce::Result getDefaultResources(juce::MemoryBlock &dataToWrite) override;
		juce::Result sendKeyPressEvent(const juce::KeyPress &event) override;
		juce::Result sendKeyPressEvent(const juce::KeyPress &event, const juce::String &target) override;

	private:
		CtrlrManager &owner;
		std::unique_ptr<juce::FileChooser> fc;
};

#endif // CTRLR_LINUX_H
