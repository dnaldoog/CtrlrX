#include "stdafx.h"
#ifdef JUCE_WINDOWS
#include "CtrlrLog.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrWindows.h"
#include <psapi.h>

CtrlrWindows::CtrlrWindows(CtrlrManager& _owner) : owner(_owner)
{
}

CtrlrWindows::~CtrlrWindows()
{
}

const Result CtrlrWindows::writeResource(void* handle, const LPSTR resourceId, const LPSTR resourceType, const MemoryBlock& resourceData)
{
	HANDLE	hResource = (HANDLE)handle;

	if (hResource)
	{
		return (UpdateResource(hResource, resourceType, resourceId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPVOID)resourceData.getData(), (DWORD)resourceData.getSize()) ? Result::ok() : Result::fail("WIN32 UpdateResource failed"));
	}
	else
	{
		return (Result::fail("Windows Native: UpdateResource, resource HANDLE cast failed"));
	}
}

const Result CtrlrWindows::readResource(void* handle, const LPSTR resourceId, const LPSTR resourceType, MemoryBlock& resourceData)
{
	HRSRC	panelResource;
	HGLOBAL panelLoadedResource;
	String  data;
	char* dataPointer;
	DWORD	dataSize;
	HMODULE myModuleHandle;

	if (handle != nullptr)
	{
		myModuleHandle = (HMODULE)handle;
	}
	else
	{
		myModuleHandle = GetModuleHandle(File::getSpecialLocation(File::currentExecutableFile).getFullPathName().toUTF8());
	}

	if (myModuleHandle)
	{
		panelResource = FindResource(myModuleHandle, resourceId, resourceType);

		if (panelResource)
		{
			panelLoadedResource = LoadResource(myModuleHandle, panelResource);

			if (panelLoadedResource)
			{
				dataSize = SizeofResource(myModuleHandle, panelResource);
				dataPointer = (char*)LockResource(panelLoadedResource);

				resourceData = MemoryBlock(dataPointer, dataSize);

				return (Result::ok());
			}
			else
			{
				return (Result::fail("Windows Native: LoadResource() failed"));
			}
		}
		else
		{
			return (Result::fail("Windows Native: FindResource() failed"));
		}
	}
	else
	{
		return (Result::fail("Windows Native: GetModuleHandle() for: \"" + File::getSpecialLocation(File::currentExecutableFile).getFullPathName() + "\" failed"));
	}
}

const Result CtrlrWindows::exportWithDefaultPanel(CtrlrPanel* panelToWrite, const bool isRestricted, const bool signPanel)
{
	if (panelToWrite == nullptr)
	{
		return (Result::fail("Windows Native: exportWithDefaultPanel got nullptr for panel"));
	}

	File	me = File::getSpecialLocation(File::currentExecutableFile);
	File	newMe;
	HANDLE	hResource;
	MemoryBlock panelExportData, panelResourcesData;
	//MemoryBlock iconData(BinaryData::ico_midi_png, BinaryData::ico_midi_pngSize);

	FileChooser fc(CTRLR_NEW_INSTANCE_DIALOG_TITLE,
		me.getParentDirectory().getChildFile(File::createLegalFileName(panelToWrite->getProperty(Ids::name))).withFileExtension(me.getFileExtension()),
		"*" + me.getFileExtension(),
		panelToWrite->getOwner().getProperty(Ids::ctrlrNativeFileDialogs));

	if (fc.browseForFileToSave(true))
	{
		newMe = fc.getResult();

		if (!newMe.hasFileExtension(me.getFileExtension()))
		{
			newMe = newMe.withFileExtension(me.getFileExtension());
		}

		// Load the original executable into memory for binary search and replace
		MemoryBlock executableData;
		if (me.loadFileAsData(executableData))
		{
			// Get the filename for use in substitutions
			String filename = panelToWrite->getProperty(Ids::name).toString();
			String lcFirstFourChars = filename.substring(0, 4).toLowerCase();
			String firstSixChars = filename.substring(0, jmin(6, filename.length()));
			String firstSixteenChars = filename.substring(0, jmin(16, filename.length()));

			// Convert to hex for substitution
			MemoryBlock lcFirstFourHex;
			for (int i = 0; i < lcFirstFourChars.length(); ++i)
			{
				uint8 byte = static_cast<uint8>(lcFirstFourChars[i]);
				lcFirstFourHex.append(&byte, 1);
			}

			// Convert firstSixteenChars to hex for substitution
			MemoryBlock firstSixteenHex;
			for (int i = 0; i < firstSixteenChars.length(); ++i)
			{
				uint8 byte = static_cast<uint8>(firstSixChars[i]);
				firstSixteenHex.append(&byte, 1);
			}

			// Convert firstSixChars to hex for substitution
			MemoryBlock firstSixHex;
			for (int i = 0; i < firstSixChars.length(); ++i)
			{
				uint8 byte = static_cast<uint8>(firstSixChars[i]);
				firstSixHex.append(&byte, 1);
			}
			// Pad with zeros if less than 6 characters
			size_t padding = 6 - firstSixChars.length();
			for (size_t i = 0; i < padding; ++i)
			{
				uint8 zero = 0;
				firstSixHex.append(&zero, 1);
			}

			// 1. Replace "Instrument|Tools" with "Instrument|Synth"
			MemoryBlock searchHex1, replaceHex1;
			hexStringToBytes("49 6E 73 74 72 75 6D 65 6E 74 7C 54 6F 6F 6C 73 00 00 00 00 43 74 72 6C 72", searchHex1);
			hexStringToBytes("49 6E 73 74 72 75 6D 65 6E 74 7C 53 79 6E 74 68 00 00 00 00 43 74 72 6C 72", replaceHex1);
			replaceAllOccurrences(executableData, searchHex1, replaceHex1);

			// 2. Replace "cTrX" (63 54 72 58) with lowercase first 4 chars of filename
			MemoryBlock searchHex2;
			hexStringToBytes("63 54 72 58", searchHex2);
			replaceAllOccurrences(executableData, searchHex2, lcFirstFourHex);

			// 3. Replace "cTrl" (63 54 72 6C) with lowercase first 4 chars of filename
			MemoryBlock searchHex3;
			hexStringToBytes("63 54 72 6C", searchHex3);
			replaceAllOccurrences(executableData, searchHex3, lcFirstFourHex);

			// 4. Replace first 32 instances of "CtrlrX" (43 74 72 6C 72 58) with first 6 chars of filename
			MemoryBlock searchHex4;
			hexStringToBytes("43 74 72 6C 72 58", searchHex4);
			replaceFirstNOccurrences(executableData, searchHex4, firstSixHex, 32);

			MemoryBlock searchHexProjName;
			hexStringToBytes("43 74 72 6C 72 58 20 50 72 6F 6A 65 63 74", searchHexProjName);
			replaceFirstNOccurrences(executableData, searchHexProjName, firstSixteenHex, 1);

			// Save the modified executable
			if (!newMe.replaceWithData(executableData.getData(), executableData.getSize()))
			{
				return (Result::fail("Windows Native: Failed to write modified executable data"));
			}
		}
		else
		{
			// Fallback to regular file copy if we can't load the file
			if (!me.copyFileTo(newMe))
			{
				return (Result::fail("Windows Native: exportMeWithNewResource can't copy \"" + me.getFullPathName() + "\" to \"" + newMe.getFullPathName() + "\""));
			}
		}
	}
	else
	{
		return (Result::fail("Windows Native: exportMeWithNewResource \"Save file\" dialog failed"));
	}

	hResource = BeginUpdateResource(newMe.getFullPathName().toUTF8(), FALSE);

	if (hResource)
	{
		String error;

		if ((error = CtrlrPanel::exportPanel(panelToWrite, File(), newMe, &panelExportData, &panelResourcesData, isRestricted)) == "")
		{
			if (writeResource(hResource, MAKEINTRESOURCE(CTRLR_INTERNAL_PANEL_RESID), RT_RCDATA, panelExportData)
				&& writeResource(hResource, MAKEINTRESOURCE(CTRLR_INTERNAL_RESOURCES_RESID), RT_RCDATA, panelResourcesData)
				)
			{
				EndUpdateResource(hResource, FALSE);
			}
			else
			{
				return (Result::fail("Windows Native: exportMeWithNewResource writeResource[panel] failed"));
			}
		}
		else
		{
			return (Result::fail("Windows Native: exportMeWithNewResource exportPanel error: \"" + error + "\""));
		}

		return (Result::ok());
	}

	return (Result::fail("Windows Native: exportMeWithNewResource BeginUpdateResource failed"));
}

// Helper function to convert hex string to binary data
void CtrlrWindows::hexStringToBytes(const String& hexString, MemoryBlock& result)
{
	result.reset();
	String cleanedHex = hexString.removeCharacters(" \t\r\n");

	for (int i = 0; i < cleanedHex.length(); i += 2)
	{
		if (i + 1 < cleanedHex.length())
		{
			String byteStr = cleanedHex.substring(i, i + 2);
			int byteVal = byteStr.getHexValue32();
			uint8 byte = static_cast<uint8>(byteVal);
			result.append(&byte, 1);
		}
	}
}

// Replace all occurrences of searchData with replaceData in the targetData
void CtrlrWindows::replaceAllOccurrences(MemoryBlock& targetData, const MemoryBlock& searchData, const MemoryBlock& replaceData)
{
	if (searchData.getSize() != replaceData.getSize() || searchData.getSize() == 0)
	{
		DBG("Invalid search/replace data sizes");
		return;
	}

	const uint8* rawData = static_cast<const uint8*>(targetData.getData());
	size_t dataSize = targetData.getSize();
	size_t searchSize = searchData.getSize();

	for (size_t i = 0; i <= dataSize - searchSize; ++i)
	{
		if (memcmp(rawData + i, searchData.getData(), searchSize) == 0)
		{
			// Replace the data
			targetData.copyFrom(replaceData.getData(), i, replaceData.getSize());
			// Update rawData pointer as the memory might have been reallocated
			rawData = static_cast<const uint8*>(targetData.getData());
		}
	}
}

// Replace only the first N occurrences of searchData with replaceData in the targetData
void CtrlrWindows::replaceFirstNOccurrences(MemoryBlock& targetData, const MemoryBlock& searchData, const MemoryBlock& replaceData, int maxOccurrences)
{
	if (searchData.getSize() != replaceData.getSize() || searchData.getSize() == 0)
	{
		DBG("Invalid search/replace data sizes");
		return;
	}

	const uint8* rawData = static_cast<const uint8*>(targetData.getData());
	size_t dataSize = targetData.getSize();
	size_t searchSize = searchData.getSize();
	int occurrencesFound = 0;

	for (size_t i = 0; i <= dataSize - searchSize && occurrencesFound < maxOccurrences; ++i)
	{
		if (memcmp(rawData + i, searchData.getData(), searchSize) == 0)
		{
			// Replace the data
			targetData.copyFrom(replaceData.getData(), i, replaceData.getSize());
			// Update rawData pointer as the memory might have been reallocated
			rawData = static_cast<const uint8*>(targetData.getData());
			occurrencesFound++;
		}
	}
}

const Result CtrlrWindows::getDefaultPanel(MemoryBlock& dataToWrite)
{
#ifdef DEBUG_INSTANCE
	File temp("c:\\devel\\debug_small.bpanelz");
	temp.loadFileAsData(dataToWrite);
	return (Result::ok());
#endif

	return (readResource(nullptr, MAKEINTRESOURCE(CTRLR_INTERNAL_PANEL_RESID), RT_RCDATA, dataToWrite));
}

const Result CtrlrWindows::getDefaultResources(MemoryBlock& dataToWrite)
{
#ifdef DEBUG_INSTANCE
	File temp("c:\\devel\\debug_small.bpanelz");

	MemoryBlock data;
	{
		ScopedPointer <FileInputStream> fis(temp.createInputStream());
		fis->readIntoMemoryBlock(data);
	}

	ValueTree t = ValueTree::readFromGZIPData(data.getData(), data.getSize());

	if (t.isValid())
	{
		ValueTree r = t.getChildWithName(Ids::resourceExportList);
		if (r.isValid())
		{
			MemoryOutputStream mos(dataToWrite, false);
			{
				GZIPCompressorOutputStream gzipOutputStream(&mos);
				r.writeToStream(gzipOutputStream);
				gzipOutputStream.flush();
			}
			return (Result::ok());
		}
	}
	else
	{
		return (Result::fail("Windows Native: getDefaultResources got data but couldn't parse it as a compressed ValueTree"));
	}
#endif

	return (readResource(nullptr, MAKEINTRESOURCE(CTRLR_INTERNAL_RESOURCES_RESID), RT_RCDATA, dataToWrite));
}

const Result CtrlrWindows::getSignature(MemoryBlock& dataToWrite)
{
	return (readResource(nullptr, MAKEINTRESOURCE(CTRLR_INTERNAL_SIGNATURE_RESID), RT_RCDATA, dataToWrite));
}

const Result CtrlrWindows::registerFileHandler()
{
	if (!JUCEApplication::isStandaloneApp())
		return (Result::ok());

	const char* const exts[] = { ".panel", ".panelz", ".bpanel", ".bpanelz", nullptr };
	StringArray extensions(exts);

	for (int i = 0; i < extensions.size(); i++)
	{
		if (!WindowsRegistry::registerFileAssociation(extensions[i], "ctrlr" + extensions[i], "Ctrlr panel file (" + extensions[i] + ")", File::getSpecialLocation(File::currentApplicationFile), -1, true))
			return (Result::fail("Can't register [" + extensions[i] + "] file extension"));
	}

	return (Result::ok());
}

static void sendKey(const KeyPress& event)
{
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wScan = 0;
	input.ki.dwFlags = 0;

	// Modifier Down
	if (event.getModifiers().isCommandDown())
	{
		input.ki.wVk = VK_CONTROL;
		SendInput(1, &input, sizeof(INPUT));
	}
	if (event.getModifiers().isAltDown())
	{
		input.ki.wVk = VK_MENU;
		SendInput(1, &input, sizeof(INPUT));
	}
	if (event.getModifiers().isShiftDown())
	{
		input.ki.wVk = VK_SHIFT;
		SendInput(1, &input, sizeof(INPUT));
	}

	// KEY Down
	input.ki.wVk = event.getKeyCode();
	SendInput(1, &input, sizeof(INPUT));

	// KEY Up
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));

	// MODIFIER Up
	if (event.getModifiers().isCommandDown())
	{
		input.ki.wVk = VK_CONTROL;
		SendInput(1, &input, sizeof(INPUT));
	}
	if (event.getModifiers().isAltDown())
	{
		input.ki.wVk = VK_MENU;
		SendInput(1, &input, sizeof(INPUT));
	}
	if (event.getModifiers().isShiftDown())
	{
		input.ki.wVk = VK_SHIFT;
		SendInput(1, &input, sizeof(INPUT));
	}
}

const Result CtrlrWindows::sendKeyPressEvent(const KeyPress& event)
{
	return (sendKeyPressEvent(event, ""));
}

const Result CtrlrWindows::sendKeyPressEvent(const KeyPress& event, const String& targetWindowName)
{
	HWND firstwindow = FindWindowEx(NULL, NULL, NULL, NULL);
	HWND window = firstwindow;
	TCHAR windowtext[MAX_PATH];
	// INPUT input;

	if (targetWindowName != "")
	{
		while (1)
		{

			GetWindowText(window, windowtext, MAX_PATH);
			if (strstr(windowtext, targetWindowName.getCharPointer()) != NULL)
				break;

			window = FindWindowEx(NULL, window, NULL, NULL);
			if (window == NULL || window == firstwindow)
				return (Result::fail("Can't find target window: " + targetWindowName));
		}
	}

	SetForegroundWindow(window);
	sendKey(event);
	return (Result::ok());
}
#endif
