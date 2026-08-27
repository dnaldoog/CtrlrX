#ifndef __CTRLR_WINDOWS__
#define __CTRLR_WINDOWS__

#include "CtrlrNative.h"
#include <Windows.h>
#include <fstream> // Added v5.6.32
class CtrlrPanel;

class CtrlrWindows : public CtrlrNative {
	public:
		CtrlrWindows(CtrlrManager &_owner);
		~CtrlrWindows();

		/* resource handling */
		 Result writeResource(void *handle, const LPCWSTR resourceId, const LPCWSTR resourceType,
								   const MemoryBlock &resourceData);
		 Result readResource(void *handle, const LPCWSTR resourceId, const LPCWSTR resourceType,
								  MemoryBlock &resourceData);

		void exportWithDefaultPanel(CtrlrPanel *panelToWrite, const bool isRestricted, const bool signPanel,
									std::function<void(juce::Result)> callback = nullptr) override;

		 Result getDefaultPanel(MemoryBlock &dataToWrite) override;
		 Result getDefaultResources(MemoryBlock &dataToWrite) override;
		 Result registerFileHandler() override;
		 Result sendKeyPressEvent(const KeyPress &event) override;
		 Result sendKeyPressEvent(const KeyPress &event, const String &targetWindowName) override;
		static void replaceAllOccurrences(MemoryBlock &targetData, const MemoryBlock &searchData,
										  const MemoryBlock &replaceData); // Added v5.6.32
		static void replaceFirstNOccurrences(MemoryBlock &targetData, const MemoryBlock &searchData,
											 const MemoryBlock &replaceData, int maxOccurrences); // Added v5.6.32
		static void hexStringToBytes(const String &hexString, MemoryBlock &result);				  // Added v5.6.32
		static void hexStringToBytes(const juce::String &hexString, int maxLength,
									 juce::MemoryBlock &result);	   // Added v5.6.32
		String bytesToHexString(const juce::MemoryBlock &memoryBlock); // Added v5.6.32
		const Result codesignFileWindows(const File &fileToSign, const String &certificatePath,
										 const String &certificatePassword); // Added v5.6.32

	private:
		CtrlrManager &owner;
		void stringToUtf16Bytes(const juce::String& text, int maxBytes, juce::MemoryBlock& result);
		std::unique_ptr<FileChooser> fc; // Added v5.6.31
};

class PluginLogger { // Added v5.6.32
	public:
		PluginLogger(const juce::File &pluginExecutableFile) {
			String fileExt = pluginExecutableFile.getFileExtension();
			if (fileExt == ".exe") {
				logFile = pluginExecutableFile.getParentDirectory().getChildFile("CtrlrX_export_log.txt");
			} else {
				logFile = File::getSpecialLocation(File::userDesktopDirectory)
							  .getChildFile("CtrlrX_export_log.txt"); // VST3 folder is read only unless admin
			}

			if (!logFile.exists()) {
				logFile.create();
			}
		}

		void log(const juce::String &message) {
			std::ofstream outfile(logFile.getFullPathName().toStdString(), std::ios_base::app);
			if (outfile.is_open()) {
				outfile << juce::Time::getCurrentTime().toString(true, true, true, true) << ": "
						<< message.toStdString() << std::endl;
				outfile.close();
			} else {
				std::cerr << "Error: Could not open log file for writing." << std::endl;
			}
		}

		void logResult(const juce::Result &result) {
			if (result.wasOk()) {
				log("Result: OK");
			} else {
				log("Result: FAIL - " + result.getErrorMessage());
			}
		}

	private:
		juce::File logFile;
};

#endif
