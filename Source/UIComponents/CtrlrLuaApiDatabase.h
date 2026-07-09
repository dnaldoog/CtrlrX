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
    
    /** Load API from a physical file on disk */
    bool loadFromFile(const juce::File& xmlFile);

    /** Load API from a memory buffer (e.g. BinaryData) */
    bool loadFromMemory(const char* data, int size);
    
    /** Attempts to load from BinaryData first, then falls back to disk */
    bool loadFromDefaultLocation();
    
    /** Helper to find the physical XML file in various standard locations */
    static juce::File getDefaultXmlPath();
    
    const juce::XmlElement* getXmlRoot() const { return xmlRoot.get(); }
    const Class* getClass(const juce::String& className) const;
    const juce::Array<Class>& getAllClasses() const;
    
    bool isLoaded() const;
    void clear();
    
private:
    /** Internal parser that populates the classes array from an XML element */
    bool parseXml(const juce::XmlElement& root);

    juce::Array<Class> classes;
    bool loaded = false;
    std::unique_ptr<juce::XmlElement> xmlRoot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaApiDatabase)
};