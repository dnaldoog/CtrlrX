#include "stdafx.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrLog.h"
#include "CtrlrProperties.h"
#include "CtrlrMacros.h"
#include "CtrlrIDs.h"

CtrlrProperties::CtrlrProperties(CtrlrManager &_owner) : owner(_owner)
{
	PropertiesFile::Options options;
	options.applicationName = owner.getInstanceName();
	options.filenameSuffix = ".settings";
	options.folderName = File::createLegalFileName(owner.getInstanceName());
	options.storageFormat = PropertiesFile::storeAsBinary;
	options.millisecondsBeforeSaving = 250;
	options.osxLibrarySubFolder = "Preferences";

#if JUCE_LINUX
	// 1. Resolve XDG config folder (~/.config/CtrlrX) and old folder (~/CtrlrX)
	File xdgConfigDir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(options.folderName);
	File oldHomeDir = File::getSpecialLocation(File::userHomeDirectory).getChildFile(options.folderName);

	// 2. Silent Migration: Move legacy ~/CtrlrX to ~/.config/CtrlrX
	if (oldHomeDir.exists() && oldHomeDir.isDirectory() && !oldHomeDir.isSymbolicLink()) {
		xdgConfigDir.getParentDirectory().createDirectory();

		if (!xdgConfigDir.exists()) {
			if (oldHomeDir.moveFileTo(xdgConfigDir)) {
				oldHomeDir.createSymbolicLink(xdgConfigDir, true);
				_DBG("CtrlrProperties: Successfully migrated legacy ~/CtrlrX to " + xdgConfigDir.getFullPathName());
			}
		}
	}

	// 3. Define the exact file target (~/.config/CtrlrX/CtrlrX.settings)
	String settingsFileName = options.applicationName + options.filenameSuffix;
	File customSettingsFile = xdgConfigDir.getChildFile(settingsFileName);

	// 4. Initialize storage using custom File target
	applicationProperties.setStorageParameters(options);

	// Explicitly open custom file target for user settings
	applicationProperties.closeFiles();

	// Fallback assignment via raw PropertiesFile instantiation if setStorageParameters defaults fail
	options.folderName = ".config/" + options.folderName;
#endif

	applicationProperties.setStorageParameters(options);
}

CtrlrProperties::~CtrlrProperties()
{
}

bool CtrlrProperties::saveIfNeeded(const bool force)
{
    if (force)
    {
        if (applicationProperties.getUserSettings())
        {
            return (applicationProperties.getUserSettings()->save());
        }

        if (applicationProperties.getCommonSettings(true))
        {
            return (applicationProperties.getCommonSettings(true)->save());
        }
    }
	return (applicationProperties.saveIfNeeded());
}

ApplicationProperties &CtrlrProperties::getProperties()
{
	return (applicationProperties);
}