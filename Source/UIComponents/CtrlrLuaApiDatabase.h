#pragma once
#include "stdafx.h"
/*
    CtrlrLuaApiDatabase
    Loads and exposes Lua API information generated from luabind bindings.
    XML source: Source/Resources/XML/LuaAPI.xml
    JUCE 6 safe
    Ctrlr coding style
*/
class CtrlrLuaApiDatabase
{
public:
    // ==========================================
    struct Method
    {
        juce::String name;
        juce::String type;      // instance, static, enum
        juce::String args;      // method arguments
    };
    struct EnumValue
    {
        juce::String name;
        juce::String value;
    };
    struct Enum
    {
        juce::String name;
        juce::Array<EnumValue> values;
    };
    struct Class
    {
        juce::String name;
        juce::String cppName;
        juce::String alias;
        juce::Array<Method> instanceMethods;
        juce::Array<Method> staticMethods;
        juce::Array<Enum> enums;
    };
    // ==========================================
    CtrlrLuaApiDatabase();
    ~CtrlrLuaApiDatabase();
    
    // Load from specific file
    bool loadFromFile(const juce::File& xmlFile);
    
    // Load from default location
    bool loadFromDefaultLocation();
    
    // Get the default XML file path
    static juce::File getDefaultXmlPath();
    
    const juce::XmlElement* getXmlRoot() const { return xmlRoot.get(); }
    const Class* getClass(const juce::String& className) const;
    const juce::Array<Class>& getAllClasses() const;
    bool isLoaded() const;
    
private:
    juce::Array<Class> classes;
    bool loaded = false;
    void clear();
    bool parseXml(const juce::XmlElement& root);
    std::unique_ptr<juce::XmlElement> xmlRoot;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaApiDatabase)
};