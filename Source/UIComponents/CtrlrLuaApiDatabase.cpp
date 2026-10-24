#include "CtrlrLuaApiDatabase.h"
#include "CtrlrLua/MethodEditor/CtrlrLuaApiLookup.h" // fast C++ database backend replaces LuaAPI.xml

CtrlrLuaApiDatabase::CtrlrLuaApiDatabase()
{
    loadFromDefaultLocation();
}

CtrlrLuaApiDatabase::~CtrlrLuaApiDatabase()
{
    clear();
}

bool CtrlrLuaApiDatabase::loadFromDefaultLocation()
{
    return loadFromMemory(nullptr, 0); // Delegates directly to our C++ mapper
}

bool CtrlrLuaApiDatabase::loadFromMemory(const char*, int)
{
    clear();
    
    // Map data instantly from the compiled C++ CtrlrLuaApiLookup database
    auto& db = CtrlrLuaApiLookup::getInstance();
    auto classNames = db.getAllClassNames();

    for (const auto& className : classNames) {
        const auto* apiClass = db.getClassInfo(className);
        if (!apiClass) continue;

        Class classData;
        classData.name = apiClass->name;
        classData.cppName = apiClass->name; // Can map alias/cppName if stored
        classData.alias = "";

        // Map instance methods
        for (const auto& m : apiClass->methods) {
            Method method;
            method.name = m.name;
            method.type = "instance";
            method.args = m.args;
            classData.instanceMethods.add(method);
        }

        // Map static methods
        for (const auto& m : apiClass->staticMethods) {
            Method method;
            method.name = m.name;
            method.type = "static";
            method.args = m.args;
            classData.staticMethods.add(method);
        }

        classes.add(classData);
    }

    loaded = (classes.size() > 0);
    if (loaded) {
        DBG("CtrlrLuaApiDatabase: Successfully mapped from CtrlrLuaApiLookup (RAM-native)");
    } else {
        DBG("CtrlrLuaApiDatabase Error: No classes found in C++ lookup database.");
    }
    
    return loaded;
}

bool CtrlrLuaApiDatabase::loadFromFile(const juce::File&)
{
    // Deprecated via physical files, but kept for interface compatibility
    return loadFromMemory(nullptr, 0);
}

// Helper locations (stubbed for compatibility, no longer hits disk)
juce::File CtrlrLuaApiDatabase::getDefaultXmlPath()
{
    return juce::File();
}

const CtrlrLuaApiDatabase::Class* CtrlrLuaApiDatabase::getClass(const juce::String& className) const
{
    for (const auto& c : classes) {
        if (c.name == className || c.cppName == className || c.alias == className) 
            return &c;
    }
    return nullptr;
}

const juce::Array<CtrlrLuaApiDatabase::Class>& CtrlrLuaApiDatabase::getAllClasses() const 
{ 
    return classes; 
}

bool CtrlrLuaApiDatabase::isLoaded() const 
{ 
    return loaded; 
}

void CtrlrLuaApiDatabase::clear() 
{ 
    classes.clear(); 
    xmlRoot.reset(); 
    loaded = false; 
}