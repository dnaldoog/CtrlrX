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
                            
                            // If method name matches class name, it's a constructor
                            if (name == lc.name) {
                                lc.constructors.add(args);
                            }
                            else {
                                lc.methods.add({ name, args, false });
                                allMethodNames.add(name);
                            }
                        }
                    }

                    // Parse STATIC methods (Triggered by '.')
                    // This matches the new output from Python Script v10
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
					
					classes.set(lc.name, lc);    // Store as "utils" or "panel"
					
					// Also store as "CtrlrLuaUtils" or "CtrlrPanel" if different
					if (cppName.isNotEmpty() && cppName != lc.name) {
						classes.set(cppName, lc);
					}
					
					classNames.add(lc.name);
				}
			}
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

    // 3. Load Templates (Method Body Templates)
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

    // Final Post-Processing (Performance Optimization)
    allMethodNames.removeDuplicates(false); // Case-sensitive for Lua
    allMethodNames.sort(true);               // Case-insensitive sort for UI display
    
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
            results.push_back({ m, utilityMethodNames.contains(m) ? TypeUtility : TypeMethod });
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

juce::String CtrlrLuaMethodAutoCompleteManager::getMethodParams(const juce::String& className,
                                                              const juce::String& methodNameOrClass)
{
    // --- 1. HARDCODED COMMON FALLBACKS (Using 'number' for Lua consistency) ---
    // This ensures the bubble is stable and fast for common JUCE/Ctrlr calls
    if (methodNameOrClass == "setBounds")
        return "number x, number y, number width, number height";
    
    if (methodNameOrClass == "setSize")
        return "number width, number height";
    
    if (methodNameOrClass == "setTopLeftPosition")
        return "number x, number y";

    if (methodNameOrClass == "setHeight")
        return "number newHeight"; // Only 1 argument

    if (methodNameOrClass == "setWidth")
        return "number newWidth";  // Only 1 argument
    // --- 2. DATABASE LOOKUP ---
    if (!classes.contains(className))
        return "";

    const auto& lc = classes.getReference(className);
    juce::StringArray allPossibleParams;

    // A. Check for Constructor calls
    if (methodNameOrClass == className && !lc.constructors.isEmpty())
    {
        for (auto& c : lc.constructors) {
            juce::String trimmed = c.trim();
            if (trimmed.isNotEmpty() && trimmed != "()") {
                allPossibleParams.add(trimmed);
                _DBG("MANAGER: Found CONSTRUCTOR for [" + className + "] Params: [" + trimmed + "]");
            }
        }
    }

    // B. Search Instance Methods
    for (auto& m : lc.methods)
    {
        if (m.name == methodNameOrClass)
        {
            juce::String p = m.parameters.trim();
            if (p.isNotEmpty() && p != "()") {
                allPossibleParams.add(p);
                _DBG("MANAGER: Found INSTANCE method [" + methodNameOrClass + "] in [" + className + "] Params: [" + p + "]");
            }
        }
    }
        
    // C. Search Static Methods
    for (auto& m : lc.staticMethods)
    {
        if (m.name == methodNameOrClass)
        {
            juce::String p = m.parameters.trim();
            if (p.isNotEmpty() && p != "()") {
                allPossibleParams.add(p);
                _DBG("MANAGER: Found STATIC method [" + methodNameOrClass + "] in [" + className + "] Params: [" + p + "]");
            }
        }
    }

    if (allPossibleParams.isEmpty()) {
        _DBG("MANAGER: No valid params found for [" + methodNameOrClass + "] in class [" + className + "]");
        return "";
    }

    // Join with newline to show overloads in the bubble
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
