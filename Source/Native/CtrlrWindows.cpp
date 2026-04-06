#include "stdafx.h"
#ifdef JUCE_WINDOWS
#define NOMINMAX // Added v5.6.32
#include "CtrlrLog.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrWindows.h"
#include <psapi.h>

#include <fstream> // Added v5.6.32
#include <algorithm> // Added v5.6.32
#include <vector> // Added v5.6.32
#include <random> // Added v5.6.32

CtrlrWindows::CtrlrWindows(CtrlrManager &_owner) : owner(_owner)
{
}

CtrlrWindows::~CtrlrWindows()
{
}

const Result CtrlrWindows::writeResource (void *handle, const LPSTR resourceId, const LPSTR resourceType, const MemoryBlock &resourceData)
{
	HANDLE	hResource = (HANDLE)handle;

	if (hResource)
	{
		return (UpdateResource (hResource, resourceType, resourceId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPVOID) resourceData.getData(), (DWORD)resourceData.getSize()) ? Result::ok() : Result::fail("WIN32 UpdateResource failed"));
	}
	else
	{
		return (Result::fail("Windows Native: UpdateResource, resource HANDLE cast failed"));
	}
}

const Result CtrlrWindows::readResource (void *handle, const LPSTR resourceId, const LPSTR resourceType, MemoryBlock &resourceData)
{
	HRSRC	panelResource;
	HGLOBAL panelLoadedResource;
	String  data;
	char	*dataPointer;
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
				dataSize	= SizeofResource (myModuleHandle, panelResource);
				dataPointer = (char *) LockResource (panelLoadedResource);

				resourceData = MemoryBlock(dataPointer,dataSize);

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
		return (Result::fail("Windows Native: GetModuleHandle() for: \""+File::getSpecialLocation(File::currentExecutableFile).getFullPathName()+"\" failed"));
	}
}

const Result CtrlrWindows::exportWithDefaultPanel(CtrlrPanel* panelToWrite, const bool isRestricted, const bool signPanel)
{
    if (panelToWrite == nullptr)
        return (Result::fail("Windows Native: exportWithDefaultPanel got nullptr for panel"));

    // 1. Setup Variables & Logger
    CtrlrManager& manager = panelToWrite->getOwner();
    File me = File::getSpecialLocation(File::currentExecutableFile);
    PluginLogger logger(me); 
    String fileExtension = me.getFileExtension();
    File newMe;
    MemoryBlock panelExportData, panelResourcesData;

    logger.log("Starting exportWithDefaultPanel. Source extension: " + fileExtension);
    
    // 2. Determine Initial Directory (Sticky Logic)
    String lastPath = manager.getProperty("lastExportPath").toString();
    File targetDir;

    if (lastPath.isNotEmpty() && File(lastPath).isDirectory())
    {
        targetDir = File(lastPath);
    }
    else
    {
        // First time run logic:
        if (fileExtension.equalsIgnoreCase(".vst3") || fileExtension.equalsIgnoreCase(".dll"))
        {
            // JUCE doesn't have a direct "Common Files" enum, so we build it from Program Files
            targetDir = File::getSpecialLocation(File::globalApplicationsDirectory)
                            .getChildFile("Common Files")
                            .getChildFile("VST3");
            
            // Fallback if the path is inaccessible or doesn't exist
            if (!targetDir.exists())
                targetDir = File::getSpecialLocation(File::userDocumentsDirectory);
        }
        else
        {
            targetDir = File::getSpecialLocation(File::userDocumentsDirectory);
        }
    }

    // 3. File Chooser
    String defaultFileName = File::createLegalFileName(panelToWrite->getProperty(Ids::name));
    File defaultFile = targetDir.getChildFile(defaultFileName).withFileExtension(fileExtension);

    FileChooser exportFc (CTRLR_NEW_INSTANCE_DIALOG_TITLE,
                         defaultFile,
                         "*" + fileExtension,
                         manager.getProperty(Ids::ctrlrNativeFileDialogs));

    if (exportFc.browseForFileToSave(true))
    {
        newMe = exportFc.getResult();
        
        // Save sticky path
        manager.setProperty("lastExportPath", newMe.getParentDirectory().getFullPathName());
		// This forces Ctrlr to write the new path to the actual settings file on disk

		// Optional: Persist this to the global XML settings file on disk
        // Disable the following line if you prefer session-only memory.
        manager.saveState();

        if (!newMe.hasFileExtension(fileExtension))
            newMe = newMe.withFileExtension(fileExtension);

        if (!me.copyFileTo(newMe))
        {
            logger.log("Error: Failed to copy executable to " + newMe.getFullPathName());
            return Result::fail("Windows Native: Can't copy binary.");
        }
        logger.log("Executable copied successfully to: " + newMe.getFullPathName());
    }
    else
    {
        logger.log("User cancelled the export dialog.");
        return Result::fail("User cancelled export.");
    }

    // 4. Update Win32 Resources (Panel Injection)
    HANDLE hResource = BeginUpdateResource(newMe.getFullPathName().toUTF8(), FALSE);
    if (hResource)
    {
        String error = CtrlrPanel::exportPanel(panelToWrite, File(), newMe, &panelExportData, &panelResourcesData, isRestricted);
        
        if (error.isEmpty())
        {
            if (writeResource(hResource, MAKEINTRESOURCE(CTRLR_INTERNAL_PANEL_RESID), RT_RCDATA, panelExportData)
                && writeResource(hResource, MAKEINTRESOURCE(CTRLR_INTERNAL_RESOURCES_RESID), RT_RCDATA, panelResourcesData))
            {
                EndUpdateResource(hResource, FALSE);
                logger.log("Resources updated successfully.");
            }
            else
            {
                EndUpdateResource(hResource, TRUE);
                logger.log("Error: writeResource failed.");
                return Result::fail("Windows Native: writeResource failed");
            }
        }
        else
        {
            EndUpdateResource(hResource, TRUE);
            logger.log("Error: exportPanel returned: " + error);
            return Result::fail("Export Error: " + error);
        }
    }
    else
    {
        logger.log("Error: BeginUpdateResource failed. File might be locked.");
        return Result::fail("Windows Native: BeginUpdateResource failed");
    }

    // 5. Binary String Replacement (Rebranding)
    juce::Thread::sleep(500); 
    
    if (newMe.existsAsFile()) 
    {
        if (fileExtension.equalsIgnoreCase(".vst3") || fileExtension.equalsIgnoreCase(".dll")) 
        {
            const bool replaceVst3PluginIds = panelToWrite->getProperty(Ids::panelReplaceVst3PluginIds);

            if (replaceVst3PluginIds) 
            {
                MemoryBlock executableData;
                if (newMe.loadFileAsData(executableData))
                {
                    logger.log("Modifying VST3/DLL Identifiers...");

                    // Replacement blocks
                    MemoryBlock pName, pCode, mName, mCode, pType;
                    hexStringToBytes(panelToWrite->getProperty(Ids::name).toString(), 32, pName);
                    hexStringToBytes(panelToWrite->getProperty(Ids::panelInstanceUID).toString(), 4, pCode);
                    hexStringToBytes(panelToWrite->getProperty(Ids::panelAuthorName).toString(), 16, mName);
                    hexStringToBytes(panelToWrite->getProperty(Ids::panelInstanceManufacturerID).toString(), 4, mCode);
                    hexStringToBytes(panelToWrite->getProperty(Ids::panelPlugType).toString(), 16, pType);

                    // Patterns
                    MemoryBlock sName, sCode, sMName, sMCode, sType;
                    hexStringToBytes("43 74 72 6C 72 58 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20", sName);
                    hexStringToBytes("63 54 72 58", sCode);
                    hexStringToBytes("43 74 72 6C 72 58 20 50 72 6F 6A 65 63 74 20 20", sMName);
                    hexStringToBytes("63 54 72 6C", sMCode);
                    hexStringToBytes("49 6E 73 74 72 75 6D 65 6E 74 7C 54 6F 6F 6C 73", sType);

                    replaceAllOccurrences(executableData, sName, pName);
                    replaceAllOccurrences(executableData, sCode, pCode);
                    replaceAllOccurrences(executableData, sMName, mName);
                    replaceAllOccurrences(executableData, sMCode, mCode);
                    replaceAllOccurrences(executableData, sType, pType);

                    newMe.replaceWithData(executableData.getData(), executableData.getSize());
                    logger.log("Binary identifiers replaced.");
                }
            }
        }

        // 6. Codesigning
        juce::Thread::sleep(500);
        String certPath = panelToWrite->getProperty(Ids::panelCertificateWinPath).toString();
        String certPass = panelToWrite->getProperty(Ids::panelCertificateWinPassCode).toString();
            
        if (certPath.isNotEmpty() && File(certPath).existsAsFile() && certPass.isNotEmpty())
        {
            const Result codesignResult = codesignFileWindows(newMe, certPath, certPass);
            if (!codesignResult.wasOk()) 
            {
                logger.log("Codesigning failed: " + codesignResult.getErrorMessage());
                return codesignResult;
            }
            logger.log("Codesigning successful.");
        }
    }
    
    return Result::ok();
}


// Codesign the exported binary
const Result CtrlrWindows::codesignFileWindows(const File& fileToSign, const String& certificatePath, const String& certificatePassword)
{
	StringArray commandParts;
	commandParts.add("signtool");
	commandParts.add("sign");
	commandParts.add("/f");
	commandParts.add(certificatePath);
	commandParts.add("/p");
	commandParts.add(certificatePassword);
	commandParts.add("/t");
	commandParts.add("http://timestamp.digicert.com"); // Timestamp server
	commandParts.add(fileToSign.getFullPathName());
	
	juce::ChildProcess childProcess;
	if (childProcess.start(commandParts)) {
		childProcess.waitForProcessToFinish(-1);

		if (!childProcess.isRunning()) { // Check if process has finished
			if (childProcess.getExitCode() == 0) {
				return juce::Result::ok(); // Codesign successful

			}
			else {
				return juce::Result::fail("Codesign failed with exit code: " + juce::String(childProcess.getExitCode())); // Codesign failed
			}
		}
		else {
			return juce::Result::fail("Codesign process did not finish properly."); // Process still running
		}

	}
	else {
		return juce::Result::fail("Failed to start codesign process."); // Failed to start process
	}
}


// Convert hex string to binary data
void CtrlrWindows::hexStringToBytes(const String& hexString, MemoryBlock& result) {
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


// Convert hex string to binary data with defined maxLength
void CtrlrWindows::hexStringToBytes(const juce::String& hexString, int maxLength, juce::MemoryBlock& result) {
	result.setSize(0); // Clear the MemoryBlock before use
	juce::String sanitizedHexString = hexString.removeCharacters("\t\r\n"); //use juce::String
	std::vector<uint8> bytes;

	for (int i = 0; i < std::min((int)sanitizedHexString.length(), maxLength); ++i) { // Use std::min
		bytes.push_back(static_cast<uint8>(sanitizedHexString[i]));
	}

	while (bytes.size() < maxLength) {
		bytes.push_back(0); // Pad with zeros
	}

	result.append(bytes.data(), bytes.size());
}


// Convert binary data to hex string
String CtrlrWindows::bytesToHexString(const juce::MemoryBlock& memoryBlock) {
	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	for (size_t i = 0; i < memoryBlock.getSize(); ++i) {
		ss << std::setw(2) << static_cast<int>(static_cast<const uint8*>(memoryBlock.getData())[i]);
	}
	return juce::String(ss.str());
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


const Result CtrlrWindows::getDefaultPanel(MemoryBlock &dataToWrite)
{
#ifdef DEBUG_INSTANCE
	File temp("c:\\devel\\debug_small.bpanelz");
	temp.loadFileAsData (dataToWrite);
	return (Result::ok());
#endif

	return (readResource (nullptr, MAKEINTRESOURCE(CTRLR_INTERNAL_PANEL_RESID), RT_RCDATA, dataToWrite));
}

const Result CtrlrWindows::getDefaultResources(MemoryBlock& dataToWrite)
{
#ifdef DEBUG_INSTANCE
	File temp("c:\\devel\\debug_small.bpanelz");

	MemoryBlock data;
	{
		ScopedPointer <FileInputStream> fis (temp.createInputStream());
		fis->readIntoMemoryBlock (data);
	}

	ValueTree t = ValueTree::readFromGZIPData(data.getData(), data.getSize());

	if (t.isValid())
	{
		ValueTree r = t.getChildWithName (Ids::resourceExportList);
		if (r.isValid())
		{
			MemoryOutputStream mos (dataToWrite, false);
			{
				GZIPCompressorOutputStream gzipOutputStream (&mos);
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

	return (readResource (nullptr, MAKEINTRESOURCE(CTRLR_INTERNAL_RESOURCES_RESID), RT_RCDATA, dataToWrite));
}

const Result CtrlrWindows::registerFileHandler()
{
	if (!JUCEApplication::isStandaloneApp())
		return (Result::ok());

	const char* const exts[] = { ".panel", ".panelz", ".bpanel", ".bpanelz", nullptr };
	StringArray extensions(exts);

	for (int i=0; i<extensions.size(); i++)
	{
		if (!WindowsRegistry::registerFileAssociation (extensions[i], "ctrlr"+extensions[i], "Ctrlr panel file ("+extensions[i]+")", File::getSpecialLocation(File::currentApplicationFile), -1, true))
			return (Result::fail("Can't register ["+extensions[i]+"] file extension"));
	}

	return (Result::ok());
}

static void sendKey(const KeyPress &event)
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

const Result CtrlrWindows::sendKeyPressEvent (const KeyPress &event)
{
	return (sendKeyPressEvent(event, ""));
}

const Result CtrlrWindows::sendKeyPressEvent (const KeyPress &event, const String &targetWindowName)
{
	HWND firstwindow = FindWindowEx(NULL, NULL, NULL, NULL);
    HWND window = firstwindow;
    TCHAR windowtext[MAX_PATH];
	// INPUT input;

	if (targetWindowName != "")
	{
		while(1)
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
