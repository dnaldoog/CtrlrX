#include "CtrlrPanelResourceManager.h"
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLog.h"
#include "CtrlrMacros.h"
#include "CtrlrPanel.h"
#include "CtrlrPanelResource.h"
#include "CtrlrProcessor.h"
#include "CtrlrUtilities.h"
#include "stdafx.h"
#include "stdafx_luabind.h"

/** Resource Manager class implementation

*/

CtrlrPanelResourceManager::CtrlrPanelResourceManager(CtrlrPanel &_owner)
	: owner(_owner), lastLoadedResource(nullptr), managerTree(Ids::panelResources) {}

CtrlrPanelResourceManager::~CtrlrPanelResourceManager() {}

void CtrlrPanelResourceManager::panelUIDChanged() { initManager(); }

void CtrlrPanelResourceManager::restoreSavedState(const ValueTree &savedState) {
	for (int i = 0; i < savedState.getNumChildren(); i++) {
		if (savedState.getChild(i).hasType(Ids::resource)) {
			CtrlrPanelResource *res = getResource(savedState.getChild(i).getProperty(Ids::resourceName).toString());
			if (res) {
				res->setProperty(Ids::resourceSourceFile, savedState.getChild(i).getProperty(Ids::resourceSourceFile));
			}
		}
	}
}

void CtrlrPanelResourceManager::initManager() {
	resources.clear();
	resourceHashCodes.clear();
	lastLoadedResource = nullptr;

	const File newResourcesDirectory = owner.getPanelDirectory();

	// Check if the directory has migrated
	if (newResourcesDirectory != resourcesDirectory && resourcesDirectory != File()) {
		if (resourcesDirectory.getNumberOfChildFiles(File::findFiles) == 0) {
			resourcesDirectory.deleteRecursively();

			// Move directly to setting up the new directory
			resourcesDirectory = newResourcesDirectory;
		} else {
			const File oldDirectoryToDelete = resourcesDirectory;

			// Capture only the variables we need by value/reference cleanly
			AW::showNativeDialogBox(
				AW::Question, "Are you sure?", "The resource directory has changed, do you want to delete the old one?",
				"Yes", "No", true, [this, oldDirectoryToDelete, newResourcesDirectory](bool userClickedYes) mutable {
					if (userClickedYes) {
						oldDirectoryToDelete.deleteRecursively();
					}

					// Update state and scan files inside the callback
					this->resourcesDirectory = newResourcesDirectory;
					if (this->resourcesDirectory.isDirectory()) {
						Array<File> resourceFiles;
						this->resourcesDirectory.findChildFiles(resourceFiles, File::findFiles, false);
						for (int i = 0; i < resourceFiles.size(); i++) {
							this->addResource(resourceFiles[i]);
						}
					}
				});

			// Return early! The async callback above handles the rest of the setup
			return;
		}
	} else {
		// Normal startup path: point to the directory
		resourcesDirectory = newResourcesDirectory;
	}

	// This section handles normal startup or the immediate empty-directory deletion path
	if (!resourcesDirectory.isDirectory()) {
		if (!resourcesDirectory.createDirectory()) {
			_ERR("CtrlrResourceManager::ctor failed to create resources directory");
			resourcesDirectory = File();
		}
	} else {
		Array<File> resourceFiles;
		resourcesDirectory.findChildFiles(resourceFiles, File::findFiles, false);
		for (int i = 0; i < resourceFiles.size(); i++) {
			addResource(resourceFiles[i]);
		}
	}
}

void CtrlrPanelResourceManager::checkMissingResources(
	ValueTree &panelResourcesTree) { // Check missing resources from ValueTree
	ValueTree currentResource;
	String resourceName;
	for (int i = 0; i < panelResourcesTree.getNumChildren(); i++) {
		currentResource = panelResourcesTree.getChild(i);
		if (currentResource.hasType(Ids::resource)) {
			resourceName = currentResource.getProperty(Ids::resourceName).toString();
			CtrlrPanelResource *res = getResource(resourceName);
			if (!res) { // Resource not find in resources directory => try and load it from the source file
				String resourceSourcePath = currentResource.getProperty(Ids::resourceSourceFile);
				File resourceFile;
				if (File::isAbsolutePath(resourceSourcePath)) {
					resourceFile = File(resourceSourcePath);
				} else {
					resourceFile = owner.getPanelResourcesDir().getChildFile(resourceSourcePath);
				}
				if (resourceFile.existsAsFile()) {
					addResource(resourceFile, resourceName);
				}
			}
		}
	}
}

int CtrlrPanelResourceManager::getNumResources() { return (resources.size()); }

CtrlrPanelResource *CtrlrPanelResourceManager::getResource(const int resourceIndex) {
	return (resources[resourceIndex]);
}

CtrlrPanelResource *CtrlrPanelResourceManager::getResource(const String &resourceName) {
	if (lastLoadedResource.get() && !lastLoadedResource.wasObjectDeleted())
		if (lastLoadedResource->getName() == resourceName)
			return (lastLoadedResource);

	for (int i = 0; i < resources.size(); i++) {
		if (resources[i]->getName() == resourceName) {
			lastLoadedResource = resources[i];
			return (resources[i]);
		}
	}
	return (0);
}

const Image CtrlrPanelResourceManager::getResourceAsImage(const String &resourceName) {
	CtrlrPanelResource *res = getResource(resourceName);
	if (res != 0) {
		return (res->asImage());
	}

	return (Image());
}

const Font CtrlrPanelResourceManager::getResourceAsFont(const String &resourceName) {
	CtrlrPanelResource *res = getResource(resourceName);
	if (res != 0) {
		return (res->asFont());
	}

	return (Font());
}

void CtrlrPanelResourceManager::reloadComboContents(ComboBox &comboToUpdate) {
	const String lastSelected = comboToUpdate.getText();
	int newSelectedId = -1;
	int i = 0;

	comboToUpdate.clear();

	for (i = 0; i < resources.size(); i++) {
		comboToUpdate.addItem(resources[i]->getName(), i + 1);
		if (lastSelected == resources[i]->getName()) {
			newSelectedId = i + 1;
		}
	}

	comboToUpdate.addItem(COMBO_NONE_ITEM, i + 1);

	comboToUpdate.setSelectedId(newSelectedId, dontSendNotification);
}

CtrlrPanel &CtrlrPanelResourceManager::getOwner() { return (owner); }

int CtrlrPanelResourceManager::getHashCode(const String &resourceName, const bool preloadResource) {
	CtrlrPanelResource *r = getResource(resourceName);

	if (r) {
		if (preloadResource)
			r->load();

		return (r->getHashCode());
	}

	return (-1);
}

int CtrlrPanelResourceManager::getResourceIndex(const String &resourceName) {
	for (int i = 0; i < resources.size(); i++) {
		if (resources[i]->getName() == resourceName)
			return (i);
	}
	return (-1);
}

bool CtrlrPanelResourceManager::resourceExists(const File &resourceFile) {
	if (getResource(resourceFile.getFileNameWithoutExtension())) {
		return (true);
	}

	return (false);
}

Result CtrlrPanelResourceManager::importResource(const ValueTree &resourceTree) {
	String resourceName = resourceTree.getProperty(Ids::resourceName).toString();

	// Decode incoming data upfront
	MemoryBlock resourceData;
	if (!resourceData.fromBase64Encoding(resourceTree.getProperty(Ids::resourceData).toString())) {
		return Result::fail("ImportResource resource: " + resourceName + " failed to decode base64 encoded data");
	}

	// Safely resolve destination directory using the owner panel
	File resDir = owner.getPanelDirectory();
	if (!resDir.isDirectory()) {
		resDir = owner.getPanelResourcesDir();
	}

	if (!resDir.isDirectory()) {
		return Result::fail("Import resource failed, resource directory does not exist: " + resDir.getFullPathName());
	}

	String filename = resourceTree.getProperty(Ids::resourceFile).toString();
	File resourceDest = resDir.getChildFile(File::createLegalFileName(filename));

	// --- Path A: Resource already exists internally ---
	if (auto *existingResource = getResource(resourceName)) {
		if (!(bool)owner.getCtrlrManagerOwner().getProperty(Ids::ctrlrOverwriteResources)) {
			return Result::fail("ImportResource resource: " + resourceName +
								" failed, a resource with this name already exists");
		}

		// Check content hashes to avoid unnecessary disk writes
		int64 incomingHash = (int64)resourceTree.getProperty(Ids::resourceHash);
		int64 cachedHash = existingResource->getHashCode();

		if (incomingHash != 0 && cachedHash != 0 && incomingHash == cachedHash) {
			_DBG("importResource: [" + resourceName + "] hash match, skipping overwrite");
			return Result::ok();
		}

		_DBG("importResource: [" + resourceName + "] hash changed, overwriting");

		if (!resourceDest.replaceWithData(resourceData.getData(), (int)resourceData.getSize())) {
			return Result::fail("ImportResource can't replace file contents, resource: " +
								resourceDest.getFullPathName());
		}

		_DBG("importResource: [" + resourceName + "] overwritten on disk OK: " + resourceDest.getFullPathName());

		// Refresh internal cache tracking
		existingResource->reloadFromSourceFile();
		return Result::ok();
	}

	// --- Path B: Fresh import ---
	_DBG("importResource: [" + resourceName + "] fresh import starting");

	if (!resourceDest.replaceWithData(resourceData.getData(), (int)resourceData.getSize())) {
		return Result::fail("ImportResource can't write fresh file data, resource: " + resourceDest.getFullPathName());
	}

	_DBG("importResource: [" + resourceName + "] fresh import OK: " + resourceDest.getFullPathName());

	// Pass source file to addResource (which will detect source == resourceDest and register it safely)
	return addResource(resourceDest, resourceTree.getProperty(Ids::resourceName));
}

Result CtrlrPanelResourceManager::addResource(const File &source, const String &name) {
	File resDir = owner.getPanelDirectory();
	if (!resDir.isDirectory()) {
		resDir = owner.getPanelResourcesDir();
	}

	if (!resDir.isDirectory()) {
		return Result::fail("Can't copy resource, destination directory does not exist: " + resDir.getFullPathName());
	}

	File resourceDest = resDir.getChildFile(source.getFileName());

	// Check if resource already exists internally
	if (auto *existingResource = getResource(source.getFileNameWithoutExtension())) {
		if (!(bool)owner.getCtrlrManagerOwner().getProperty(Ids::ctrlrOverwriteResources)) {
			return Result::fail("This resource already exists");
		}

		// Prevent file corruption when copying onto itself
		if (source != resourceDest) {
			if (!source.copyFileTo(resourceDest)) {
				return Result::fail("Failed to overwrite file in resources directory: " +
									resourceDest.getFullPathName());
			}
		}

		existingResource->reloadFromSourceFile();
		owner.panelResourcesChanged();
		return Result::ok();
	}

	// Copy file if it's external
	if (source != resourceDest) {
		if (!source.copyFileTo(resourceDest)) {
			return Result::fail("Failed to copy file to resources directory: " + resourceDest.getFullPathName());
		}
	}

	// Allocate new object and add to management tracking structures
	auto *newResource = new CtrlrPanelResource(*this, resourceDest, source, name);
	resources.add(newResource);
	resourceHashCodes.add(newResource->getHashCode());
	managerTree.addChild(newResource->getResourceTree(), -1, nullptr);

	owner.panelResourcesChanged();
	return Result::ok();
}

Result CtrlrPanelResourceManager::removeResource(CtrlrPanelResource *resourceToRemove) {
	if (resourceToRemove == nullptr)
		return Result::fail("Cannot remove null resource");

	return removeResource(resources.indexOf(resourceToRemove));
}

Result CtrlrPanelResourceManager::removeResource(const int resourceIndex) {
	// Array bounds protection
	if (!isPositiveAndBelow(resourceIndex, resources.size())) {
		return Result::fail("Unable to remove resource with invalid index: " + String(resourceIndex));
	}

	// Store local pointer before modifying containers
	CtrlrPanelResource *res = resources[resourceIndex];

	if (res != nullptr) {
		resourceHashCodes.removeAllInstancesOf(res->getHashCode());

		File targetFile = res->getFile();
		if (targetFile.exists() && !targetFile.deleteFile()) {
			return Result::fail("Removing resource partially failed, can't delete resource file: " +
								targetFile.getFullPathName());
		}

		managerTree.removeChild(res->getResourceTree(), nullptr);

		// true = OwnedArray deletes the object allocation safely from heap
		resources.remove(resourceIndex, true);

		owner.panelResourcesChanged();
		return Result::ok();
	}

	return Result::fail("Unable to remove resource at index: " + String(resourceIndex));
}

Result CtrlrPanelResourceManager::removeResourceRange(const int resourceIndexStart,
													  const int numberOfResourcesToRemove) {
	for (int i = resourceIndexStart; i < resourceIndexStart + numberOfResourcesToRemove; i++) {
		if (resources[i]) {
			Result ret = removeResource(i);

			if (!ret.wasOk()) {
				return (ret);
			}
		}
	}

	return (Result::ok());
}

void CtrlrPanelResourceManager::wrapForLua(lua_State *L) {
	using namespace luabind;

	module(L)[class_<CtrlrPanelResource>("CtrlrPanelResource")
				  .def("asImage", &CtrlrPanelResource::asImage)
				  .def("asText", &CtrlrPanelResource::asText)
				  .def("asFont", &CtrlrPanelResource::asFont)
				  .def("asXml", &CtrlrPanelResource::asXml)
				  .def("asAudioFormat", &CtrlrPanelResource::asAudioFormat)
				  .def("asData", &CtrlrPanelResource::asData)
				  .def("getName", &CtrlrPanelResource::getName)
				  .def("getSize", (double (CtrlrPanelResource::*)())&CtrlrPanelResource::
									  getSize) // Updated v5.5.35. Uncommented for :
											   // https://github.com/damiensellier/CtrlrX/issues/192
				  .def("getSizeDouble", &CtrlrPanelResource::getSizeDouble) // Added v5.6.34.
				  .def("getHashCode", &CtrlrPanelResource::getHashCode)
				  .def("load", &CtrlrPanelResource::load)
				  .def("loadIfNeeded", &CtrlrPanelResource::loadIfNeeded)
				  .def("getType", &CtrlrPanelResource::getType)
				  .def("getTypeDescription", &CtrlrPanelResource::getTypeDescription)
				  .def("getFile", &CtrlrPanelResource::getFile)
				  .def("createInputStream", &CtrlrPanelResource::createInputStream) // Added v5.6.34. gzip support
				  .def("asGzipText", &CtrlrPanelResource::asGzipText)				//  Added v5.6.34. gzip support
			  ,
			  class_<CtrlrPanelResourceManager>("CtrlrPanelResourceManager")
				  .def("getResource",
					   (CtrlrPanelResource * (CtrlrPanelResourceManager::*)(const int)) &
						   CtrlrPanelResourceManager::getResource,
					   dependency(result, _1))
				  .def("getResource",
					   (CtrlrPanelResource * (CtrlrPanelResourceManager::*)(const String &)) &
						   CtrlrPanelResourceManager::getResource,
					   dependency(result, _1))
				  //.def("getResource", (CtrlrPanelResource *(CtrlrPanelResourceManager::*)(const
				  // int))&CtrlrPanelResourceManager::getResource) .def("getResource", (CtrlrPanelResource
				  //*(CtrlrPanelResourceManager::*)(const String &))&CtrlrPanelResourceManager::getResource)
				  .def("getNumResources", &CtrlrPanelResourceManager::getNumResources)
				  .def("getResourceIndex", &CtrlrPanelResourceManager::getResourceIndex)
				  .def("getResourceAsImage", &CtrlrPanelResourceManager::getResourceAsImage)
				  .def("getResourceAsFont", &CtrlrPanelResourceManager::getResourceAsFont)];
}

const String CtrlrPanelResourceManager::getTypeDescription(const CtrlrPanelResourceType type) {
	switch (type) {
	case ImageRes:
		return ("Image");
	case AudioRes:
		return ("Audio");
	case FontRes:
		return ("Font");
	case TextRes:
		return ("Text");
	case XmlRes:
		return ("Xml");
	case DataRes:
	default:
		return ("Data");
	}
}

CtrlrPanelResourceType CtrlrPanelResourceManager::guessType(const File &resourceFile) {
	// Image ?
	Image image = ImageCache::getFromFile(resourceFile);

	if (!image.isNull()) {
		return (ImageRes);
	}

	// Audio ?
	std::unique_ptr<AudioFormatReader> audio(
		owner.getCtrlrManagerOwner().getAudioFormatManager().createReaderFor(resourceFile));

	if (audio != nullptr) {
		return (AudioRes);
	}

	// Font ?
	if (resourceFile.hasFileExtension("ttf") || resourceFile.hasFileExtension("otf")) {
		return (FontRes);
	}

	// XML ?
	std::unique_ptr<XmlElement> xml(XmlDocument::parse(resourceFile).release());

	if (xml != nullptr) {
		return (XmlRes);
	}

	// Text ?

	if (resourceFile.hasFileExtension("txt")) {
		return (TextRes);
	}

	return (DataRes);
}

int CtrlrPanelResourceManager::getResourceHashCode(const int resourceIndex) {
	if (resources[resourceIndex]) {
		return (resources[resourceIndex]->getHashCode());
	}

	return (-1);
}

int CtrlrPanelResourceManager::getResourceIndexByHashCode(const int hashCode) {
	for (int i = 0; i < resources.size(); i++) {
		if (resources[i]) {
			if (resources[i]->getHashCode() == hashCode)
				return (i);
		}
	}

	return (-1);
}

Result CtrlrPanelResourceManager::restoreState(const ValueTree &savedState,
											   std::function<void(Result)> completionCallback) {
	for (int i = 0; i < savedState.getNumChildren(); i++) {
		if (savedState.getChild(i).hasType(Ids::resourceLicense)) {

			// Allocate the layout elements safely on the heap so they survive the async window lifetime
			auto *licenseWindow =
				new AlertWindow("License agreement", "You must agree to the below license", AlertWindow::QuestionIcon);

			auto *licenseText = new TextEditor();
			licenseText->setMultiLine(true);
			licenseText->setReadOnly(true);
			licenseText->setText(savedState.getChild(i).getProperty(Ids::resourceData));
			licenseText->setSize(500, 400);

			licenseWindow->addCustomComponent(licenseText);
			licenseWindow->addButton("Yes", 1);
			licenseWindow->addButton("No", 0);

#if JUCE_VERSION < 0x070000
			// --- Legacy JUCE 6 Path (Synchronous) ---
			bool accepted = (licenseWindow->runModalLoop() == 1);
			delete licenseWindow; // Cleans up custom components automatically

			if (!accepted) {
				Result failRes = Result::fail("User did not agree to embedded license");
				if (completionCallback)
					completionCallback(failRes);
				return failRes;
			}
#else
			// --- Modern JUCE 7/8 Path (Asynchronous) ---
			licenseWindow->enterModalState(
				true, ModalCallbackFunction::create([licenseWindow, completionCallback](int result) {
					bool accepted = (result == 1);
					delete licenseWindow; // Clean up memory allocation from heap

					if (!accepted) {
						if (completionCallback)
							completionCallback(Result::fail("User did not agree to embedded license"));
					} else {
						if (completionCallback)
							completionCallback(Result::ok());
					}
				}),
				true);

			// Return a pending status immediately so the caller knows it is waiting on UI interaction
			return Result::ok();
#endif
		}
	}

	// No license found, or processed synchronously
	Result okRes = Result::ok();
	if (completionCallback)
		completionCallback(okRes);
	return okRes;
}

#if 0 // old JUCE 6 code
Result CtrlrPanelResourceManager::restoreState(const ValueTree &savedState) {
	for (int i = 0; i < savedState.getNumChildren(); i++) {
		if (savedState.getChild(i).hasType(Ids::resourceLicense)) {
			AlertWindow licenseWindow("License agreement", "You must agree to the below license",
									  AlertWindow::QuestionIcon);
			TextEditor licenseText;
			licenseText.setMultiLine(true);
			licenseText.setReadOnly(true);
			licenseText.setText(savedState.getChild(i).getProperty(Ids::resourceData));
			licenseText.setSize(500, 400);
			licenseWindow.addCustomComponent(&licenseText);
			licenseWindow.addButton("Yes", 1);
			licenseWindow.addButton("No", 0);
			if (!licenseWindow.runModalLoop()) {
				return (Result::fail("User did not agree to embedded license"));
			}
		}

		if (savedState.getChild(i).hasType(Ids::resourceBlob) || savedState.getChild(i).hasType(Ids::resourceImage) ||
			savedState.getChild(i).hasType(Ids::resource)) {
			Result importResult = importResource(savedState.getChild(i));
			if (!importResult.wasOk()) {
				if (owner.getDialogStatus())
					WARN(importResult.getErrorMessage());
				else
					_WRN(importResult.getErrorMessage());
			}
		}
	}

	return (Result::ok());
}
#endif
Array<CtrlrPanelResource *> CtrlrPanelResourceManager::getResourcesCopy() {
	Array<CtrlrPanelResource *> ret;
	for (int i = 0; i < resources.size(); i++) {
		ret.add(resources[i]);
	}
	return (ret);
}
