#ifndef CTRLR_LUA_MANAGER
#define CTRLR_LUA_MANAGER

#ifdef _WIN32
#pragma warning(disable:4100)
#endif

#include "Methods/CtrlrLuaMethodManager.h"
#include "CtrlrLuaApiDatabase.h"

// Forward declarations
class LAudioFormatManager;
class CtrlrPanel;
class CtrlrModulator;
class CtrlrLuaAudioConverter;
class CtrlrLuaUtils;
class CtrlrLuaMultiTimer;
class CtrlrLuaDebugger;

class CtrlrLuaManager : public ValueTree::Listener
{
public:
	CtrlrLuaManager(CtrlrPanel& _owner);
	~CtrlrLuaManager();

	void assignDefaultObjects(lua_State* L);
	void wrapBasicIO(lua_State* L);
	void wrapCore(lua_State* L);
	void wrapCtrlrClasses(lua_State* L);
	void wrapJuceClasses(lua_State* L);
	void wrapJuceCoreClasses(lua_State* L);
	void wrapConverters(lua_State* L);
	void wrapUtilities(lua_State* L);

	bool runCode(const String& code, const String name = "");

	void setProperty(const Identifier& name, const var& newValue, const bool isUndoable = false);
	const var& getProperty(const Identifier& name) const { return luaManagerTree.getProperty(name); }
	const var getProperty(const Identifier& name, const var& defaultReturnValue) const { return luaManagerTree.getProperty(name, defaultReturnValue); }

	UndoManager* getUndoManager() { return (0); }
	ValueTree& getLuaManagerTree() { return (luaManagerTree); }
   const CtrlrLuaApiDatabase* getDatabase() const { return &luaApi; }
	void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property);
	void valueTreeChildrenChanged(ValueTree& treeWhoseChildHasChanged) {}
	void valueTreeParentChanged(ValueTree& treeWhoseParentHasChanged) {}
	void valueTreeChildAdded(ValueTree& parentTree, ValueTree& child) {}
	void valueTreeChildRemoved(ValueTree& parentTree, ValueTree& child, int) {}
	void valueTreeChildOrderChanged(ValueTree& parentTreeWhoseChildrenHaveMoved, int, int) {}

	static void log(const String& message);

	void createLuaState();
	void createLuaStateAudio();
	void restoreState(const ValueTree& savedState);

	const String getLastError() { return (lastError); }
	lua_State* getLuaState() { return (luaState); }
	lua_State* getLuaStateAudio() { return (luaStateAudio); }

	const bool isRestoring();
	const bool isLuaDisabled();

	CtrlrPanel& getOwner() { return (owner); }
	CtrlrLuaMethodManager& getMethodManager();
	CtrlrLuaDebugger& getDebugger();

	// Access to Lua API database
	CtrlrLuaApiDatabase& getLuaApiDatabase() { return luaApi; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaManager);

	CtrlrLuaMethodManager* methodManager;
	CtrlrLuaDebugger* ctrlrLuaDebugger;
	CtrlrLuaUtils* utils;
	CtrlrLuaMultiTimer* multiTimer;
	CtrlrLuaAudioConverter* audioConverter;
	LAudioFormatManager* luaAudioFormatManager;
	CtrlrPanel& owner;
	ValueTree luaManagerTree;

	lua_State* luaState;
	lua_State* luaStateAudio;

	String lastError;
	File scriptsDir;
	File importersDir;

	// Lua API database for class browser
	CtrlrLuaApiDatabase luaApi;
};

#endif // CTRLR_LUA_MANAGER