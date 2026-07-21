#include "stdafx.h"
#include "stdafx_luabind.h"
#include "CtrlrLuaUtils.h"
#include "CtrlrLog.h"
#include "JuceClasses/LMemoryBlock.h"
#include <juce_core/juce_core.h>


/*
 * Copyright (c) 2010 The Beige Maze Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#define DSI_VOICE_DATA_MAX 16535

struct DSIVoiceData
{
	int size;
	unsigned char* data;
};

typedef struct DSIVoiceData UnpackedVoice, PackedVoice;

UnpackedVoice unpackDsiMidiData(PackedVoice packed)
{
	unsigned char data[DSI_VOICE_DATA_MAX];
    int packbyte = 0;  /* Composite of high bits of next 7 bytes */
    int pos = 0;       /* Current position of 7 */
    int ixp;           /* Packed byte index */
    int size = 0;      /* Unpacked voice size */
    unsigned char c;   /* Current source byte */
    for (ixp = 0; ixp < packed.size; ixp++)
    {
        c = packed.data[ixp];
        if (pos == 0) {
            packbyte = c;
        } else {
            if (packbyte & (1 << (pos - 1))) {c |= 0x80;}
            data[size++] = c;
        }
        pos++;
        pos &= 0x07;
        if (size > DSI_VOICE_DATA_MAX) break;
    }

    UnpackedVoice unpacked = {size, data};
    return unpacked;
}

PackedVoice packDsiMidiData(UnpackedVoice unpacked)
{
	uint8 data[DSI_VOICE_DATA_MAX];
    uint8 packbyte = 0;  /* Composite of high bits of next 7 bytes */
    int pos = 0;       /* Current position of 7 */
    int ixu;           /* Unpacked byte index */
    int size = 0;      /* Packed voice size */
    uint8 packet[7];     /* Current packet */
    uint8 i;             /* Packet output index */
    unsigned char c;   /* Current source byte */
    for (ixu = 0; ixu < unpacked.size; ixu++)
    {
        c = unpacked.data[ixu];
        if (pos == 7) {
        	data[size++] = (uint8)packbyte;
            for (i = 0; i < pos; i++)
            {
            	data[size++] = packet[i];
            }
            packbyte = 0;
            pos = 0;
        }
        if (c & 0x80) {
            packbyte += (1 << pos);
            c &= 0x7f;
        }
        packet[pos] = c;
        pos++;
        if ((size + 8) > DSI_VOICE_DATA_MAX) break;
    }
    data[size++] = packbyte;
    for (i = 0; i < pos; i++)
    {
    	data[size++] = packet[i];
    }

	PackedVoice packed = {size, data};
    return packed;
}

LMemoryBlock *CtrlrLuaUtils::unpackDsiData (MemoryBlock &dataToUnpack)
{
	PackedVoice packed;
	packed.data = (uint8 *)dataToUnpack.getData();
	packed.size = (int)dataToUnpack.getSize();

	UnpackedVoice unpacked = unpackDsiMidiData(packed);

	return (new LMemoryBlock ((uint8 *)unpacked.data, unpacked.size));
}

LMemoryBlock *CtrlrLuaUtils::packDsiData (MemoryBlock &dataToPack)
{
	UnpackedVoice unpacked,packed;
	unpacked.data = (uint8*)dataToPack.getData();
	unpacked.size = (int)dataToPack.getSize();

	packed = packDsiMidiData(unpacked);

	return (new LMemoryBlock ((uint8 *)packed.data, packed.size));
}
#if JUCE_VERSION < 0x070000
void CtrlrLuaUtils::warnWindow (const String title, const String message)
{
	AlertWindow::showMessageBox (AlertWindow::WarningIcon, title, message);
}

void CtrlrLuaUtils::infoWindow (const String title, const String message)
{
	AlertWindow::showMessageBox (AlertWindow::InfoIcon, title, message);
}

bool CtrlrLuaUtils::questionWindow (const String title, const String message, const String button1Text,
									const String button2Text)
{
	return (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, title, message, button1Text, button2Text));
}

File CtrlrLuaUtils::openFileWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
									const String &filePatternsAllowed, bool useOSNativeDialogBox)
{
	FileChooser dialog(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed, useOSNativeDialogBox);
	if (dialog.browseForFileToOpen(0))
	{
		return (dialog.getResult());
	}
	else
	{
		return (File());
	}
}

void CtrlrLuaUtils::openMultipleFilesWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
											const String &filePatternsAllowed, bool useOSNativeDialogBox,
											luabind::object const& table)
{
	FileChooser dialog(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed, useOSNativeDialogBox);
	if (dialog.browseForMultipleFilesToOpen(nullptr))
	{
		if (luabind::type(table) == LUA_TTABLE)
		{
			Array <File> res = dialog.getResults();

			for (int i=0; i<res.size(); i++)
			{
				table[i+1] = res[i];
			}
		}
	}
}

File CtrlrLuaUtils::saveFileWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
									const String &filePatternsAllowed, bool useOSNativeDialogBox)
{
	FileChooser dialog(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed, useOSNativeDialogBox);
	if (dialog.browseForFileToSave (true))
	{
		return (dialog.getResult());
	}
	else
	{
		return (File());
	}
}

File CtrlrLuaUtils::getDirectoryWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory)
{
	FileChooser dialog(dialogBoxTitle, initialFileOrDirectory);
	if (dialog.browseForDirectory())
	{
		return (dialog.getResult());
	}
	else
	{
		return (File());
	}
}

String CtrlrLuaUtils::askForTextInputWindow (const String title, const String message, const String initialInputContent, const String onScreenLabel, const bool isPassword, const String button1Text, const String button2Text)
{
	AlertWindow w(title, message, AlertWindow::QuestionIcon, 0);
	w.addTextEditor ("userInput", initialInputContent,  onScreenLabel, isPassword);
	w.addButton (button1Text, 1);
	w.addButton (button2Text, 0);
	if (w.runModalLoop() == 1) // Updated v5.6.34. Thanks to @dobo365
	{
		return (w.getTextEditorContents("userInput"));
	}
	else
	{
		return ("-1");
	}
}
#else

void CtrlrLuaUtils::questionWindow(const String title, const String message, const String button1Text,
								   const String button2Text, std::function<void(bool)> callback) {
	juce::AlertWindow::showOkCancelBox(juce::AlertWindow::QuestionIcon, title, message, button1Text, button2Text,
									   nullptr, juce::ModalCallbackFunction::create([callback](int result) {
										   if (callback)
											   callback(result == 1); // 1 = button1 (OK)
									   }));
}
// -----------------------------------------------------------------------------
// 2. File Choosers (Async with Callbacks)
// -----------------------------------------------------------------------------

void CtrlrLuaUtils::openFileWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
								   const String &filePatternsAllowed, bool useOSNativeDialogBox,
								   std::function<void(const File &)> callback) {
	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed,
													  useOSNativeDialogBox);
	auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

	dialog->launchAsync(flags, [dialog, callback](const juce::FileChooser &fc) {
		if (callback)
			callback(fc.getResult());
	});
}

void CtrlrLuaUtils::openMultipleFilesWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
											const String &filePatternsAllowed, bool useOSNativeDialogBox,
											luabind::object table) { // Pass luabind::object by value for async safety
	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed,
													  useOSNativeDialogBox);
	auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles |
				 juce::FileBrowserComponent::canSelectMultipleItems;

	dialog->launchAsync(flags, [dialog, table](const juce::FileChooser &fc) {
		if (luabind::type(table) == LUA_TTABLE) {
			Array<File> res = fc.getResults();
			for (int i = 0; i < res.size(); i++) {
				table[i + 1] = res[i];
			}
		}
	});
}

void CtrlrLuaUtils::saveFileWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
								   const String &filePatternsAllowed, bool useOSNativeDialogBox,
								   std::function<void(const File &)> callback) {
	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed,
													  useOSNativeDialogBox);
	auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles |
				 juce::FileBrowserComponent::warnAboutOverwriting;

	dialog->launchAsync(flags, [dialog, callback](const juce::FileChooser &fc) {
		if (callback)
			callback(fc.getResult());
	});
}

void CtrlrLuaUtils::getDirectoryWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
									   std::function<void(const File &)> callback) {
	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory);
	auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;

	dialog->launchAsync(flags, [dialog, callback](const juce::FileChooser &fc) {
		if (callback)
			callback(fc.getResult());
	});
}
// If you prefer preserving modal dialogs for legacy Lua scripts without changing Lua function signatures to callbacks,
// use AlertWindow directly:
// bool CtrlrLuaUtils::questionWindow(const String title, const String message,
//                                      const String button1Text, const String button2Text) {
//     return juce::AlertWindow::showOkCancelBox(
//         juce::AlertWindow::QuestionIcon, title, message, button1Text, button2Text);
// }
#if 0 // Old JUCE 6 code
// -----------------------------------------------------------------------------
// 1. Alert Windows
// -----------------------------------------------------------------------------

void CtrlrLuaUtils::warnWindow(const String title, const String message) {
	juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, title, message);
}

void CtrlrLuaUtils::infoWindow(const String title, const String message) {
	juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, title, message);
}

void CtrlrLuaUtils::questionWindow(const String title, const String message, const String button1Text,
								   const String button2Text, std::function<void(bool)> callback) {
	juce::AlertWindow::showOkCancelBox(juce::AlertWindow::QuestionIcon, title, message, button1Text, button2Text,
									   nullptr, juce::ModalCallbackFunction::create([callback](int result) {
										   if (callback)
											   callback(result == 1); // 1 = OK/button1
									   }));
}

// -----------------------------------------------------------------------------
// 2. File Choosers (Async with Callbacks)
// -----------------------------------------------------------------------------

void CtrlrLuaUtils::openFileWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
								   const String &filePatternsAllowed, bool useOSNativeDialogBox,
								   std::function<void(const File &)> callback) {
	// Heap allocate dialog so it stays alive while user picks file
	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed,
													  useOSNativeDialogBox);

	auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

	dialog->launchAsync(flags, [dialog, callback](const juce::FileChooser &fc) {
		File result = fc.getResult();
		if (callback)
			callback(result);
	});
}

void CtrlrLuaUtils::openMultipleFilesWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
											const String &filePatternsAllowed, bool useOSNativeDialogBox,
											luabind::object const &table) {
	// Capture table by value/ref safely in async context
	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed,
													  useOSNativeDialogBox);

	auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles |
				 juce::FileBrowserComponent::canSelectMultipleItems;

	dialog->launchAsync(flags, [dialog, table](const juce::FileChooser &fc) {
		if (luabind::type(table) == LUA_TTABLE) {
			Array<File> res = fc.getResults();

			for (int i = 0; i < res.size(); i++) {
				table[i + 1] = res[i];
			}
		}
	});
}

void CtrlrLuaUtils::saveFileWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
								   const String &filePatternsAllowed, bool useOSNativeDialogBox,
								   std::function<void(const File &)> callback) {
	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed,
													  useOSNativeDialogBox);

	auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

	dialog->launchAsync(flags, [dialog, callback](const juce::FileChooser &fc) {
		File result = fc.getResult();
		if (callback)
			callback(result);
	});
}

void CtrlrLuaUtils::getDirectoryWindow(const String &dialogBoxTitle, const File &initialFileOrDirectory,
									   std::function<void(const File &)> callback) {
	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory);

	auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;

	dialog->launchAsync(flags, [dialog, callback](const juce::FileChooser &fc) {
		File result = fc.getResult();
		if (callback)
			callback(result);
	});
}

// -----------------------------------------------------------------------------
// 3. Text Input Alert Window
// -----------------------------------------------------------------------------

void CtrlrLuaUtils::askForTextInputWindow(const String title, const String message, const String initialInputContent,
										  const String onScreenLabel, const bool isPassword, const String button1Text,
										  const String button2Text, std::function<void(const String &)> callback) {
	auto w = std::make_shared<juce::AlertWindow>(title, message, juce::AlertWindow::QuestionIcon, nullptr);

	w->addTextEditor("userInput", initialInputContent, onScreenLabel, isPassword);
	w->addButton(button1Text, 1);
	w->addButton(button2Text, 0);

	w->enterModalState(true, juce::ModalCallbackFunction::create([w, callback](int result) {
						   if (callback) {
							   if (result == 1)
								   callback(w->getTextEditorContents("userInput"));
							   else
								   callback("-1"); // Preserved original "-1" cancellation return logic
						   }
					   }));
}

#endif
#endif
StringArray CtrlrLuaUtils::getMidiInputDevices() // Update v5.6.35. For JUCE 8
{
    StringArray devices;
    for (auto& d : MidiInput::getAvailableDevices())
        devices.add (d.name);
    
    return devices;
}

StringArray CtrlrLuaUtils::getMidiOutputDevices() // Update v5.6.35. For JUCE 8
{
    StringArray devices;
    for (auto& d : MidiOutput::getAvailableDevices())
        devices.add (d.name);
    
    return devices;
}

juce::String CtrlrLuaUtils::base64_encode(const juce::String& stringToEncode) // Added v5.6.34. Thanks to @dnaldoog
{
	return juce::Base64::toBase64(stringToEncode);
}


juce::String CtrlrLuaUtils::base64_decode(const juce::String& base64String) // Added v5.6.34. Thanks to @dnaldoog
{
	juce::MemoryOutputStream decodedStream;
	if (juce::Base64::convertFromBase64(decodedStream, base64String))
	{
		const void* data = decodedStream.getData();
		const size_t size = decodedStream.getDataSize();
		if (size > 0)
			return juce::String::fromUTF8(static_cast<const char*>(data), (int)size);
	}
	return juce::String();
}

void CtrlrLuaUtils::wrapForLua (lua_State *L)
{
	using namespace luabind;

	module(L)
    [
		class_<CtrlrLuaUtils>("CtrlrLuaUtils")
			.def("unpackDsiData", &CtrlrLuaUtils::unpackDsiData, adopt(result))
			.def("packDsiData", &CtrlrLuaUtils::packDsiData, adopt(result))
			.def("warnWindow", &CtrlrLuaUtils::warnWindow)
			.def("infoWindow", &CtrlrLuaUtils::infoWindow)
			.def("questionWindow", &CtrlrLuaUtils::questionWindow)
			.def("openFileWindow", &CtrlrLuaUtils::openFileWindow)
			.def("openMultipleFilesWindow", &CtrlrLuaUtils::openMultipleFilesWindow)
			.def("saveFileWindow", &CtrlrLuaUtils::saveFileWindow)
			.def("getDirectoryWindow", &CtrlrLuaUtils::getDirectoryWindow)
			.def("askForTextInputWindow", &CtrlrLuaUtils::askForTextInputWindow)
			.def("getMidiInputDevices", &CtrlrLuaUtils::getMidiInputDevices)
			.def("getMidiOutputDevices", &CtrlrLuaUtils::getMidiOutputDevices)
			.def("getVersionMajor", &CtrlrLuaUtils::getVersionMajor)
			.def("getVersionMinor", &CtrlrLuaUtils::getVersionMinor)
			.def("getVersionRevision", &CtrlrLuaUtils::getVersionRevision)
			.def("getVersionString", &CtrlrLuaUtils::getVersionString)
			.def("getPi", &CtrlrLuaUtils::getPi)
	 		.def("get16bitSigned", &CtrlrLuaUtils::get16bitSigned) // Added v5.6.34. Thanks to @dnaldoog
			.def("get8bitSigned", &CtrlrLuaUtils::get8bitSigned) // Added v5.6.34. Thanks to @dnaldoog
			.def("base64_encode", &CtrlrLuaUtils::base64_encode) // Added v5.6.34. Thanks to @dnaldoog
			.def("base64_decode", &CtrlrLuaUtils::base64_decode) // Added v5.6.34. Thanks to @dnaldoog
	];
}
