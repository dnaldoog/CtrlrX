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

                    // Parse INSTANCE methods (accessed via :)
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

                    // Parse STATIC methods (accessed via .)
                    if (auto* sList = classXml->getChildByName("static_methods"))
                    {
                        forEachXmlChildElement (*sList, s)
                        {
                            juce::String staticMethodName = s->getStringAttribute("name");
                            juce::String staticParams = s->getStringAttribute("args");
                            
                            lc.staticMethods.add({ staticMethodName, staticParams, true });
                            allMethodNames.addIfNotAlreadyThere(staticMethodName);
                        }
                    }

                    classes.set(lc.name, lc);
                    classNames.add(lc.name);
                }
            }
        }
    }

    // 2. Manual Injection for Helper Libraries
    // --- CtrlrLuaUtils ---
    LuaClass utilsCls;
    utilsCls.name = "CtrlrLuaUtils";
    juce::StringArray uMethods = { "warnWindow", "infoWindow", "questionWindow", "openFileWindow", "saveFileWindow", "base64_encode", "base64_decode" };
    for (auto& m : uMethods) {
        utilsCls.staticMethods.add({ m, "", true });
        allMethodNames.addIfNotAlreadyThere(m);
    }
    classes.set(utilsCls.name, utilsCls);
    classNames.add(utilsCls.name);

    // --- Lua table library ---
    LuaClass tableLib;
    tableLib.name = "table";
    juce::StringArray tMethods = { "insert", "remove", "sort", "concat", "getn", "maxn" };
    for (auto& m : tMethods) {
        tableLib.staticMethods.add({ m, "", true });
        allMethodNames.addIfNotAlreadyThere(m);
    }
    classes.set(tableLib.name, tableLib);
    classNames.add(tableLib.name);

    // --- Lua math library ---
    LuaClass mathLib;
    mathLib.name = "math";
    juce::StringArray mathMethods = { "abs", "floor", "ceil", "min", "max", "sqrt", "pow", "random", "sin", "cos", "tan", "pi" };
    for (auto& m : mathMethods) {
        mathLib.staticMethods.add({ m, "", true });
        allMethodNames.addIfNotAlreadyThere(m);
    }
    classes.set(mathLib.name, mathLib);
    classNames.add(mathLib.name);

    // 3. Load the TEMPLATES
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

    // 4. EXPANDED LIBRARY: Inject ESSENTIAL TOKENS & KEYWORDS
    juce::StringArray essentialTokens = {
        // Ctrlr Globals
        "panel", "mod", "value", "source", "comp", "event",
        "canvas", "g", "midi", "multiMidi", "path", "arguments",
        "utils", "resources", "timer", "repaint", "setBounds",
        "setVisible", "setName", "getComponent", "setFont", "drawText",
        "fillAll", "setColour", "warn", "info", "question",
        "getDeltaTime", "getFrameStart", "AlertWindow", "showMessageBox",
        "sendMidiMessage", "MidiMessage", "table", "math", "string", "debug",
        
        // Lua Keywords (for space-based completion)
        "local", "function", "if", "then", "else", "elseif", "end",
        "for", "while", "do", "return", "break", "nil", "true", "false"
    };
    
    for (const auto& token : essentialTokens) {
        allMethodNames.addIfNotAlreadyThere(token);
    }

    // Final sorting
    classNames.sort(true);
    allMethodNames.sort(true);
    utilityMethodNames.sort(true);

    _DBG("Autocomplete: Loaded " + juce::String(classes.size()) + " classes, "
         + juce::String(allMethodNames.size()) + " total methods/tokens.");
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getGlobalSuggestions(const juce::String& prefix)
{
    std::vector<SuggestionItem> results;

    // 1. Essential Tokens (Globals) -> Icon "V" (TypeGlobal)
    juce::StringArray globals = { "local", "panel", "mod", "value", "source", "comp", "event", "canvas", "g", "midi", "console" };
    for (auto& g : globals) {
        if (g.startsWithIgnoreCase(prefix))
            results.push_back({ g, TypeGlobal });
    }

    // 2. Classes -> Icon "C" (TypeClass)
    for (auto& c : classNames) {
        if (c.startsWithIgnoreCase(prefix))
            results.push_back({ c, TypeClass });
    }

    // 3. Methods/Utilities -> Icon "M" or "f"
    for (auto& m : allMethodNames) {
        if (m.startsWithIgnoreCase(prefix)) {
            // Check if it was already added as a global/class to avoid duplicates
            bool alreadyAdded = false;
            for (auto& r : results) { if (r.text == m) { alreadyAdded = true; break; } }
            
            if (!alreadyAdded) {
                if (utilityMethodNames.contains(m))
                    results.push_back({ m, TypeUtility });
                else
                    results.push_back({ m, TypeMethod });
            }
        }
    }

    return results;
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getMethodSuggestionsForClass(const juce::String& className, const juce::String& prefix, bool includeInstance)
{
    std::vector<SuggestionItem> results;

    if (classes.contains(className))
    {
        auto& cls = classes.getReference(className);
        
        // 1. Try the primary list based on the separator (. or :)
        auto& primaryList = includeInstance ? cls.methods : cls.staticMethods;
        for (auto& m : primaryList)
        {
            if (prefix.isEmpty() || m.name.startsWithIgnoreCase(prefix))
                results.push_back({ m.name, TypeMethod });
        }

        // 2. FALLBACK: If primary list is empty, search the other list too
        // This handles cases where the XML defines methods as instance but they are used as static
        if (results.empty())
        {
            auto& secondaryList = includeInstance ? cls.staticMethods : cls.methods;
            for (auto& m : secondaryList)
            {
                if (prefix.isEmpty() || m.name.startsWithIgnoreCase(prefix))
                    results.push_back({ m.name, TypeMethod });
            }
        }
    }

    return results;
}

juce::String CtrlrLuaMethodAutoCompleteManager::getMethodParams(const juce::String& methodName)
{
    // 1. Clean the input
    juce::String cleanName = methodName.upToFirstOccurrenceOf("(", false, false).trim();

	// 2. Check the XML-loaded classes (Using JUCE HashMap Iterator)
    juce::HashMap<juce::String, LuaClass>::Iterator it (classes);
    while (it.next())
    {
        // Remove the '&' because getValue() returns by value
        auto lc = it.getValue();
        
        // Check standard methods
        for (const auto& m : lc.methods)
        {
            if (m.name == cleanName && m.parameters.isNotEmpty())
                return m.parameters;
        }
        // Check static methods
        for (const auto& m : lc.staticMethods)
        {
            if (m.name == cleanName && m.parameters.isNotEmpty())
                return m.parameters;
        }
    }

    // 3. HARDCODED DICTIONARY (Manual Overrides)
    if (cleanName == "setControlParameter")  return "String key, String value";
    if (cleanName == "getModulatorByName")   return "String name";
    if (cleanName == "getModulatorByIndex")  return "int index";
    if (cleanName == "getModulatorValue")    return "String name";
    if (cleanName == "getModulator")         return "String name";
    if (cleanName == "setBounds")            return "int x, int y, int w, int h";
    if (cleanName == "setVisible")           return "bool shouldBeVisible";
    if (cleanName == "setName")              return "String newName";
    if (cleanName == "setColour")            return "int colourId, Colour newColour";
    if (cleanName == "repaint")              return "";
    if (cleanName == "drawText")             return "String text, int x, int y, int w, int h, Justification alignment";
    if (cleanName == "fillAll")              return "Colour colour";
    if (cleanName == "setFont")              return "float height";
    if (cleanName == "warn")                 return "String message";
    if (cleanName == "info")                 return "String title, String message";
    if (cleanName == "question")             return "String title, String message";
    if (cleanName == "AlertWindow")          return "String title, String message, String button1";
    if (cleanName == "sendMidiMessage")      return "MidiMessage message";
    if (cleanName == "MidiMessage")          return "int byte1, int byte2, int byte3";
    if (cleanName == "startTimer")           return "int timerId, int intervalMs";
    if (cleanName == "stopTimer")            return "int timerId";

    return "";
}
