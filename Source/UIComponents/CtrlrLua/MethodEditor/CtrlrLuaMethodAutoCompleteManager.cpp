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
                                
                                // Only add to global if it's not a reserved library name
                                if (name != "string" && name != "math" && name != "table")
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
                            
                            if (name != "string" && name != "math" && name != "table")
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
	
	if (!classes.contains("MemoryBlock")) {
		LuaClass mbClass;
		mbClass.name = "MemoryBlock";
		classes.set("MemoryBlock", mbClass);
		classNames.add("MemoryBlock");
	}
	
    if (!classes.contains("Component")) {
        LuaClass comp; comp.name = "Component";
        juce::StringArray ui = { "setBounds", "setSize", "setVisible", "getX", "getY", "getWidth", "getHeight", "repaint", "setHeight", "setWidth" };
        for (auto& n : ui) {
            LuaMethod m; m.name = n;
            if (n == "setBounds") m.parameters = "(int x, int y, int width, int height)";
            else if (n == "setSize") m.parameters = "(int width, int height)";
            else if (n == "setHeight") m.parameters = "(int height)";
            else if (n == "setWidth") m.parameters = "(int width)";
            else if (n == "setVisible") m.parameters = "(bool shouldBeVisible)";
            else m.parameters = "()";

            comp.methods.add(m);
            
            if (m.parameters != "()") {
                LuaMethod d; d.name = n; d.parameters = "()";
                comp.methods.add(d);
            }
            allMethodNames.add(n);
        }
        classes.set(comp.name, comp);
        classNames.add(comp.name);
    }

    if (!classes.contains("CtrlrObject")) {
        LuaClass obj; obj.name = "CtrlrObject";
        obj.parentClass = "Component";
        juce::StringArray om = { "getProperty", "setProperty", "getPropertyInt", "getPropertyString" };
        for (auto& n : om) {
            LuaMethod m; m.name = n;
            m.parameters = (n == "setProperty") ? "(String name, var value)" : "(String name)";
            obj.methods.add(m);
            LuaMethod d; d.name = n; d.parameters = "()";
            obj.methods.add(d);
            allMethodNames.add(n);
        }
        classes.set(obj.name, obj);
        classNames.add(obj.name);
    }

    // 3. HARD-FIX SPECIFIC INSTANCES
    if (classes.contains("CtrlrPanel")) {
        auto& panel = classes.getReference("CtrlrPanel");
        panel.parentClass = "Component";
        juce::StringArray missing = {
            "getModulatorByName", "getModulator", "getModulatorByIndex", "getNumModulators",
            "sendMidiMessageNow", "getCanvas", "getPanelEditor", "getGlobalVariable", "setGlobalVariable"
        };
        for (auto& mName : missing) {
            bool exists = false;
            for (auto& em : panel.methods) { if (em.name == mName) { exists = true; break; } }
            if (!exists) {
                LuaMethod lm;
                lm.name = mName;
                lm.parameters = (mName.contains("ByName")) ? "(String name)" :
                                (mName.contains("ByIndex")) ? "(int index)" : "()";
                panel.methods.add(lm);
                // allMethodNames.addIfNotAlreadyThere(mName); // Keep commented as per your logic
                if (lm.parameters != "()") {
                    LuaMethod def; def.name = mName; def.parameters = "()";
                    panel.methods.insert(0, def);
                }
            }
        }
    }

    // --- 4. Detailed Library Definitions (Pre-Injection) ---
    if (!classes.contains("string")) {
        LuaClass strLib; strLib.name = "string";
        juce::StringArray strMethods = {
            "byte(s, i, j)", "char(...)", "dump(function)", "find(s, pattern, init, plain)",
            "format(formatstring, ...)", "gmatch(s, pattern)", "gsub(s, pattern, repl, n)",
            "len(s)", "lower(s)", "match(s, pattern, init)", "rep(s, n)",
            "reverse(s)", "sub(s, i, j)", "upper(s)"
        };
        for (auto& mEntry : strMethods) {
            LuaMethod lm;
            lm.name = mEntry.contains("(") ? mEntry.upToFirstOccurrenceOf("(", false, false) : mEntry;
            lm.parameters = mEntry.contains("(") ? "(" + mEntry.fromFirstOccurrenceOf("(", false, false) : "()";
            lm.isStatic = true;
            strLib.staticMethods.add(lm);
            strLib.methods.add(lm);
        }
        classes.set("string", strLib);
        classNames.add("string");
    }

    if (!classes.contains("math")) {
        LuaClass mathLib; mathLib.name = "math";
        juce::StringArray mathMethods = {
            "abs(x)", "acos(x)", "asin(x)", "atan(x)", "ceil(x)", "cos(x)",
            "deg(x)", "exp(x)", "floor(x)", "fmod(x, y)", "huge", "log(x)",
            "max(x, ...)", "min(x, ...)", "modf(x)", "pi", "pow(x, y)",
            "rad(x)", "random(m, n)", "randomseed(x)", "sin(x)", "sqrt(x)", "tan(x)"
        };
        for (auto& mEntry : mathMethods) {
            LuaMethod lm;
            lm.name = mEntry.contains("(") ? mEntry.upToFirstOccurrenceOf("(", false, false) : mEntry;
            lm.parameters = mEntry.contains("(") ? "(" + mEntry.fromFirstOccurrenceOf("(", false, false) : "";
            lm.isStatic = true;
            mathLib.staticMethods.add(lm);
            mathLib.methods.add(lm);
        }
        classes.set("math", mathLib);
        classNames.add("math");
    }

    // 5. Inject Common Libs (Fills in missing methods for table/string/math)
    auto injectStaticLib = [this](juce::String libName, juce::StringArray methods) {
        if (!classes.contains(libName)) {
            LuaClass newLc; newLc.name = libName;
            classes.set(libName, newLc);
        }
        auto& lc = classes.getReference(libName);
        for (auto& mName : methods) {
            bool exists = false;
            for (auto& sm : lc.staticMethods) { if (sm.name == mName) { exists = true; break; } }
            for (auto& m : lc.methods) { if (m.name == mName) { exists = true; break; } }
            if (!exists) {
                LuaMethod lm; lm.name = mName; lm.parameters = "()"; lm.isStatic = true;
                lc.staticMethods.add(lm);
                lc.methods.add(lm);
            }
        }
        if (!classNames.contains(libName)) classNames.add(libName);
    };

    injectStaticLib("table", { "insert", "remove", "sort", "concat", "unpack" });
    injectStaticLib("math", { "abs", "floor", "ceil", "min", "max", "sqrt", "sin", "cos", "pi", "random" });
    injectStaticLib("string", { "format", "sub", "upper", "lower", "find", "gsub", "len" });

    // 6. MemoryBlock and Modulator specialized fixes
    if (classes.contains("MemoryBlock")) {
        auto& mb = classes.getReference("MemoryBlock");
        if (!mb.constructors.contains("()")) mb.constructors.insert(0, "()");
        else {
            int idx = mb.constructors.indexOf("()");
            if (idx > 0) { mb.constructors.remove(idx); mb.constructors.insert(0, "()"); }
        }
        bool hasEmptyInit = false;
        for (int i = 0; i < mb.methods.size(); ++i) {
            if (mb.methods[i].name == "init" && (mb.methods[i].parameters == "()" || mb.methods[i].parameters.isEmpty())) {
                LuaMethod m = mb.methods.removeAndReturn(i);
                mb.methods.insert(0, m);
                hasEmptyInit = true; break;
            }
        }
        if (!hasEmptyInit) {
            LuaMethod lm; lm.name = "init"; lm.parameters = "()";
            mb.methods.insert(0, lm);
        }
        allMethodNames.addIfNotAlreadyThere("init");
    }

    if (classes.contains("CtrlrModulator")) {
        classes.getReference("CtrlrModulator").parentClass = "CtrlrObject";
    }

    // 7. Finalize and Sort
    allMethodNames.removeDuplicates(false);
    allMethodNames.sort(true);
    classNames.removeDuplicates(false);
    classNames.sort(true);

    _DBG("AUTOCOMPLETE: Definition Loading Complete.");
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getGlobalSuggestions(const juce::String& prefix) {
    std::vector<SuggestionItem> results;
    juce::StringArray added;
    
    // 1. MANDATORY LIBRARIES (The "C" Icon)
    // We add these first. No matter what, if "str" is typed, "string" is added here.
    juce::StringArray libraries = { "math", "table", "string", "utils" };
    for (auto& lib : libraries) {
        if (lib.startsWithIgnoreCase(prefix)) {
            results.push_back({ lib, TypeClass });
            added.add(lib.toLowerCase());
        }
    }

    // 2. GLOBALS / KEYWORDS
    juce::StringArray globals = { "local", "function", "if", "then", "else", "elseif", "end", "for", "while", "do", "return", "break", "nil", "true", "false", "panel", "mod", "value", "source", "comp", "event", "canvas", "g", "midi", "console" };
    for (auto& g : globals) {
        if (g.startsWithIgnoreCase(prefix) && !added.contains(g.toLowerCase())) {
            results.push_back({ g, TypeGlobal });
            added.add(g.toLowerCase());
        }
    }

    // 3. CLASSES (Ctrlr specific)
    for (auto& c : classNames) {
        if (c.startsWithIgnoreCase(prefix) && !added.contains(c.toLowerCase())) {
            results.push_back({ c, TypeClass });
            added.add(c.toLowerCase());
        }
    }

    // 4. METHODS (The "M" Icon)
    for (auto& m : allMethodNames) {
        if (m.startsWithIgnoreCase(prefix)) {
            juce::String lowerM = m.toLowerCase();
            
            // THE CRITICAL FIX:
            // If the name is exactly "string" or "math", skip it.
            // We ALREADY added the Class version in Step 1.
            if (libraries.contains(lowerM)) continue;

            if (!added.contains(lowerM)) {
                SuggestionType type = utilityMethodNames.contains(m) ? TypeUtility : TypeMethod;
                results.push_back({ m, type });
                added.add(lowerM);
            }
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
    // 1. HARDCODED FALLBACKS (Absolute Priority)
    if (methodNameOrClass == "setHeight") return "int newHeight";
    if (methodNameOrClass == "setWidth") return "int newWidth";
    if (methodNameOrClass == "setProperty") return "String propertyName, var newValue";
    if (methodNameOrClass == "getProperty") return "String propertyName";
    if (methodNameOrClass == "getPropertyInt") return "String propertyName";
    if (methodNameOrClass == "getPropertyString") return "String propertyName";
    if (methodNameOrClass == "getModulatorByName") return "String name";
    if (methodNameOrClass == "setBounds") return "int x, int y, int width, int height";
    if (methodNameOrClass == "setSize") return "int width, int height";

    // Helper lambda to clean up "(arg1, arg2)" into "arg1, arg2" for the bubble
    auto sanitizeForBubble = [](juce::String p) -> juce::String {
        p = p.trim();
        if (p == "()" || p.isEmpty() || p == "( )") return "";
        if (p.startsWith("(") && p.endsWith(")"))
            return p.substring(1, p.length() - 1).trim();
        return p;
    };

    // Helper lambda for Global Search
    auto performGlobalSearch = [&](const juce::String& methodName) {
        juce::StringArray results;
        for (juce::HashMap<juce::String, LuaClass>::Iterator it (classes); it.next();) {
            const auto& lc = it.getValue();
            for (auto& m : lc.methods) {
                if (m.name == methodName) {
                    juce::String cleaned = sanitizeForBubble(m.parameters);
                    if (cleaned.isNotEmpty() && !results.contains(cleaned)) results.add(cleaned);
                }
            }
        }
        return results.joinIntoString("\n").trim();
    };

    // 2. SEARCH LOGIC
    if (className.isEmpty())
    {
        return performGlobalSearch(methodNameOrClass);
    }
    else
    {
        juce::String currentClass = className;
        while (currentClass.isNotEmpty() && classes.contains(currentClass))
        {
            const auto& lc = classes.getReference(currentClass);
            
            // Search Instance Methods - Return the first one found (respects our insert(0) order)
            for (auto& m : lc.methods) {
                if (m.name == methodNameOrClass) {
                    return sanitizeForBubble(m.parameters);
                }
            }
            
            // Search Static Methods
            for (auto& m : lc.staticMethods) {
                if (m.name == methodNameOrClass) {
                    return sanitizeForBubble(m.parameters);
                }
            }

            // Search Constructors
            if (methodNameOrClass == lc.name) {
                if (lc.constructors.size() > 0) {
                    return sanitizeForBubble(lc.constructors[0]);
                }
            }

            currentClass = lc.parentClass; // Climb the tree
        }

        // 3. FALLBACK TO GLOBAL if specific class search yielded nothing
        return performGlobalSearch(methodNameOrClass);
    }

    return "";
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


juce::String CtrlrLuaMethodAutoCompleteManager::getClassNameForVariable(const juce::String& varName, const juce::String& code) {
    // 1. Hardcoded shortcuts
    if (varName == "panel")  return "CtrlrPanel";
    if (varName == "mod")    return "CtrlrModulator";
    if (varName == "comp")   return "CtrlrComponent";
    if (varName == "g")      return "Graphics";
    
    // 2. DYNAMIC LOOK-BACK
    // We use the 'code' string passed in from the editor
    int assignmentPos = code.lastIndexOf(varName + " =");
    if (assignmentPos != -1) {
        juce::String afterEquals = code.substring(assignmentPos + varName.length() + 2).trimStart();
        for (auto& className : classNames) {
            if (afterEquals.startsWith(className)) {
                return className;
            }
        }
    }
    
    // 3. Fallbacks
    if (varName == "m" || varName == "mb" || varName == "mem") return "MemoryBlock";
    if (varName == "f") return "File";
    
    return "";
}
