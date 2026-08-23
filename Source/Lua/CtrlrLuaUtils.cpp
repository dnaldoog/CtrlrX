#include "CtrlrLuaUtils.h"
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLog.h"
#include "JuceClasses/LMemoryBlock.h"
#include "stdafx.h"
#include "stdafx_luabind.h"
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

struct DSIVoiceData {
		int size;
		unsigned char *data;
};

typedef struct DSIVoiceData UnpackedVoice, PackedVoice;

UnpackedVoice unpackDsiMidiData(PackedVoice packed) {
	unsigned char data[DSI_VOICE_DATA_MAX];
	int packbyte = 0; /* Composite of high bits of next 7 bytes */
	int pos = 0;	  /* Current position of 7 */
	int ixp;		  /* Packed byte index */
	int size = 0;	  /* Unpacked voice size */
	unsigned char c;  /* Current source byte */
	for (ixp = 0; ixp < packed.size; ixp++) {
		c = packed.data[ixp];
		if (pos == 0) {
			packbyte = c;
		} else {
			if (packbyte & (1 << (pos - 1))) {
				c |= 0x80;
			}
			data[size++] = c;
		}
		pos++;
		pos &= 0x07;
		if (size > DSI_VOICE_DATA_MAX)
			break;
	}

	UnpackedVoice unpacked = {size, data};
	return unpacked;
}

PackedVoice packDsiMidiData(UnpackedVoice unpacked) {
	uint8 data[DSI_VOICE_DATA_MAX];
	uint8 packbyte = 0; /* Composite of high bits of next 7 bytes */
	int pos = 0;		/* Current position of 7 */
	int ixu;			/* Unpacked byte index */
	int size = 0;		/* Packed voice size */
	uint8 packet[7];	/* Current packet */
	uint8 i;			/* Packet output index */
	unsigned char c;	/* Current source byte */
	for (ixu = 0; ixu < unpacked.size; ixu++) {
		c = unpacked.data[ixu];
		if (pos == 7) {
			data[size++] = (uint8)packbyte;
			for (i = 0; i < pos; i++) {
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
		if ((size + 8) > DSI_VOICE_DATA_MAX)
			break;
	}
	data[size++] = packbyte;
	for (i = 0; i < pos; i++) {
		data[size++] = packet[i];
	}

	PackedVoice packed = {size, data};
	return packed;
}

LMemoryBlock *CtrlrLuaUtils::unpackDsiData(MemoryBlock &dataToUnpack) {
	PackedVoice packed;
	packed.data = (uint8 *)dataToUnpack.getData();
	packed.size = (int)dataToUnpack.getSize();

	UnpackedVoice unpacked = unpackDsiMidiData(packed);

	return (new LMemoryBlock((uint8 *)unpacked.data, unpacked.size));
}

LMemoryBlock *CtrlrLuaUtils::packDsiData(MemoryBlock &dataToPack) {
	UnpackedVoice unpacked, packed;
	unpacked.data = (uint8 *)dataToPack.getData();
	unpacked.size = (int)dataToPack.getSize();

	packed = packDsiMidiData(unpacked);

	return (new LMemoryBlock((uint8 *)packed.data, packed.size));
}
void CtrlrLuaUtils::warnWindow(const String title, const String message) {
	AW::showMessageBox(AW::Warning, title, message);
}

void CtrlrLuaUtils::infoWindow(const String title, const String message) {
	AW::showMessageBox(AW::Info, title, message);
}

String CtrlrLuaUtils::askForTextInputWindow(const String title, const String message, const String initialInputContent,
											const String onScreenLabel, const bool isPassword, const String button1Text,
											const String button2Text) {
	auto *mm = juce::MessageManager::getInstance();

	if (!mm->isThisTheMessageThread()) {
		jassertfalse;
		return String();
	}

	auto *w = new juce::AlertWindow(title, message, juce::AlertWindow::QuestionIcon, nullptr);
	w->addTextEditor("userInput", initialInputContent, onScreenLabel, isPassword);
	w->addButton(button1Text, 1);
	w->addButton(button2Text, 0);

	bool finished = false;
	String result;

	AW::runCustomAlertAsyncSafe(w, [w, &finished, &result](int res) {
		result = (res == 1) ? w->getTextEditorContents("userInput") : "-1";
		finished = true;
	});

	while (!finished)
		mm->runDispatchLoopUntil(20);

	return result;
}
void CtrlrLuaUtils::askForTextInputWindowAsync(const String title, const String message,
											   const String initialInputContent, const String onScreenLabel,
											   const bool isPassword, const String button1Text,
											   const String button2Text, luabind::object callback) {
	auto *w = new juce::AlertWindow(title, message, juce::AlertWindow::QuestionIcon, nullptr);
	w->addTextEditor("userInput", initialInputContent, onScreenLabel, isPassword);
	w->addButton(button1Text, 1);
	w->addButton(button2Text, 0);

	AW::runCustomAlertAsyncSafe(w, [w, callback](int result) mutable {
		if (callback.is_valid() && luabind::type(callback) == LUA_TFUNCTION) {
			try {
				if (result == 1)
					callback(w->getTextEditorContents("userInput"));
				else
					callback(String("-1"));
			} catch (const luabind::error &e) {
				_DBG("Lua callback exception in askForTextInputWindowAsync: " + String(e.what()));
			}
		}
	});
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
											luabind::object callback) {
	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed,
													  useOSNativeDialogBox);
	auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles |
				 juce::FileBrowserComponent::canSelectMultipleItems;

	dialog->launchAsync(flags, [dialog, callback](const juce::FileChooser &fc) mutable {
		if (luabind::type(callback) == LUA_TFUNCTION) {
			Array<File> res = fc.getResults();
			lua_State *L = callback.interpreter();

			// Create a native Lua table
			luabind::object fileTable = luabind::newtable(L);
			for (int i = 0; i < res.size(); i++) {
				fileTable[i + 1] = res[i];
			}

			// Call the Lua function directly passing fileTable as an argument
			try {
				callback(fileTable); // <--- Direct operator() invocation!
			} catch (const luabind::error &e) {
				// Log Lua error
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

StringArray CtrlrLuaUtils::getMidiInputDevices() // Update v5.6.35. For JUCE 8
{
	StringArray devices;
	for (auto &d : MidiInput::getAvailableDevices())
		devices.add(d.name);

	return devices;
}

StringArray CtrlrLuaUtils::getMidiOutputDevices() // Update v5.6.35. For JUCE 8
{
	StringArray devices;
	for (auto &d : MidiOutput::getAvailableDevices())
		devices.add(d.name);

	return devices;
}

juce::String CtrlrLuaUtils::base64_encode(const juce::String &stringToEncode) // Added v5.6.34. Thanks to @dnaldoog
{
	return juce::Base64::toBase64(stringToEncode);
}

juce::String CtrlrLuaUtils::base64_decode(const juce::String &base64String) // Added v5.6.34. Thanks to @dnaldoog
{
	juce::MemoryOutputStream decodedStream;
	if (juce::Base64::convertFromBase64(decodedStream, base64String)) {
		const void *data = decodedStream.getData();
		const size_t size = decodedStream.getDataSize();
		if (size > 0)
			return juce::String::fromUTF8(static_cast<const char *>(data), (int)size);
	}
	return juce::String();
}

File CtrlrLuaUtils::saveFileWindowSync(const String &dialogBoxTitle, const File &initialFileOrDirectory,
									   const String &filePatternsAllowed, bool useOSNativeDialogBox) {
	auto *mm = juce::MessageManager::getInstance();

	if (!mm->isThisTheMessageThread()) {
		jassertfalse;
		return File();
	}

	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed,
													  useOSNativeDialogBox);
	auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles |
				 juce::FileBrowserComponent::warnAboutOverwriting;

	bool finished = false;
	File result;

	dialog->launchAsync(flags, [dialog, &finished, &result](const juce::FileChooser &fc) {
		result = fc.getResult();
		finished = true;
	});

	while (!finished)
		mm->runDispatchLoopUntil(20);

	return result;
}

File CtrlrLuaUtils::openFileWindowSync(const String &dialogBoxTitle, const File &initialFileOrDirectory,
									   const String &filePatternsAllowed, bool useOSNativeDialogBox) {
	auto *mm = juce::MessageManager::getInstance();

	if (!mm->isThisTheMessageThread()) {
		jassertfalse;
		return File();
	}

	auto dialog = std::make_shared<juce::FileChooser>(dialogBoxTitle, initialFileOrDirectory, filePatternsAllowed,
													  useOSNativeDialogBox);
	auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

	bool finished = false;
	File result;

	dialog->launchAsync(flags, [dialog, &finished, &result](const juce::FileChooser &fc) {
		result = fc.getResult();
		finished = true;
	});

	while (!finished)
		mm->runDispatchLoopUntil(20);

	return result;
}

bool CtrlrLuaUtils::questionWindow(const String title, const String message, const String button1Text, const String button2Text) {
	auto *mm = juce::MessageManager::getInstance();

	if (!mm->isThisTheMessageThread()) {
		jassertfalse;
		return false;
	}

	bool finished = false;
	bool userClickedYes = false;

	AW::showOkCancelAsyncSafe(AW::Question, title, message,
		[&finished, &userClickedYes](bool result) {
			userClickedYes = result;
			finished = true;
		},
		button1Text, button2Text);

	while (!finished)
		mm->runDispatchLoopUntil(20);

	return userClickedYes;
}

void CtrlrLuaUtils::questionWindowAsync(const String title, const String message,
                                        const String button1Text, const String button2Text, luabind::object callback) {
	std::function<void(bool)> juceCallback = [callback](bool result) mutable {
		if (callback.is_valid() && luabind::type(callback) == LUA_TFUNCTION) {
			try {
				callback(result);
			} catch (const luabind::error &e) {
				_DBG("Lua callback exception in questionWindowAsync: " + String(e.what()));
			}
		}
	};

	AW::showOkCancelAsyncSafe(AW::Question, title, message, juceCallback, button1Text, button2Text);
}
void CtrlrLuaUtils::wrapForLua(lua_State *L) {
	using namespace luabind;
	// typedef bool (CtrlrLuaUtils::*SyncQuestionWin)(const String, const String, const String, const String);
	// typedef void (CtrlrLuaUtils::*AsyncQuestionWin)(const String, const String, const String, const String,
	// 												luabind::object);

	module(L)[class_<CtrlrLuaUtils>("CtrlrLuaUtils")
				  .def("unpackDsiData", &CtrlrLuaUtils::unpackDsiData, adopt(result))
				  .def("packDsiData", &CtrlrLuaUtils::packDsiData, adopt(result))
				  .def("warnWindow", &CtrlrLuaUtils::warnWindow)
				  .def("infoWindow", &CtrlrLuaUtils::infoWindow)
				  .def("questionWindow", &CtrlrLuaUtils::questionWindow)
				  .def("questionWindowAsync", &CtrlrLuaUtils::questionWindowAsync)
				  .def("openFileWindow", &CtrlrLuaUtils::openFileWindowSync)
				  .def("openMultipleFilesWindow", &CtrlrLuaUtils::openMultipleFilesWindow)
				  .def("saveFileWindow", &CtrlrLuaUtils::saveFileWindowSync)
				  .def("getDirectoryWindow", &CtrlrLuaUtils::getDirectoryWindow)
				  .def("askForTextInputWindow", &CtrlrLuaUtils::askForTextInputWindow)
				  .def("askForTextInputWindowAsync", &CtrlrLuaUtils::askForTextInputWindowAsync)
				  .def("getMidiInputDevices", &CtrlrLuaUtils::getMidiInputDevices)
				  .def("getMidiOutputDevices", &CtrlrLuaUtils::getMidiOutputDevices)
				  .def("getVersionMajor", &CtrlrLuaUtils::getVersionMajor)
				  .def("getVersionMinor", &CtrlrLuaUtils::getVersionMinor)
				  .def("getVersionRevision", &CtrlrLuaUtils::getVersionRevision)
				  .def("getVersionString", &CtrlrLuaUtils::getVersionString)
				  .def("getPi", &CtrlrLuaUtils::getPi)
				  .def("get16bitSigned", &CtrlrLuaUtils::get16bitSigned) // Added v5.6.34. Thanks to @dnaldoog
				  .def("get8bitSigned", &CtrlrLuaUtils::get8bitSigned)	 // Added v5.6.34. Thanks to @dnaldoog
				  .def("base64_encode", &CtrlrLuaUtils::base64_encode)	 // Added v5.6.34. Thanks to @dnaldoog
				  .def("base64_decode", &CtrlrLuaUtils::base64_decode)	 // Added v5.6.34. Thanks to @dnaldoog
	];
}
