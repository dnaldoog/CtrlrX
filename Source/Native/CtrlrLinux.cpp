#include "stdafx.h"
#ifdef __linux__

#define PACKAGE "Ctrlr"

#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLinux.h"
#include "CtrlrMacros.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelResource.h"
#include "keys.h"
#include <cstring>
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

static MemoryBlock makeUtf16Buffer(const String &text, const String &templateText) {
	int targetCharCount = templateText.length();
	MemoryBlock block(targetCharCount * 2, true);

	String paddedText =
		text.length() > targetCharCount ? text.substring(0, targetCharCount) : text.paddedRight(' ', targetCharCount);

	const CharPointer_UTF16 utf16Ptr = paddedText.toUTF16();
	block.copyFrom(utf16Ptr.getAddress(), 0, targetCharCount * 2);
	return block;
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

static File getVST3PluginPath() {
	std::ifstream maps("/proc/self/maps");
	std::string line;

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

					if (path.find(".vst3/Contents/") != std::string::npos) {
						_DBG("Detection: Found VST3 path: " + currentFile.getFullPathName());
						return currentFile;
					}

					if (currentFile != hostExe) {
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

	return hostExe;
}

static bool isVST2Plugin() {
	File me = getVST3PluginPath();
	bool hasSOExtension = me.hasFileExtension(".so");
	bool notInVST3 = !me.getFullPathName().contains(".vst3/");
	bool isNotHost = (me != File::getSpecialLocation(File::currentApplicationFile));

	return isNotHost && hasSOExtension && notInVST3;
}

static String sanitizeAppId(const String &name) {
	String clean = name.toLowerCase().retainCharacters("abcdefghijklmnopqrstuvwxyz0123456789-_ ");
	clean = clean.replaceCharacter(' ', '-');
	while (clean.contains("--")) {
		clean = clean.replace("--", "-");
	}
	return clean.trim();
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

	File parentDir = me.getParentDirectory();
	File contentsDir = parentDir.getParentDirectory();
	File bundleDir = contentsDir.getParentDirectory();

	bool isVST3 = bundleDir.getFileName().endsWith(".vst3");
	bool isVST2 = isVST2Plugin();

	_DBG("Export detection: isVST3=" + String((int)isVST3) + ", isVST2=" + String((int)isVST2));
	_DBG("Current binary path: " + me.getFullPathName());

	String panelName = File::createLegalFileName(panelToWrite->getProperty(Ids::name));
	String appIdName = sanitizeAppId(panelToWrite->getProperty(Ids::name).toString());

	// --- PATH AUTO-ROUTING LAMBDA EXECUTION FOR BOTH PLUGINS AND STANDALONES ---
	auto performExportPipeline = [this, panelToWrite, me, isVST3, isVST2, isRestricted, appIdName,
								  notifyAndReturn](const File &chosenFile) {
		File newMe;

		if (isVST3) {
			File bundleDir =
				chosenFile.getFileName().endsWith(".vst3") ? chosenFile : chosenFile.withFileExtension(".vst3");
			File binaryDir = bundleDir.getChildFile("Contents/x86_64-linux");
			String binaryName = bundleDir.getFileNameWithoutExtension() + ".so";
			File binaryFile = binaryDir.getChildFile(binaryName);

			if (!binaryDir.createDirectory() || !me.copyFileTo(binaryFile)) {
				notifyAndReturn(Result::fail("Failed to create VST3 structure or copy binary."));
				return;
			}

			File sourceModuleInfo =
				me.getParentDirectory().getParentDirectory().getChildFile("Resources/moduleinfo.json");
			if (sourceModuleInfo.existsAsFile()) {
				File resourcesDir = bundleDir.getChildFile("Contents/Resources");
				if (resourcesDir.createDirectory()) {
					File destModuleInfo = resourcesDir.getChildFile("moduleinfo.json");
					sourceModuleInfo.copyFileTo(destModuleInfo);
				}
			}
			newMe = binaryFile;
		} else if (isVST2) {
			newMe = chosenFile.hasFileExtension(".so") ? chosenFile : chosenFile.withFileExtension(".so");
			if (!me.copyFileTo(newMe)) {
				notifyAndReturn(Result::fail("Failed to copy VST2 binary."));
				return;
			}
		} else {
			// STANDALONE BUILD: destination depends on
			// uiPanelLinuxExpDest — silently install into the
			// user's system dirs (~/.local/bin), or use whatever location
			// the user picked via the file chooser (classic/portable mode).
			ValueTree editorTreeForInstall = panelToWrite->getPanelTree().getChildWithName(Ids::uiPanelEditor);
			bool exportToUserSystemDirs = editorTreeForInstall.isValid() &&
										  (bool)editorTreeForInstall.getProperty(Ids::uiPanelLinuxExpDest, false);

			if (exportToUserSystemDirs) {
				File localBinDir = File::getSpecialLocation(File::userHomeDirectory).getChildFile(".local/bin");
				localBinDir.createDirectory();
				newMe = localBinDir.getChildFile(appIdName);
			} else {
				newMe = chosenFile;
			}

			if (!me.copyFileTo(newMe)) {
				notifyAndReturn(
					Result::fail("Failed to copy Standalone binary to \"" + newMe.getFullPathName() + "\"."));
				return;
			}
		}

		MemoryBlock panelExportData, panelResourcesData;
		CtrlrPanel p(owner, "", 0);
		String error = p.exportPanel(panelToWrite, File(), newMe, &panelExportData, &panelResourcesData, isRestricted);

		if (error.isNotEmpty()) {
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

				MemoryBlock searchPluginName = hexToBytes(
					"43 74 72 6C 72 58 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20");
				MemoryBlock searchManufacturerCode = hexToBytes("63 54 72 58");
				MemoryBlock searchManufacturerName = hexToBytes("43 74 72 6C 72 58 20 50 72 6F 6A 65 63 74 20 20");
				MemoryBlock searchPluginCode = hexToBytes("63 54 72 6C");
				MemoryBlock searchPlugTypeHex = hexToBytes("49 6E 73 74 72 75 6D 65 6E 74 7C 54 6F 6F 6C 73");

				replaceAllOccurrences(binaryData, searchPluginName, pluginNameBytes);
				replaceAllOccurrences(binaryData, searchPluginCode, pluginCodeBytes);
				replaceAllOccurrences(binaryData, searchManufacturerName, manufacturerNameBytes);
				replaceAllOccurrences(binaryData, searchManufacturerCode, manufacturerCodeBytes);
				replaceAllOccurrences(binaryData, searchPlugTypeHex, plugTypeBytes);

				MemoryBlock searchUtf16ManufName =
					hexToBytes("43 00 74 00 72 00 6C 00 72 00 58 00 20 00 50 00 72 00 6F 00 6A 00 65 00 63 00 74 00");
				MemoryBlock replaceUtf16ManufName = makeUtf16Buffer(manufacturerName, "CtrlrX Project");
				MemoryBlock searchUtf16PluginName = hexToBytes("43 00 74 00 72 00 6C 00 72 00 58 00");
				MemoryBlock replaceUtf16PluginName = makeUtf16Buffer(pluginName, "CtrlrX");

				replaceAllOccurrences(binaryData, searchUtf16ManufName, replaceUtf16ManufName);
				replaceAllOccurrences(binaryData, searchUtf16PluginName, replaceUtf16PluginName);

				newMe.replaceWithData(binaryData.getData(), binaryData.getSize());

				if (isVST3) {
					File moduleInfoFile =
						newMe.getParentDirectory().getParentDirectory().getChildFile("Resources/moduleinfo.json");
					if (moduleInfoFile.existsAsFile()) {
						String moduleInfoText = moduleInfoFile.loadFileAsString();
						auto bytesToHexUpper = [](const MemoryBlock &block) {
							String hex;
							auto *d = static_cast<const uint8 *>(block.getData());
							for (size_t i = 0; i < block.getSize(); ++i)
								hex += String::toHexString((int)d[i]).paddedLeft('0', 2).toUpperCase();
							return hex;
						};
						String newCidSuffix = bytesToHexUpper(manufacturerCodeBytes) + bytesToHexUpper(pluginCodeBytes);
						moduleInfoText = moduleInfoText.replace("635472586354726C", newCidSuffix, true);
						moduleInfoText = moduleInfoText.replace("CtrlrX                          ", pluginName, true);
						moduleInfoText =
							moduleInfoText.replace("CtrlrX Project                  ", manufacturerName, true);
						moduleInfoFile.replaceWithText(moduleInfoText);
					}
				}
			}
		}

		SimpleEmbeddedDataManager dataManager(newMe.getFullPathName().toStdString());
		dataManager.initialize();
		dataManager.writeSection(CTRLR_INTERNAL_PANEL_SECTION, panelExportData);

		if (panelResourcesData.getSize() > 0)
			dataManager.writeSection(CTRLR_INTERNAL_RESOURCES_SECTION, panelResourcesData);

		chmod(newMe.getFullPathName().toUTF8().getAddress(),
			  S_IRUSR | S_IWUSR | S_IXUSR | S_IXOTH | S_IRGRP | S_IXGRP | S_IROTH);

		if (!isVST3 && !isVST2) {
			ValueTree editorTree = panelToWrite->getPanelTree().getChildWithName(Ids::uiPanelEditor);
			bool exportToUserSystemDirs =
				editorTree.isValid() && (bool)editorTree.getProperty(Ids::uiPanelLinuxExpDest, false);
			String iconResourceName =
				editorTree.isValid() ? editorTree.getProperty(Ids::uiPanelIconResource).toString() : String();

			// FIX: the .desktop launcher must always be created — previously
			// this entire block, .desktop file included, only ran when an
			// icon resource was selected. GNOME/Cinnamon/etc. discover
			// launchable apps *only* via .desktop files, so exporting with
			// no icon chosen was silently producing an installed-but-
			// undiscoverable app: correctly copied to ~/.local/bin, but with
			// no launcher entry for any desktop environment to find it by.
			File applicationsDir, desktopDest, iconDir;
			bool isSystemDesktopFile = false;

			if (exportToUserSystemDirs) {
				applicationsDir =
					File::getSpecialLocation(File::userHomeDirectory).getChildFile(".local/share/applications");
				applicationsDir.createDirectory();
				desktopDest = applicationsDir.getChildFile(appIdName + ".desktop");

				iconDir = File::getSpecialLocation(File::userHomeDirectory).getChildFile(".local/share/icons");
				iconDir.createDirectory();
				isSystemDesktopFile = true;
			} else {
				// Classic/portable mode: companion files sit next to the
				// exported binary rather than the user's system dirs, so
				// the whole export stays one movable unit.
				desktopDest = newMe.getSiblingFile(newMe.getFileNameWithoutExtension() + ".desktop");
				iconDir = newMe.getSiblingFile(newMe.getFileNameWithoutExtension() + "-icon");
				iconDir.createDirectory();
			}

			String iconLine; // stays empty — Icon= is omitted — if no icon was generated

			if (iconResourceName.isNotEmpty()) {
				CtrlrPanelResource *iconResource = panelToWrite->getResourceManager().getResource(iconResourceName);

				if (iconResource != nullptr) {
					String iconBaseName =
						exportToUserSystemDirs ? appIdName : (newMe.getFileNameWithoutExtension() + "-icon");
					File svgDest = iconDir.getChildFile(iconBaseName + ".svg");
					File pngDest = iconDir.getChildFile(iconBaseName + ".png");

					if (iconResource->getFile().copyFileTo(svgDest)) {
						if (auto drawable = juce::Drawable::createFromSVGFile(svgDest)) {
							const int size = 256;
							Image pngImage(Image::ARGB, size, size, true, SoftwareImageType());
							Graphics g(pngImage);
							drawable->drawWithin(g, Rectangle<float>(0, 0, (float)size, (float)size),
												 RectanglePlacement::centred, 1.0f);

							PNGImageFormat pngFormat;
							FileOutputStream out(pngDest);
							if (out.openedOk())
								pngFormat.writeImageToStream(pngImage, out);
						}

						iconLine =
							"Icon=" + (pngDest.existsAsFile() ? pngDest.getFullPathName() : svgDest.getFullPathName()) +
							"\n";
					} else {
						_DBG("Warning: failed to copy icon resource for exported instance");
					}
				} else {
					_DBG("Warning: icon resource '" + iconResourceName + "' not found for exported instance");
				}
			}

			String desktopContent = "[Desktop Entry]\n"
									"Type=Application\n"
									"Name=" +
									panelToWrite->getProperty(Ids::name).toString() +
									"\n"
									"Exec=\"" +
									newMe.getFullPathName() + "\"\n" + iconLine +
									"Terminal=false\n"
									"StartupWMClass=" +
									appIdName + "\n";

			desktopDest.replaceWithText(desktopContent);
			chmod(desktopDest.getFullPathName().toUTF8().getAddress(),
				  S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

			if (isSystemDesktopFile) {
				String cmd = "update-desktop-database " + applicationsDir.getFullPathName();
				int dummy = system(cmd.toUTF8().getAddress());
				(void)dummy;
			}

			// --- EXPORT COMPLETE MESSAGE ---
			String completionMessage =
				exportToUserSystemDirs
					? panelToWrite->getProperty(Ids::name).toString() +
						  " installed successfully to ~/.local/bin\n\nIt is now accessible directly from your "
						  "applications launcher menu."
					: panelToWrite->getProperty(Ids::name).toString() + " exported successfully to:\n" +
						  newMe.getFullPathName();

			juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Export Complete", completionMessage,
												   "OK");
		}

		_DBG("Export pipeline successfully executed.");
		notifyAndReturn(Result::ok());
	};

	// --- DETERMINATION RULE: PROMPT IF PLUGIN, EXECUTE SILENTLY IF STANDALONE ---
	if (isVST3 || isVST2) {
		File suggestedFile = isVST3 ? bundleDir.getParentDirectory().getChildFile(panelName + ".vst3")
									: me.getParentDirectory().getChildFile(panelName + ".so");
		String filePattern = isVST3 ? "*.vst3" : "*.so";
		const bool useNativeDialog = panelToWrite->getOwner().getProperty(Ids::ctrlrNativeFileDialogs);

		FC::saveFileAsync(CTRLR_NEW_INSTANCE_DIALOG_TITLE, suggestedFile, filePattern, useNativeDialog,
						  [performExportPipeline, notifyAndReturn](const File &chosenFile) {
							  if (chosenFile == File()) {
								  notifyAndReturn(Result::fail("User cancelled plugin export operation."));
								  return;
							  }
							  performExportPipeline(chosenFile);
						  });
	} else {
		// STANDALONE: dispatch based on uiPanelLinuxExpDest
		ValueTree editorTreeForDispatch = panelToWrite->getPanelTree().getChildWithName(Ids::uiPanelEditor);
		bool exportToUserSystemDirs =
			editorTreeForDispatch.isValid() && (bool)editorTreeForDispatch.getProperty(Ids::uiPanelLinuxExpDest, false);

		if (exportToUserSystemDirs) {
			// Silent, no dialog — installs straight into ~/.local/bin
			performExportPipeline(File());
		} else {
			// Classic behavior: ask the user where to save it, same as VST3/VST2
			File suggestedFile = me.getParentDirectory().getChildFile(panelName);
			const bool useNativeDialog = panelToWrite->getOwner().getProperty(Ids::ctrlrNativeFileDialogs);

			FC::saveFileAsync(CTRLR_NEW_INSTANCE_DIALOG_TITLE, suggestedFile, "*", useNativeDialog,
							  [performExportPipeline, notifyAndReturn](const File &chosenFile) {
								  if (chosenFile == File()) {
									  notifyAndReturn(Result::fail("User cancelled the export operation."));
									  return;
								  }
								  performExportPipeline(chosenFile);
							  });
		}
	}
}

// --- Getter functions ---

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
