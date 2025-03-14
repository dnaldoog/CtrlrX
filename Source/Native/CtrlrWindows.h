#ifndef __CTRLR_WINDOWS__
#define __CTRLR_WINDOWS__

#include "CtrlrNative.h"
#include <Windows.h>
class CtrlrPanel;

class CtrlrWindows : public CtrlrNative
{
public:
	CtrlrWindows(CtrlrManager& _owner);
	~CtrlrWindows();

	/* resource handling */
	const Result writeResource(void* handle, const LPSTR resourceId, const LPSTR resourceType, const MemoryBlock& resourceData);
	const Result readResource(void* handle, const LPSTR resourceId, const LPSTR resourceType, MemoryBlock& resourceData);
	const Result exportWithDefaultPanel(CtrlrPanel* panelToWrite, const bool isRestricted = false, const bool signPanel = false);
	const Result exportWithDefaultPanel_(CtrlrPanel* panelToWrite, const bool isRestricted = false, const bool signPanel = false);
	const Result getDefaultPanel(MemoryBlock& dataToWrite);
	const Result getDefaultResources(MemoryBlock& dataToWrite);
	const Result getSignature(MemoryBlock& dataToWrite);
	const Result registerFileHandler();
	const Result sendKeyPressEvent(const KeyPress& event);
	const Result sendKeyPressEvent(const KeyPress& event, const String& targetWindowName);
	static void hexStringToBytes(const String& hexString, MemoryBlock& result);
	static void  replaceAllOccurrences(MemoryBlock& targetData, const MemoryBlock& searchData, const MemoryBlock& replaceData);
	static void replaceFirstNOccurrences(MemoryBlock& targetData, const MemoryBlock& searchData, const MemoryBlock& replaceData, int maxOccurrences);
	//static void findPatternAndInsertFilename(MemoryBlock& targetData, const MemoryBlock& searchData, const MemoryBlock& replaceData, int location, int maxOccurrences);
private:
	CtrlrManager& owner;
};
#endif