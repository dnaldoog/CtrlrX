#include "CtrlrLuaMethodAutoCompleteManager.h"

// This is required to access BinaryData::LuaAPI_xml
#include "BinaryData.h"
#include "CtrlrMacros.h"
#include "CtrlrLog.h"

CtrlrLuaMethodAutoCompleteManager::CtrlrLuaMethodAutoCompleteManager()
{
    // Load the API from the binary resources as soon as the manager is created
    loadDefinitions();
}

void CtrlrLuaMethodAutoCompleteManager::loadDefinitions()
{
    // Clear everything first to avoid duplicates on re-load
    classes.clear();
    classNames.clear();
    allMethodNames.clear();
    utilityMethodNames.clear();

    // 1. Load the MAIN API (Classes and Methods from LuaAPI.xml)
    if (BinaryData::LuaAPI_xml != nullptr)
    {
        juce::XmlDocument doc (juce::String::createStringFromData (BinaryData::LuaAPI_xml, BinaryData::LuaAPI_xmlSize));
        std::unique_ptr<juce::XmlElement> root (doc.getDocumentElement());

        if (root != nullptr && root->hasTagName("LuaAPI"))
        {
            forEachXmlChildElement (*root, classXml)
            {
                if (classXml->hasTagName("class"))
                {
                    LuaClass lc;
                    lc.name = classXml->getStringAttribute("name");

                    if (auto* mList = classXml->getChildByName("methods"))
                    {
                        forEachXmlChildElement (*mList, m)
                        {
                            juce::String methodName = m->getStringAttribute("name");
                            juce::String params = m->getStringAttribute("args");
                            lc.methods.add({ methodName, params, false });
                            allMethodNames.addIfNotAlreadyThere(methodName);
                        }
                    }

                    if (auto* sList = classXml->getChildByName("static_methods"))
                    {
                        forEachXmlChildElement (*sList, s)
                        {
                            juce::String staticMethodName = s->getStringAttribute("name");
                            juce::String staticParams = s->getStringAttribute("args");
                            lc.methods.add({ staticMethodName, staticParams, true });
                            allMethodNames.addIfNotAlreadyThere(staticMethodName);
                        }
                    }

                    classes.set(lc.name, lc);
                    classNames.add(lc.name);
                }
            }
        }
    }

    // 2. Load the TEMPLATES
    if (BinaryData::CtrlrLuaMethodTemplates_xml != nullptr)
    {
        juce::XmlDocument doc (juce::String::createStringFromData (BinaryData::CtrlrLuaMethodTemplates_xml, BinaryData::CtrlrLuaMethodTemplates_xmlSize));
        std::unique_ptr<juce::XmlElement> root (doc.getDocumentElement());

        if (root != nullptr)
        {
            forEachXmlChildElement (*root, child)
            {
                if (child->hasTagName("luaMethod"))
                {
                    juce::String methodName = child->getStringAttribute("name");
                    if (methodName.isNotEmpty())
                        allMethodNames.addIfNotAlreadyThere(methodName);
                }
                
                if (child->hasTagName("utilityMethods"))
                {
                    forEachXmlChildElement (*child, util)
                    {
                        if (util->hasTagName("utility"))
                        {
                            juce::String uName = util->getStringAttribute("name");
                            if (uName.isNotEmpty())
                            {
                                allMethodNames.addIfNotAlreadyThere(uName);
                                utilityMethodNames.addIfNotAlreadyThere(uName);
                            }
                        }
                    }
                }
            }
        }
    }

    // 3. EXPANDED LIBRARY: Inject ESSENTIAL TOKENS & MISSING METHODS
    // Add any methods here that you want to appear in the list regardless of XML content
    juce::StringArray essentialTokens = {
        // --- Global Objects ---
        "panel", "mod", "value", "source", "comp", "event",
        "canvas", "g", "midi", "multiMidi", "path", "arguments",
        "utils", "resources", "timer",

        // --- Modulator Methods ---
        "getModulatorByName", "getModulatorValue", "setControlParameter",
        "getModulatorByIndex", "getNumModulators", "getModulator",
        
        // --- UI / Graphics ---
        "repaint", "setBounds", "setVisible", "setName", "getComponent",
        "setFont", "drawText", "fillAll", "setColour",
        
        // --- Utilities & Dialogs ---
        "warn", "info", "question", "getDeltaTime", "getFrameStart",
        "AlertWindow", "showMessageBox",
        
        // --- MIDI ---
        "sendMidiMessage", "MidiMessage"
    };
    
    for (const auto& token : essentialTokens) {
        allMethodNames.addIfNotAlreadyThere(token);
    }

    // Final sorting for a clean UI list
    classNames.sort(true);
    allMethodNames.sort(true);
    utilityMethodNames.sort(true);

    _DBG("Autocomplete: Loaded " + juce::String(classNames.size()) + " classes, "
         + juce::String(allMethodNames.size()) + " methods.");
}
juce::String CtrlrLuaMethodAutoCompleteManager::resolveClass(const juce::String& symbol, const juce::String& fullDocumentText)
{
    // 1. Direct match: Handles 'utils', 'MemoryBlock', 'Path', etc. 
    // because the Python script renamed them in the XML.
    if (classes.contains(symbol))
        return symbol;

    // 2. Variable shorthand aliases (The only ones not in XML)
    if (symbol == "panel")     return "CtrlrPanel";
    if (symbol == "mod")       return "CtrlrModulator";
    if (symbol == "comp")      return "CtrlrComponent";

    // 3. Variable Assignment Search (e.g., m = MemoryBlock())
    int assignPos = fullDocumentText.lastIndexOf(symbol + "=");
    if (assignPos == -1) assignPos = fullDocumentText.lastIndexOf(symbol + " =");

    if (assignPos != -1)
    {
        int startOfClass = fullDocumentText.indexOf(assignPos, "=") + 1;
        while (startOfClass < fullDocumentText.length() &&
            juce::CharacterFunctions::isWhitespace(fullDocumentText[startOfClass]))
            startOfClass++;

        int endOfClass = startOfClass;
        while (endOfClass < fullDocumentText.length() &&
            (juce::CharacterFunctions::isLetterOrDigit(fullDocumentText[endOfClass])))
            endOfClass++;

        juce::String foundClass = fullDocumentText.substring(startOfClass, endOfClass);

        if (classes.contains(foundClass))
            return foundClass;
    }

    return "";
}
std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getGlobalSuggestions(const juce::String& prefix)
{
    std::vector<SuggestionItem> results;

    // 1. Common Ctrlr Globals (the "Start Points")
    juce::StringArray globals = { "panel", "mod", "comp", "utils", "storage", "vst" };
    for (auto& g : globals) {
        if (g.startsWithIgnoreCase(prefix))
            results.push_back({ g, TypeGlobal });
    }

    // 2. All Classes from XML (MemoryBlock, Path, etc.)
    for (auto& c : classNames) {
        if (c.startsWithIgnoreCase(prefix))
            results.push_back({ c, TypeClass });
    }

    return results;
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getMethodSuggestions(const juce::String& className,
    const juce::String& prefix,
    LookupType type)
{
    std::vector<SuggestionItem> results;

    // Check if we actually have this class in our map
    if (classes.contains(className))
    {
        const auto& lc = classes[className];

        for (const auto& m : lc.methods)
        {
            // 1. Filter by prefix (e.g., user typed "panel:getM...")
            if (prefix.isNotEmpty() && !m.name.startsWithIgnoreCase(prefix))
                continue;

            // 2. Filter by Static vs Instance
            // If trigger was ':', we only want instance methods (isStatic == false)
            if (type == LookupInstance && !m.isStatic)
            {
                results.push_back({ m.name, TypeMethod });
            }
            // If trigger was '.', we only want static methods (isStatic == true)
            else if (type == LookupStatic && m.isStatic)
            {
                results.push_back({ m.name, TypeMethod });
            }
        }
    }

    return results;
}
juce::String CtrlrLuaMethodAutoCompleteManager::getMethodParams(const juce::String& methodName)
{
    // 1. Clean the input: remove () if they were accidentally passed in
    juce::String cleanName = methodName.upToFirstOccurrenceOf("(", false, false).trim();

    // 2. Check the XML-loaded classes first
    for (const auto& lc : classes)
    {
        for (const auto& m : lc.methods)
        {
            if (m.name == cleanName && m.parameters.isNotEmpty())
                return m.parameters;
        }
    }

    // 3. HARDCODED DICTIONARY (Manual Overrides for Auto-Insertion)
    
    // --- Modulator / Panel Actions ---
    if (cleanName == "setControlParameter")  return "String key, String value";
    if (cleanName == "getModulatorByName")   return "String name";
    if (cleanName == "getModulatorByIndex")  return "int index";
    if (cleanName == "getModulatorValue")    return "String name";
    if (cleanName == "getModulator")         return "String name";

    // --- UI / Component Layout ---
    if (cleanName == "setBounds")            return "int x, int y, int w, int h";
    if (cleanName == "setVisible")           return "bool shouldBeVisible";
    if (cleanName == "setName")              return "String newName";
    if (cleanName == "setColour")            return "int colourId, Colour newColour";
    if (cleanName == "repaint")              return ""; // No params needed, just adds ()
    
    // --- Graphics / Drawing (inside paint) ---
    if (cleanName == "drawText")             return "String text, int x, int y, int w, int h, Justification alignment";
    if (cleanName == "fillAll")              return "Colour colour";
    if (cleanName == "setFont")              return "float height";

    // --- Utilities & Messaging ---
    if (cleanName == "warn")                 return "String message";
    if (cleanName == "info")                 return "String title, String message";
    if (cleanName == "question")             return "String title, String message";
    if (cleanName == "AlertWindow")          return "String title, String message, String button1";
    
    // --- MIDI ---
    if (cleanName == "sendMidiMessage")      return "MidiMessage message";
    if (cleanName == "MidiMessage")          return "int byte1, int byte2, int byte3";

    // --- Timer ---
    if (cleanName == "startTimer")           return "int timerId, int intervalMs";
    if (cleanName == "stopTimer")            return "int timerId";

    return "";
}
