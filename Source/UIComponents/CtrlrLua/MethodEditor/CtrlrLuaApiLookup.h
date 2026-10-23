/*
  ==============================================================================

    CtrlrLuaApiLookup.h
    Created: 19 Sep 2026
    Author:  zan64

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <unordered_map>
#include <vector>

struct ApiMethod {
    juce::String name;
    juce::String type;   // "instance" or "static"
    juce::String args;   // e.g., "(float, float)"
    int overload = 1;
};

struct ApiClass {
    juce::String name;
    std::vector<ApiMethod> methods;
    std::vector<ApiMethod> staticMethods;
};

class CtrlrLuaApiLookup {
public:
    static CtrlrLuaApiLookup& getInstance() {
        static CtrlrLuaApiLookup instance;
        return instance;
    }

    const ApiClass* getClassInfo(const juce::String& className) const {
        auto it = classMap.find(className);
        return (it != classMap.end()) ? &it->second : nullptr;
    }

    bool hasClass(const juce::String& className) const {
        return classMap.find(className) != classMap.end();
    }
juce::StringArray getAllClassNames() const {
    juce::StringArray names;
    for (const auto& pair : classMap) {
        names.add(pair.first);
    }
    names.sort(true);
    return names;
}
private:
    CtrlrLuaApiLookup(); // Populated by your generated .cpp file
    
    std::unordered_map<juce::String, ApiClass> classMap;
};