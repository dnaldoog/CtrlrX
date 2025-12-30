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

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getGlobalSuggestions(const juce::String& prefix)
{
    std::vector<SuggestionItem> results;

    // 1. Essential Tokens (Globals) -> Icon "V"
    juce::StringArray globals = { "panel", "mod", "value", "source", "comp", "event", "canvas", "g", "midi", "console" };
    for (auto& g : globals) {
        if (g.startsWithIgnoreCase(prefix))
            results.push_back({ g, TypeGlobal });
    }

    // 2. Classes -> Icon "C"
    for (auto& c : classNames) {
        if (c.startsWithIgnoreCase(prefix))
            results.push_back({ c, TypeClass });
    }

    // 3. Methods/Utilities -> Icon "M" or "f"
    for (auto& m : allMethodNames) {
        if (m.startsWithIgnoreCase(prefix)) {
            if (utilityMethodNames.contains(m))
                results.push_back({ m, TypeUtility });
            else
                results.push_back({ m, TypeMethod });
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
