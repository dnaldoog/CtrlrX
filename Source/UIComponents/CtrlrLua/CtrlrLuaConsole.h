#ifndef __JUCER_HEADER_CTRLRLUACONSOLE_CTRLRLUACONSOLE_8D76A685__
#define __JUCER_HEADER_CTRLRLUACONSOLE_CTRLRLUACONSOLE_8D76A685__

#include "CtrlrWindowManagers/CtrlrChildWindowContent.h"
#include "CtrlrLog.h"

class CtrlrPanel;
class CtrlrLookAndFeel;


class CtrlrLuaConsole  : public CtrlrChildWindowContent,
                         public CtrlrLog::Listener,
                         public KeyListener
{
public:
    CtrlrLuaConsole (CtrlrPanel &_owner);
    ~CtrlrLuaConsole();

	enum ToolbarItems
	{
		_none,
		snipsItem,
		historyItem,
		clearOutput
	};

	enum MoveDirection
	{
		NONE,
		UP,
		DOWN
	};

	void messageLogged (CtrlrLog::CtrlrLogMessage message);
	bool keyPressed (const KeyPress& key, Component* originatingComponent);
	void runCode(const String &code);
	const PopupMenu getSnipsMenu(const int mask);
	const PopupMenu getRecentFilesMenu(const int mask);
	void snipsItemClicked(Button *b);
	void historyItemClicked(Button *b);

	uint8 getType()							{ return (CtrlrPanelWindowManager::LuaConsole); }
	String getContentName()					{ return ("LUA Console"); }
	void buttonClicked (Button *button);
	void getAllToolbarItemIds (Array< int > &ids);
	void getDefaultItemSet (Array< int > &ids);
	ToolbarItemComponent *createItem (int itemId);

	StringArray getMenuBarNames();
	PopupMenu getMenuForIndex(int topLevelMenuIndex, const String &menuName);
	void menuItemSelected(int menuItemID, int topLevelMenuIndex);
	void focusGained(FocusChangeType cause);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    bool keyPressed (const KeyPress& key);
    void clearConsoleOutput();
    void clearConsoleInput();
    Label inputHintLabel;

    juce_UseDebuggingNewOperator

private:
	CtrlrPanel &owner;
	StringArray snips;
	CodeDocument outputDocument, inputDocument;
	StretchableLayoutManager layoutManager;

	// Maintain console history and allow user to explore it via up/down keys
	bool nextUpKeyPressWillbeFirst;
	int lastMoveDirection;
	int lastCommandNumInHistory;
	String currentInputString;
	StringArray inputHistory;

    //==============================================================================
    CodeEditorComponent* luaConsoleOutput;
    CodeEditorComponent* luaConsoleInput;
    StretchableLayoutResizerBar* resizer;


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    CtrlrLuaConsole (const CtrlrLuaConsole&);
    const CtrlrLuaConsole& operator= (const CtrlrLuaConsole&);
};


#endif   // __JUCER_HEADER_CTRLRLUACONSOLE_CTRLRLUACONSOLE_8D76A685__
