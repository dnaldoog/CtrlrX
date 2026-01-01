#pragma once

#include <JuceHeader.h>

/** Represents a single Lua method from the XML */
struct LuaMethod {
    juce::String name;
	juce::String parameters; // e.g., "int index, String name"
    bool isStatic;
};

/** Define the types for our icons */
enum SuggestionType {
    TypeClass,
    TypeMethod,
    TypeGlobal,
    TypeUtility
};
/** Defines whether we are looking for instance methods (:) or static methods (.) */
enum LookupType
{
    LookupInstance,
    LookupStatic
};
struct SuggestionItem {
    juce::String text;
    SuggestionType type;
};

/** Represents a Lua class and its associated methods */
struct LuaClass {
    juce::String name;
    juce::Array<LuaMethod> methods;
};

// Renamed to match your filenames and the .cpp implementation
class CtrlrLuaMethodAutoCompleteManager {
public:
    CtrlrLuaMethodAutoCompleteManager();
    ~CtrlrLuaMethodAutoCompleteManager() = default;

    void loadDefinitions();

    /** * UPDATED: Both now return a vector of SuggestionItems
     * to support icons in the popup.
     */
    std::vector<SuggestionItem> getGlobalSuggestions(const juce::String& prefix);
    std::vector<SuggestionItem> getMethodSuggestions(const juce::String& className,
        const juce::String& prefix,
        LookupType type);
    juce::String resolveClass(const juce::String& symbol, const juce::String& fullDocumentText);
	juce::String getMethodParams(const juce::String& methodName);

private:
    juce::HashMap<juce::String, LuaClass> classes;
    juce::StringArray classNames;
    juce::StringArray allMethodNames;
    juce::StringArray utilityMethodNames; // Store utilities separately for the "f" icon

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaMethodAutoCompleteManager)
};
