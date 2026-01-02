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

    // 1. Load the MAIN API from XML
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
                    lc.parentClass = classXml->getStringAttribute("inherits");
                    juce::String cppName = classXml->getStringAttribute("cpp_name");

                    if (auto* mList = classXml->getChildByName("methods"))
                    {
                        forEachXmlChildElement(*mList, m) {
                            juce::String name = m->getStringAttribute("name");
                            juce::String args = m->getStringAttribute("args").trim();
                            
                            // SANITIZATION
                            if (args.isEmpty() || args == "()" || args.containsOnly(", ") || args.containsOnly("(, )")) {
                                args = "()";
                            } else {
                                if (!args.startsWith("(")) args = "(" + args;
                                if (!args.endsWith(")")) args = args + ")";
                                if (args.substring(1, args.length() - 1).trim().containsOnly(", ")) args = "()";
                            }

                            bool alreadyExists = false;
                            for (auto& existing : lc.methods) {
                                if (existing.name == name && existing.parameters == args) {
                                    alreadyExists = true; break;
                                }
                            }

                            if (!alreadyExists) {
                                LuaMethod lm; lm.name = name; lm.parameters = args; lm.isStatic = false;
                                lc.methods.add(lm);
                                
                                // Priority insertion for no-argument version
                                if (args != "()") {
                                    bool hasNoArg = false;
                                    for(auto& ex : lc.methods) { if(ex.name == name && ex.parameters == "()") { hasNoArg = true; break; } }
                                    if (!hasNoArg) {
                                        LuaMethod def; def.name = name; def.parameters = "()";
                                        lc.methods.insert(0, def);
                                    }
                                }
                                allMethodNames.add(name);
                            }
                        }
                    }

                    if (auto* sList = classXml->getChildByName("static_methods"))
                    {
                        forEachXmlChildElement(*sList, s) {
                            juce::String name = s->getStringAttribute("name");
                            juce::String args = s->getStringAttribute("args").trim();
                            if (args.isEmpty() || args.containsOnly(", ")) args = "()";
                            else {
                                if (!args.startsWith("(")) args = "(" + args;
                                if (!args.endsWith(")")) args = args + ")";
                            }
                            LuaMethod sm; sm.name = name; sm.parameters = args; sm.isStatic = true;
                            lc.staticMethods.add(sm);
                            allMethodNames.add(name);
                        }
                    }

                    classes.set(lc.name, lc);
                    if (cppName.isNotEmpty() && cppName != lc.name) classes.set(cppName, lc);
                    classNames.add(lc.name);
                }
            }
        }
    }

    // 2. Manual Injection of Base Classes (THE CHAIN)
    // Create Component first
    if (!classes.contains("Component")) {
        LuaClass comp; comp.name = "Component";
        juce::StringArray ui = { "setBounds", "setSize", "setVisible", "getX", "getY", "getWidth", "getHeight", "repaint", "setHeight", "setWidth" };
        for (auto& n : ui) {
            LuaMethod m; m.name = n;
            m.parameters = (n == "setBounds") ? "(int x, int y, int width, int height)" : (n == "setSize") ? "(int width, int height)" : "()";
            comp.methods.add(m);
            if (m.parameters != "()") {
                LuaMethod d; d.name = n; d.parameters = "()";
                comp.methods.insert(0, d);
            }
            allMethodNames.add(n);
        }
        classes.set(comp.name, comp);
        classNames.add(comp.name);
    }

    // Create CtrlrObject and make it inherit from Component
    if (!classes.contains("CtrlrObject")) {
        LuaClass obj; obj.name = "CtrlrObject";
        obj.parentClass = "Component"; // <--- Crucial link for mod:setHeight
        juce::StringArray om = { "getProperty", "setProperty", "getPropertyInt", "getPropertyString" };
        for (auto& n : om) {
            LuaMethod m; m.name = n; m.parameters = "(String name)";
            obj.methods.add(m);
            LuaMethod d; d.name = n; d.parameters = "()";
            obj.methods.insert(0, d);
            allMethodNames.add(n);
        }
        classes.set(obj.name, obj);
        classNames.add(obj.name);
    }

    // 3. HARD-FIX SPECIFIC INSTANCES
    if (classes.contains("CtrlrPanel")) {
        classes.getReference("CtrlrPanel").parentClass = "Component";
    }
    if (classes.contains("CtrlrModulator")) {
        classes.getReference("CtrlrModulator").parentClass = "CtrlrObject";
    }

    // 4. Inject Common Libs
    auto injectStaticLib = [this](juce::String libName, juce::StringArray methods) {
        LuaClass lc;
        if (classes.contains(libName)) lc = classes.getReference(libName);
        else lc.name = libName;
        for (auto& m : methods) {
            bool exists = false;
            for (auto& sm : lc.staticMethods) { if (sm.name == m) { exists = true; break; } }
            if (!exists) {
                LuaMethod lm; lm.name = m; lm.parameters = "()"; lm.isStatic = true;
                lc.staticMethods.add(lm);
            }
            allMethodNames.add(m);
        }
        classes.set(libName, lc);
        if (!classNames.contains(libName)) classNames.add(libName);
    };

    injectStaticLib("table", { "insert", "remove", "sort", "concat", "unpack" });
    injectStaticLib("math", { "abs", "floor", "ceil", "min", "max", "sqrt", "sin", "cos", "pi", "random" });
    injectStaticLib("string", { "format", "sub", "upper", "lower", "find", "gsub", "len" });

    // 5. Finalize
    allMethodNames.removeDuplicates(false);
    allMethodNames.sort(true);
    classNames.removeDuplicates(false);
    classNames.sort(true);

    _DBG("AUTOCOMPLETE: Definition Loading Complete. Chain: Modulator -> Object -> Component");
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
    juce::StringArray addedNames;
    juce::String currentClass = className;

    _DBG("--- START SEARCH: [" + prefix + "] in Class: [" + className + "] ---");

    while (currentClass.isNotEmpty())
    {
        if (!classes.contains(currentClass)) {
            _DBG("    ! Class [" + currentClass + "] not found in 'classes' map. Stopping.");
            break;
        }

        const auto& lc = classes.getReference(currentClass);
        int classMatches = 0;

        _DBG("    Searching: [" + currentClass + "] (Parent: " + (lc.parentClass.isEmpty() ? "None" : lc.parentClass) + ")");

        // 1. Instance Methods
        if (includeInstance) {
            for (auto& m : lc.methods) {
                if (m.name.startsWithIgnoreCase(prefix) && !addedNames.contains(m.name)) {
                    suggestions.push_back({ m.name, TypeMethod });
                    addedNames.add(m.name);
                    classMatches++;
                }
            }
        }

        // 2. Static Methods
        if (includeStatic) {
            for (auto& m : lc.staticMethods) {
                if (m.name.startsWithIgnoreCase(prefix) && !addedNames.contains(m.name)) {
                    suggestions.push_back({ m.name, TypeMethod });
                    addedNames.add(m.name);
                    classMatches++;
                }
            }
        }

        // 3. Properties
        if (includeProperties) {
            for (auto& p : lc.properties) {
                if (p.startsWithIgnoreCase(prefix) && !addedNames.contains(p)) {
                    suggestions.push_back({ p, TypeProperty });
                    addedNames.add(p);
                    classMatches++;
                }
            }
        }

        _DBG("    Found " + juce::String(classMatches) + " matches in [" + currentClass + "]");

        // THE JUMP: Move to parent for the next loop iteration
        currentClass = lc.parentClass;
        if (currentClass.isNotEmpty()) {
            _DBG("    Moving up the tree to: [" + currentClass + "]");
        }
    }

    _DBG("--- SEARCH COMPLETE: Total " + juce::String(suggestions.size()) + " matches found ---");
    return suggestions;
}

juce::String CtrlrLuaMethodAutoCompleteManager::getMethodParams(const juce::String& className, const juce::String& methodNameOrClass)
{
    // 1. HARDCODED FALLBACKS (Preserved)
    if (methodNameOrClass == "addColourItem") return "String name, Colour colour, int index";
    if (methodNameOrClass == "setBounds") return "int x, int y, int width, int height";
    if (methodNameOrClass == "getModulatorByName") return "String name";
    if (methodNameOrClass == "getPropertyInt") return "String propertyName";

    juce::StringArray allPossibleParams;

    // Helper lambda to search all classes globally
    auto performGlobalSearch = [&](const juce::String& methodName) {
        for (juce::HashMap<juce::String, LuaClass>::Iterator it (classes); it.next();) {
            const auto& lc = it.getValue();
            for (auto& m : lc.methods) {
                if (m.name == methodName) {
                    juce::String p = m.parameters.trim().isEmpty() ? "()" : m.parameters.trim();
                    if (!allPossibleParams.contains(p)) allPossibleParams.add(p);
                }
            }
            for (auto& m : lc.staticMethods) {
                if (m.name == methodName) {
                    juce::String p = m.parameters.trim().isEmpty() ? "()" : m.parameters.trim();
                    if (!allPossibleParams.contains(p)) allPossibleParams.add(p);
                }
            }
        }
    };

    // 2. SEARCH LOGIC
    if (className.isEmpty())
    {
        performGlobalSearch(methodNameOrClass);
    }
    else
    {
        juce::String currentClass = className;
        
        while (currentClass.isNotEmpty() && classes.contains(currentClass))
        {
            const auto& lc = classes.getReference(currentClass);
            
            // Search Instance Methods
            for (auto& m : lc.methods) {
                if (m.name == methodNameOrClass) {
                    juce::String p = m.parameters.trim().isEmpty() ? "()" : m.parameters.trim();
                    if (!allPossibleParams.contains(p)) allPossibleParams.add(p);
                }
            }
            
            // Search Static Methods
            for (auto& m : lc.staticMethods) {
                if (m.name == methodNameOrClass) {
                    juce::String p = m.parameters.trim().isEmpty() ? "()" : m.parameters.trim();
                    if (!allPossibleParams.contains(p)) allPossibleParams.add(p);
                }
            }

            // Search Constructors (Only if the name matches the class exactly)
            if (methodNameOrClass == lc.name) {
                for (auto& c : lc.constructors) {
                    juce::String p = c.trim().isEmpty() ? "()" : c.trim();
                    // CRITICAL FIX: Ensure we don't add empty commas or duplicates
                    if (!allPossibleParams.contains(p)) allPossibleParams.add(p);
                }
            }

            currentClass = lc.parentClass; // Climb the tree
        }

        // 3. FALLBACK TO GLOBAL if specific class search yielded nothing
        if (allPossibleParams.isEmpty()) {
            performGlobalSearch(methodNameOrClass);
        }
    }

    // Filter out any accidentally added empty strings before joining
    allPossibleParams.removeEmptyStrings();
    
    return allPossibleParams.joinIntoString("\n").trim();
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
