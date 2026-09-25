#include "CtrlrSettings.h"
#include "CtrlrInlineUtilitiesGUI.h"
#include "stdafx.h"

CtrlrSettings::CtrlrSettings (CtrlrManager &_owner) : Component ("Global Properties"), owner(_owner), propertyPanel (nullptr)
{

	owner.getManagerTree().addListener(this);
	addAndMakeVisible (propertyPanel = new PropertyPanel());
    
    propertyPanel->setName ("propertyPanel");
    
    Array <PropertyComponent*> globalProperties;
    Array <PropertyComponent*> midiProperties;
    Array <PropertyComponent*> guiProperties;
    Array <PropertyComponent*> debugProperties;
    Array <PropertyComponent*> directoriesProperties;
    
    Array <PropertyComponent*> emptyProperties; // Placeholder
    Array <PropertyComponent*> instanceProperties; // Useless
    
// Removed v5.6.31
//	for (int i=0; i<owner.getManagerTree().getNumProperties(); i++)
//	{
//		globalProperties.add
//		(
//			// owner.getIDManager().createComponentForProperty (owner.getManagerTree().getPropertyName(i), owner.getManagerTree(), nullptr)
//		);
//	}
    
    // Ctrlr Behaviour section
    globalProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrVersionSeparator"), owner.getManagerTree(), nullptr));
    globalProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrVersionCompressed"), owner.getManagerTree(), nullptr));

    if (JUCEApplication::isStandaloneApp()) // Added v5.6.35
    {
        globalProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrAutoSave"), owner.getManagerTree(), nullptr));
        globalProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrAutoSaveInterval"), owner.getManagerTree(), nullptr));
    }

    // Useless because ctrlrShutdownDelay is overriden in ctrlrProcessor.cpp to 512
    // #ifdef JUCE_MAC
    // globalProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrShutdownDelay"), owner.getManagerTree(), nullptr)); // ifdef JUCE_OSX not working
    // #endif
    
    globalProperties.add(owner.getIDManager().createComponentForProperty(Identifier("luaCtrlrSaveState"), owner.getManagerTree(), nullptr));
    globalProperties.add(owner.getIDManager().createComponentForProperty(Identifier("luaCtrlrRestoreState"), owner.getManagerTree(), nullptr));

    // Midi section
    // midiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrLogMidiInput"), owner.getManagerTree(), nullptr)); // Useless, not assigned
    // midiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrLogMidiOutput"), owner.getManagerTree(), nullptr)); // Useless, not assigned
    // midiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrLogOptions"), owner.getManagerTree(), nullptr)); // Useless because it helps storing Monitor Log options as Byte value
    midiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrMidiMonInputBufferSize"), owner.getManagerTree(), nullptr));
    midiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrMidiMonOutputBufferSize"), owner.getManagerTree(), nullptr));

    // GUI section
    // guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrPropertiesAreURLs"), owner.getManagerTree(), nullptr)); // Useless, not assigned
    guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrNativeFileDialogs"), owner.getManagerTree(), nullptr));
    guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrNativeAlerts"), owner.getManagerTree(), nullptr));
    guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrPropertyLineImprovedLegibility"), owner.getManagerTree(), nullptr));
    // guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrUseEditorWrapper"), owner.getManagerTree(), nullptr)); // Useless, only for Ableton live.  Automatically set to true via CtrlrProcessor.cpp OS & DAW detection
    
    if (JUCEApplication::isStandaloneApp())
    {
        guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrLookAndFeel"), owner.getManagerTree(), nullptr));
        guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrColourScheme"), owner.getManagerTree(), nullptr));
    }

    guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrFontSizeBaseValue"), owner.getManagerTree(), nullptr));
    guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrScrollbarThickness"), owner.getManagerTree(), nullptr));
    guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrMenuBarHeight"), owner.getManagerTree(), nullptr));
    guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrTabBarDepth"), owner.getManagerTree(), nullptr));
    guiProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrPropertyLineheightBaseValue"), owner.getManagerTree(), nullptr)); // Added v5.6.33.
    
    // Debug section
    debugProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrLogToFile"), owner.getManagerTree(), nullptr));
    debugProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrWarningInBootstrapState"), owner.getManagerTree(), nullptr));
    debugProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrLuaDebug"), owner.getManagerTree(), nullptr));
    debugProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrLuaDisabled"), owner.getManagerTree(), nullptr));
    debugProperties.add(owner.getIDManager().createComponentForProperty(Identifier("uiLuaConsoleInputRemoveAfterRun"), owner.getManagerTree(), nullptr)); // Clear Console After Execute
    
    // Directory section
	directoriesProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrRecentOpenedPanelFiles"),
																			  owner.getManagerTree(), nullptr));
	directoriesProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrLastBrowsedFileDirectory"), owner.getManagerTree(), nullptr));
    directoriesProperties.add(owner.getIDManager().createComponentForProperty(Identifier("ctrlrLastBrowsedResourceDir"), owner.getManagerTree(), nullptr));
    
    // propertyPanel->addSection("Global | Restart to apply settings", globalProperties, true);
    propertyPanel->addSection("Global", globalProperties, true);
    propertyPanel->addSection("MIDI", midiProperties, true);
    propertyPanel->addSection("GUI", guiProperties, true);
    propertyPanel->addSection("Debug", debugProperties, true);
    propertyPanel->addSection("Paths", directoriesProperties, true);
    
	propertyPanel->getViewport().setScrollBarThickness(owner.getManagerTree().getProperty(Ids::ctrlrScrollbarThickness));
    
// --- FIX FOR HIGH RESOLUTION / SMALLER DISPLAYS ---
    const int totalContentHeight = propertyPanel->getTotalContentHeight();

    // Get primary display work area height (excluding taskbars/dock)
    int maxAvailableHeight = 700; // Safe fallback height
    
    if (auto* primaryDisplay = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        // Limit max window height to ~75% of screen work area height
        maxAvailableHeight = juce::roundToInt(primaryDisplay->userArea.getHeight() * 0.75f);
    }

    // Set height to content size if small, or clamp to maxAvailableHeight
    const int targetHeight = juce::jmin(totalContentHeight + 20, maxAvailableHeight);

    setSize (600, targetHeight);
}

void CtrlrSettings::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (DocumentWindow::backgroundColourId)); // Added v5.6.31
}

void CtrlrSettings::resized()
{
    propertyPanel->setBounds (0, 0, getWidth() - 0, getHeight() - 0);
}

// Called automatically by JUCE whenever any setting property is modified

CtrlrSettings::~CtrlrSettings()
{
    // Unregister 'this' instance before destruction completes
    owner.getManagerTree().removeListener (this);

    deleteAndZero (propertyPanel);

    if (settingsWereModified)
    {
        const juce::String msg = "If changes are not visible or effective, please restart CtrlrX.";

        if (juce::JUCEApplication::isStandaloneApp())
        {
            juce::NativeMessageBox::showMessageBoxAsync (juce::AlertWindow::InfoIcon, "CtrlrX Preferences", msg);
        }
        else
        {
            AW::showMessageBox (AW::Info, "CtrlrX Preferences", msg);
        }
    }
}

void CtrlrSettings::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == owner.getManagerTree())
    {
        settingsWereModified = true;
    }
}


void CtrlrSettings::restart()
{
    // Check if multiple instances are allowed
    auto runningInstance = JUCEApplication::getInstance();
    bool multiInstance = runningInstance->JUCEApplication::moreThanOneInstanceAllowed();
        
    if (multiInstance) {
        Logger::writeToLog("Multiple instances allowed.");
    }
    else{
        Logger::writeToLog("Multiple instances not allowed.");
    }
    
    String executablePath = File::getSpecialLocation(File::currentExecutableFile).getFullPathName();
    File exeFile = File(executablePath);
    
    if (exeFile.exists())
    {
        Logger::writeToLog("Executable file found");
        
        int result = 1; // Assume failure initially
        
        auto osType = SystemStats::getOperatingSystemType();
        Logger::writeToLog("OS type: " + String(osType));
        
        String osName = SystemStats::getOperatingSystemName();
        Logger::writeToLog("OS Name: " + String(osName));
        
        if ((SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {  // To test whether any version of OSX is running
            // For OSX & macOS
            Logger::writeToLog("Launching on macOS using ChildProcess.");
            ChildProcess childProcess;
            
            
            // Attempt to start the process with ChildProcess
            if (childProcess.start(exeFile.getFullPathName())) {
                Logger::writeToLog("ChildProcess start() successful.");
                result = 0;
            } else {
                Logger::writeToLog("ChildProcess start() failed.");

                // If ChildProcess fails, try startAsProcess()
                Logger::writeToLog("Trying startAsProcess().");
                result = exeFile.startAsProcess();
                if (result == 0) {
                    Logger::writeToLog("startAsProcess() successful.");
                } else {
                    Logger::writeToLog("startAsProcess() failed.");
                }
            }
        }
        
        else if (SystemStats::getOperatingSystemType() == SystemStats::Windows){
            // For Windows
            Logger::writeToLog("Launching on Windows using exeFile.startAsProcess().");
            result = exeFile.startAsProcess();
        }
        
        else{
            // Handle unsupported operating systems
            Logger::writeToLog("Launching on other operating system.");
            std::cerr << "Launching on other operating system." << std::endl;
            result = 1;
        }
                
        if (result == 1){
            // Handle error (e.g., log error message)
            std::cerr << "Error launching executable: " << result << std::endl;
            Logger::writeToLog("Error launching executable: " + String(result));
        }
        else
        {
            Logger::writeToLog("Executable launched successfully.");
        }
    }
    else{
        // Handle error: executable file not found
        std::cerr << "Executable file not found." << std::endl;
        Logger::writeToLog("Executable file not found.");
    }
        
    Logger::writeToLog("Sending quit request to current application.");
    JUCEApplication::getInstance()->systemRequestedQuit();
    Logger::writeToLog("Quit request sent. Waiting for 500ms.");
    Thread::sleep(500);
}

