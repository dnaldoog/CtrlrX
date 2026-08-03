#include "stdafx.h"
#ifdef __linux__ // Updated v5.6.35. Thanks to @dnaldoog. SEE:
				 // https://github.com/damiensellier/CtrlrX/pull/193#issuecomment-3561230356
#define PACKAGE "Ctrlr"

#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLinux.h"
#include "CtrlrMacros.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "keys.h"
#include <cstring> // For strlen, memcpy
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

// --- Utility Functions ---

static MemoryBlock hexToBytes(const String &hexString) {
	MemoryBlock result;
	String cleaned = hexString.removeCharacters(" \t\r\n");

	for (int i = 0; i < cleaned.length(); i += 2) {
		if (i + 1 < cleaned.length()) {
			String byteStr = cleaned.substring(i, i + 2);
			uint8 byte = (uint8)byteStr.getHexValue32();
			result.append(&byte, 1);
		}
	}
	return result;
}

static MemoryBlock stringToFixedBytes(const String &str, int fixedSize) {
	MemoryBlock result;
	result.setSize(fixedSize, true);

	const char *chars = str.toUTF8();
	int copySize = jmin(fixedSize, (int)strlen(chars));
	memcpy(result.getData(), chars, copySize);

	return result;
}

static int replaceAllOccurrences(MemoryBlock &target, const MemoryBlock &search, const MemoryBlock &replace) {
	if (search.getSize() != replace.getSize() || search.getSize() == 0) {
		return 0;
	}

	int count = 0;
	const uint8 *data = static_cast<const uint8 *>(target.getData());
	size_t dataSize = target.getSize();
	size_t searchSize = search.getSize();

	for (size_t i = 0; i <= dataSize - searchSize; ++i) {
		if (memcmp(data + i, search.getData(), searchSize) == 0) {
			target.copyFrom(replace.getData(), (int)i, replace.getSize());
			data = static_cast<const uint8 *>(target.getData());
			count++;
		}
	}

	return count;
}

// FIX: Improved detection logic for VST2 .so files on Linux
static File getVST3PluginPath() {
	std::ifstream maps("/proc/self/maps");
	std::string line;

	// Get the path of the host executable (REAPER in this case)
	File hostExe = File::getSpecialLocation(File::currentApplicationFile);

	while (std::getline(maps, line)) {
		if (line.find(".so") != std::string::npos) {

			size_t pathStart = line.find('/');
			if (pathStart != std::string::npos) {
				std::string path = line.substr(pathStart);
				size_t soEnd = path.find(".so");
				if (soEnd != std::string::npos) {
					path = path.substr(0, soEnd + 3);
					File currentFile = File(String(path));

					// 1. VST3 detection (high confidence, return immediately)
					if (path.find(".vst3/Contents/") != std::string::npos) {
						_DBG("Detection: Found VST3 path: " + currentFile.getFullPathName());
						return currentFile;
					}

					// 2. VST2 detection: It must be a loaded .so file and NOT the host executable.
					if (currentFile != hostExe) {
						// Check if the path contains common VST paths or user paths (to exclude system libs like
						// libc.so)
						if (currentFile.getFullPathName().contains("/.vst/") ||
							currentFile.getFullPathName().contains("/vst/") ||
							currentFile.getFullPathName().contains("/plugins/") ||
							currentFile.getFullPathName().contains("CtrlrX.so")) {
							_DBG("Detection: Found VST2 path: " + currentFile.getFullPathName());
							return currentFile;
						}
					}
				}
			}
		}
	}

	// Fallback: Returns the host executable path if no plugin is found (this is the standalone case).
	return hostExe;
}

static bool isVST2Plugin() {
	File me = getVST3PluginPath();
	bool hasSOExtension = me.hasFileExtension(".so");
	bool notInVST3 = !me.getFullPathName().contains(".vst3/");
	// Also explicitly check if the path is NOT the host executable path
	bool isNotHost = (me != File::getSpecialLocation(File::currentApplicationFile));

	return isNotHost && hasSOExtension && notInVST3;
}

// --- SimpleEmbeddedDataManager Class ---
class SimpleEmbeddedDataManager {
	public:
		struct DataSection {
				std::string name;
				size_t offset;
				size_t size;
				bool compressed;
		};

		static const std::string MAGIC_HEADER;
		static const std::string SECTION_DELIMITER;

	private:
		std::vector<DataSection> sections;
		std::string filePath;

		bool findSections(std::ifstream &file) {
			sections.clear();
			file.seekg(0, std::ios::end);
			size_t fileSize = file.tellg();

			size_t searchSize = std::min((size_t)8192, fileSize);
			size_t searchStart = fileSize - searchSize;

			file.seekg(searchStart);
			std::string buffer(searchSize, '\0');
			file.read(&buffer[0], searchSize);

			size_t headerPos = buffer.rfind(MAGIC_HEADER);

			if (headerPos != std::string::npos) {
				size_t absolutePos = searchStart + headerPos;
				return parseSections(file, absolutePos + MAGIC_HEADER.length());
			}

			return false;
		}

		bool parseSections(std::ifstream &file, size_t startPos) {
			file.seekg(startPos);
			std::string line;

			while (std::getline(file, line)) {
				if (line == SECTION_DELIMITER)
					break;

				std::istringstream iss(line);
				std::string name, offsetStr, sizeStr, compressedStr;

				if (std::getline(iss, name, ':') && std::getline(iss, offsetStr, ':') &&
					std::getline(iss, sizeStr, ':') && std::getline(iss, compressedStr)) {

					if (name.empty() || offsetStr.empty() || sizeStr.empty() || compressedStr.empty()) {
						continue;
					}

					try {
						DataSection section;
						section.name = name;
						section.offset = std::stoull(offsetStr);
						section.size = std::stoull(sizeStr);
						section.compressed = (compressedStr == "1");
						sections.push_back(section);
					} catch (const std::exception &) {
						continue;
					}
				}
			}

			return !sections.empty();
		}

	public:
		SimpleEmbeddedDataManager(const std::string &path) : filePath(path) {}

		bool initialize() {
			std::ifstream file(filePath, std::ios::binary);
			if (!file.is_open())
				return false;
			return findSections(file);
		}

		bool readSection(const std::string &sectionName, MemoryBlock &output) {
			std::ifstream file(filePath, std::ios::binary);
			if (!file.is_open())
				return false;

			for (const auto &section : sections) {
				if (section.name == sectionName) {
					file.seekg(section.offset);

					MemoryBlock rawData;
					rawData.setSize(section.size, false);
					file.read(static_cast<char *>(rawData.getData()), section.size);

					output = rawData;
					return true;
				}
			}
			return false;
		}

		bool writeSection(const std::string &sectionName, const MemoryBlock &data) {
			std::ifstream originalFile(filePath, std::ios::binary);
			if (!originalFile.is_open())
				return false;

			originalFile.seekg(0, std::ios::end);
			size_t originalSize = originalFile.tellg();
			originalFile.seekg(0, std::ios::beg);

			MemoryBlock originalData;
			originalData.setSize(originalSize, false);
			originalFile.read(static_cast<char *>(originalData.getData()), originalSize);
			originalFile.close();

			size_t dataOffset = originalSize;
			bool sectionExists = false;

			for (auto &section : sections) {
				if (section.name == sectionName) {
					section.offset = dataOffset;
					section.size = data.getSize();
					section.compressed = false;
					sectionExists = true;
					break;
				}
			}

			if (!sectionExists) {
				DataSection newSection;
				newSection.name = sectionName;
				newSection.offset = dataOffset;
				newSection.size = data.getSize();
				newSection.compressed = false;
				sections.push_back(newSection);
			}

			std::string tempPath = filePath + ".tmp";
			std::ofstream newFile(tempPath, std::ios::binary);
			if (!newFile.is_open())
				return false;

			newFile.write(static_cast<const char *>(originalData.getData()), originalData.getSize());
			newFile.write(static_cast<const char *>(data.getData()), data.getSize());

			newFile << MAGIC_HEADER;
			for (const auto &section : sections) {
				newFile << section.name << ":" << section.offset << ":" << section.size << ":"
						<< (section.compressed ? "1" : "0") << "\n";
			}
			newFile << SECTION_DELIMITER << "\n";

			newFile.close();

			return (rename(tempPath.c_str(), filePath.c_str()) == 0);
		}
};

const std::string SimpleEmbeddedDataManager::MAGIC_HEADER = "\n\n__CTRLR_EMBEDDED_DATA_V2__\n";
const std::string SimpleEmbeddedDataManager::SECTION_DELIMITER = "__END_SECTIONS__";

// --- CtrlrLinux Implementation ---

CtrlrLinux::CtrlrLinux(CtrlrManager &_owner) : owner(_owner) {}
CtrlrLinux::~CtrlrLinux() {}

void CtrlrLinux::exportWithDefaultPanel(CtrlrPanel *panelToWrite, const bool isRestricted, const bool signPanel,
										std::function<void(juce::Result)> callback) {

	auto notifyAndReturn = [callback](const juce::Result &res) {
		if (callback)
			callback(res);
	};

	if (panelToWrite == nullptr) {
		notifyAndReturn(Result::fail("Linux native, panel pointer is invalid"));
		return;
	}

	File me = getVST3PluginPath();

	// Check if the current binary is running as VST3 or VST2/Standalone
	File parentDir = me.getParentDirectory();
	File contentsDir = parentDir.getParentDirectory();
	File bundleDir = contentsDir.getParentDirectory();

	bool isVST3 = bundleDir.getFileName().endsWith(".vst3");
	bool isVST2 = isVST2Plugin();

	_DBG("Export detection: isVST3=" + String((int)isVST3) + ", isVST2=" + String((int)isVST2));
	_DBG("Current binary path: " + me.getFullPathName());

	String panelName = File::createLegalFileName(panelToWrite->getProperty(Ids::name));

	File suggestedFile;
	String filePattern;

	if (isVST3) {
		suggestedFile = bundleDir.getParentDirectory().getChildFile(panelName + ".vst3");
		filePattern = "*.vst3";
	} else if (isVST2) {
		suggestedFile = me.getParentDirectory().getChildFile(panelName + ".so");
		filePattern = "*.so";
		_DBG("VST2 export detected. suggestedFile = " + suggestedFile.getFullPathName());
	} else {
		suggestedFile = me.getParentDirectory().getChildFile(panelName);
		filePattern = "*";
		_DBG("Standalone export detected. suggestedFile = " + suggestedFile.getFullPathName());
	}

	const bool useNativeDialog = panelToWrite->getOwner().getProperty(Ids::ctrlrNativeFileDialogs);

	FC::saveFileAsync(
		CTRLR_NEW_INSTANCE_DIALOG_TITLE, suggestedFile, filePattern, useNativeDialog,
		[this, panelToWrite, me, isVST3, isVST2, isRestricted, notifyAndReturn](const File &chosenFile) {
			if (chosenFile == File()) {
				_DBG("User cancelled the export operation.");
				notifyAndReturn(Result::fail("User cancelled the export operation."));
				return;
			}

			_DBG("FileChooser returned: " + chosenFile.getFullPathName());
			File newMe;

			if (isVST3) {
				File bundleDir =
					chosenFile.getFileName().endsWith(".vst3") ? chosenFile : chosenFile.withFileExtension(".vst3");

				File binaryDir = bundleDir.getChildFile("Contents/x86_64-linux");
				String binaryName = bundleDir.getFileNameWithoutExtension() + ".so";
				File binaryFile = binaryDir.getChildFile(binaryName);

				if (!binaryDir.createDirectory()) {
					_DBG("Failed to create VST3 bundle directory structure");
					notifyAndReturn(Result::fail("Failed to create VST3 bundle directory structure"));
					return;
				}

				if (!me.copyFileTo(binaryFile)) {
					_DBG("Linux native, VST3 copyFileTo failed");
					notifyAndReturn(Result::fail("Linux native, VST3 copyFileTo failed"));
					return;
				}

				newMe = binaryFile;
			} else {
				newMe = chosenFile;

				if (isVST2) {
					if (!newMe.getFullPathName().endsWith(".so")) {
						newMe = newMe.withFileExtension(".so");
						_DBG("VST2 export: Added missing .so extension: " + newMe.getFullPathName());
					}
				} else {
					_DBG("Standalone export: Final file created without extension = " + newMe.getFullPathName());
				}

				if (!me.copyFileTo(newMe)) {
					_DBG("Linux native, Standalone/VST2 copyFileTo failed");
					notifyAndReturn(Result::fail("Linux native, Standalone/VST2 copyFileTo failed"));
					return;
				}
			}

			MemoryBlock panelExportData, panelResourcesData;
			CtrlrPanel p(owner, "", 0);
			String error =
				p.exportPanel(panelToWrite, File(), newMe, &panelExportData, &panelResourcesData, isRestricted);

			if (error.isNotEmpty()) {
				_DBG("CtrlrPanel::exportPanel failed: " + error);
				notifyAndReturn(Result::fail("CtrlrPanel::exportPanel failed: " + error));
				return;
			}

			if (isVST3 || isVST2) {
				MemoryBlock binaryData;
				if (newMe.loadFileAsData(binaryData)) {
					String pluginName = panelToWrite->getProperty(Ids::name).toString();
					String pluginCode = panelToWrite->getProperty(Ids::panelInstanceUID).toString();
					String manufacturerName = panelToWrite->getProperty(Ids::panelAuthorName).toString();
					String manufacturerCode = panelToWrite->getProperty(Ids::panelInstanceManufacturerID).toString();
					String plugType = panelToWrite->getProperty(Ids::panelPlugType).toString();

					MemoryBlock pluginNameBytes = stringToFixedBytes(pluginName, 32);
					MemoryBlock pluginCodeBytes = stringToFixedBytes(pluginCode, 4);
					MemoryBlock manufacturerNameBytes = stringToFixedBytes(manufacturerName, 16);
					MemoryBlock manufacturerCodeBytes = stringToFixedBytes(manufacturerCode, 4);
					MemoryBlock plugTypeBytes = stringToFixedBytes(plugType, 16);

					MemoryBlock searchPluginName = hexToBytes("43 74 72 6C 72 58 20 20 20 20 20 20 20 20 20 20 20 20 "
															  "20 20 20 20 20 20 20 20 20 20 20 20 20 20");
					MemoryBlock searchPluginCode = hexToBytes("63 54 78 58");
					MemoryBlock searchManufacturerName = hexToBytes("43 74 72 6C 72 58 20 50 72 6F 6A 65 63 74 20 20");
					MemoryBlock searchManufacturerCode = hexToBytes("63 54 72 6C");
					MemoryBlock searchPlugTypeHex = hexToBytes("49 6E 73 74 72 75 6D 65 6E 74 7C 54 6F 6F 6C 73");

					int totalReplacements = 0;
					totalReplacements += replaceAllOccurrences(binaryData, searchPluginName, pluginNameBytes);
					totalReplacements += replaceAllOccurrences(binaryData, searchPluginCode, pluginCodeBytes);
					totalReplacements +=
						replaceAllOccurrences(binaryData, searchManufacturerName, manufacturerNameBytes);
					totalReplacements +=
						replaceAllOccurrences(binaryData, searchManufacturerCode, manufacturerCodeBytes);
					totalReplacements += replaceAllOccurrences(binaryData, searchPlugTypeHex, plugTypeBytes);

					_DBG("Binary patching complete: " + String(totalReplacements) + " replacements");

					if (!newMe.replaceWithData(binaryData.getData(), binaryData.getSize())) {
						_DBG("Failed to write patched binary");
						notifyAndReturn(Result::fail("Failed to write patched binary"));
						return;
					}
				}
			}

			SimpleEmbeddedDataManager dataManager(newMe.getFullPathName().toStdString());
			dataManager.initialize();

			if (!dataManager.writeSection(CTRLR_INTERNAL_PANEL_SECTION, panelExportData)) {
				_DBG("Failed to write panel data");
				notifyAndReturn(Result::fail("Failed to write panel data"));
				return;
			}

			if (panelResourcesData.getSize() > 0) {
				if (!dataManager.writeSection(CTRLR_INTERNAL_RESOURCES_SECTION, panelResourcesData)) {
					_DBG("Failed to write resources");
					notifyAndReturn(Result::fail("Failed to write resources"));
					return;
				}
			}

			if (!isVST3) {
				if (chmod(newMe.getFullPathName().toUTF8().getAddress(),
						  S_IRUSR | S_IWUSR | S_IXUSR | S_IXOTH | S_IRGRP | S_IXGRP | S_IROTH)) {
					_DBG("chmod failed");
					notifyAndReturn(Result::fail("chmod failed"));
					return;
				}
			}

			_DBG("Export succeeded for: " + newMe.getFullPathName());
			notifyAndReturn(Result::ok());
		});
}
// --- Getter functions (unchanged logic) ---

Result CtrlrLinux::getDefaultPanel(MemoryBlock &dataToWrite) {
#ifdef DEBUG_INSTANCE
	File temp("/home/r.kubiak/devel/debug.bpanelz");
	temp.loadFileAsData(dataToWrite);
	return Result::ok();
#endif

	File pluginBinary = getVST3PluginPath();
	SimpleEmbeddedDataManager dataManager(pluginBinary.getFullPathName().toStdString());

	if (dataManager.initialize() && dataManager.readSection(CTRLR_INTERNAL_PANEL_SECTION, dataToWrite)) {
		return Result::ok();
	}

	return Result::fail("Failed to retrieve panel data");
}

Result CtrlrLinux::getDefaultResources(MemoryBlock &dataToWrite) {
	File pluginBinary = getVST3PluginPath();
	SimpleEmbeddedDataManager dataManager(pluginBinary.getFullPathName().toStdString());

	if (dataManager.initialize() && dataManager.readSection(CTRLR_INTERNAL_RESOURCES_SECTION, dataToWrite)) {
		return Result::ok();
	}

	return Result::fail("Failed to retrieve resources");
}

Result CtrlrLinux::sendKeyPressEvent(const KeyPress &event) { return ctrlr_sendKeyPressEvent(event); }
Result CtrlrLinux::sendKeyPressEvent(const KeyPress &event, const String &targetWindowName) {
    if (targetWindowName.isNotEmpty()) {
        _DBG("Linux native: sendKeyPressEvent with a target window name is not yet implemented; "
             "sending to the currently focused window instead.");
    }
    return ctrlr_sendKeyPressEvent(event);
}
#endif
