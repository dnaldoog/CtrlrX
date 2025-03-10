#ifndef __CTRLR_MAC__
#define __CTRLR_MAC__
#include <fstream> // Added v5.6.32. Required for vst3 logger
#include "CtrlrNative.h"


typedef struct vmtotal vmtotal_t;

class CtrlrMac : public CtrlrNative
{
	public:
		CtrlrMac(CtrlrManager &_owner);
		~CtrlrMac();
		const Result exportWithDefaultPanel(CtrlrPanel*  panelToWrite, const bool isRestricted=false, const bool signPanel=false);
		const Result getDefaultPanel(MemoryBlock &dataToWrite);
		const Result getDefaultResources(MemoryBlock& dataToWrite);
		const Result setBundleInfo (CtrlrPanel *sourceInfo, const File &bundle);
		const Result setBundleInfoCarbon (CtrlrPanel *sourceInfo, const File &bundle);
        static void  replaceAllOccurrences (MemoryBlock& targetData, const MemoryBlock& searchData, const MemoryBlock& replaceData); // Added v5.6.32
        static void replaceFirstNOccurrences (MemoryBlock& targetData, const MemoryBlock& searchData, const MemoryBlock& replaceData, int maxOccurrences); // Added v5.6.32
        static void hexStringToBytes(const String& hexString, MemoryBlock& result); // Added v5.6.32
        static void hexStringToBytes(const juce::String& hexString, int maxLength, juce::MemoryBlock& result); // Added v5.6.32
        String bytesToHexString(const juce::MemoryBlock& memoryBlock); // Added v5.6.32
        const Result codesignFileMac(const juce::String& newMePathName, const juce::String& panelCertificateMacIdentity); // Added v5.6.32
    
    private:
        CtrlrManager &owner;
        std::unique_ptr<FileChooser> fc; // Added v5.6.31
        std::unique_ptr<FileChooser> fileChooser; // Added v5.6.31
        std::unique_ptr<FileChooser> chosenFolder; // Added v5.6.31
};


class PluginLogger { // Added v5.6.32
    public:
        PluginLogger(const juce::File& pluginExecutableFile) {
            logFile = pluginExecutableFile.getParentDirectory().getChildFile("CtrlrX_export_log.txt");
            if (!logFile.exists()) {
                logFile.create();
            }
        }

        void log(const juce::String& message) {
            std::ofstream outfile(logFile.getFullPathName().toStdString(), std::ios_base::app);
            if (outfile.is_open()) {
                outfile << juce::Time::getCurrentTime().toString(true, true, true, true) << ": " << message.toStdString() << std::endl;
                outfile.close();
            } else {
                std::cerr << "Error: Could not open log file for writing." << std::endl;
            }
        }

        void logResult(const juce::Result& result) {
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
