#include "CtrlrLuaApiDatabase.h"

CtrlrLuaApiDatabase::CtrlrLuaApiDatabase()
{
    // Optionally auto-load from default location
    loadFromDefaultLocation();
}

CtrlrLuaApiDatabase::~CtrlrLuaApiDatabase()
{
    clear();
}

juce::File CtrlrLuaApiDatabase::getDefaultXmlPath()
{
    // Try multiple potential locations in order of preference
    
    // 1. Try relative to executable (for installed applications)
    juce::File exeDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                            .getParentDirectory();
    
    juce::File xmlFile = exeDir.getChildFile("Resources/XML/LuaAPI.xml");
    if (xmlFile.existsAsFile())
        return xmlFile;
    
    // 2. Try one level up (for macOS .app bundles)
    xmlFile = exeDir.getParentDirectory().getChildFile("Resources/XML/LuaAPI.xml");
    if (xmlFile.existsAsFile())
        return xmlFile;
    
    // 3. Try application data directory
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                .getChildFile("Ctrlr");
    
    xmlFile = appDataDir.getChildFile("Resources/XML/LuaAPI.xml");
    if (xmlFile.existsAsFile())
        return xmlFile;
    
    // 4. Try current working directory (for development)
    xmlFile = juce::File::getCurrentWorkingDirectory().getChildFile("Resources/XML/LuaAPI.xml");
    if (xmlFile.existsAsFile())
        return xmlFile;
    
    // 5. Try Source directory (for development builds)
    xmlFile = juce::File::getCurrentWorkingDirectory().getChildFile("Source/Resources/XML/LuaAPI.xml");
    if (xmlFile.existsAsFile())
        return xmlFile;
    
    // 6. Return the preferred default path even if it doesn't exist yet
    return exeDir.getChildFile("Resources/XML/LuaAPI.xml");
}

bool CtrlrLuaApiDatabase::loadFromDefaultLocation()
{
    juce::File xmlFile = getDefaultXmlPath();
    
    if (!xmlFile.existsAsFile())
    {
        DBG("LuaAPI.xml not found at: " + xmlFile.getFullPathName());
        return false;
    }
    
    return loadFromFile(xmlFile);
}

bool CtrlrLuaApiDatabase::loadFromFile(const juce::File& xmlFile)
{
    clear();
    
    if (!xmlFile.existsAsFile())
    {
        DBG("XML file does not exist: " + xmlFile.getFullPathName());
        return false;
    }
    
    xmlRoot = juce::XmlDocument::parse(xmlFile);
    
    if (xmlRoot == nullptr)
    {
        DBG("Failed to parse XML file: " + xmlFile.getFullPathName());
        return false;
    }
    
    loaded = parseXml(*xmlRoot);
    
    if (loaded)
    {
        DBG("Successfully loaded LuaAPI.xml: " + juce::String(classes.size()) + " classes");
    }
    
    return loaded;
}

bool CtrlrLuaApiDatabase::parseXml(const juce::XmlElement& root)
{
    classes.clear();
    
    // Iterate through all <class> elements
    for (auto* classNode : root.getChildWithTagNameIterator("class"))
    {
        Class classData;
        classData.name = classNode->getStringAttribute("name");
        classData.cppName = classNode->getStringAttribute("cpp_name");
        classData.alias = classNode->getStringAttribute("alias");
        
        // Parse <methods> section
        if (auto* methodsNode = classNode->getChildByName("methods"))
        {
            for (auto* methodNode : methodsNode->getChildWithTagNameIterator("method"))
            {
                Method method;
                method.name = methodNode->getStringAttribute("name");
                method.type = methodNode->getStringAttribute("type");
                method.args = methodNode->getStringAttribute("args");
                
                // Sort into instance or static methods
                if (method.type == "static")
                    classData.staticMethods.add(method);
                else
                    classData.instanceMethods.add(method);
            }
        }
        
        // Parse <enums> section if present
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

const CtrlrLuaApiDatabase::Class* CtrlrLuaApiDatabase::getClass(const juce::String& className) const
{
    for (const auto& c : classes)
    {
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