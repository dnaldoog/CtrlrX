#include "CtrlrPanelMIDISnapshot.h"
#include "CtrlrComponents/CtrlrComponent.h"
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLog.h"
#include "CtrlrLuaManager.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "stdafx.h"

CtrlrPanelMIDISnapshot::CtrlrPanelMIDISnapshot(CtrlrPanel &_owner)
	:	owner(_owner),
		Thread("MIDI Snapshot"),
		snapshotDelay(50),
		showDialog(false),
		alertWindow(nullptr),
		wasCancelledByUser(false),
		luaPanelMidiSnapshotPreCbk(nullptr),
		luaPanelMidiSnapshotPostCbk(nullptr)
{
	buffer.ensureSize(8192);
}

CtrlrPanelMIDISnapshot::~CtrlrPanelMIDISnapshot()
{
    if (alertWindow)
        deleteAndZero (alertWindow);

    stopThread (500);
}

void CtrlrPanelMIDISnapshot::sendSnapshot()
{
	triggerAsyncUpdate();
}

void CtrlrPanelMIDISnapshot::handleAsyncUpdate()
{
	gatherSnapshotData();
	startThread();

	startTimer (100);
    if (alertWindow)
    {
        const ScopedLock sl (messageLock);
        alertWindow->setMessage (message);
        alertWindow->enterModalState();
    }
}

void CtrlrPanelMIDISnapshot::gatherSnapshotData()
{
	buffer.clear();

	for (int i=0; i<owner.getModulators().size(); i++)
	{
		CtrlrModulator *m = owner.getModulators()[i];
		if (m->getMidiMessagePtr())
		{
			if (m->getComponent())
			{
				if ((int)m->getComponent()->getProperty(Ids::componentRadioGroupId) > 0)
				{
					if (m->getComponent()->getToggleState() == false)
						continue;
					else if (!(bool)m->getProperty(Ids::modulatorExcludeFromSnapshot))
						addCtrlrMidiMessageToBuffer (buffer, m->getMidiMessage());
				}
				else if (!(bool)m->getProperty(Ids::modulatorExcludeFromSnapshot))
				{
					addCtrlrMidiMessageToBuffer (buffer, m->getMidiMessage());
				}
			}
		}
	}

	showDialog = owner.getProperty(Ids::panelMidiSnapshotShowProgress);

	if (showDialog)
    {
        if (alertWindow == nullptr)
        {
            alertWindow = LookAndFeel::getDefaultLookAndFeel().createAlertWindow ("Sending MIDI Snapshot", String(), "Stop", String(), String(), AlertWindow::NoIcon, 1, nullptr);
            alertWindow->setEscapeKeyCancels (false);
            alertWindow->addProgressBarComponent (progress);
        }
    }
    else
    {
        alertWindow = nullptr;
    }

if (CtrlrLuaMethod* method = luaPanelMidiSnapshotPreCbk.get())
{
    if (method->isValid())
    {
        owner.getCtrlrLuaManager().getMethodManager().call(method, &buffer);
    }
}
}
void CtrlrPanelMIDISnapshot::setStatusMessage (const String& newStatusMessage)
{
    const ScopedLock sl (messageLock);
    message = newStatusMessage;
}

void CtrlrPanelMIDISnapshot::run()
{
	MidiBuffer::Iterator i(buffer);
	MidiMessage m; int t; int k=0;
	while (i.getNextEvent(m,t))
	{
	    progress = k / (double) buffer.getNumEvents();
		owner.sendMidi(m);
		k++;
		wait(snapshotDelay);
	}
}

void CtrlrPanelMIDISnapshot::setDelay(const int _snapshotDelay)
{
	snapshotDelay = _snapshotDelay;
}

void CtrlrPanelMIDISnapshot::timerCallback()
{
    bool threadStillRunning = isThreadRunning();

    if (! threadStillRunning)
    {
        stopTimer();
        stopThread (500);

        if (alertWindow)
        {
            if (alertWindow->isCurrentlyModal())
                alertWindow->exitModalState (1);

            alertWindow->setVisible (false);
        }

        wasCancelledByUser = threadStillRunning;

        if (luaPanelMidiSnapshotPostCbk && !luaPanelMidiSnapshotPostCbk.wasObjectDeleted())
        {
            if (luaPanelMidiSnapshotPostCbk->isValid())
            {
                owner.getCtrlrLuaManager().getMethodManager().call (luaPanelMidiSnapshotPostCbk, &buffer);
            }
        }

        return; // (this may be deleted now)
    }

    if (alertWindow)
    {
        const ScopedLock sl (messageLock);
        alertWindow->setMessage (message);
    }
}

void CtrlrPanelMIDISnapshot::setPreLuaCallback(CtrlrLuaMethod *method)
{
    luaPanelMidiSnapshotPreCbk = method;
}

void CtrlrPanelMIDISnapshot::setPostLuaCallback(CtrlrLuaMethod *method)
{
    luaPanelMidiSnapshotPostCbk = method;
}

void CtrlrPanelMIDISnapshot::loadSnapshotFromFile() {
	juce::WeakReference<CtrlrPanelMIDISnapshot> safeThis(this);

	FC::openFileAsync("Load Snapshot File", File::getSpecialLocation(File::userDocumentsDirectory), "*.syx;*.mid",
					  true, // useNativeDialog
					  [safeThis](const juce::File &sourceFile) {
						  // Early return if user cancelled or object was deleted
						  if (sourceFile == juce::File() || safeThis.wasObjectDeleted())
							  return;

						  if (sourceFile.getFileExtension().equalsIgnoreCase(".syx")) {
							  juce::MemoryBlock syxData;
							  if (sourceFile.loadFileAsData(syxData)) {
								  const uint8 *data = static_cast<const uint8 *>(syxData.getData());
								  int size = (int)syxData.getSize();
								  int pos = 0;
								  int sampleOffset = 1;

								  juce::MidiBuffer incomingBuffer;

								  while (pos < size) {
									  if (data[pos] == 0xf0) // SysEx Start
									  {
										  int sysexLen = 1;
										  while ((pos + sysexLen) < size && data[pos + sysexLen] != 0xf7)
											  sysexLen++;

										  if ((pos + sysexLen) < size && data[pos + sysexLen] == 0xf7)
											  sysexLen++; // Include F7 byte

										  juce::MidiMessage msg(data + pos, sysexLen);

										  // 1. Send out to hardware synth
										  safeThis->owner.sendMidi(msg);

										  // 2. Add to buffer for panel internal matching & UI updates
										  incomingBuffer.addEvent(msg, sampleOffset++);

										  pos += sysexLen;
									  } else {
										  pos++;
									  }
								  }

								  // 3. Dispatch incoming buffer to panel modulators & listeners
								  if (incomingBuffer.getNumEvents() > 0) {
									  safeThis->owner.panelReceivedMidi(incomingBuffer, inputDevice);
								  }

								  if (auto *ed = safeThis->owner.getEditor())
									  ed->notify("Snapshot loaded and sent.", nullptr, NotifySuccess);
							  }
						  }
					  });
}

void CtrlrPanelMIDISnapshot::saveSnapshotToFile()
{
    // 1. Gather current panel state into the buffer
    gatherSnapshotData();

    if (buffer.isEmpty())
    {
        owner.notify("Save Snapshot: No MIDI data to save.", nullptr, NotifyFailure);
        return;
    }

    // 2. Set default directory and default target filename
    juce::File defaultDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    juce::File defaultFile = defaultDir.getChildFile("Snapshot.syx");

    juce::WeakReference<CtrlrPanelMIDISnapshot> safeThis(this);

    // 3. Launch file chooser with explicit wildcard filters
    FC::saveFileAsync(
        "Save Snapshot As...",
        defaultFile,
        "*.syx;*.mid",
        true,
        [safeThis](const juce::File &targetFile)
        {
            if (safeThis.wasObjectDeleted())
                return;

            // Check if user cancelled
            if (targetFile == juce::File() || targetFile.isDirectory())
                return;

            // Ensure extension is preserved if user typed name without extension
            juce::File fileToSave = targetFile;
            if (fileToSave.getFileExtension().isEmpty())
                fileToSave = fileToSave.withFileExtension(".syx");

            const bool isSysex = fileToSave.getFileExtension().equalsIgnoreCase(".syx");
            const bool isMidi  = fileToSave.getFileExtension().equalsIgnoreCase(".mid");

            if (isSysex)
            {
                // Convert buffer messages into a flat raw byte block
                juce::MemoryBlock syxBlock;
                for (const auto metadata : safeThis->buffer)
                {
                    const auto& msg = metadata.getMessage();
                    syxBlock.append(msg.getRawData(), msg.getRawDataSize());
                }

                if (syxBlock.getSize() == 0)
                {
                    safeThis->owner.notify("Save Snapshot: Empty SysEx payload.", nullptr, NotifyFailure);
                    return;
                }

                // Write raw SysEx byte block to file
                if (fileToSave.replaceWithData(syxBlock.getData(), syxBlock.getSize()))
                {
                    safeThis->owner.notify("Snapshot saved to " + fileToSave.getFileName(), nullptr, NotifySuccess);
                }
                else
                {
                    safeThis->owner.notify("Failed to write SysEx file.", nullptr, NotifyFailure);
                }
            }
            else if (isMidi)
            {
                juce::MidiMessageSequence seq;
                for (const auto metadata : safeThis->buffer)
                {
                    seq.addEvent(metadata.getMessage(), metadata.samplePosition);
                }
                seq.updateMatchedPairs();

                juce::MidiFile midiFile;
                midiFile.setTicksPerQuarterNote(96);
                midiFile.addTrack(seq);

                juce::FileOutputStream stream(fileToSave);
                if (stream.openedOk() && midiFile.writeTo(stream))
                {
                    safeThis->owner.notify("Snapshot saved to " + fileToSave.getFileName(), nullptr, NotifySuccess);
                }
                else
                {
                    safeThis->owner.notify("Failed to write MIDI file.", nullptr, NotifyFailure);
                }
            }
        }
    );
}