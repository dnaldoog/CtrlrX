#ifndef __CTRLR_OWNED_MIDI_MESSAGE__
#define __CTRLR_OWNED_MIDI_MESSAGE__

#include "CtrlrMidiMessage.h"

class CtrlrModulator;
#if JUCE_VERSION >= 0x070000
class CtrlrMidiMessageOwner {
	public:
		virtual ~CtrlrMidiMessageOwner() = default; // Added a virtual destructor for safe deletion lifecycle

		virtual int getMidiChannelForOwnedMidiMessages() = 0;
		virtual CtrlrSysexProcessor *getSysexProcessor() = 0;

		// Namespaces added here for JUCE 6/7/8 cross-compatibility
		virtual juce::Array<int, juce::CriticalSection> &getGlobalVariables() = 0;
};
#else
class CtrlrMidiMessageOwner {
	public:
		virtual int getMidiChannelForOwnedMidiMessages() = 0;
		virtual CtrlrSysexProcessor *getSysexProcessor() = 0;
		virtual Array<int, CriticalSection> &getGlobalVariables() = 0;
};
#endif

class CtrlrOwnedMidiMessage : public CtrlrMidiMessage {
	public:
		CtrlrOwnedMidiMessage(CtrlrMidiMessageOwner &_owner);
		CtrlrOwnedMidiMessage(CtrlrMidiMessageOwner &_owner, const Identifier &type);
		~CtrlrOwnedMidiMessage();
		void setControllerNumber(const int controllerNumber);
		void setMidiMessageType(const CtrlrMidiMessageType newType);
		void valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property);
		void patternChanged();
		void setChannel(const int midiChannel);
		int getChannel() const;
		void compilePattern(); /* same as above but does not trigger owner rehash */
		const Array<int, CriticalSection> &getGlobalVariables();
		CtrlrSysexProcessor *getSysexProcessor();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrOwnedMidiMessage)

	private:
		CtrlrMidiMessageOwner &owner;
};

#endif
