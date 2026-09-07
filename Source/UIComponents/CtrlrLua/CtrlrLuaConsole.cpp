#include "stdafx.h"
/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  3 Apr 2012 10:45:28pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLuaManager.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrPanel/CtrlrPanel.h"
//[/Headers]

#include "CtrlrLuaConsole.h"

//[MiscUserDefs] You can add your own user definitions and misc code here...
const StringArray joinFileArray(const Array<File> ar) {
	StringArray s;

	for (int i = 0; i < ar.size(); i++) {
		s.add(ar[i].getFullPathName());
	}
	return (s);
}
//[/MiscUserDefs]

//==============================================================================
CtrlrLuaConsole::CtrlrLuaConsole(CtrlrPanel &_owner)
	: owner(_owner), luaConsoleOutput(0), luaConsoleInput(0), resizer(0) {
	addAndMakeVisible(luaConsoleOutput = new CodeEditorComponent(outputDocument, 0));
	luaConsoleOutput->setName(L"luaConsoleOutput");
	luaConsoleOutput->setScrollbarThickness(owner.getOwner().getProperty(Ids::ctrlrScrollbarThickness));

	addAndMakeVisible(luaConsoleInput = new CodeEditorComponent(inputDocument, 0));
	luaConsoleInput->setName(L"luaConsoleInput");
	luaConsoleInput->setScrollbarThickness(owner.getOwner().getProperty(Ids::ctrlrScrollbarThickness));

	addAndMakeVisible(resizer = new StretchableLayoutResizerBar(&layoutManager, 1, false));

	//[UserPreSize]
	// Constructor — Update the resizer bar height
	layoutManager.setItemLayout(0, -0.001, -1.0, -0.65); // Output window (e.g., 65%)
	layoutManager.setItemLayout(1, 12, 22, 24);			 // Resizer Bar: min 12px, preferred 22px, max 24px
	layoutManager.setItemLayout(2, -0.001, -1.0, -0.30); // Input window (e.g., ~30%)

	luaConsoleInput->setFont(
		Font(owner.getCtrlrManagerOwner().getFontManager().getDefaultMonoFontName(), 15, Font::plain));
	luaConsoleOutput->setFont(
		Font(owner.getCtrlrManagerOwner().getFontManager().getDefaultMonoFontName(), 15, Font::plain));
	luaConsoleInput->setColour(
		CodeEditorComponent::backgroundColourId,
		Colour(0xffffffff)); // findColour(CodeEditorComponent::backgroundColourId)); // was Colour(0xffffffff));
	luaConsoleOutput->setColour(
		CodeEditorComponent::backgroundColourId,
		Colour(0xffffffff)); // findColour(CodeEditorComponent::backgroundColourId)); // was Colour(0xffffffff));
	luaConsoleInput->setColour(CodeEditorComponent::highlightColourId,
							   findColour(CodeEditorComponent::highlightColourId));
	luaConsoleOutput->setColour(CodeEditorComponent::highlightColourId,
								findColour(CodeEditorComponent::highlightColourId));
	luaConsoleInput->setColour(CodeEditorComponent::defaultTextColourId,
							   Colour(0xff000000)); // findColour(CodeEditorComponent::defaultTextColourId));
	luaConsoleOutput->setColour(CodeEditorComponent::defaultTextColourId,
								Colour(0xff000000)); // findColour(CodeEditorComponent::defaultTextColourId));
	luaConsoleInput->setColour(CodeEditorComponent::lineNumberBackgroundId,
							   findColour(CodeEditorComponent::lineNumberBackgroundId));
	luaConsoleOutput->setColour(CodeEditorComponent::lineNumberBackgroundId,
								findColour(CodeEditorComponent::lineNumberBackgroundId));
	luaConsoleInput->setColour(CodeEditorComponent::backgroundColourId, Colour(0xfffdf6e3));
	luaConsoleOutput->setColour(CodeEditorComponent::lineNumberTextId,
								findColour(CodeEditorComponent::defaultTextColourId));

	luaConsoleInput->addKeyListener(this);
	owner.getCtrlrManagerOwner().getCtrlrLog().addListener(this);
	nextUpKeyPressWillbeFirst = true;
	lastCommandNumInHistory = -1;
	lastMoveDirection = NONE;
	currentInputString = "";
	// constructor, near the other addAndMakeVisible calls
	// constructor — replace the previous inputHintLabel setup with this
	// Constructor setup:
	addAndMakeVisible(inputHintLabel);
	inputHintLabel.setText("lua editor - commands: Run [Enter]   New Line [Shift + Enter]", dontSendNotification);
	// inputHintLabel.setText("Run [Enter ↵]   New Line [Shift ⇧ + Enter ↵]", dontSendNotification);
	inputHintLabel.setFont(Font(Font::getDefaultSansSerifFontName(), 12.0f, Font::bold));
	inputHintLabel.setJustificationType(Justification::centred);
	// inputHintLabel.setFont(Font(11.0f, Font::bold));
	inputHintLabel.setColour(Label::textColourId, Colours::darkgrey);
	inputHintLabel.setColour(Label::backgroundColourId, Colours::transparentBlack);
	inputHintLabel.setInterceptsMouseClicks(false, false);
	// luaConsoleOutput->setWantsKeyboardFocus(false);
	// luaConsoleInput->grabKeyboardFocus();
	//[/UserPreSize]

	setSize(600, 400);

	//[Constructor] You can add your own custom stuff here..
	snips.addTokens(owner.getProperty(Ids::uiLuaConsoleSnips).toString(), "$", "\'\"");
	//[/Constructor]
}

CtrlrLuaConsole::~CtrlrLuaConsole() {
	//[Destructor_pre]. You can add your own custom destruction code here..
	owner.getCtrlrManagerOwner().getCtrlrLog().removeListener(this);
	//[/Destructor_pre]

	deleteAndZero(luaConsoleOutput);
	deleteAndZero(luaConsoleInput);
	deleteAndZero(resizer);

	//[Destructor]. You can add your own custom destruction code here..
	//[/Destructor]
}
void CtrlrLuaConsole::clearConsoleOutput() {
	outputDocument.replaceAllContent("");
	luaConsoleOutput->moveCaretToEnd(false);
}

void CtrlrLuaConsole::clearConsoleInput() {
	inputDocument.replaceAllContent("");
	luaConsoleInput->moveCaretToEnd(false);
}
//==============================================================================
void CtrlrLuaConsole::paint(Graphics &g) {
	//[UserPrePaint] Add your own custom painting code here..
	//[/UserPrePaint]

	//[UserPaint] Add your own custom painting code here..
	//[/UserPaint]
}

void CtrlrLuaConsole::resized() {
	Component *comps[] = {luaConsoleOutput, resizer, luaConsoleInput};
	layoutManager.layOutComponents(comps, 3, 0, 0, getWidth(), getHeight(), true, true);

	// Give the hint text a clean padded area inside the thicker resizer bar
	inputHintLabel.setBounds(resizer->getX() + 8, resizer->getY(), resizer->getWidth() - 16, resizer->getHeight());
	inputHintLabel.toFront(false);
}

bool CtrlrLuaConsole::keyPressed(const KeyPress &key) {
	//[UserCode_keyPressed] -- Add your code here...
	return false; // Return true if your handler uses this key event, or false to allow it to be passed-on.
				  //[/UserCode_keyPressed]
}

//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
bool CtrlrLuaConsole::keyPressed(const KeyPress &key, Component *originatingComponent) {
	// Ignore key presses originating from anywhere other than the console input
	if (originatingComponent != luaConsoleInput) {
		return false;
	}

	const int keyCode = key.getKeyCode();
	const ModifierKeys mods = key.getModifiers();

	// 1. Enter key handling (Run vs Newline)
	if (keyCode == KeyPress::returnKey) {
		if (mods.isShiftDown()) {
			luaConsoleInput->insertTextAtCaret("\n");
			return true;
		}

		if (!mods.isAnyModifierKeyDown()) {
			runCode(inputDocument.getAllContent());

			if (static_cast<bool>(owner.getProperty(Ids::uiLuaConsoleInputRemoveAfterRun))) {
				inputDocument.replaceAllContent("");
			}
			return true;
		}
	}

	// 2. Command History navigation (Ctrl + Up / Ctrl + Down)
	if (mods.isCtrlDown() && !inputHistory.isEmpty()) {
		if (keyCode == KeyPress::upKey) {
			if (nextUpKeyPressWillbeFirst) {
				currentInputString = inputDocument.getAllContent();
				nextUpKeyPressWillbeFirst = false;
			}

			luaConsoleInput->loadContent(inputHistory[lastCommandNumInHistory]);
			lastCommandNumInHistory = jmax(0, lastCommandNumInHistory - 1);
			lastMoveDirection = UP;
			return true;
		}

		if (keyCode == KeyPress::downKey) {
			// At the end of history: restore unsaved typing buffer
			if (lastCommandNumInHistory >= inputHistory.size() - 1) {
				if (currentInputString.isNotEmpty()) {
					luaConsoleInput->loadContent(currentInputString);
					nextUpKeyPressWillbeFirst = true;
				}
				return true;
			}

			lastCommandNumInHistory = jmin(inputHistory.size() - 1, lastCommandNumInHistory + 1);
			luaConsoleInput->loadContent(inputHistory[lastCommandNumInHistory]);
			lastMoveDirection = DOWN;
			return true;
		}
	}

	return false;
}

void CtrlrLuaConsole::runCode(const String &code) {
	luaConsoleOutput->moveCaretToEnd(false);
	luaConsoleOutput->insertTextAtCaret("\n");
	luaConsoleOutput->insertTextAtCaret(">>> " + code + "\n");
	// add running code into history
	if (code.isNotEmpty()) {
		inputHistory.addIfNotAlreadyThere(code);
		nextUpKeyPressWillbeFirst = true;
		lastCommandNumInHistory = inputHistory.size() - 1;
		lastMoveDirection = NONE;
		currentInputString = "";
	}
	owner.getCtrlrLuaManager().runCode(code);
	// luaConsoleInput->clear();
}

void CtrlrLuaConsole::messageLogged(CtrlrLog::CtrlrLogMessage message) {
	if (message.level == CtrlrLog::Lua) {
		// luaConsoleOutput->setCaretPosition (luaConsoleOutput->getText().length());
		luaConsoleOutput->insertTextAtCaret(message.message + "\n");
	}
	if (message.level == CtrlrLog::LuaError) {
		// luaConsoleOutput->setCaretPosition (luaConsoleOutput->getText().length());
		luaConsoleOutput->insertTextAtCaret(message.message + "\n");
	}
}

const PopupMenu CtrlrLuaConsole::getSnipsMenu(const int mask) {
	PopupMenu m;

	for (int i = 0; i < snips.size(); i++) {
		m.addItem(mask + i, snips[i]);
	}

	return (m);
}

void CtrlrLuaConsole::snipsItemClicked(Button *b) {
	// 1. Heap-allocate PopupMenu so it survives beyond this function scope
	auto m = std::make_shared<PopupMenu>();

	m->addItem(1, "Add input to snips");
	m->addSubMenu("Run snip", getSnipsMenu(1024));
	m->addSubMenu("Remove snip", getSnipsMenu(4096));
	m->addItem(2, "Toggle input removal after run", true,
			   (bool)owner.getProperty(Ids::uiLuaConsoleInputRemoveAfterRun));

	// 2. Wrap 'this' in a SafePointer to prevent use-after-free if the Console closes
	Component::SafePointer<CtrlrLuaConsole> safeThis(this);

	// 3. Call showMenuAsyncSafe targeting button 'b'
	PU::showMenuAsyncSafe(*m,
						  b, // Target component (Button) passed as 2nd argument
						  [this, safeThis, m](int ret) {
							  // Check component safety before handling result
							  if (safeThis == nullptr || ret == 0)
								  return;

							  if (ret == 1) {
								  snips.add(inputDocument.getAllContent());
							  } else if (ret >= 1024 && ret < 4096) {
								  runCode(snips[ret - 1024]);
							  } else if (ret >= 4096) {
								  snips.remove(ret - 4096);
							  } else if (ret == 2) {
								  owner.setProperty(Ids::uiLuaConsoleInputRemoveAfterRun,
													!owner.getProperty(Ids::uiLuaConsoleInputRemoveAfterRun));

							  } else if (ret == 4) {
								  inputDocument.getAllContent().clear();
							  }
							  owner.setProperty(Ids::uiLuaConsoleSnips, snips.joinIntoString("$"));
						  });
}

StringArray CtrlrLuaConsole::getMenuBarNames() {
	const char *const names[] = {"File", "View", "Actions", nullptr};
	return StringArray(names);
}

PopupMenu CtrlrLuaConsole::getMenuForIndex(int topLevelMenuIndex, const String &menuName) {
	PopupMenu menu;
	if (topLevelMenuIndex == 0) {
		menu.addItem(2, "Add input to snips");
		menu.addSubMenu("Run snip", getSnipsMenu(1024));
		menu.addSubMenu("Remove snip", getSnipsMenu(4096));
		// menu.addSeparator(); Updated v5.6.31
		// menu.addItem (1, "Close", false); // Updated v5.6.31
	} else if (topLevelMenuIndex == 1) {
		menu.addItem(3, "Remove test code after run?", true,
					 (bool)owner.getProperty(Ids::uiLuaConsoleInputRemoveAfterRun));
	} else if (topLevelMenuIndex == 2) {
		menu.addItem(10, "Clear Console");
		menu.addItem(11, "Clear Input");
	}

	return (menu);
}

void CtrlrLuaConsole::menuItemSelected(int menuItemID, int topLevelMenuIndex) {
	if (topLevelMenuIndex == 0 && menuItemID == 1) {
		// close handle
		// owner.getWindowManager().toggle (CtrlrPanelWindowManager::LuaConsole, false); // Crashes
	}
	if (menuItemID == 2) {
		snips.add(inputDocument.getAllContent());
	}
	if (menuItemID >= 1024 && menuItemID < 4096) {
		runCode(snips[menuItemID - 1024]);
	}
	if (menuItemID >= 4096) {
		snips.remove(menuItemID - 4096);
	}
	if (menuItemID == 3) {
		owner.setProperty(Ids::uiLuaConsoleInputRemoveAfterRun,
						  !owner.getProperty(Ids::uiLuaConsoleInputRemoveAfterRun));
	}
	if (menuItemID == 10) {
		clearConsoleOutput();
		return; // skip the trailing setProperty(uiLuaConsoleSnips...) call below, it doesn't apply here
	}
	if (menuItemID == 11) {
		clearConsoleInput();
		return;
	}
	owner.setProperty(Ids::uiLuaConsoleSnips, snips.joinIntoString("$"));
}

void CtrlrLuaConsole::focusGained(FocusChangeType cause) {
	luaConsoleInput->grabKeyboardFocus();
}
//[/MiscUserCode]

//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CtrlrLuaConsole" componentName=""
                 parentClasses="public CtrlrChildWindowContent, public CtrlrLog::Listener, public KeyListener"
                 constructorParams="CtrlrPanel &amp;_owner" variableInitialisers="owner(_owner)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="600" initialHeight="400">
  <METHODS>
    <METHOD name="keyPressed (const KeyPress&amp; key)"/>
  </METHODS>
  <BACKGROUND backgroundColour="0"/>
  <GENERICCOMPONENT name="luaConsoleOutput" id="cf0696d15c4f91e3" memberName="luaConsoleOutput"
                    virtualName="" explicitFocusOrder="0" pos="0 0 0M 69%" class="CodeEditorComponent"
                    params="outputDocument, 0"/>
  <GENERICCOMPONENT name="luaConsoleInput" id="9630267470906dc" memberName="luaConsoleInput"
                    virtualName="" explicitFocusOrder="0" pos="0 70% 0M 30%" class="CodeEditorComponent"
                    params="inputDocument, 0"/>
  <GENERICCOMPONENT name="" id="f4fe604fd1cb0e52" memberName="resizer" virtualName=""
                    explicitFocusOrder="0" pos="0 69% 0M 1%" class="StretchableLayoutResizerBar"
                    params="&amp;layoutManager, 1, false"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
