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
    classes.clear();
    classNames.clear();
    allMethodNames.clear();
    utilityMethodNames.clear();

    // 1. Load the MAIN API (Classes, Methods, Properties, Constructors, Static Methods)
    if (BinaryData::LuaAPI_xml != nullptr)
    {
        juce::XmlDocument doc(juce::String::createStringFromData(BinaryData::LuaAPI_xml, BinaryData::LuaAPI_xmlSize));
        std::unique_ptr<juce::XmlElement> root(doc.getDocumentElement());

        if (root != nullptr && root->hasTagName("LuaAPI"))
        {
            forEachXmlChildElement(*root, classXml)
            {
                if (classXml->hasTagName("class"))
                {
                    LuaClass lc;
                    lc.name = classXml->getStringAttribute("name");
                    juce::String cppName = classXml->getStringAttribute("cpp_name");

                    // Parse explicit CONSTRUCTORS tag (if present)
                    if (auto* cList = classXml->getChildByName("constructors"))
                    {
                        forEachXmlChildElement(*cList, c) {
                            lc.constructors.add(c->getStringAttribute("args"));
                        }
                    }

                    // Parse INSTANCE methods (Triggered by ':')
                    if (auto* mList = classXml->getChildByName("methods"))
                    {
                        forEachXmlChildElement(*mList, m) {
                            juce::String name = m->getStringAttribute("name");
                            juce::String args = m->getStringAttribute("args");
                            
                            // FIXED: Don't swallow critical methods into constructors just because names match
                            if (name == "getModulatorByName" || name == "getModulator")
                            {
                                lc.methods.add({ name, args, false });
                                allMethodNames.add(name);
                            }
                            else if (name == lc.name || name == cppName) {
                                lc.constructors.add(args);
                            }
                            else {
                                lc.methods.add({ name, args, false });
                                allMethodNames.add(name);
                            }
                        }
                    }

                    // Parse STATIC methods (Triggered by '.')
                    if (auto* sList = classXml->getChildByName("static_methods"))
                    {
                        forEachXmlChildElement(*sList, s) {
                            juce::String name = s->getStringAttribute("name");
                            juce::String args = s->getStringAttribute("args");
                            
                            lc.staticMethods.add({ name, args, true });
                            allMethodNames.add(name);
                        }
                    }

                    // Parse PROPERTIES (Triggered by '.')
                    if (auto* pList = classXml->getChildByName("properties"))
                    {
                        forEachXmlChildElement(*pList, p) {
                            juce::String propName = p->getStringAttribute("name");
                            lc.properties.add(propName);
                            allMethodNames.add(propName);
                        }
                    }
                    
                    classes.set(lc.name, lc);
                    if (cppName.isNotEmpty() && cppName != lc.name) {
                        classes.set(cppName, lc);
                    }
                    classNames.add(lc.name);
                }
            }
        }
    }

    // --- SAFETY NET: Force Inject Essential Ctrlr Methods if XML missed them ---
    if (classes.contains("CtrlrPanel"))
    {
        auto& panelClass = classes.getReference("CtrlrPanel");
        bool hasGetMod = false;
        for (auto& m : panelClass.methods) { if (m.name == "getModulatorByName") { hasGetMod = true; break; } }
        
        if (!hasGetMod) {
            panelClass.methods.add({ "getModulatorByName", "String name", false });
            allMethodNames.add("getModulatorByName");
        }
    }

    // 2. Manual Injection for Standard Lua Libraries
    auto injectStaticLib = [this](juce::String libName, juce::StringArray methods) {
        LuaClass lc;
        lc.name = libName;
        for (auto& m : methods) {
            lc.staticMethods.add({ m, "", true });
            allMethodNames.add(m);
        }
        classes.set(libName, lc);
        classNames.add(libName);
    };

    injectStaticLib("table", { "insert", "remove", "sort", "concat", "unpack" });
    injectStaticLib("math", { "abs", "floor", "ceil", "min", "max", "sqrt", "sin", "cos", "pi", "random" });
    injectStaticLib("string", { "format", "sub", "upper", "lower", "find", "gsub", "len" });

    // 3. Load Templates
    if (BinaryData::CtrlrLuaMethodTemplates_xml != nullptr)
    {
        juce::XmlDocument doc (juce::String::createStringFromData (BinaryData::CtrlrLuaMethodTemplates_xml, BinaryData::CtrlrLuaMethodTemplates_xmlSize));
        std::unique_ptr<juce::XmlElement> root (doc.getDocumentElement());
        if (root != nullptr)
        {
            forEachXmlChildElement (*root, child)
            {
                if (child->hasTagName("luaMethod")) {
                    juce::String mName = child->getStringAttribute("name");
                    if (mName.isNotEmpty()) allMethodNames.add(mName);
                }
                if (child->hasTagName("utilityMethods")) {
                    forEachXmlChildElement (*child, util) {
                        juce::String uName = util->getStringAttribute("name");
                        if (uName.isNotEmpty()) {
                            allMethodNames.add(uName);
                            utilityMethodNames.add(uName);
                        }
                    }
                }
            }
        }
    }

    // 4. Essential Keywords & Global Variables
    juce::StringArray tokens = {
        "local", "function", "if", "then", "else", "elseif", "end", "for", "while", "do",
        "return", "break", "nil", "true", "false", "panel", "mod", "value", "source",
        "comp", "event", "canvas", "g", "midi", "console"
    };
    for (auto& t : tokens) allMethodNames.add(t);

    allMethodNames.removeDuplicates(false);
    allMethodNames.sort(true);
    classNames.removeDuplicates(false);
    classNames.sort(true);
    utilityMethodNames.removeDuplicates(false);
    utilityMethodNames.sort(true);
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getGlobalSuggestions(const juce::String& prefix) {
    std::vector<SuggestionItem> results;
    juce::StringArray added;

    // 1. Globals/Keywords (Icon "V")
    juce::StringArray globals = { "local", "function", "if", "then", "else", "elseif", "end", "for", "while", "do", "return", "break", "nil", "true", "false", "panel", "mod", "value", "source", "comp", "event", "canvas", "g", "midi", "console" };
    for (auto& g : globals) {
        if (g.startsWithIgnoreCase(prefix)) {
            results.push_back({ g, TypeGlobal });
            added.add(g);
        }
    }

    // 2. Classes (Icon "C")
    for (auto& c : classNames) {
        if (c.startsWithIgnoreCase(prefix) && !added.contains(c)) {
            results.push_back({ c, TypeClass });
            added.add(c);
        }
    }

    // 3. The rest (Methods/Utilities)
    for (auto& m : allMethodNames) {
        if (m.startsWithIgnoreCase(prefix) && !added.contains(m)) {
            // Check if it's a utility method or a general class method
            SuggestionType type = utilityMethodNames.contains(m) ? TypeUtility : TypeMethod;
            results.push_back({ m, type });
            added.add(m);
        }
    }
    return results;
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getMethodSuggestionsForClass(
    const juce::String& className, const juce::String& prefix,
    bool includeInstance, bool includeStatic, bool includeProperties)
{
    std::vector<SuggestionItem> suggestions;
    
    if (!classes.contains(className)) return suggestions;
    const auto& lc = classes.getReference(className);

    // 1. Add Instance Methods (Triggered by ':')
    if (includeInstance) {
        for (const auto& m : lc.methods) {
            // --- TWEAK A: FILTERING LOGIC ---
            // Skip methods starting with 'L' if followed by the class name (e.g., LMemoryBlock)
            // or if the method name is exactly the class name (constructor duplicate).
            if (m.name.startsWith("L") && m.name.substring(1) == className)
                continue;
            
            if (m.name == className && suggestions.size() > 0)
                continue;

            if (m.name.startsWithIgnoreCase(prefix))
                suggestions.push_back({ m.name, TypeMethod });
        }
    }

    // 2. Add Static Methods (Triggered by '.')
    if (includeStatic) {
        for (const auto& m : lc.staticMethods) {
            // Apply same filtering for internal static names if applicable
            if (m.name.startsWith("L") && m.name.substring(1) == className)
                continue;

            if (m.name.startsWithIgnoreCase(prefix))
                suggestions.push_back({ m.name, TypeMethod });
        }
    }

    // 3. Add Properties (Triggered by '.')
    if (includeProperties) {
        for (const auto& p : lc.properties) {
            if (p.startsWithIgnoreCase(prefix))
                suggestions.push_back({ p, TypeProperty });
        }
    }

    return suggestions;
}

juce::String CtrlrLuaMethodAutoCompleteManager::getMethodParams(const juce::String& className, const juce::String& methodNameOrClass)
{
    // 1. HARDCODED FALLBACKS
    if (methodNameOrClass == "addColourItem") return "String name, Colour colour, int index";
    if (methodNameOrClass == "setBounds") return "int x, int y, int width, int height";
    if (methodNameOrClass == "getModulatorByName") return "String name";
    if (methodNameOrClass == "getPropertyInt") return "String propertyName";

    juce::StringArray allPossibleParams;

    // Helper lambda to search all classes globally
    // FIXED: Using getValue() for JUCE HashMap Iterator
    auto performGlobalSearch = [&](const juce::String& methodName) {
        for (juce::HashMap<juce::String, LuaClass>::Iterator it (classes); it.next();) {
            const auto& lc = it.getValue();
            for (auto& m : lc.methods)
                if (m.name == methodName) allPossibleParams.add(m.parameters.isEmpty() ? "()" : m.parameters);
            for (auto& m : lc.staticMethods)
                if (m.name == methodName) allPossibleParams.add(m.parameters.isEmpty() ? "()" : m.parameters);
        }
    };

    // 2. SEARCH LOGIC
    if (className.isEmpty()) {
        performGlobalSearch(methodNameOrClass);
    } else if (classes.contains(className)) {
        const auto& lc = classes.getReference(className);
        for (auto& m : lc.methods)
            if (m.name == methodNameOrClass) allPossibleParams.add(m.parameters.isEmpty() ? "()" : m.parameters);
        for (auto& m : lc.staticMethods)
            if (m.name == methodNameOrClass) allPossibleParams.add(m.parameters.isEmpty() ? "()" : m.parameters);
            
        if (allPossibleParams.isEmpty()) performGlobalSearch(methodNameOrClass);
    }

    return allPossibleParams.joinIntoString("\n");
}

juce::String CtrlrLuaMethodAutoCompleteManager::resolveReturnType(const juce::String& className, const juce::String& methodName)
{
    // 1. Context-Specific Overrides (Priority)
    if (className == "CtrlrPanel") {
        if (methodName.contains("getModulator")) return "CtrlrModulator";
    }
    
    if (className == "CtrlrModulator") {
        if (methodName == "getComponent") return "CtrlrComponent";
    }

    // 2. Global Method Name Mappings (Fallback)
    if (methodName == "getModulatorByName" || methodName == "getModulator" || methodName == "getModulatorByIndex")
        return "CtrlrModulator";

    if (methodName == "getComponent" || methodName == "getOwner" || methodName == "getControl")
        return "CtrlrComponent";

    if (methodName == "getPanel" || methodName == "getOwnerPanel")
        return "CtrlrPanel";

    if (methodName == "getMemoryBlock" || methodName == "getData")
        return "MemoryBlock";

    if (methodName == "getLuaManager")
        return "CtrlrLuaManager";

    if (methodName == "getCanvas")
        return "Graphics"; // Matches your getClassNameForVariable mapping

    if (methodName == "getGlobalTimer")
        return "Timer";
	
	if (methodName == "setBounds")
        return "number x, number y, number width, number height";

    // 3. XML Database Lookup (Optional)
    // If you want to be super advanced, you could look up the 'type'
    // attribute in your LuaAPI.xml here, but hardcoding common ones
    // is much faster for performance.

    return "";
}

juce::StringArray CtrlrLuaMethodAutoCompleteManager::getClassNames() const
{
    juce::StringArray names;

    // JUCE HashMap iterator usage
    for (juce::HashMap<juce::String, LuaClass>::Iterator it (classes); it.next();)
    {
        names.add (it.getKey());
    }

    return names;
}
