#include "CtrlrLuaApiDatabase.h"
#include "BinaryData.h" // Ensure this is included to access the XML data

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
    // 1. PRIORITY: Load from BinaryData (Compiled into the EXE)
    // This is the "Fail-safe" for GitHub builds and Linux
    if (BinaryData::LuaAPI_xml != nullptr)
    {
        if (loadFromMemory(BinaryData::LuaAPI_xml, BinaryData::LuaAPI_xmlSize))
        {
            DBG("LuaAPI Database: Successfully loaded from BinaryData");
            return true;
        }
    }

    // 2. FALLBACK: Load from Physical File
    // This allows developers to swap the XML without recompiling
    juce::File xmlFile = getDefaultXmlPath();
    if (xmlFile.existsAsFile())
    {
        if (loadFromFile(xmlFile))
        {
            DBG("LuaAPI Database: Successfully loaded from File: " + xmlFile.getFullPathName());
            return true;
        }
    }
    
    DBG("LuaAPI Database Error: Could not find API definitions in BinaryData or File System.");
    return false;
}

bool CtrlrLuaApiDatabase::loadFromMemory(const char* data, int size)
{
    clear();

    if (data == nullptr || size <= 0)
        return false;

    // Use CreateStringFromData to ensure null-termination/correct encoding
    xmlRoot = juce::XmlDocument::parse(juce::String::createStringFromData(data, size));

    if (xmlRoot == nullptr)
    {
        DBG("LuaAPI Database: Failed to parse XML from memory.");
        return false;
    }

    loaded = parseXml(*xmlRoot);
    return loaded;
}

bool CtrlrLuaApiDatabase::loadFromFile(const juce::File& xmlFile)
{
    clear();
    
    if (!xmlFile.existsAsFile())
        return false;
    
    xmlRoot = juce::XmlDocument::parse(xmlFile);
    
    if (xmlRoot == nullptr)
        return false;
    
    loaded = parseXml(*xmlRoot);
    return loaded;
}

// --- REST OF THE FILE REMAINS THE SAME ---

bool CtrlrLuaApiDatabase::parseXml(const juce::XmlElement& root)
{
    classes.clear();
    
    for (auto* classNode : root.getChildWithTagNameIterator("class"))
    {
        Class classData;
        classData.name = classNode->getStringAttribute("name");
        classData.cppName = classNode->getStringAttribute("cpp_name");
        classData.alias = classNode->getStringAttribute("alias");
        
        if (auto* methodsNode = classNode->getChildByName("methods"))
        {
            for (auto* methodNode : methodsNode->getChildWithTagNameIterator("method"))
            {
                Method method;
                method.name = methodNode->getStringAttribute("name");
                method.type = methodNode->getStringAttribute("type");
                method.args = methodNode->getStringAttribute("args");
                
                if (method.type == "static")
                    classData.staticMethods.add(method);
                else
                    classData.instanceMethods.add(method);
            }
        }
        
        if (auto* enumsNode = classNode->getChildByName("enums"))
        {
            for (auto* enumNode : enumsNode->getChildWithTagNameIterator("enum"))
            {
                Enum enumData;
                enumData.name = enumNode->getStringAttribute("name");
                
                for (auto* valueNode : enumNode->getChildWithTagNameIterator("value"))
                {
                    EnumValue enumValue;
                    enumValue.name = valueNode->getStringAttribute("name");
                    enumValue.value = valueNode->getStringAttribute("value");
                    enumData.values.add(enumValue);
                }
                classData.enums.add(enumData);
            }
        }
        classes.add(classData);
    }
    return classes.size() > 0;
}

// Helper locations for local development
juce::File CtrlrLuaApiDatabase::getDefaultXmlPath()
{
    juce::File exeDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory();
    
    // Relative to exe
    juce::File xmlFile = exeDir.getChildFile("Resources/XML/LuaAPI.xml");
    if (xmlFile.existsAsFile()) return xmlFile;
    
    // MacOS Bundle style
    xmlFile = exeDir.getParentDirectory().getChildFile("Resources/XML/LuaAPI.xml");
    if (xmlFile.existsAsFile()) return xmlFile;
    
    // Source dir (Dev)
    xmlFile = juce::File::getCurrentWorkingDirectory().getChildFile("Source/Resources/XML/LuaAPI.xml");
    if (xmlFile.existsAsFile()) return xmlFile;

    return exeDir.getChildFile("Resources/XML/LuaAPI.xml");
}

const CtrlrLuaApiDatabase::Class* CtrlrLuaApiDatabase::getClass(const juce::String& className) const
{
    for (const auto& c : classes)
        if (c.name == className || c.cppName == className || c.alias == className) return &c;
    return nullptr;
}

const juce::Array<CtrlrLuaApiDatabase::Class>& CtrlrLuaApiDatabase::getAllClasses() const { return classes; }
bool CtrlrLuaApiDatabase::isLoaded() const { return loaded; }
void CtrlrLuaApiDatabase::clear() { classes.clear(); xmlRoot.reset(); loaded = false; }