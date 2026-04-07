#include "CtrlrLuaMethodAutoCompleteManager.h"
#include "BinaryData.h"
#include "CtrlrMacros.h"
#include "CtrlrLog.h"

CtrlrLuaMethodAutoCompleteManager::CtrlrLuaMethodAutoCompleteManager()
{
    loadDefinitions();
}

void CtrlrLuaMethodAutoCompleteManager::loadDefinitions()
{
    const juce::ScopedLock sl(lock);

    classes.clear();
    classNames.clear();
    allMethodNames.clear();
    utilityMethodNames.clear();

    // =========================================================================
    // 1. Load MAIN API from XML
    // =========================================================================
    if (BinaryData::LuaAPI_xml != nullptr)
    {
        juce::XmlDocument doc(juce::String::createStringFromData(BinaryData::LuaAPI_xml, BinaryData::LuaAPI_xmlSize));
        std::unique_ptr<juce::XmlElement> root(doc.getDocumentElement());

        if (root != nullptr && root->hasTagName("LuaAPI"))
        {
            forEachXmlChildElement(*root, classXml)
            {
                if (!classXml->hasTagName("class")) continue;

                LuaClass lc;
                lc.name = classXml->getStringAttribute("name");
                lc.parentClass = classXml->getStringAttribute("inherits");
                juce::String cppName = classXml->getStringAttribute("cpp_name");

                // Instance Methods
                if (auto* mList = classXml->getChildByName("methods"))
                {
                    forEachXmlChildElement(*mList, m)
                    {
                        juce::String name = m->getStringAttribute("name");
                        juce::String args = m->getStringAttribute("args").trim();

                        if (args.isEmpty() || args == "()" || args.containsOnly(", ") || args.containsOnly("(, )"))
                            args = "()";
                        else
                        {
                            if (!args.startsWith("(")) args = "(" + args;
                            if (!args.endsWith(")"))   args = args + ")";
                            if (args.substring(1, args.length() - 1).trim().containsOnly(", ")) args = "()";
                        }

                        bool alreadyExists = false;
                        for (auto& existing : lc.methods)
                            if (existing.name == name && existing.parameters == args) { alreadyExists = true; break; }

                        if (!alreadyExists)
                        {
                            LuaMethod lm;
                            lm.name = name;
                            lm.parameters = args;
                            lm.isStatic = false;
                            if (lc.name == "CtrlrLuaUtils") { lm.isStatic = true; lc.staticMethods.add(lm); }
                            lc.methods.add(lm);
                            if (name != "string" && name != "math" && name != "table")
                                allMethodNames.add(name);
                        }
                    }
                }

                // Static Methods � skip type="constructor" entries (handled by constructors block)
                if (auto* sList = classXml->getChildByName("static_methods"))
                {
                    forEachXmlChildElement(*sList, s)
                    {
                        if (s->getStringAttribute("type") == "constructor") continue;

                        juce::String name = s->getStringAttribute("name");
                        juce::String args = s->getStringAttribute("args").trim();

                        if (args.isEmpty() || args.containsOnly(", ")) args = "()";
                        else { if (!args.startsWith("(")) args = "(" + args; if (!args.endsWith(")")) args = args + ")"; }

                        LuaMethod sm; sm.name = name; sm.parameters = args; sm.isStatic = true;
                        lc.staticMethods.add(sm);
                        lc.methods.add(sm);
                        if (name != "string" && name != "math" && name != "table")
                            allMethodNames.add(name);
                    }
                }

                // Constructors � goes into lc.constructors ONLY (not staticMethods)
                if (auto* cList = classXml->getChildByName("constructors"))
                {
                    forEachXmlChildElement(*cList, c)
                    {
                        juce::String args = c->getStringAttribute("args").trim();
                        if (!args.startsWith("(")) args = "(" + args;
                        if (!args.endsWith(")"))   args = args + ")";
                        juce::String params = (args == "()" || args.isEmpty()) ? "()" : args;
                        if (!lc.constructors.contains(params))
                            lc.constructors.add(params);
                    }
                }

                classes.set(lc.name, lc);
                if (cppName.isNotEmpty() && cppName != lc.name) classes.set(cppName, lc);
                classNames.add(lc.name);
            }
        }
    }

    // =========================================================================
    // 2. Hardcoded Constructor Classes
    //    Guaranteed correct regardless of XML � also clears any leaked statics.
    // =========================================================================

    auto setupConstructorClass = [this](const juce::String& name, const juce::StringArray& ctors)
        {
            if (!classes.contains(name)) { LuaClass lc; lc.name = name; classes.set(name, lc); }
            classNames.addIfNotAlreadyThere(name);
            auto& lc = classes.getReference(name);
            lc.constructors.clear();
            lc.staticMethods.clear();
            for (auto& c : ctors) lc.constructors.add(c);
        };

    setupConstructorClass("Rectangle", { "()", "(int x, int y, int width, int height)", "(int width, int height)" });
    setupConstructorClass("RectangleFloat", { "()", "(float x, float y, float width, float height)", "(float width, float height)" });
    setupConstructorClass("Point", { "()", "(float x, float y)" });
    setupConstructorClass("Path", { "()", "(const Path& other)" });

    // =========================================================================
    // 3. Manual Injection of Base / Utility Classes
    // =========================================================================

    if (!classes.contains("LMemoryBlock"))
    {
        LuaClass mbClass; mbClass.name = "MemoryBlock";
        LuaMethod initM; initM.name = "init"; initM.parameters = "()";
        mbClass.methods.add(initM);
        classes.set("LMemoryBlock", mbClass);
        classes.set("MemoryBlock", mbClass);
        classNames.addIfNotAlreadyThere("MemoryBlock");
        classNames.addIfNotAlreadyThere("LMemoryBlock");
    }

    if (!classes.contains("Listener")) { LuaClass l; l.name = "Listener"; classes.set("Listener", l); classNames.addIfNotAlreadyThere("Listener"); }
    if (!classes.contains("Timer")) { LuaClass t; t.name = "Timer";    classes.set("Timer", t);    classNames.addIfNotAlreadyThere("Timer"); }
    // if (!classes.contains("Path"))
    // {
    // LuaClass pathClass; 
    // pathClass.name = "Path";
    // // Add common methods
    // for (auto& n : juce::StringArray{ "addRectangle", "addEllipse", "addQuadrilateral", "lineTo", "startNewSubPath", "closeSubPath", "clear" })
    // {
    //     LuaMethod m; m.name = n; m.parameters = "()";
    //     pathClass.methods.add(m);
    // }
    // classes.set("Path", pathClass);
    // classNames.addIfNotAlreadyThere("Path");
    // }
    if (!classes.contains("Component"))
    {
        LuaClass comp; comp.name = "Component";
        for (auto& n : juce::StringArray{ "setBounds", "setSize", "setVisible", "getX", "getY", "getWidth", "getHeight", "repaint", "setHeight", "setWidth" })
        {
            LuaMethod m; m.name = n;
            if (n == "setBounds")  m.parameters = "(int x, int y, int width, int height)";
            else if (n == "setSize")    m.parameters = "(int width, int height)";
            else if (n == "setHeight")  m.parameters = "(int height)";
            else if (n == "setWidth")   m.parameters = "(int width)";
            else if (n == "setVisible") m.parameters = "(bool shouldBeVisible)";
            else                        m.parameters = "()";
            comp.methods.add(m);
            allMethodNames.add(n);
        }
        classes.set(comp.name, comp);
        classNames.add(comp.name);
    }

    if (!classes.contains("CtrlrObject"))
    {
        LuaClass obj; obj.name = "CtrlrObject"; obj.parentClass = "Component";
        for (auto& n : juce::StringArray{ "getProperty", "setProperty", "getPropertyInt", "getPropertyString" })
        {
            LuaMethod m; m.name = n;
            m.parameters = (n == "setProperty") ? "(String name, var value)" : "(String name)";
            obj.methods.add(m);
            allMethodNames.add(n);
        }
        classes.set(obj.name, obj);
        classNames.add(obj.name);
    }

    if (!classes.contains("CtrlrMidiMessage")) { LuaClass m; m.name = "CtrlrMidiMessage"; classes.set("CtrlrMidiMessage", m); classNames.addIfNotAlreadyThere("CtrlrMidiMessage"); }
    {
        auto& mid = classes.getReference("CtrlrMidiMessage");
        mid.constructors.clear(); mid.methods.clear();
        mid.constructors.add("(String hexData)");
        mid.constructors.add("({byteTable})");
        mid.constructors.add("(MemoryBlock data)");
        for (auto& n : juce::StringArray{ "getSize", "getData", "getType", "getChannel", "getTimeStamp", "setTimeStamp" })
        {
            LuaMethod lm; lm.name = n; lm.parameters = "()";
            mid.methods.add(lm);
            allMethodNames.addIfNotAlreadyThere(n);
        }
    }

    // Justification enum statics
    if (!classes.contains("Justification")) { LuaClass j; j.name = "Justification"; classes.set("Justification", j); classNames.addIfNotAlreadyThere("Justification"); }
    {
        auto& j = classes.getReference("Justification");
        j.staticMethods.clear();
        for (auto& e : juce::StringArray{ "left", "right", "horizontallyCentred", "top", "bottom", "verticallyCentred", "centred" })
        {
            LuaMethod m; m.name = e; m.parameters = ""; m.isStatic = true;
            j.staticMethods.add(m);
        }
    }

    // =========================================================================
    // 4. Hard-fix Specific Instances
    // =========================================================================

    if (classes.contains("CtrlrPanel"))
    {
        auto& panel = classes.getReference("CtrlrPanel");
        panel.parentClass = "Component";
        for (auto& mName : juce::StringArray{ "getModulatorByName", "getModulator", "getModulatorByIndex", "getNumModulators", "sendMidiMessageNow", "getCanvas", "getPanelEditor", "getGlobalVariable", "setGlobalVariable" })
        {
            for (int i = panel.methods.size(); --i >= 0;)
                if (panel.methods.getReference(i).name == mName) panel.methods.remove(i);

            LuaMethod lm; lm.name = mName;
            if (mName == "sendMidiMessageNow") lm.parameters = "(MidiMessage) | (hexString) | ({table})";
            else if (mName.contains("ByName"))      lm.parameters = "(String name)";
            else if (mName.contains("ByIndex"))     lm.parameters = "(int index)";
            else if (mName == "setGlobalVariable")  lm.parameters = "(int index, int value)";
            else                                    lm.parameters = "()";
            panel.methods.add(lm);
        }
    }

    if (classes.contains("CtrlrMidiMessage"))
    {
        auto& midiClass = classes.getReference("CtrlrMidiMessage");
        midiClass.constructors.clear();
        midiClass.constructors.add("(String hexData)");
        midiClass.constructors.add("({table bytes})");
        midiClass.constructors.add("(MemoryBlock data)");
    }

    // =========================================================================
    // 5. Detailed Library Definitions (string, math, table)
    // =========================================================================

    {
        LuaClass strLib; strLib.name = "string";
        for (auto& e : juce::StringArray{ "byte(s, i, j)", "char(...)", "dump(function)", "find(s, pattern, init, plain)", "format(formatstring, ...)", "gmatch(s, pattern)", "gsub(s, pattern, repl, n)", "len(s)", "lower(s)", "match(s, pattern, init)", "rep(s, n)", "reverse(s)", "sub(s, i, j)", "upper(s)" })
        {
            LuaMethod lm;
            lm.name = e.upToFirstOccurrenceOf("(", false, false).trim();
            lm.parameters = e.substring(e.indexOf("(")).trim();
            lm.isStatic = true;
            strLib.staticMethods.add(lm);
            strLib.methods.add(lm);
        }
        classes.set("string", strLib);
        classes.set("String", strLib);
        classNames.addIfNotAlreadyThere("string");
    }

    if (!classes.contains("math"))
    {
        LuaClass mathLib; mathLib.name = "math";
        for (auto& e : juce::StringArray{ "abs(x)", "acos(x)", "asin(x)", "atan(x)", "ceil(x)", "cos(x)", "deg(x)", "exp(x)", "floor(x)", "fmod(x, y)", "huge", "log(x)", "max(x, ...)", "min(x, ...)", "modf(x)", "pi", "pow(x, y)", "rad(x)", "random(m, n)", "randomseed(x)", "sin(x)", "sqrt(x)", "tan(x)" })
        {
            if (e == "pi" || e == "huge") { mathLib.properties.add(e); continue; }
            LuaMethod lm;
            lm.name = e.upToFirstOccurrenceOf("(", false, false).trim();
            lm.parameters = e.substring(e.indexOf("(")).trim();
            lm.isStatic = true;
            mathLib.staticMethods.add(lm);
            mathLib.methods.add(lm);
        }
        classes.set("math", mathLib);
        classNames.addIfNotAlreadyThere("math");
    }

    // =========================================================================
    // 6. Inject Common Libs
    // =========================================================================

    auto injectStaticLib = [this](juce::String libName, juce::StringArray methods)
        {
            if (!classes.contains(libName)) { LuaClass lc; lc.name = libName; classes.set(libName, lc); }
            auto& lc = classes.getReference(libName);
            for (auto& mName : methods)
            {
                bool exists = false;
                for (auto& sm : lc.staticMethods) if (sm.name == mName) { exists = true; break; }
                for (auto& m : lc.methods)       if (m.name == mName) { exists = true; break; }
                if (!exists) { LuaMethod lm; lm.name = mName; lm.parameters = "()"; lm.isStatic = true; lc.staticMethods.add(lm); lc.methods.add(lm); }
            }
            classNames.addIfNotAlreadyThere(libName);
        };

    injectStaticLib("table", { "insert", "remove", "sort", "concat", "unpack" });

    // =========================================================================
    // 7. MemoryBlock Specialized Fixes
    // =========================================================================

    if (classes.contains("LMemoryBlock") && !classes.contains("MemoryBlock"))
        classes.set("MemoryBlock", classes["LMemoryBlock"]);

    if (classes.contains("MemoryBlock"))
    {
        auto& mb = classes.getReference("MemoryBlock");
        if (!mb.constructors.contains("()")) mb.constructors.insert(0, "()");
        else { int idx = mb.constructors.indexOf("()"); if (idx > 0) { mb.constructors.remove(idx); mb.constructors.insert(0, "()"); } }

        bool hasEmptyInit = false;
        for (int i = 0; i < mb.methods.size(); ++i)
            if (mb.methods[i].name == "init" && (mb.methods[i].parameters == "()" || mb.methods[i].parameters.isEmpty()))
            {
                LuaMethod m = mb.methods.removeAndReturn(i);
                mb.methods.insert(0, m);
                hasEmptyInit = true; break;
            }
        if (!hasEmptyInit) { LuaMethod lm; lm.name = "init"; lm.parameters = "()"; mb.methods.insert(0, lm); }
        allMethodNames.addIfNotAlreadyThere("init");
    }

    if (classes.contains("CtrlrModulator"))
        classes.getReference("CtrlrModulator").parentClass = "CtrlrObject";

    if (classes.contains("CtrlrLuaUtils"))
    {
        auto& utilsClass = classes.getReference("CtrlrLuaUtils");
        if (utilsClass.staticMethods.isEmpty())
            for (auto& m : utilsClass.methods) utilsClass.staticMethods.add(m);
    }

    // =========================================================================
    // 8. Finalize and Sort
    // =========================================================================
    allMethodNames.removeDuplicates(false);
    allMethodNames.sort(true);
    classNames.removeDuplicates(false);
    classNames.sort(true);

    _DBG("AUTOCOMPLETE: Definition Loading Complete.");
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getGlobalSuggestions(const juce::String& prefix)
{
    std::vector<SuggestionItem> results;
    juce::StringArray added;

    // VIP: MemoryBlock gets two entries (constructor + static accessor)
    if (juce::String("MemoryBlock").startsWithIgnoreCase(prefix))
    {
        results.push_back({ "MemoryBlock", TypeMethod });
        results.push_back({ "MemoryBlock.", TypeStatic });
        added.add("memoryblock");
        added.add("lmemoryblock");
    }

    // Mandatory libraries
    for (auto& lib : juce::StringArray{ "math", "table", "string", "utils", "MemoryBlock" })
    {
        if (lib.startsWithIgnoreCase(prefix)) { results.push_back({ lib, TypeClass }); added.add(lib.toLowerCase()); }
    }

    // Keywords / globals
    for (auto& g : juce::StringArray{ "local", "function", "if", "then", "else", "elseif", "end", "for", "while", "do", "return", "break", "nil", "true", "false", "panel", "mod", "value", "source", "comp", "event", "canvas", "g", "midi", "console" })
    {
        if (g.startsWithIgnoreCase(prefix) && !added.contains(g.toLowerCase()))
        {
            results.push_back({ g, TypeGlobal }); added.add(g.toLowerCase());
        }
    }

    // Classes
    for (auto& c : classNames)
    {
        if (added.contains(c.toLowerCase())) continue;
        if (c.startsWithIgnoreCase(prefix))
        {
            const auto& lc = classes.getReference(c);
            if (!lc.constructors.isEmpty())  results.push_back({ c + "()", TypeClass });
            if (!lc.staticMethods.isEmpty()) results.push_back({ c + ".", TypeStatic });
            added.add(c.toLowerCase());
        }
    }

    // Methods
    juce::StringArray libraries = { "math", "table", "string", "utils", "memoryblock" };
    for (auto& m : allMethodNames)
    {
        if (m.startsWithIgnoreCase(prefix))
        {
            juce::String lowerM = m.toLowerCase();
            if (libraries.contains(lowerM) || added.contains(lowerM)) continue;
            SuggestionType type = utilityMethodNames.contains(m) ? TypeUtility : TypeMethod;
            results.push_back({ m, type });
            added.add(lowerM);
        }
    }

    return results;
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getMethodSuggestionsForClass(
    const juce::String& className, const juce::String& prefix,
    bool includeInstance, bool includeStatic, bool includeProperties)
{
    const juce::ScopedLock sl(lock);

    std::vector<SuggestionItem> suggestions;
    juce::StringArray addedExactSignatures;
    juce::String currentClass = className;

    _DBG("--- START SEARCH: [" + prefix + "] in Class: [" + className + "] ---");

    while (currentClass.isNotEmpty())
    {
        juce::String actualKey = "";
        for (auto& name : classNames)
            if (name.equalsIgnoreCase(currentClass)) { actualKey = name; break; }

        if (actualKey.isEmpty() || !classes.contains(actualKey))
        {
            _DBG("    ! Class [" + currentClass + "] not found in definitions.");
            break;
        }

        const LuaClass lc = classes[actualKey];
        int classMatches = 0;

        // Static & Enums ('.' trigger)
        if (includeStatic)
        {
            for (auto& m : lc.staticMethods)
            {
                if (prefix.isNotEmpty() && !m.name.startsWithIgnoreCase(prefix)) continue;
                SuggestionItem item;
                if (m.parameters.isEmpty() || m.parameters == " ") { item.text = m.name; item.type = TypeProperty; }
                else { item.text = m.name + " " + m.parameters; item.type = TypeStatic; }
                if (!addedExactSignatures.contains(item.text)) { suggestions.push_back(item); addedExactSignatures.add(item.text); classMatches++; }
            }

            // Show constructors when accessing via dot (e.g. after ClassName.)
            for (auto& cParams : lc.constructors)
            {
                if (prefix.isNotEmpty() && !lc.name.startsWithIgnoreCase(prefix)) continue;
                juce::String sig = lc.name + " " + (cParams.isEmpty() ? "()" : cParams);
                if (!addedExactSignatures.contains(sig)) { suggestions.push_back({ sig, TypeMethod }); addedExactSignatures.add(sig); classMatches++; }
            }
        }

        // Instance Methods (':' trigger)
        if (includeInstance)
        {
            for (auto& m : lc.methods)
            {
                if (prefix.isNotEmpty() && !m.name.startsWithIgnoreCase(prefix)) continue;
                juce::String sig = m.name + " " + m.parameters;
                if (!addedExactSignatures.contains(sig)) { suggestions.push_back({ sig, TypeMethod }); addedExactSignatures.add(sig); classMatches++; }
            }
        }

        _DBG("    Found " + juce::String(classMatches) + " matches in [" + actualKey + "]");

        currentClass = lc.parentClass;
        if (currentClass.equalsIgnoreCase(actualKey) || currentClass.isEmpty()) break;
    }

    _DBG("--- SEARCH COMPLETE: Total " + juce::String(suggestions.size()) + " matches ---");
    return suggestions;
}

juce::String CtrlrLuaMethodAutoCompleteManager::getMethodParams(const juce::String& className, const juce::String& methodNameOrClass)
{
    // Hardcoded priority overrides
    if (methodNameOrClass == "setHeight")          return "int newHeight";
    if (methodNameOrClass == "setWidth")           return "int newWidth";
    if (methodNameOrClass == "setProperty")        return "String propertyName, var newValue";
    if (methodNameOrClass == "getProperty")        return "String propertyName";
    if (methodNameOrClass == "getPropertyInt")     return "String propertyName";
    if (methodNameOrClass == "getPropertyString")  return "String propertyName";
    if (methodNameOrClass == "getModulatorByName") return "String name";
    if (methodNameOrClass == "setBounds")          return "int x, int y, int width, int height";
    if (methodNameOrClass == "setSize")            return "int width, int height";
    if (methodNameOrClass == "fromRGBA")           return "int red, int green, int blue, int alpha";
    if (methodNameOrClass == "fromRGB")            return "int red, int green, int blue";
    if (methodNameOrClass == "fromFloatRGBA")      return "float red, float green, float blue, float alpha";
    if (methodNameOrClass == "greyLevel")          return "float brightness";
    if (methodNameOrClass == "fromHSV")            return "float hue, float saturation, float brightness, float alpha";
    if (methodNameOrClass == "translation")        return "float x, float y";
    if (methodNameOrClass == "rotation")           return "float angleInRadians";
    if (methodNameOrClass == "scale")              return "float scaleFactor";

    auto sanitizeForBubble = [](juce::String p) -> juce::String
        {
            p = p.trim();
            if (p.startsWith("(") && p.endsWith(")")) p = p.substring(1, p.length() - 1).trim();
            if (p == "()" || p.isEmpty() || p == "( )") return "";
            return p;
        };

    auto performGlobalSearch = [&](const juce::String& methodName)
        {
            juce::StringArray results;
            for (juce::HashMap<juce::String, LuaClass>::Iterator it(classes); it.next();)
            {
                const auto& lc = it.getValue();
                juce::Array<LuaMethod> allMethods;
                allMethods.addArray(lc.methods);
                allMethods.addArray(lc.staticMethods);
                for (auto& m : allMethods)
                    if (m.name == methodName) { juce::String cleaned = sanitizeForBubble(m.parameters); if (!results.contains(cleaned)) results.add(cleaned); }
                if (lc.name == methodName)
                    for (auto& c : lc.constructors) { juce::String cleaned = sanitizeForBubble(c); if (!results.contains(cleaned)) results.add(cleaned); }
            }
            return results.joinIntoString("\n").trim();
        };

    if (className.isEmpty())
        return performGlobalSearch(methodNameOrClass);

    juce::String currentClass = className;
    while (currentClass.isNotEmpty() && classes.contains(currentClass))
    {
        const auto& lc = classes.getReference(currentClass);
        for (auto& m : lc.methods)       if (m.name == methodNameOrClass) return sanitizeForBubble(m.parameters);
        for (auto& m : lc.staticMethods) if (m.name == methodNameOrClass) return sanitizeForBubble(m.parameters);
        if (methodNameOrClass == lc.name && lc.constructors.size() > 0)
            return sanitizeForBubble(lc.constructors.joinIntoString("\n"));
        currentClass = lc.parentClass;
    }

    return performGlobalSearch(methodNameOrClass);
}

juce::String CtrlrLuaMethodAutoCompleteManager::resolveReturnType(const juce::String& className, const juce::String& methodName)
{
    juce::String currentClass = className;
    while (currentClass.isNotEmpty() && classes.contains(currentClass))
    {
        const auto& lc = classes.getReference(currentClass);
        if (currentClass == "CtrlrPanel")
        {
            if (methodName.contains("getModulator")) return "CtrlrModulator";
            if (methodName == "getComponent")        return "CtrlrComponent";
        }
        if (currentClass == "CtrlrModulator")
        {
            if (methodName == "getComponent") return "CtrlrComponent";
            if (methodName == "getPanel")     return "CtrlrPanel";
        }
        if (currentClass == "CtrlrComponent")
            if (methodName == "getOwner") return "CtrlrModulator";
        currentClass = lc.parentClass;
    }

    if (methodName == "getModulatorByName" || methodName == "getModulator" || methodName == "getModulatorByIndex") return "CtrlrModulator";
    if (methodName == "getComponent" || methodName == "getOwner" || methodName == "getControl")                    return "CtrlrComponent";
    if (methodName == "getPanel" || methodName == "getOwnerPanel")                                                 return "CtrlrPanel";
    if (methodName == "getMemoryBlock" || methodName == "getData")                                                 return "MemoryBlock";
    if (methodName == "getLuaManager")                                                                             return "CtrlrLuaManager";
    if (methodName == "getCanvas")                                                                                 return "Graphics";
    if (methodName == "getGlobalTimer")                                                                            return "Timer";
    if (methodName == "getLastMidiMessage" || methodName == "getMidiMessage")                                      return "CtrlrMidiMessage";
    if (methodName == "setBounds")                                                                                 return "number x, number y, number width, number height";

    return "";
}

juce::StringArray CtrlrLuaMethodAutoCompleteManager::getClassNames() const
{
    juce::StringArray names;
    for (juce::HashMap<juce::String, LuaClass>::Iterator it(classes); it.next();)
        names.add(it.getKey());
    return names;
}

juce::String CtrlrLuaMethodAutoCompleteManager::getClassNameForVariable(const juce::String& varName, const juce::String& code)
{
    // Static/Global Shortcuts
    if (varName == "panel")                                               return "CtrlrPanel";
    if (varName == "mod")                                                 return "CtrlrModulator";
    if (varName == "comp")                                                return "CtrlrComponent";
    if (varName == "g")                                                   return "Graphics";
    if (varName == "utils")                                               return "CtrlrLuaUtils";
    if (varName == "math")                                                return "math";
    if (varName == "table")                                               return "table";
    if (varName == "string")                                              return "string";
    if (varName == "MemoryBlock" || varName == "memoryBlock")             return "LMemoryBlock";
    if (varName == "CtrlrMidiMessage")                                    return "CtrlrMidiMessage";
    if (varName.equalsIgnoreCase("Path"))                                 return "Path";
    if (varName.equalsIgnoreCase("Rectangle"))                            return "Rectangle";
    if (varName.equalsIgnoreCase("RectangleFloat"))                       return "RectangleFloat";
    if (varName.equalsIgnoreCase("Point"))                                return "Point";

    // Chain resolution (e.g. mb:someMethod():)
    if (varName.contains(":") || varName.contains("."))
    {
        juce::String normalized = varName.replace(".", ":");
        juce::StringArray tokens;
        tokens.addTokens(normalized, ":", "");
        juce::String currentType = "";
        for (int i = 0; i < tokens.size(); ++i)
        {
            juce::String segment = tokens[i].upToFirstOccurrenceOf("(", false, false).trim();
            if (i == 0) currentType = getClassNameForVariable(segment, code);
            else        currentType = resolveReturnType(currentType, segment);
            if (currentType.isEmpty()) break;
        }
        if (currentType.isNotEmpty()) return currentType;
    }

    // Dynamic look-back (assignments: p = Path())
    int assignmentPos = code.lastIndexOf(varName + " =");
    if (assignmentPos == -1) assignmentPos = code.lastIndexOf(varName + "=");

  if (assignmentPos != -1)
{
    // Start after the variable name
    juce::String rhs = code.substring(assignmentPos + varName.length()).trimStart();
    
    // STRIP THE EQUALS SIGN IF PRESENT
    if (rhs.startsWith("=")) 
        rhs = rhs.substring(1).trimStart();

    // Now rhs is "Path()..." 
    juce::String potentialClass = rhs.upToFirstOccurrenceOf("(", false, false).trim();
    
    // Check for exact class name matches
    for (auto& name : classNames)
    {
        if (name.equalsIgnoreCase(potentialClass))
            return name; 
    }


        if (potentialClass.equalsIgnoreCase("MemoryBlock") || potentialClass.equalsIgnoreCase("LMemoryBlock")) return "LMemoryBlock";

        if (rhs != varName && rhs.isNotEmpty())
        {
            if (rhs.contains(".")) { juce::String base = rhs.upToFirstOccurrenceOf(".", false, false).trim(); for (auto& name : classNames) if (name.equalsIgnoreCase(base)) return name; }
            return getClassNameForVariable(rhs, code);
        }
    }

    // Static name fallbacks
    if (varName == "m" || varName == "mb" || varName == "mem" || varName == "memoryBlock") return "LMemoryBlock";
    if (varName == "f") return "File";

    return "";
}