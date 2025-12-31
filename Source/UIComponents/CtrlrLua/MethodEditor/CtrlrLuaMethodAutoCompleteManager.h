#pragma once

#include <JuceHeader.h>

/** Represents a single Lua method from the XML */
struct LuaMethod {
    juce::String name;
    juce::String parameters; // e.g., "int index, String name"
    bool isStatic;           // True if found in <static_methods>, False if in <methods>
};

/** Define the types for our icons in the SuggestionPopup */
enum SuggestionType {
    TypeClass,      // Icon "C"
    TypeMethod,     // Icon "M"
    TypeGlobal,     // Icon "V"
    TypeUtility     // Icon "f"
};

/** The item used by the ListBox to display suggestions */
struct SuggestionItem {
    juce::String text;
    SuggestionType type;
};

/** Represents a Lua class and its associated methods */
struct LuaClass {
    juce::String name;
    juce::Array<LuaMethod> methods;        // Instance methods (accessed via :)
    juce::Array<LuaMethod> staticMethods;  // Static methods (accessed via .)
};

class CtrlrLuaMethodAutoCompleteManager {
public:
    CtrlrLuaMethodAutoCompleteManager();
    ~CtrlrLuaMethodAutoCompleteManager() = default;

    /** Parses the XML and populates the internal 'classes' HashMap */
    void loadDefinitions();

    /** * Returns a list of global suggestions (Classes, Utilities, Globals)
     * used when the user is typing a fresh word.
     */
    std::vector<SuggestionItem> getGlobalSuggestions(const juce::String& prefix);

    /** * Context-Aware: Returns methods belonging specifically to a class.
     * @param className The class name (e.g., "CtrlrPanel")
     * @param prefix    The current typing prefix after the ':' or '.'
     * @param includeStatic If true, looks in staticMethods; if false, looks in methods.
     */
    std::vector<SuggestionItem> getMethodSuggestionsForClass(const juce::String& className,
                                                       const juce::String& prefix,
                                                       bool includeInstance);

    /** Returns the parameter string for the CallTip bubble */
    juce::String getMethodParams(const juce::String& methodName);

    /** * Helper to map common Lua variables to their C++ Class Types.
     * You can expand this as you find more common variables.
     */
    juce::String getClassNameForVariable(const juce::String& varName) {
        if (varName == "panel")  return "CtrlrPanel";
        if (varName == "mod")    return "CtrlrModulator";
        if (varName == "comp")   return "CtrlrComponent";
        if (varName == "g")      return "Graphics";
        if (varName == "canvas") return "Component";
        return "";
    }

private:
    /** The main database: Maps Class Name -> LuaClass Object */
    juce::HashMap<juce::String, LuaClass> classes;

    /** Quick lookup lists for global-level autocompletion */
    juce::StringArray classNames;
	juce::StringArray allMethodNames;
    juce::StringArray utilityMethodNames;
    
    // Internal helper to find a method's params across all classes
    LuaMethod* findMethodInCache(const juce::String& methodName);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaMethodAutoCompleteManager)
};
