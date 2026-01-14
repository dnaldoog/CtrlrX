#pragma once
#include <JuceHeader.h>

struct LuaMethod {
    juce::String name;
    juce::String parameters;
    bool isStatic;
};

struct LuaClass {
    juce::String name;
    juce::String parentClass;
    juce::Array<LuaMethod> methods;
};

enum SuggestionType {
    TypeClass,
    TypeMethod,       // Instance Method
    TypeStaticMethod, // Static Method (S)
    TypeEnum,         // Enum/Value (E)
    TypeGlobal,       // Variable/Global
    TypeUtility
};

enum LookupType { LookupInstance, LookupStatic };

struct SuggestionItem {
    juce::String text;
    SuggestionType type;
};

class CtrlrLuaMethodAutoCompleteManager {
public:
    CtrlrLuaMethodAutoCompleteManager();

    void loadDefinitions();

    // The Resolver
    juce::String resolveClass(const juce::String& symbol, const juce::String& fullDocumentText);

    // The Suggester
    std::vector<SuggestionItem> getGlobalSuggestions(const juce::String& prefix);

    // The Method Suggester (Now handles the tree climbing)
    std::vector<SuggestionItem> getMethodSuggestions(const juce::String& className,
        const juce::String& prefix,
        LookupType type);

    // The Parameter Lookup (Ensure this takes TWO arguments)
    juce::String getMethodParams(const juce::String& className, const juce::String& methodName);
    juce::String getClassNameForVariable(const juce::String& varName, const juce::String& code);
    juce::String resolveReturnType(const juce::String& className, const juce::String& methodName);
private:
    juce::HashMap<juce::String, LuaClass> classes;
    juce::StringArray classNames;
    juce::StringArray allMethodNames;
    juce::StringArray utilityMethodNames;

};