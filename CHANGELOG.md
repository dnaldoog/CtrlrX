
### CtrlrX v8.63 - Changelog 10/05/2026

- Added label component save object to JSON file
- Repositioned Save patch to File menu
- Removed Edit menu in restricted instances
- Removed *Save CTRLR State* menu item placeholder
- https://github.com/dnaldoog/CtrlrX/commit/4c8e78bb17e3e0ae70d17f09dc1ad61809d39561

### CtrlrX v8.62 - Changelog 10/04/2026

- Added AboutWindow and Send Snapshot to Internal Functions callback.
- Added [Flag Matrix TAB](https://github.com/dnaldoog/CtrlrX/commit/9c11bea382a0bd152945dd68516e5035e7d9070f) for "is Static" "Save To File" and "Exclude Snapshot"
with fuzzy search
- Reimplemented Snapshot save/load from/to file.
- Safe JSON Parsing: Robust DynamicObject property extraction.
- Smart Filtering: Defaulting to save parameters while bypassing static UI elements.
- MIDI Bus Protection: Pausing panelMidiPauseOut during parameter application to prevent hardware choking.
- Confirmation Dialogs: Preventing accidental patch overwrites.
- Add panel UID as check for loading JSON file.
- Added clipboard of UID so user can edit JSON file directly if dev changed UID.
 and the JSON recorded the old UID.
- AlertWindow dialog for end user to set Global MIDI delay and snapshot MIDI delay.
- Fix Modulator list [ViewTree crash](https://github.com/dnaldoog/CtrlrX/commit/cd770ef997390e02a5b701be7062e8080a108a59)
- Fix [Fuzzy search](https://github.com/dnaldoog/CtrlrX/commit/385a67185b5ef0454b1f52e45e69b568d3035423) for Modulator list ViewTree
- https://github.com/dnaldoog/CtrlrX/commit/4bae97756434339d20cc91f3cebbe9adaca0c809

### CtrlrX v8.61 - Changelog 10/02/2026
- Fixed duplicate	panels appearing in exported instance (restricted was okay)
- Restored previously removed what() and how() functions critical	to the gui version in lua editor
- Added weak reference guard to exporting panel in Async	environment
- https://github.com/dnaldoog/CtrlrX/commit/740f3525c82738c917be052ffafc44619c17337f


### CtrlrX v8.60 -  Changelog 09/29/2026

- Refactor CtrlrEditor.cpp/h
- Contrasting font to BG property editor
- Added resize grips
- fix screen zoom in scaled panel mode in editor.
- fixed RPM error expecting libcurl to be compiled using GnuTLS
- https://github.com/dnaldoog/CtrlrX/commit/7da8f958ddceec7ab26ed639845ac2b2a475284e

### CtrlrX v8.59 -  Changelog 08/28/2026
- Rewrite of exported Viewport logic
- - Added three options
  1. **Scrollable** (Panel sits within designated Viewport with scrollbars)
  2. **Fixed** (panel cannot be resized)
  3. **Scaled** (panel stretches with mouse drag)
- Added remove property menu option
- Added help for Viewport options
- https://github.com/dnaldoog/CtrlrX/commit/69ad2ea5dd3954f246d055e636a1b45f07d43ae1
### CtrlrX v8.58 -  Changelog 09/25/2026
- reduced preferences window size for smaller screens
- added guard against null `CtrlrLuaMethodProperty::refresh()  if (owner == nullptr) {}`
- https://github.com/dnaldoog/CtrlrX/commit/3ceffc7d223cbbae368526e4bdee13a3b092d057
### CtrlrX v8.57 -  Changelog 09/24/2026
- Removed tables from `methodList `combo
- Alphabetically sorted by User,(Mouse) Event,Callback,System
- Version 8.57 updates some code that somehow wasn't committed for version 8.56
- https://github.com/dnaldoog/CtrlrX/commit/c4ff271f811116b62b12c41db33a1fe1fc55743e
### CtrlrX v8.55 -  Changelog 09/23/2016
- updated nondescriptive label names in CtrlrLuaMethodCodeEditorSettings.cpp
- Fixed long standing issue with uiImageButton unresponsive to mouse events
- https://github.com/dnaldoog/CtrlrX/commit/27b87b250e435c7513cdcfd5e7f422b3e9f213e9

### CtrlrX v8.54 -  Changelog 09/22/2016

- Fixed new overload Luabind missing definition getResourceAsImage() for SVG parsing.
- https://github.com/dnaldoog/CtrlrX/commit/67d11ac83252eaf3cc1fd33fffc5dcde41d7d6bd
- Included rcedit in iss installer / changed YML to shell: pwsh for Windows Installer
-  - (_Custom icons were not working in remote CI but working locally_)
- - **macOS**: Exported panel instances (standalone, AU, VST3, AAX) could
  fail to launch on Apple Silicon with no error message, due to an
  invalidated code signature after export. Exports are now re-signed
  automatically (ad-hoc by default, or with a configured certificate).
  Verify with `codesign -dv --verbose=4 <path>`.
### CtrlrX v8.53 -  Changelog 09/21/2026

- Linux can now export restricted instance with custom SVG icon
- Linux export option automatically to `/home/$USER/.local/bin`
- - Custom icon and app will show in Linux app menu
- Fuzzy search for Modulator List
- https://github.com/dnaldoog/CtrlrX/commit/0c8d99260364488cadcb359ec55f1320507416d4
### CtrlrX v8.52 -  Changelog 09/19/2026
- Removed all obsolete debugger code
- MacOS - can now export app with custom icon
- Added Simple LuaAPI class browser to lua Editor 
    - Based on data returned by what(o) and how(), not from LuaAPI.xml - coming directly from luabind's internal dynamic reflection system.
- https://github.com/dnaldoog/CtrlrX/commit/6b7eea778c8e3fe07d423c8cc83bc787bdc6905f
### CtrlrX v8.51 -  Changelog 09/19/2026
- Add SVG as background; scales to current window size
- Windows only - export executable with custom icon
- Refactored [dialog windows](https://github.com/damiensellier/CtrlrX/issues/327#issuecomment-5723388623) 
- https://github.com/dnaldoog/CtrlrX/commit/787c352e1b040c7408245be2b2183f189b347cb4
- https://github.com/dnaldoog/CtrlrX/commit/e88f4308040fb3d478b3ffebc774fd92c83b0c50
- https://github.com/dnaldoog/CtrlrX/commit/3ef9c59773f0735b4c86614223fc8a0e0e4b2b51
- https://github.com/dnaldoog/CtrlrX/commit/5f8ed2fddf9c249f88f9adb4a5d1d0e7615b59e3

### CtrlrX v8.50 -  Changelog
- Fix segfault crash in Linux DAW Ardour [Re:](https://github.com/damiensellier/CtrlrX/issues/328)
- Add PluginHostType/SystemStats to [Luabind](https://github.com/damiensellier/CtrlrX/commit/6f0125670935d511ce37b6ecb808f0f2f87fa02c)
- Updated LuaAPI.xml to include various new Async functions
- https://github.com/dnaldoog/CtrlrX/commit/d39a776599cbd5c2c611bbc46df948888534b1a3

September 17 2026

### CtrlrX v8.49 -  Changelog
- Fix crash on second launch: restoreState() destroyed the active panel before restoring into it
- Fix missing Native dialog overwrite prompt - refactor namespace FC
- https://github.com/dnaldoog/CtrlrX/commit/9277436c5095b5dee4204e844a7eacd31cf532b3


September 16 2026
### CtrlrX v8.48 -  Changelog
- Added `panel:isWaylandSession()` detection script
- Added Warning window for dev using panel drag and drop handlers
- Refactored Wayland detection code
- https://github.com/dnaldoog/CtrlrX/commit/706330b86d05c17a6434d074325f8c84b9f69e96

September 10 2026

### CtrlrX v8.47 -  Changelog
- Update CtrlrMac.cpp codesigning
- https://github.com/damiensellier/CtrlrX/commit/a41c63d25ea1b9a8217e2740bea22868c4da1246

September 10 2026
### CtrlrX v8.46 -  Changelog
- Removed white square surround from Gnome tray logo `CMakeLists.txt`
- Added Help menu item feature for Ctrlr Expressions
- Fixed missing tray icon in Debian
- Debian / vst3 name plugin not detected `cat "$HOME/.vst3/CtrlrX.vst3/Contents/Resources/moduleinfo.json"` changed / CtrlrLinux.cpp trim function on plugin name
- Reverted CtrlrMac.cpp 
- https://github.com/dnaldoog/CtrlrX/commit/21101af42c5a9ec3c7026a443a71efc806cf86a8

September 09 2026
### CtrlrX v8.45 -  Changelog
- rpm/deb install to non existent ~/.vst3 folder was setting root permissions
- Linux settings and Resources are now stored in `~/.config/CtrlrX` - _not_ `~/CtrlrX `(_note that current folder `~/CtrlrX` will be moved on load_)
- https://github.com/dnaldoog/CtrlrX/commit/4231b36030352d0997918a616bba742e0e622168

September 08 2026

### CtrlrX v8.44 -  Changelog
- Changed LuaJIT toggle text `Enable Compiler/Disable Compiler `
- Restored Windows Installer
- https://github.com/dnaldoog/CtrlrX/commit/190693989aa805bd80d1b4c4292cfb5798f8a519

September 08 2026

### CtrlrX v8.43 -  Changelog

- Added desktop icons for Linux
- DEB RPM installers
- tar.gz generic file still available
- Added support for preventing crash in MacOS codesigning LuaJIT jit code (_untested_)
- CMakeLists.txt switch for LuaJIT off MacOS
-  Added LuaJIT guards for MacOS 
- https://github.com/dnaldoog/CtrlrX/commit/52c0b9f70c797c53c065e4536a0ac5b7cf65c0d7
- https://github.com/damiensellier/CtrlrX/commit/f2bf15a166d818f124b5a69adaaf3bd167283758

September 7 2026

### CtrlrX v8.42 -  Changelog

- //Commented out  Disable LuaJIT compiler on macOS to prevent W^X memory protection crashes with #if 0
- https://github.com/dnaldoog/CtrlrX/commit/e8ee92cc0e3eec8670f1bffe7c67df0a1d57e1a1

September 4 2026
### CtrlrX v8.41 -  Changelog
- Disable LuaJIT on macOS under JUCE_MAC to [avoid security crashes](https://github.com/dnaldoog/CtrlrX/blob/20ed462fa340b2e5bff1eb984704beba598479f9/Source/Misc/luabind/examples/cpp_lua_roundtrip.cpp#L62)
- Added pure virtual function declaration back in `void customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel = nullptr) {}`
- Changed logo to red theme to help determine which version is running in tray (will eventually revert back to black and white logo)
- Added radio button option for CtrlrToggleButton using LNF themes
- https://github.com/dnaldoog/CtrlrX/commit/c1d88935f7fbd2b75d09dfa91a368a3ff5f68165

September 3 2026
### CtrlrX v8.40 -  Changelog
- Restored JUCE_PLUGIN_NAME to CmakeLists.txt to ensure correct panel name in DAW
- https://github.com/dnaldoog/CtrlrX/commit/fba1ce721a50a9d6ddb85532018a1c2dd7aec4fa

August 29 2026
### CtrlrX v8.39 -  Changelog
-  replacement UTF-16 MemoryBlock needed padding to match the exact byte size of the search block CtrlrWindows.cpp
- https://github.com/dnaldoog/CtrlrX/commit/b3ab55bdff682cd20ad68b791ea04c712409d00a

August 27 2026
### CtrlrX v8.38 - Changelog
- update CtrlrWindows.cpp to #if JUCE_WINDOWS, not JUCE 8
- https://github.com/dnaldoog/CtrlrX/commit/0bd609d897ea5bb1d18ce4d6af2869063e157b65
### CtrlrX v8.37 - Changelog
-  Plain ASCII search blocks fail to match UTF-16 strings because of interleaved null bytes (`0x00`) in Windows VST.3 panel name
- Windows vst3 exports were named CtrlrX.vst3
- https://github.com/dnaldoog/CtrlrX/commit/433980b70fc48043c678734b59ac582e21cc6e19

August 27 2026
### CtrlrX v8.36 - Changelog
- Fixed CtrlrPanel.cpp bug with guard  `if (outputDevicePtr != nullptr) {      outputDevicePtr->closeDevice(); }` *line 649*
- https://github.com/dnaldoog/CtrlrX/commit/6f14285761284ac2f1e3403be9dfc5e853edf2c5

August 27 2026
### CtrlrX v8.35 - Changelog 
- lua can now write utf-8 characters to panel
- https://github.com/dnaldoog/CtrlrX/commit/ef3c811ddb1d9abc6a17c084c3407820ec571728

August 27 2026
### CtrlrX v8.34 - Changelog 
- MIDI devices were not refreshing on reload in Linux. Added 1.25ms delay before scanning MIDI devices on load.
- https://github.com/dnaldoog/CtrlrX/commit/3c2e014e25da45a09d2ce95f37b95c5b8661af42

August 26 2026
### CtrlrX v8.33 - Changelog 

August 25 2026
- Added text editor for uiButtton https://github.com/RomanKubiak/ctrlr/commit/a77a961fbfaa7362e6eba38e2d2cc05112cc79ba

### CtrlrX v8.32 - Changelog 

- Fixes unresponsive IncDec colour change for CtrlrSlider/CtrlrFixedSlider LNF- custom colours
- Fixed missing background image in CtrlrGroup

- https://github.com/dnaldoog/CtrlrX/commit/ed93a1e35f3ccfc0fd22d49326d64b01fefe18cb

August 25 2026

### CtrlrX v8.31 - Changelog 



- Added luabind support for 8/9 argument drawFittedText in LGraphics.cpp
- Fixed uiGroup not updating old panels' settings.
- https://github.com/dnaldoog/CtrlrX/commit/42f952f920d9fed98e0d372cde7e34eb31454f41

August 24 2026

### CtrlrX v8.30 - Changelog 




- askTextForInputWindow Async/Sync
- utils.openMultipleFilesWindow (never working before in previous versions of Ctrlr(X))
- utils.openFileWindow()
- Fixed Ctrlr+/ not commenting all lines on multiple line selection (See v8.29 - side effect)
- https://github.com/dnaldoog/CtrlrX/commit/28b504a7a511a4625fa40b8d4b001172bf204613

August 23 2026

### CtrlrX v8.29 - Changelog 



- Fixed lua comment code Ctrlr+/ commenting out line below
- CtrlrGroup now uses  Ids::uiGroupLookAndFeelIsCustom to record custom settings and recall after setting LNF 
- https://github.com/dnaldoog/CtrlrX/commit/2b5d32c5caf4b4decfc33e7483bda3639ecbbfad

August 23 2026


### CtrlrX v8.28 - Changelog

August 22 2026

- CtrlrToggleButton now uses  Ids::uiButtonLookAndFeelIsCustom to record custom settings and recall after setting LNF 
- Added support for LNF/User colour retention in uiCombo 
- Added missing Play button in Debug Window
- https://github.com/dnaldoog/CtrlrX/commit/d7c60591aee6cd337bfb75f91c1ecb0fda4c4a1f
### CtrlrX v8.27 - Changelog

August 21 2026

* Add recall user colours for
    * uiButton
     * uiFixedSlider
     * uiSlider
- Restored missing fonts to lua debugger


### CtrlrX v8.26 - Changelog

August 21 2026

- Added Async questionWindow function and backwards compatibility for older Sync version
- In progress fix for buggy LNF toggling between LNF and custom colours
- Fixed blank property filed for panel properties
- https://github.com/dnaldoog/CtrlrX/commit/c0202e861dafaef6228feb3f35cccd0f7dbe9ee1
### CtrlrX v8.25 - Changelog

August 17 2026

Combo always aligning left even when set to centre on load
- https://github.com/dnaldoog/CtrlrX/commit/85dac7e3ea9e93ae46fcc2d746b085dbefd0f413
### CtrlrX v8.24 - Changelog
August 17 2026
- uiCombo setProperty was set to String "centred" which produced 0 not 36
- https://github.com/dnaldoog/CtrlrX/commit/5afdcd0447c4029b0a858e1e2bff79f8151b2a7e
### CtrlrX v8.23 - Changelog
- Fixed right click menu squashing down to panel size. On very small panels was unusable
- Changed highlight colour in CtrlrpanelEditor darkMagenta/White
- Added 'red' Ctrlr logo to about page (Temporary for development)
- https://github.com/dnaldoog/CtrlrX/commit/5a5dae6fd9cda6ddd678c3cd710816ecdaee2842
### CtrlrX v8.22 - Changelog
- Fixed `m:showAt(comp,h)` freezing Ctrlr on close by making the menu non-blocking: the click completes normally,
and the menu's action now runs once the user actually makes a
selection, instead of the app freezing to wait for it.
- https://github.com/dnaldoog/CtrlrX/commit/0f878fb039e0e6d55048b705022945255e8627ca
### CtrlrX v8.21 - Changelog
- Add Luabind support for ret=PopupMenu:showAt(comp,height);
- Fix for Fuzzy Search / popup menus appearing as overlay on second mouse enter
- About window shows build version, not JUCE version
- https://github.com/dnaldoog/CtrlrX/commit/52203c3872b55f4c1470b7a93e126093302e7111
### CtrlrX v8.20 - Changelog
- Add backspace to fuzzy search.
- https://github.com/dnaldoog/CtrlrX/commit/b4e14a0be2dc4de1640f08c3baccb4e61e4b2ed2
### CtrlrX v8.19 - Changelog
- Added right click create table/class in CtrlrLuaEditor
- https://github.com/dnaldoog/CtrlrX/commit/b5e6c447dfd773cda5af1732d5b91f36dd40254f
### CtrlrX v8.18 - Changelog
- restored valueTreePropertyChanged from 5.6.36 - uiSlider value colour was overriden with 'white' colour
- Added initLookAndFeelDefaults in CtrlrInlineUtilitiesGUI.h gui{} to address findColour() default asserts noise
### CtrlrX v8.17 - Changelog
- Made improvements to Layer Drag Drop
- Layer Editor now has indicator line to show drag drop destination target
- Smoother drag/drop experience. Various refactors using Base Juce code
- https://github.com/dnaldoog/CtrlrX/commit/088bb3abc8a0d5a9d37250811104064f7b191cd3
### CtrlrX v8.16 - Changelog
- Fix FuzzySearch toggle EditMode fix re: void CtrlrCombo::panelEditModeChanged(const bool isInEditMode)
- Fix infinite loop _crash_ with mouseMove on components JUCE 8 only
- Refactored console editor. Added clear input/output options
- https://github.com/dnaldoog/CtrlrX/commit/b67f930b0dfca67747078850d62c8cff7b16cba4
### CtrlrX v8.15 - Changelog
- Fix FuzzySearch toggle EditMode disengaging (thanks Damien Sellier)
- https://github.com/damiensellier/CtrlrX/commit/1d9a6b16ebad6b43e45bc75e5ba7a9570413066f
- https://github.com/dnaldoog/CtrlrX/commit/4deb1f33d2c28c1004eb0e8d440b78eb73be9818
### CtrlrX v8.14 - Changelog
- Redesign/refactor of MIDI Monitor
- Right click MIDI monitor 'Clear' option
- Added option to clear all MIDI types for display
- New colour theme
- Added MIDI Device name to MIDI in/out title (Window must be closed to refresh)
- Restored on/off AutoComplete toggle in lua editor preferences
- https://github.com/dnaldoog/CtrlrX/commit/e0ead77882ef904ba5fc53c01740b0cf14440283
### CtrlrX v8.13 - Changelog
- Added on/off switch for autocomplete in lua editor preferences
- https://github.com/dnaldoog/CtrlrX/commit/7303c16b9b32b4686fd1471ce9df269e267d7623
### CtrlrX v8.12 - Changelog
- Some Properties were missing in CtrlrSlider.cpp valueTreePropertyChanged()
- https://github.com/dnaldoog/CtrlrX/commit/7303c16b9b32b4686fd1471ce9df269e267d7623
### CtrlrX v8.11 - Changelog
- Add theming for Combo LNF _uiComboMenuHighlightColour_
- https://github.com/dnaldoog/CtrlrX/commit/50bfd787bc6968f9bcbb4565ff3200c5d15d1ba7
### CtrlrX v8.10 - Changelog
- Reverted CtrlrCombo.cpp to https://github.com/dnaldoog/CtrlrX/commit/9b54b5c9202357a789bd6140b8817855e184dfc8
### CtrlrX v8.09 - Changelog
- Restored styling of uiSlider, uiFixedSlide
- Restored Ctrlr &lt;Sans-serif&gt; default fonts
- Fixed _FONT_Electronic Highway Sign_ not changing size
### CtrlrX v8.08 - Changelog
- FONT_LCD was uniquely triggering the CoreText 0xffff pointer crash on Apple Silicon M2
- FONT_LCD.ttf had 'hyphen' incorrectly mapped to U+2010 // should be U+002D which was blank.
- Fixed Digi.ttf not rendering correctly
- Now you can add hyphen to text in FONT_LCD.
- Also renamed Font name and Family name to match to avoid 'missing' error when panel was saved in Windows but loaded in MacOS
- Added guards to CtrlrFontManager to prevent Font Loading crash in MacOS M2 (not tested)
- Added default findColour() to CtrlrManager constructor to avoid numerous asserts in debug mode with older panels
- https://github.com/dnaldoog/CtrlrX/commit/e2ad825427079d7446cd9b75bd7859d81408cc65
### CtrlrX v8.07 - Changelog
* Restore missing Save panel code
* Restored save file notification for Save/Save versioned/Save As ...
* Fixed CtrlrPanel not updating default Constructor _Panel name_
### CtrlrX v8.06 - Changelog
* Scaling fix for LNF uiCombo Arrows
* Fixed Font rendering in Combo
* https://github.com/dnaldoog/CtrlrX/commit/ca3f9367593565c1f4c11f6783d53ecfb61f83dd
### CtrlrX v8.05 - Changelog
* JUCE 8 LookAndFeel_V4 modernization & CtrlrCombo LF refactor
* fixed LNF for combo
* https://github.com/dnaldoog/CtrlrX/commit/85df57a1313b8d21273cbecff3540b39a380b461
### CtrlrX v8.04 - Changelog
* **Right click editor menu**  positions near mouse
* https://github.com/dnaldoog/CtrlrX/commit/23b48ece290bc39be05814c4e11e430975ae6871
### CtrlrX v8.03 - Changelog
* **Fixed LookAndFeel** for uiSlider. Fixed default V3 theme for uiCombo
* If custom LNF set on component and panel LNF changed, custom LNF remains.
