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
        juce::Array<Method> instanceMethods;
        juce::Array<Method> staticMethods;
        juce::Array<Enum> enums;
    };
    // ==========================================
    CtrlrLuaApiDatabase();
    ~CtrlrLuaApiDatabase();
    bool loadFromFile(const juce::File& xmlFile);
    void loadMethodsForClass(const juce::String& className);
    const juce::XmlElement* getXmlRoot() const { return xmlRoot.get(); }
    const Class* getClass(const juce::String& className) const;
    const juce::Array<Class>& getAllClasses() const;
    bool isLoaded() const;
private:
    juce::Array<Class> classes;
    bool loaded = false;
    void clear();
    bool parseXml(const juce::XmlElement& root);
private:
    std::unique_ptr<juce::XmlElement> xmlRoot;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaApiDatabase)
};