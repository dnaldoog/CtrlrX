#ifndef __CTRLR_EDITOR__
#define __CTRLR_EDITOR__

#include "CtrlrMacros.h"
#include "CtrlrWindowManagers/CtrlrChildWindowContainer.h"
#include "CtrlrSettings.h"

class CtrlrManager;
class CtrlrProcessor;
class CtrlrExpressionsHelp : public Component
{
public:
    CtrlrExpressionsHelp()
    {
        addAndMakeVisible(helpText);

        helpText.setMultiLine(true);
        helpText.setReadOnly(true);
        helpText.setScrollbarsShown(true);
        helpText.setCaretVisible(false);
        helpText.setPopupMenuEnabled(true);
        helpText.setWantsKeyboardFocus(false);

        // Set the help text content
        String content =
            "EXPRESSIONS HELP\n\n"

            "EXAMPLE USAGE:\n\n\n"
            "setGlobal(0, modulatorValue * 2)  \n"
            "  This sets global variable 0 to twice the current modulator value.\n\n"

            "setGlobal(0, setBitRangeAsInt(global.k0, 4, 2, 0))\n"
            "  This clears bits 4 and 5 in global variable 0.\n\n"

            "modulatorValue*127\n"
            "   Useful for on/off boolean values where you want to map uiButton false/true to 0,127\n\n"


            "CONSTANTS:\n\n"

            "modulatorValue\n"
            "  The current linear value of the modulator, this is the index of the\n"
            "  array of values; is always positive.\n\n"

            "modulatorMappedValue\n"
            "  The current mapped value in case of components that have mappings.\n"
            "  This might be negative.\n\n"

            "modulatorMax\n"
            "  The maximum value the modulator can have (non mapped)\n\n"

            "modulatorMin\n"
            "  The minimum value the modulator can have (non mapped)\n\n"

            "modulatorMappedMax\n"
            "  The maximum value the modulator can have (mapped)\n\n"

            "modulatorMappedMin\n"
            "  The maximum value the modulator can have (mapped)\n\n"

            "vstIndex\n"
            "  The VST/AU index of the parameter as seen by the host program\n\n"

            "midiValue\n"
            "  The current value stored in the MIDI MESSAGE associated with the\n"
            "  modulator.\n\n"

            "midiNumber\n"
            "  The number of the MIDI MESSAGE controller if applicable\n\n\n"

            "FUNCTIONS:\n\n"

            "ceil(x)\n"
            "  Returns the smallest integral value of the parameter\n\n"

            "abs(x)\n"
            "  Returns the absolute value of the parameter\n\n"

            "floor(x)\n"
            "  Returns the largest integral value that is not greater than the\n"
            "  parameter\n\n"

            "mod(a,b)\n"
            "  Divides two numbers and returns the result of the MODULO operation %.\n"
            "  Examples: 10 % 3 = 1, 0 % 5 = 0; 30 % 6 = 0; 32 % 5 = 2\n\n"

            "fmod(numerator,denominator)\n"
            "  Returns the floating-point remainder of the two parameters passed in\n\n"

            "pow(a,b)\n"
            "  Returns the first parameter raised to the power of the second (a^b)\n\n"

            "gte(a,b,retTrue,retFalse)\n"
            "  Return the larger or equal of the two passed parameters (a >= b).\n"
            "  Example: gte(modulatorValue, 0, modulatorValue, 128-modulatorValue)\n"
            "  will return modulatorValue if modulatorValue is greater than 0 and\n"
            "  (128 - modulatorValue) if it is less than zero\n\n"

            "gt(a,b,retTrue,retFalse)\n"
            "  Same as gte but greater than without the equal sign (a > b)\n\n"

            "lt(a,b,retTrue,retFalse)\n"
            "  Same as gte but less than (a < b)\n\n"

            "lte(a,b,retTrue,retFalse)\n"
            "  Same as gte but less than or equal (a <= b)\n\n"

            "eq(a,b,retTrue,retFalse)\n"
            "  Equals sign true if (a == b)\n\n"

            "max(a,b)\n"
            "  Returns the bigger of two parameters.\n\n"

            "min(a,b)\n"
            "  Returns the smaller of two parameters.\n\n"

            "getBitRangeAsInt(value, startBit, numBits)\n"
            "  Gets a number of bits (numBits) starting at position startBit as an\n"
            "  Integer and returns that integer.\n\n"

            "setBitRangeAsInt(value, startBit, numBits, valueToSet)\n"
            "  Sets a range of bits in value\n\n"

            "clearBit(value, bitToClear)\n"
            "  Clears a bit at position bitToClear in the value and returns that\n"
            "  modified value.\n\n"

            "isBitSet(value, bitPosition)\n"
            "  Return true if a bit at position bitPosition in value is set,\n"
            "  false otherwise.\n\n"

            "setBit(value, bitToSet)\n"
            "  Sets one bit in an integer at position (bitToSet) and returns the\n"
            "  modified value with the bit set.\n\n"

            "setGlobal(globalIndex, newValueToSet)\n"
            "  This sets the value of one of the global variables in the panel,\n"
            "  and returns that set value so the expression can continue.\n";

        helpText.setText(content);

        // Use a custom font for better readability
        Font monoFont = Font(Font::getDefaultMonospacedFontName(), 13.0f, Font::plain);
        helpText.setFont(monoFont);
    }

    void resized() override
    {
        helpText.setBounds(getLocalBounds().reduced(4));
    }

    void parentHierarchyChanged() override
    {
        updateColors();
    }

    void lookAndFeelChanged() override
    {
        updateColors();
    }

    void updateColors()
    {
        Colour bgColour = findColour(TextEditor::backgroundColourId);

        // Determine if background is light or dark
        float brightness = bgColour.getBrightness();
        Colour textColour = (brightness > 0.5f) ? Colours::black : Colours::white;

        helpText.setColour(TextEditor::textColourId, textColour);
        helpText.setColour(TextEditor::backgroundColourId, bgColour);
        helpText.setColour(TextEditor::outlineColourId, findColour(Slider::textBoxOutlineColourId));

        helpText.applyColourToAllText(textColour);
        helpText.repaint();
    }

private:
    TextEditor helpText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrExpressionsHelp)
};


class CtrlrEditor  : public AudioProcessorEditor,
					 public ApplicationCommandTarget,
					 public MenuBarModel,
					 public Slider::Listener //,
					 // public LookAndFeel_V4 // Removed v5.6.34 to set lnf version independently
{
	public:
		CtrlrEditor (CtrlrProcessor *ownerFilter, CtrlrManager &_owner);
		~CtrlrEditor();
    
        JUCE_DECLARE_WEAK_REFERENCEABLE(CtrlrEditor) // Added v5.6.34. To replace masterReference.clear() on destruction
        
		enum TopMenuIDs
		{
			MenuFile,
			MenuEdit,
			MenuView,
			MenuPanel,
			MenuMidi,
			MenuPrograms,
			MenuTools,
			MenuHelp
		};

		enum TopMenuRestrictedIDs
		{
			MenuRestrictedFile,
			MenuRestrictedEdit,
			MenuRestrictedView,
			MenuRestrictedMidi,
			MenuRestrictedPrograms,
			MenuRestrictedTools,
			MenuRestrictedHelp
		};

		enum CommandIDs
		{
			doSaveState					= 0x2000,
			doOpenPanel					= 0x2001,
			doNewPanel					= 0x2002,
			showGlobalSettingsDialog	= 0x2003,
			showMidiMonitor             = 0x2005,
			showLogViewer				= 0x2006,
			showMidiCalculator          = 0x2007,
			showAboutDialog             = 0x2008,
			showKeyboardMappingDialog	= 0x200a,
			showExpressionHelp			= 0x2009,

			/* Panel commands */

			doZoomIn					= 0x3001,
			doZoomOut					= 0x3002,
			doCopy						= 0x3003,
			doCut						= 0x3004,
			doPaste						= 0x3005,
			doUndo						= 0x3024,
			doRedo						= 0x3025,
			doSave						= 0x3006,
			doSaveAs					= 0x3007,
			doSaveVersioned				= 0x3008,
			doPanelMode					= 0x300a,
			doPanelLock					= 0x300b,
			doPanelDisableCombosOnEdit	= 0x3027,
			doSendSnapshot				= 0x300c,
			doRefreshDeviceList			= 0x200b,
			showLuaEditor				= 0x3011,
			showMidiLibrary				= 0x3012,
			showModulatorList			= 0x3013,
			showBufferEditor			= 0x3028,
			doClose						= 0x3015,
			showLayers					= 0x3016,
			showLuaConsole				= 0x3017,
			showComparatorTables		= 0x3018,
			doRefreshPropertyLists		= 0x3019,
			doViewPropertyDisplayIDs	= 0x3020,
			doExportFileText			= 0x4001,
			doExportFileZText			= 0x4002,
			doExportFileBin				= 0x4003,
			doExportFileZBin			= 0x4004,
			doExportFileZBinRes			= 0x4005,
			doExportFileInstance		= 0x4006,
			doExportGenerateUID			= 0x4007,
			doSearchForProperty         = 0x4018,
			doExportFileInstanceRestricted		= 0x4008,
			doSnapshotStore						= 0x4009,
			optMidiSnapshotOnLoad				= 0x6003,
			optMidiSnapshotOnProgramChange		= 0x6004,
			doPrevProgram						= 0x6005,
			doNextProgram						= 0x6006,
			doPrevBank							= 0x6007,
			doNextBank							= 0x6008,
			doShowMidiSettingsDialog			= 0x3010,
			optMidiInputFromHost		= MENU_OFFSET_MIDI + 2,
			optMidiInputFromHostCompare	= MENU_OFFSET_MIDI + 4,
			optMidiOutuptToHost			= MENU_OFFSET_MIDI + 8,
			optMidiThruH2H				= MENU_OFFSET_MIDI + 16,
			optMidiThruH2HChannelize	= MENU_OFFSET_MIDI + 32,
			optMidiThruH2D				= MENU_OFFSET_MIDI + 64,
			optMidiThruH2DChannelize	= MENU_OFFSET_MIDI + 128,
			optMidiThruD2D				= MENU_OFFSET_MIDI + 256,
			optMidiThruD2DChannelize	= MENU_OFFSET_MIDI + 512,
			optMidiThruD2H				= MENU_OFFSET_MIDI + 1024,
			optMidiThruD2HChannelize	= MENU_OFFSET_MIDI + 2048,

			doCrash						= 0x20,
			doDumpVstTables				= 0x21,
			doRegisterExtension			= 0x22,
			doKeyGenerator              = 0x23,
			doProgramWizard             = 0x24,
			doQuit						= 0x00fffffe
		};

		void activeCtrlrChanged();
		const WeakReference<CtrlrEditor>::SharedRef& getWeakReference();
		CtrlrManager &getOwner()											{ return (owner); }
		
        // WeakReference<CtrlrEditor>::Master masterReference;
		
        void paint (Graphics& g);
		void resized();
		void getAllCommands (Array< CommandID > &commands);
		void getCommandInfo (CommandID commandID, ApplicationCommandInfo &result);
		bool perform (const InvocationInfo &info);

		ApplicationCommandTarget* getNextCommandTarget();
		StringArray getMenuBarNames();
		PopupMenu getMenuForIndex(int topLevelMenuIndex, const String &menuName);
		void menuItemSelected(int menuItemID, int topLevelMenuIndex);
		void sliderValueChanged (Slider* slider);
		MenuBarComponent *getMenuBar();
		CtrlrPanel *getActivePanel();
		CtrlrPanelEditor *getActivePanelEditor();
		bool isPanelActive(const bool checkRestrictedInstance=false);
		const PopupMenu getRecentOpenedFilesMenu();
		const PopupMenu getMidiDeviceMenu(const CtrlrMIDIDeviceType type=inputDevice);
		const PopupMenu getMidiChannelMenu(const CtrlrMIDIDeviceType type=inputDevice);
		uint32 getMidiDeviceMenuOffset(const CtrlrMIDIDeviceType type=inputDevice);
		const Identifier getMidiPropertyName(const CtrlrMIDIDeviceType type=inputDevice);
		const StringArray getRecentOpenedFilesList();
		const var getPanelProperty(const Identifier &propertyName);
		const String getMidiSummary(const CtrlrMIDIDeviceType type=inputDevice);
		bool isRestricted();
		void performShowKeyboardMappingDialog(const int menuItemID);
		void performMidiChannelChange(const int menuItemID);
		void performMidiDeviceChange(const int menuItemID);
		void performMidiOptionChange(const int menuItemID);
		void performProgramChange(const int menuItemID);
		void performMidiHostOptionChange(const int menuItemID);
		void performMidiThruChange(const int menuItemID);
		void performRecentFileOpen(const int menuItemID);
		void performKeyGenerator();
		void performMidiDeviceRefresh();
		void setMenuBarVisible(const bool shouldBeVisible=true);
    
        // New method to set the main LookAndFeel for the editor and its children
        void setEditorLookAndFeel (const String &lookAndFeelDesc, const juce::var& colourSchemeProperty); // Added v5.6.34
    
        void performLuaEditorCommand(const int commandID); // Added v5.6.34. Required to add shortcuts to LUA Method Editor menu items
    
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrEditor)

	private:
		// Use a ScopedPointer to manage the menuBar object
		juce::ScopedPointer<juce::MenuBarComponent> menuBar;
		// MenuBarComponent *menuBar; // Updated v5.6.34. ScopedPointed will handle its destruction properly
		
		// Use a ScopedPointer to manage the current LookAndFeel object
		ScopedPointer<LookAndFeel> currentLookAndFeel; // Updated v5.6.34. ScopedPointed will handle its destruction properly
    
		bool menuHandlerCalled;
		TooltipWindow tooltipWindow;
		CtrlrManager &owner;
		ResizableCornerComponent resizer;
		CtrlrProcessor *ownerFilter;
		ComponentBoundsConstrainer constrainer;
		Label l;
		CtrlrPanel *invalidCtrlrPtr;
		Result tempResult;
		int64 lastCommandInvocationMillis;
		bool hideProgramsMenu = false;
		bool hideMidiControllerMenu = false;
		bool hideMidiThruMenu = false;
		bool hideMidiChannelMenu = false;
    
		bool vpResizable;
		double vpFixedAspectRatio;
		bool vpEnableFixedAspectRatio;
		bool vpEnableResizableLimits;
		int vpMinWidth;
		int vpMinHeight;
		int vpMaxWidth;
		int vpMaxHeight;
		int panelCanvasHeight;
		int panelCanvasWidth;
		double vpStandaloneAspectRatio;
		bool vpMenuBarVisible;
};

#endif
