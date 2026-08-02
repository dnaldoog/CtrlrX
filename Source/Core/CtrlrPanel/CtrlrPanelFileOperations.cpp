#include "CtrlrLog.h"
#include "CtrlrMacros.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrPanel/CtrlrPanelResource.h"
#include "CtrlrPanel/CtrlrPanelResourceManager.h"
#include "CtrlrProcessor.h"
#include "CtrlrUtilities.h"
#include "stdafx.h"

ValueTree CtrlrPanel::getCleanPanelTree()
{
	ValueTree exportTree = panelTree.createCopy();

	for (int i = 0; i < exportTree.getNumProperties(); i++)
	{
		exportTree.removeProperty(Ids::panelMidiOutputDevice, 0);
		exportTree.removeProperty(Ids::panelMidiInputDevice, 0);
		exportTree.removeProperty(Ids::panelMidiControllerDevice, 0);
		exportTree.removeProperty(Ids::panelIsDirty,
								  0); // Added v5.6.31. Removes the panelIsDirty property to prevent crash on load.
									  // Problem solved with v5.6.31b getPanelWindowTitle() fixed.
	}

	// Remove custom data
	if (exportTree.getChildWithName(Ids::panelCustomData).isValid())
	{
		exportTree.removeChild(exportTree.getChildWithName(Ids::panelCustomData), nullptr);
	}

	ValueTree ed = exportTree.getChildWithName(Ids::uiPanelEditor);

	if (ed.isValid())
	{
		bool hideMenuBar = (bool)ed.getProperty(Ids::uiPanelMenuBarHideOnExport);
		ed.setProperty(Ids::uiPanelMenuBarVisible, !hideMenuBar, nullptr);
	}

	// Embed external lua code in properties
	convertLuaMethodsToPropeties(getPanelLuaDir(), exportTree);

	return (exportTree);
}

String CtrlrPanel::getPanelContentDirPath()
{
	const String filePath = getProperty(Ids::panelFilePath);
	return filePath.upToLastOccurrenceOf(".", false, false);
}

File CtrlrPanel::getPanelContentDir() { return File(getPanelContentDirPath()); }

File CtrlrPanel::getPanelLuaDir() { return getPanelContentDir().getChildFile("lua"); }

String CtrlrPanel::getPanelLuaDirPath() { return getPanelLuaDir().getFullPathName(); }

File CtrlrPanel::getPanelResourcesDir() { return getPanelContentDir().getChildFile("resources"); }

String CtrlrPanel::getPanelResourcesDirPath() { return getPanelResourcesDir().getFullPathName(); }

File CtrlrPanel::getLuaMethodGroupDir(const ValueTree &methodGroup)
{
	ValueTree currentItem = methodGroup;
	StringArray temp;
	while (currentItem.isValid() && currentItem.hasType(Ids::luaMethodGroup))
	{
		String currentPath = currentItem.getProperty(Ids::name);
		if (!currentPath.isEmpty())
		{
			temp.add(currentPath);
		}
		currentItem = currentItem.getParent();
	}
	File result(getPanelLuaDirPath());
	for (int i = temp.size(); i >= 0; i--)
	{
		result = result.getChildFile(temp[i]);
	}
	return result;
}

Result CtrlrPanel::convertLuaMethodsToFiles(const String dirPath)
{
	Result res = Result::ok();
	// Try and get access to panel directory
	File panelLuaDirectory(dirPath);
	if (panelLuaDirectory.existsAsFile())
	{ // A directory with that name already exists
		res =
			Result::fail("Convert to files can't create directory (a file with that name already exists): " + dirPath);
	}
	else if (!panelLuaDirectory.exists())
	{
		res = panelLuaDirectory.createDirectory();
	}
	if (res.ok())
	{
		if (panelLuaDirectory.hasWriteAccess())
		{ // Save lua code
			res = saveLuaCode(panelLuaDirectory, this);
		}
		else
		{
			res = Result::fail("Convert to XML can't write in panel directory: " + dirPath);
		}
	}
	return res;
}

void CtrlrPanel::convertLuaMethodsToPropeties(const File &panelLuaDir, ValueTree &panelTree)
{
	ValueTree luaManager = panelTree.getChildWithName(Ids::luaManager);
	if (luaManager.isValid())
	{
		ValueTree luaMethods = luaManager.getChildWithName(Ids::luaManagerMethods);
		CtrlrPanel::convertLuaChildrenToProperties(panelLuaDir, &luaMethods);
	}
}

Result CtrlrPanel::savePanel()
{
	_DBG("CtrlrPanel::savePanel");

	bool panelWasDirty = isPanelDirty();
	setPanelDirty(false);

	Result res = Result::ok();
	const String filePath = getProperty(Ids::panelFilePath);
	File panelFile(filePath);

	if (panelFile.existsAsFile() && panelFile.hasWriteAccess()) {
		if (panelFile.hasFileExtension("panel"))
			res = savePanelXml(panelFile, this, false);
		if (panelFile.hasFileExtension("panelz"))
			res = savePanelXml(panelFile, this, true);
		if (panelFile.hasFileExtension("bpanel"))
			res = savePanelBin(panelFile, this, false);
		if (panelFile.hasFileExtension("bpanelz"))
			res = savePanelBin(panelFile, this, true);

		if (getEditor()) {
			if (res.failed())
				notify("Panel save: [" + res.getErrorMessage() + "]", nullptr, NotifyFailure);
			else
				notify("Panel saved: [" + panelFile.getFullPathName() + "]", nullptr, NotifySuccess);
		}

		if (res.wasOk()) {
			if (auto *um = getUndoManager())
				um->clearUndoHistory();
			updatePanelWindowTitle();
		} else if (panelWasDirty) {
			setPanelDirty(panelWasDirty);
		}

		return res;
	}

	// Default target path calculated synchronously
	File defaultTarget = askForPanelFileToSave(
		this, File(owner.getProperty(Ids::panelLastSaveDir)).getChildFile(getVersionString()), true, false);

	bool useNativeDialog = (bool)owner.getProperty(Ids::ctrlrNativeFileDialogs, true);

	// Launch async chooser using your FC helper
	FC::saveFileAsync("Save Panel File", defaultTarget, "*.panel;*.panelz", useNativeDialog,
					  [this, panelWasDirty](const File &ret) {
						  Result asyncRes = Result::ok();

						  if (ret != File()) {
							  asyncRes = savePanelXml(ret, this);
							  setProperty(Ids::panelFilePath, ret.getFullPathName());
							  setProperty(Ids::panelLastSaveDir, ret.getParentDirectory().getFullPathName());
						  } else {
							  asyncRes = Result::fail("Selected file is invalid");
						  }

						  if (getEditor()) {
							  if (asyncRes.failed())
								  notify("Panel save: [" + asyncRes.getErrorMessage() + "]", nullptr, NotifyFailure);
							  else
								  notify("Panel saved: [" + ret.getFullPathName() + "]", nullptr, NotifySuccess);
						  }

						  if (asyncRes.wasOk()) {
							  if (auto *um = getUndoManager())
								  um->clearUndoHistory();
							  updatePanelWindowTitle();
						  } else if (panelWasDirty) {
							  setPanelDirty(panelWasDirty);
						  }
					  });

	return Result::ok();
}
void CtrlrPanel::savePanelAs(const CommandID saveOption) {
	File initialDir(getProperty(Ids::panelLastSaveDir));

	auto handleSaveSuccess = [this](const File &fileToSave) {
		setProperty(Ids::panelFilePath, fileToSave.getFullPathName());
		setProperty(Ids::panelLastSaveDir, fileToSave.getParentDirectory().getFullPathName());

		setPanelDirty(false);
		if (auto *um = getUndoManager())
			um->clearUndoHistory();

		updatePanelWindowTitle();
	};

switch (saveOption) {
case CtrlrEditor::doExportFileText: {
    File defaultFile = askForPanelFileToSave(this, initialDir, true, false);

    FC::saveFileAsync("Export XML Panel", defaultFile, "*.panel", true, [this, handleSaveSuccess](const File &fileToSave) {
        if (fileToSave == File())
            return;

        savePanelXml(fileToSave, this);
        handleSaveSuccess(fileToSave);
    });
    break;
}

case CtrlrEditor::doExportFileZText: {
    File defaultFile = askForPanelFileToSave(this, initialDir, true, true);

    FC::saveFileAsync("Export Compressed XML Panel", defaultFile, "*.panelz", true, [this, handleSaveSuccess](const File &fileToSave) {
        if (fileToSave == File())
            return;

        savePanelXml(fileToSave, this, true);
        handleSaveSuccess(fileToSave);
    });
    break;
}

case CtrlrEditor::doExportFileBin: {
    File defaultFile = askForPanelFileToSave(this, initialDir, false, false);

    FC::saveFileAsync("Export Binary Panel", defaultFile, "*.bpanel", true, [this](const File &fileToSave) {
        if (fileToSave == File())
            return;

        savePanelBin(fileToSave, this, false);
    });
    break;
}

case CtrlrEditor::doExportFileZBin: {
    File defaultFile = askForPanelFileToSave(this, initialDir, false, true);

    FC::saveFileAsync("Export Compressed Binary Panel", defaultFile, "*.bpanelz", true, [this](const File &fileToSave) {
        if (fileToSave == File())
            return;

        savePanelBin(fileToSave, this, true);
    });
    break;
}

	case CtrlrEditor::doExportFileZBinRes: {
		exportPanel(this, initialDir);
		break;
	}

case CtrlrEditor::doExportFileInstance:
case CtrlrEditor::doExportFileInstanceRestricted:
{
    const bool isRestricted = (saveOption == CtrlrEditor::doExportFileInstanceRestricted);

    owner.getNativeObject().exportWithDefaultPanel(
        this, 
        isRestricted, 
        isRestricted, 
        [this](juce::Result res)
        {
            if (res.failed())
            {
                if (res.getErrorMessage() == "User cancelled the export operation.")
                {
                    notify("Panel instance export: Cancelled by user.", nullptr, NotifyFailure);
                }
                else
                {
                    notify("Panel instance export: [" + res.getErrorMessage() + "]", nullptr, NotifyFailure);
                    AW::showMessageBox(AW::Warning, "Panel export", 
                        "Failed to export panel as standalone instance.\n" + res.getErrorMessage());
                }
            }
            else
            {
                notify("Panel instance export: Wrote new panel instance.", nullptr, NotifySuccess);
                AW::showMessageBox(AW::Info, "Panel export", "Wrote new panel instance");
            }
        });

    break;
}

	case CtrlrEditor::doExportGenerateUID: {
		setProperty(Ids::panelUID, generateRandomUnique(juce::String(juce::Time::currentTimeMillis())));
		break;
	}

	default:
		break;
	}
}

void CtrlrPanel::savePanelVersioned()
{
	File panelFile(getProperty(Ids::panelFilePath));

	if (panelFile.existsAsFile() && panelFile.hasWriteAccess())
	{
		setProperty(Ids::panelVersionMinor, (int)getProperty(Ids::panelVersionMinor) + 1);

		if (panelFile != File())
		{
			savePanelXml(
				File(panelFile.getParentDirectory()
						 .getChildFile(getProperty(Ids::name).toString() +
									   owner.getProperty(Ids::ctrlrVersionSeparator).toString() + getVersionString())
						 .withFileExtension((bool)owner.getProperty(Ids::ctrlrVersionCompressed) ? "panelz" : "panel")),
				this, owner.getProperty(Ids::ctrlrVersionCompressed));
		}
	}
	else
	{
		savePanel();
	}
}

const String CtrlrPanel::exportPanel(CtrlrPanel *panel, const File &lastBrowsedDir, const File &destinationFile,
									 MemoryBlock *outputPanelData, MemoryBlock *outputResourcesData,
									 const bool isRestricted) {
	if (panel == nullptr)
		return "Undefined panel passed to exporter";

if (destinationFile == File())
{
    // 1. Generate default target path/filename synchronously
    File defaultFile = askForPanelFileToSave(panel, lastBrowsedDir, false, true);

    // 2. Determine native dialog preference
    bool useNativeDialog = panel ? (bool)panel->getOwner().getProperty(Ids::ctrlrNativeFileDialogs, true) : true;

    // 3. Trigger async file chooser via your FC helper
    FC::saveFileAsync(
        "Export Compressed Binary Panel",
        defaultFile,
        "*.bpanelz",
        useNativeDialog,
        [panel, lastBrowsedDir, isRestricted](const File &exportedFile)
        {
            if (exportedFile != File())
            {
                String err = exportPanel(panel, lastBrowsedDir, exportedFile, nullptr, nullptr, isRestricted);
                if (err.isNotEmpty())
                {
                    AW::showMessageBox(AW::Warning, "Panel Export", err);
                }
            }
        });

    return juce::String();
}

	panel->luaSavePanel(PanelFileExport, destinationFile);

	// Snapshot Capture
	juce::Image panelSnapshot(juce::Image::ARGB, 400, 400, true);
	if (auto *canvas = panel->getEditor() ? panel->getEditor()->getCanvas() : nullptr) {
		juce::Image snap = canvas->createComponentSnapshot(canvas->getBounds(), true);
		juce::Graphics g(panelSnapshot);
		g.drawImageWithin(snap, 0, 0, 400, 400,
						  juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, false);
	}

	// Build resources tree
	juce::ValueTree resources(Ids::resourceExportList);
	for (int i = 0; i < panel->getResourceManager().getNumResources(); ++i) {
		if (auto *res = panel->getResourceManager().getResource(i)) {
			resources.addChild(res->createTree(), -1, nullptr);
		}
	}

	juce::ValueTree exportTree = panel->getCleanPanelTree();

	if (isRestricted) {
		exportTree.setProperty(Ids::restricted, static_cast<int>(InstanceSingleRestricted), nullptr);
		auto editorTree = exportTree.getChildWithName(Ids::uiPanelEditor);
		if (editorTree.isValid()) {
			editorTree.setProperty(Ids::uiPanelEditMode, false, nullptr);
		}
	}

	if (panelSnapshot.isValid()) {
		juce::MemoryBlock imageData;
		{
			juce::MemoryOutputStream imageDataStream(imageData, true);
			juce::PNGImageFormat png;
			png.writeImageToStream(panelSnapshot, imageDataStream);
		}

		if (imageData.getSize() != 0) {
			juce::ValueTree snap(Ids::resourcePanelSnapshot);
			snap.setProperty(Ids::resourceSize, static_cast<int64>(imageData.getSize()), nullptr);
			snap.setProperty(Ids::resourceData, imageData.toBase64Encoding(), nullptr);
			resources.addChild(snap, -1, nullptr);
		}
	}

	if (destinationFile.hasWriteAccess() && outputPanelData == nullptr) {
		exportTree.addChild(resources, -1, nullptr);
		juce::MemoryOutputStream compressedData;

		{
			juce::GZIPCompressorOutputStream gzipOutputStream(&compressedData, 9, false);
			exportTree.writeToStream(gzipOutputStream);
			gzipOutputStream.flush();
		}

		if (!destinationFile.replaceWithData(compressedData.getData(), compressedData.getDataSize()))
			return "Failed writing output data to target file";

		return juce::String();
	} else if (outputPanelData != nullptr && outputResourcesData != nullptr) {
		{
			juce::MemoryOutputStream compressedPanelData(*outputPanelData, false);
			juce::GZIPCompressorOutputStream gzipOutputStream(&compressedPanelData, 9, false);
			exportTree.writeToStream(gzipOutputStream);
			gzipOutputStream.flush();
		}

		{
			juce::MemoryOutputStream compressedResourcesData(*outputResourcesData, false);
			juce::GZIPCompressorOutputStream gzipOutputStream(&compressedResourcesData, 9, false);
			resources.writeToStream(gzipOutputStream);
			gzipOutputStream.flush();
		}

		return juce::String();
	}

	return "Can't export panel, unable to write to the specified destination";
}

const ValueTree CtrlrPanel::openBinPanel(const File &panelFile)
{
	ValueTree tree;

	if (panelFile.hasFileExtension(".bpanelz"))
	{
		std::unique_ptr<FileInputStream> fileInputStream(panelFile.createInputStream().release());

		if (fileInputStream)
		{
			GZIPDecompressorInputStream gzFileInputStream(*fileInputStream);
			return (ValueTree::readFromStream(gzFileInputStream));
		}
	}
	else if (panelFile.hasFileExtension(".bpanel"))
	{
		std::unique_ptr<FileInputStream> fileInputStream(panelFile.createInputStream().release());
		if (fileInputStream)
		{
			return (ValueTree::readFromStream(*fileInputStream));
		}
	}

	return (ValueTree());
}

const ValueTree CtrlrPanel::openBinPanel(const MemoryBlock &panelData, const bool isCompressed)
{
	ValueTree tree;

	if (isCompressed)
	{
		MemoryInputStream mi(panelData, false);

		{
			GZIPDecompressorInputStream gzFileInputStream(mi);
			tree = ValueTree::readFromStream(gzFileInputStream);

			return (tree);
		}
	}
	else
	{
		{
			MemoryInputStream mi(panelData, false);
			return (ValueTree::readFromStream(mi));
		}
	}
}

const ValueTree CtrlrPanel::openXmlPanel(const File &panelFile)
{
	String xmlData;

	if (panelFile.hasFileExtension("panelz"))
	{
		std::unique_ptr<FileInputStream> fz(panelFile.createInputStream().release());
		if (fz)
		{
			GZIPDecompressorInputStream gzInput(fz.get(), false);
			xmlData = gzInput.readEntireStreamAsString();
		}
		else
		{
			_ERR("CtrlrPanel::openXmlPanel can't create input stream for file: " + panelFile.getFullPathName());
		}

		std::unique_ptr<XmlElement> xml(XmlDocument::parse(xmlData).release());

		if (xml)
		{
			return (ValueTree::fromXml(*xml));
		}
		else
		{
			_ERR("CtrlrPanel::openXmlPanel can't parse file contents as XML");
		}
	}
	else if (panelFile.hasFileExtension("panel"))
	{
		std::unique_ptr<XmlElement> xml(XmlDocument::parse(panelFile).release());
		if (xml)
		{
			return (ValueTree::fromXml(*xml));
		}
		else
		{
			_ERR("CtrlrPanel::openXmlPanel can't parse file contents as XML");
		}
	}
	else
	{
		_ERR("CtrlrPanel::openXmlPanel unknown file type");
	}

	_ERR("CtrlrPanel::openXmlPanel can't open panel");
	return (ValueTree());
}

const ValueTree CtrlrPanel::openPanel(const File &panelFile)
{
	ValueTree result;
	if (panelFile.hasFileExtension("panelz;panel"))
	{
		result = openXmlPanel(panelFile);
	}
	else if (panelFile.hasFileExtension("bpanelz;bpanel"))
	{
		result = openBinPanel(panelFile);
	}
	else
	{
		result = ValueTree();
	}
	// Patch panelFilePath property to match the actual file
	result.setProperty(Ids::panelFilePath, panelFile.getFullPathName(), nullptr);
	return result;
}

Result CtrlrPanel::savePanelBin(const File &fileToSave, CtrlrPanel *panel, const bool compressPanel)
{
	MemoryOutputStream panelBinData;

	if (panel == nullptr)
		return (Result::fail("Invalid panel pointer"));

	panel->sync();

	if (compressPanel)
	{
		panel->luaSavePanel(PanelFileBinaryCompressed, fileToSave);

		GZIPCompressorOutputStream gzOutputStream(&panelBinData);
		panel->getPanelTree().writeToStream(gzOutputStream);
		gzOutputStream.flush();
	}
	else
	{
		panel->luaSavePanel(PanelFileBinary, fileToSave);
		panel->getPanelTree().writeToStream(panelBinData);
	}

	if (fileToSave.hasWriteAccess())
	{
		if (fileToSave.replaceWithData(panelBinData.getData(), panelBinData.getDataSize()))
		{
			return (Result::ok());
		}
		else
		{
			return (Result::fail("savePanelBin replaceWithData() failed on destination file " +
								 fileToSave.getFullPathName()));
		}
	}
	else
	{

		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Can't save panel",
										 "I can't write to the specified file", "OK");
		return (Result::fail("savePanelBin now write access to file: " + fileToSave.getFullPathName()));
	}
}

void CtrlrPanel::writePanelXml(OutputStream &outputStream, CtrlrPanel *panel, const bool compressPanel)
{
	if (panel == nullptr)
		return;

	panel->sync();

	std::unique_ptr<XmlElement> panelXml(panel->getPanelTree().createXml().release());

	if (compressPanel && panelXml)
	{
		String xml = panelXml->createDocument("");
		{
			GZIPCompressorOutputStream gzipOutputStream(&outputStream, 9, false);
			gzipOutputStream.writeString(xml);
		}
	}
	if (!compressPanel && panelXml)
	{
		panelXml->writeToStream(outputStream, "");
	}
}

File CtrlrPanel::getLuaMethodSourceFile(const ValueTree *method)
{
	String path = method->getProperty(Ids::luaMethodSourcePath);
	if (File::isAbsolutePath(path))
	{
		return File(path);
	}
	else
	{
		return getPanelLuaDir().getChildFile(path);
	}
}

Result CtrlrPanel::writeLuaMethod(const File &parentDir, ValueTree *method)
{
	if (method == nullptr)
		return Result::fail("Method name is missing");

	const String methodName = method->getProperty(Ids::luaMethodName);
	const String methodCode = method->getProperty(Ids::luaMethodCode);
	if (methodName.isEmpty())
		return Result::fail("Method name is empty");

	// Create file
	const File methodFile = parentDir.getNonexistentChildFile(methodName, ".lua", false);
	if (methodFile.replaceWithText(methodCode))
	{
		method->setProperty(Ids::luaMethodName, methodFile.getFileNameWithoutExtension(), nullptr);
		method->setProperty(Ids::luaMethodSourcePath, methodFile.getRelativePathFrom(getPanelLuaDir()), nullptr);
		method->setProperty(Ids::luaMethodSource, (int)CtrlrLuaMethod::codeInFile, nullptr);
		method->removeProperty(Ids::luaMethodCode, nullptr);
		return Result::ok();
	}
	else
	{

		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Could not save lua method",
										 "Could not save lua method", "OK");
		return (Result::fail("saveLuaMethod: could not save lua method."));
	}
}

Result CtrlrPanel::writeLuaMethodGroup(const File &parentDir, ValueTree *methodGroup)
{
	if (methodGroup == nullptr)
		return Result::fail("Method group is missing");

	const String methodGroupName = methodGroup->getProperty(Ids::name);
	if (methodGroupName.isEmpty())
		return Result::fail("Method group is empty");

	// Create file
	const File methodGroupFile = parentDir.getChildFile(methodGroupName);
	Result res = methodGroupFile.createDirectory();
	if (res.wasOk())
	{ // Save contained methods and groups
		return CtrlrPanel::writeLuaChildren(methodGroupFile, methodGroup);
	}
	else
	{

		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Could not save lua method group",
										 "Could not save lua method group", "OK");

		return (Result::fail("saveLuaMethodGroup: could not save lua method group."));
	}
	return res;
}

Result CtrlrPanel::writeLuaChildren(const File &parentDir, ValueTree *parentElement)
{
	if (parentElement == nullptr)
		return Result::fail("Parent element is missing");

	Result res = Result::ok();
	for (int i = 0; i < parentElement->getNumChildren(); i++)
	{
		ValueTree child = parentElement->getChild(i);
		if (child.hasType(Ids::luaMethod))
		{
			if ((int)child.getProperty(Ids::luaMethodSource) !=
				CtrlrLuaMethod::codeInFile)
			{ // Only process methods that are not already saved in files
				res = CtrlrPanel::writeLuaMethod(parentDir, &child);
				if (res.failed())
				{ // Break on first error
					return res;
				}
			}
		}
		else if (child.hasType(Ids::luaMethodGroup))
		{
			res = CtrlrPanel::writeLuaMethodGroup(parentDir, &child);
			if (res.failed())
			{ // Break on first error
				return res;
			}
		}
	}
	return res;
}

Result CtrlrPanel::saveLuaCode(const File &panelLuaDir, CtrlrPanel *panel)
{
	ValueTree luaManager = panel->getPanelTree().getChildWithName(Ids::luaManager);
	if (luaManager.isValid())
	{
		ValueTree luaMethods = luaManager.getChildWithName(Ids::luaManagerMethods);
		return CtrlrPanel::writeLuaChildren(panelLuaDir, &luaMethods);
	}
	else
	{
		return (Result::fail("saveLuaCode failed due to missing luaManager"));
	}
}

void CtrlrPanel::convertLuaMethodToProperty(const File &panelLuaDir, ValueTree *method)
{
	if (method == nullptr)
		return;

	const String methodName = method->getProperty(Ids::luaMethodName);
	const String methodFilePath = method->getProperty(Ids::luaMethodSourcePath);
	if (methodName.isEmpty() || methodFilePath.isEmpty())
		return;
	// Get file path
	File methodFile;
	if (File::isAbsolutePath(methodFilePath))
	{
		methodFile = File(methodFilePath);
	}
	else
	{
		methodFile = panelLuaDir.getChildFile(methodFilePath);
	}

	// Read file
	method->removeProperty(Ids::luaMethodSourcePath, nullptr);
	method->setProperty(Ids::luaMethodSource, (int)CtrlrLuaMethod::codeInProperty, nullptr);
	method->setProperty(Ids::luaMethodCode, methodFile.loadFileAsString(), nullptr);
}

void CtrlrPanel::convertLuaChildrenToProperties(const File &panelLuaDir, ValueTree *parentElement)
{
	if (parentElement == nullptr)
		return;

	for (int i = 0; i < parentElement->getNumChildren(); i++)
	{
		ValueTree child = parentElement->getChild(i);
		if (child.hasType(Ids::luaMethod))
		{ // This is a method, check if it's on file
			if ((int)child.getProperty(Ids::luaMethodSource) ==
				CtrlrLuaMethod::codeInFile)
			{ // Only process methods that are saved in files
				CtrlrPanel::convertLuaMethodToProperty(panelLuaDir, &child);
			}
		}
		else if (child.hasType(Ids::luaMethodGroup))
		{ // This is a group => recursive call
			CtrlrPanel::convertLuaChildrenToProperties(panelLuaDir, &child);
		}
	}
}

Result CtrlrPanel::savePanelXml(const File &fileToSave, CtrlrPanel *panel, const bool compressPanel)
{
	MemoryOutputStream dataToSave;

	if (compressPanel)
	{
		panel->luaSavePanel(PanelFileXMLCompressed, fileToSave);
	}
	else
	{
		panel->luaSavePanel(PanelFileXML, fileToSave);
	}

	writePanelXml(dataToSave, panel, compressPanel);

	if (fileToSave.hasWriteAccess())
	{
		if (fileToSave.replaceWithData(dataToSave.getData(), dataToSave.getDataSize()))
		{
			return (Result::ok());
		}
		else
		{
			return (Result::fail("savePanelXml replaceWithData() failed on destination file " +
								 fileToSave.getFullPathName()));
		}
	} else {
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Can't save panel",
										 "I can't write to the specified file", "OK");
		return (Result::fail("savePanelXml now write access to file: " + fileToSave.getFullPathName()));
	}
}

const File CtrlrPanel::askForPanelFileToSave(CtrlrPanel *panel, const File &lastBrowsedDir, const bool isXml,
											 const bool isCompressed) {
	juce::String panelFileName = "Ctrlr Panel";

	if (panel != nullptr) {
		panelFileName = panel->getProperty(Ids::name).toString();
		panelFileName << "_" << panel->getVersionString();
	}

	if (isXml) {
		panelFileName << (isCompressed ? ".panelz" : ".panel");
	} else {
		panelFileName << (isCompressed ? ".bpanelz" : ".bpanel");
	}

	if (juce::File::isAbsolutePath(lastBrowsedDir.getFullPathName())) {
		return lastBrowsedDir.getChildFile(panelFileName);
	}

	return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile(panelFileName);
}

bool CtrlrPanel::isPanelFile(const File &fileToCheck, const bool beThorough) {
	if (beThorough)
		return false;

	return fileToCheck.hasFileExtension("bpanel;bpanelz;panel;panelz");
}

void CtrlrPanel::setSavePoint() { indexOfSavedState = currentActionIndex; }

bool CtrlrPanel::hasChangedSinceSavePoint() { return currentActionIndex != indexOfSavedState; }

bool CtrlrPanel::isPanelDirty() { return getProperty(Ids::panelIsDirty, false); }

void CtrlrPanel::setPanelDirty(const bool dirty) { setProperty(Ids::panelIsDirty, dirty); }

void CtrlrPanel::actionPerformed()
{
	currentActionIndex++;
	updatePanelWindowTitle();
}

void CtrlrPanel::actionUndone()
{
	currentActionIndex--;
	updatePanelWindowTitle();
}

const String CtrlrPanel::getPanelWindowTitle()
{
	String name = getProperty(Ids::name);
	if (JUCEApplication::isStandaloneApp()) // Updated v5.6.31b. For Standalone APP/EXE Only. Was crashing VST Hosts on
											// load in v5.6.30 & v5.6.31
	{
		if (isPanelDirty() || hasChangedSinceSavePoint())
		{
			name = name + "*";
		}

		else if (!JUCEApplication::isStandaloneApp()) // Updated v5.6.31b. For VST & AU Plugins

			if (isPanelDirty()) // Updated v5.6.31b. Was (isPanelDirty() || hasChangedSinceSavePoint()). Was crashing
								// VST Hosts on load in v5.6.30 & v5.6.31
			{
				name = name + "*";
			}
	}

	return name;
}

void CtrlrPanel::updatePanelWindowTitle()
{
	CtrlrPanelEditor *editor = getEditor(false);
	if (editor)
	{
		String newName = getPanelWindowTitle();
		if (newName != editor->getName())
		{
			editor->setName(newName);
			// Trigger editor window title update
			owner.getEditor()->activeCtrlrChanged();
		}
	}
}

void CtrlrPanel::luaManagerChanged()
{
	if (!getRestoreState())
	{
		setPanelDirty(true);
		updatePanelWindowTitle();
	}
}

void CtrlrPanel::panelResourcesChanged()
{
	if (!getRestoreState())
	{
		setPanelDirty(true);
		updatePanelWindowTitle();
	}
}
void CtrlrPanel::canClose(const bool closePanel, std::function<void(bool)> completionCallback) 
{
    CtrlrPanelWindowManager &manager = getWindowManager();
    if (manager.isCreated(CtrlrPanelWindowManager::LuaMethodEditor)) {
        CtrlrChildWindowContent *content = manager.getContent(CtrlrPanelWindowManager::LuaMethodEditor);
        if (content != nullptr) {
            content->toFront(true);
            if (!content->canCloseWindow()) {
                if (completionCallback)
                    completionCallback(false);
                return;
            }
        }
    }

    if (closePanel && (hasChangedSinceSavePoint() || isPanelDirty())) {

        juce::WeakReference<CtrlrPanel> safePanel(this);

        juce::NativeMessageBox::showYesNoCancelBox(
            juce::MessageBoxIconType::QuestionIcon, 
            "Save panel (" + getName() + ")",
            "There are unsaved changes in this panel.\nDo you want to save them before closing?", 
            nullptr,
            juce::ModalCallbackFunction::create([safePanel, completionCallback](int result) {
                if (safePanel.wasObjectDeleted()) {
                    if (completionCallback)
                        completionCallback(false);
                    return;
                }

                if (result == 1) // Save ("Yes")
                {
                    safePanel->savePanel();
                    if (completionCallback)
                        completionCallback(true);
                } 
                else if (result == 2) // Discard ("No")
                {
                    if (completionCallback)
                        completionCallback(true);
                } 
                else // Cancel (0)
                {
                    if (completionCallback)
                        completionCallback(false);
                }
            }));
        return;
    }

    if (completionCallback)
        completionCallback(true);
}
