#include "CtrlrLuaMethodAutoCompleteManager.h"

// This is required to access BinaryData::LuaAPI_xml
#include "BinaryData.h"
#include "CtrlrLog.h"
#include "CtrlrLuaApiLookup.h"
#include "CtrlrMacros.h"

CtrlrLuaMethodAutoCompleteManager::CtrlrLuaMethodAutoCompleteManager()
{
    // Load the API from the binary resources as soon as the manager is created
    loadDefinitions();
}

void CtrlrLuaMethodAutoCompleteManager::loadDefinitions()
{
    const juce::ScopedLock sl(lock);

    classes.clear();
    classNames.clear();
    allMethodNames.clear();
    utilityMethodNames.clear();

	// 1. Load data directly from the RAM-native C++ CtrlrLuaApiLookup database
	auto &db = CtrlrLuaApiLookup::getInstance();
	auto allClassNames = db.getAllClassNames();

	for (const auto &className : allClassNames) {
		const auto *apiClass = db.getClassInfo(className);
		_DBG("CHECKING CLASS: " + className);
		if (!apiClass)
			continue;

		LuaClass lc;
		lc.name = apiClass->name;
		lc.parentClass = "";

		// Map Constructors
		for (const auto &c : apiClass->constructor) {
			lc.constructors.add(c.args.isEmpty() ? "()" : c.args);

			LuaMethod lm;
			lm.name = lc.name;
			lm.parameters = c.args;
			lm.isStatic = true;
			lc.methods.add(lm);
			lc.staticMethods.add(lm);
		}

		// Map Instance Methods
		for (const auto &m : apiClass->methods) {
			LuaMethod lm;
			lm.name = m.name;
			lm.parameters = m.args.isEmpty() ? "()" : m.args;
			lm.isStatic = false;

			if (lc.name == "CtrlrLuaUtils") {
				lm.isStatic = true;
				lc.staticMethods.add(lm);
			}

			lc.methods.add(lm);
			if (m.name != "string" && m.name != "math" && m.name != "table")
				allMethodNames.add(m.name);
		}

		// Map Static Methods
		for (const auto &m : apiClass->staticMethods) {
			LuaMethod sm;
			sm.name = m.name;
			sm.parameters = m.args.isEmpty() ? "()" : m.args;
			sm.isStatic = true;

			lc.staticMethods.add(sm);
			lc.methods.add(sm);

			if (m.name != "string" && m.name != "math" && m.name != "table")
				allMethodNames.add(m.name);
		}

		classes.set(lc.name, lc);
		classNames.add(lc.name);
	}

	// 2. Manual Injections & Parameter Enhancements

	// --- Component Parameter Overrides ---
	if (!classes.contains("Component")) {
		LuaClass comp;
		comp.name = "Component";
        juce::StringArray ui = {"setBounds", "setSize", "setVisible", "getX", "getY", "getWidth", "getHeight", "repaint", "setHeight", "setWidth"};
		for (auto &n : ui) {
			LuaMethod m;
            m.name = n;
			if (n == "setBounds")
				m.parameters = "(int x, int y, int width, int height)";
			else if (n == "setSize")
				m.parameters = "(int width, int height)";
			else if (n == "setHeight")
				m.parameters = "(int height)";
			else if (n == "setWidth")
				m.parameters = "(int width)";
			else if (n == "setVisible")
				m.parameters = "(bool shouldBeVisible)";
			else
				m.parameters = "()";

			comp.methods.add(m);
			allMethodNames.add(n);
		}
		classes.set(comp.name, comp);
        classNames.add(comp.name);
	}

	// --- CtrlrObject ---
	if (!classes.contains("CtrlrObject")) {
		LuaClass obj;
        obj.name = "CtrlrObject";
		obj.parentClass = "Component";
		juce::StringArray om = {"getProperty", "setProperty", "getPropertyInt", "getPropertyString"};
		for (auto &n : om) {
			LuaMethod m;
			m.name = n;
			m.parameters = (n == "setProperty") ? "(String name, var value)" : "(String name)";
			obj.methods.add(m);
			allMethodNames.add(n);
		}
		classes.set(obj.name, obj);
		classNames.add(obj.name);
	}

	// --- CtrlrPanel Parameter Enrichment ---
	if (classes.contains("CtrlrPanel")) {
		auto &panel = classes.getReference("CtrlrPanel");
		panel.parentClass = "Component";
		juce::StringArray missing = {"getModulatorByName", "getModulator",		 "getModulatorByIndex",
									 "getNumModulators",   "sendMidiMessageNow", "getCanvas",
									 "getPanelEditor",	   "getGlobalVariable",	 "setGlobalVariable"};
		for (auto &mName : missing) {
			for (int i = panel.methods.size(); --i >= 0;) {
				if (panel.methods.getReference(i).name == mName)
					panel.methods.remove(i);
			}
			LuaMethod lm;
			lm.name = mName;
			if (mName == "sendMidiMessageNow")
				lm.parameters = "(MidiMessage) | (hexString) | ({table})";
			else if (mName.contains("ByName"))
				lm.parameters = "(String name)";
			else if (mName.contains("ByIndex"))
				lm.parameters = "(int index)";
			else if (mName == "setGlobalVariable")
				lm.parameters = "(int index, int value)";
			else
				lm.parameters = "()";
			panel.methods.add(lm);
		}
	}

	// --- MemoryBlock / LMemoryBlock Parity Bridge ---
	// if (classes.contains("MemoryBlock") && !classes.contains("LMemoryBlock")) {
	//     classes.set("LMemoryBlock", classes["MemoryBlock"]);
	//     classNames.addIfNotAlreadyThere("LMemoryBlock");
	// } else if (classes.contains("LMemoryBlock") && !classes.contains("MemoryBlock")) {
	//     classes.set("MemoryBlock", classes["LMemoryBlock"]);
	//     classNames.addIfNotAlreadyThere("MemoryBlock");
	// }

	// --- CtrlrLuaUtils Static Bridge ---
	if (classes.contains("CtrlrLuaUtils")) {
		auto &utilsClass = classes.getReference("CtrlrLuaUtils");
		if (utilsClass.staticMethods.isEmpty()) {
			for (auto &m : utilsClass.methods) {
				utilsClass.staticMethods.add(m);
			}
		}
	}

	// --- Lua Standard Library: string ---
	LuaClass strLib;
	strLib.name = "string";
	juce::StringArray strMethods = {"byte(s, i, j)",
									"char(...)",
									"dump(function)",
									"find(s, pattern, init, plain)",
									"format(formatstring, ...)",
									"gmatch(s, pattern)",
									"gsub(s, pattern, repl, n)",
									"len(s)",
									"lower(s)",
									"match(s, pattern, init)",
									"rep(s, n)",
									"reverse(s)",
									"sub(s, i, j)",
									"upper(s)"};
	for (auto &mEntry : strMethods) {
		LuaMethod lm;
		lm.name = mEntry.upToFirstOccurrenceOf("(", false, false).trim();
		lm.parameters = mEntry.substring(mEntry.indexOf("(")).trim();
		lm.isStatic = true;
		strLib.staticMethods.add(lm);
		strLib.methods.add(lm);
	}
	classes.set("string", strLib);
	classes.set("String", strLib);
	classNames.addIfNotAlreadyThere("string");
	classNames.addIfNotAlreadyThere("String");

	// --- Lua Standard Library: math ---
	LuaClass mathLib;
	mathLib.name = "math";
	juce::StringArray mathMethods = {
		"abs(x)",	 "acos(x)",	 "asin(x)",		 "atan(x)",		  "ceil(x)",	 "cos(x)",		"deg(x)",
		"exp(x)",	 "floor(x)", "fmod(x, y)",	 "log(x)",		  "max(x, ...)", "min(x, ...)", "modf(x)",
		"pow(x, y)", "rad(x)",	 "random(m, n)", "randomseed(x)", "sin(x)",		 "sqrt(x)",		"tan(x)"};
	for (auto &mEntry : mathMethods) {
		LuaMethod lm;
		lm.name = mEntry.upToFirstOccurrenceOf("(", false, false).trim();
		lm.parameters = mEntry.substring(mEntry.indexOf("(")).trim();
		lm.isStatic = true;
		mathLib.staticMethods.add(lm);
		mathLib.methods.add(lm);
	}
	classes.set("math", mathLib);
	classNames.addIfNotAlreadyThere("math");

	// 3. Finalize and Sort
	allMethodNames.removeDuplicates(false);
	allMethodNames.sort(true);
	classNames.removeDuplicates(false);
    classNames.sort(true);

	_DBG("AUTOCOMPLETE: Definitions successfully loaded from C++ CtrlrLuaApiLookup database with manual patches.");
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getGlobalSuggestions(const juce::String &prefix)
{
    std::vector<SuggestionItem> results;
    std::vector<juce::String> addedLocal; // Helper to avoid duplicate tracking issues

    auto wasAdded = [&addedLocal](const juce::String& s) {
        return std::find(addedLocal.begin(), addedLocal.end(), s.toLowerCase()) != addedLocal.end();
    };

    auto markAdded = [&addedLocal](const juce::String& s) {
        addedLocal.push_back(s.toLowerCase());
    };

    // 1. MANDATORY LIBRARIES / CLASSES (The "C" Icon)
    juce::StringArray libraries = {"math", "table", "string", "utils", "MemoryBlock"};
    for (auto &lib : libraries)
    {
        if (lib.startsWithIgnoreCase(prefix) && !wasAdded(lib))
        {
            results.push_back({lib, TypeClass});
            markAdded(lib);
        }
    }

    // 2. GLOBALS / KEYWORDS
    juce::StringArray globals = {"local", "function", "if", "then", "else", "elseif", "end", "for", "while", "do", "return", "break", "nil", "true", "false", "panel", "mod", "value", "source", "comp", "event", "canvas", "g", "midi", "console"};
    for (auto &g : globals)
    {
        if (g.startsWithIgnoreCase(prefix) && !wasAdded(g))
        {
            results.push_back({g, TypeGlobal});
            markAdded(g);
        }
    }

    // 3. CLASSES (Fixed to never drop classes lacking explicit constructor metadata)
    for (auto &c : classNames)
    {
        if (wasAdded(c))
            continue;

        if (c.startsWithIgnoreCase(prefix))
        {
            const auto &lc = classes.getReference(c);
            bool addedAny = false;

            if (!lc.constructors.isEmpty())
            {
                results.push_back({c + "()", TypeClass});
                addedAny = true;
            }

            if (!lc.staticMethods.isEmpty())
            {
                results.push_back({c + ".", TypeStatic});
                addedAny = true;
            }

            // SAFETY FALLBACK: If a class has no constructors or static methods listed 
            // (like ZipFile), still show the class name so it doesn't get swallowed!
            if (!addedAny)
            {
                results.push_back({c, TypeClass});
            }

            markAdded(c);
        }
    }

    // 4. METHODS (The "M" Icon)
    for (auto &m : allMethodNames)
    {
        if (m.startsWithIgnoreCase(prefix))
        {
            juce::String lowerM = m.toLowerCase();

            if (wasAdded(lowerM))
                continue;

            SuggestionType type = utilityMethodNames.contains(m) ? TypeUtility : TypeMethod;
            results.push_back({m, type});
            markAdded(lowerM);
        }
    }

    return results;
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getClassSuggestions(const juce::String &prefix)
{
    std::vector<SuggestionItem> results;

    // Libraries you'd plausibly assign from or call into: x = utils.xxx / x = MemoryBlock()
    // (drop this block if you only want entries from classNames)
    static const juce::StringArray libraries = {"math", "table", "string", "utils", "MemoryBlock"};
    for (auto &lib : libraries)
    {
        if (lib.startsWithIgnoreCase(prefix) && !classNames.contains(lib, true))
            results.push_back({lib, TypeClass});
    }

    for (auto &c : classNames)
    {
        if (!c.startsWithIgnoreCase(prefix))
            continue;

        const auto &lc = classes.getReference(c);
        bool addedAny = false;

        if (!lc.constructors.isEmpty())
        {
            results.push_back({c + "()", TypeClass});   // x = Foo()
            addedAny = true;
        }

        if (!lc.staticMethods.isEmpty())
        {
            results.push_back({c + ".", TypeStatic});   // x = Foo.someStatic(...)
            addedAny = true;
        }

        if (!addedAny)
            results.push_back({c, TypeClass});          // never swallow classes lacking metadata
    }

    std::sort(results.begin(), results.end(),
              [](const SuggestionItem &a, const SuggestionItem &b)
              { return a.text.compareIgnoreCase(b.text) < 0; });

    return results;
}


std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getMethodSuggestionsForClass(
    const juce::String &className, const juce::String &prefix,
    bool includeInstance, bool includeStatic, bool includeProperties)
{
    const juce::ScopedLock sl(lock);

    std::vector<SuggestionItem> suggestions;
    juce::StringArray addedExactSignatures;
    juce::String currentClass = className;

    _DBG("--- START SEARCH: [" + prefix + "] in Class: [" + className + "] ---");

    while (currentClass.isNotEmpty())
    {
        // --- CASE INSENSITIVE LOOKUP FIX ---
        juce::String actualKey = "";
        for (auto &name : classNames)
        {
            if (name.equalsIgnoreCase(currentClass))
            {
                actualKey = name;
                break;
            }
        }

        // If we can't find the class in our definitions, stop searching the hierarchy
        if (actualKey.isEmpty() || !classes.contains(actualKey))
        {
            _DBG("    ! Class [" + currentClass + "] not found in definitions.");
            break;
        }

        const LuaClass lc = classes[actualKey];
        int classMatches = 0;

        // --- 1. Static & Enums (The '.' trigger) ---
        if (includeStatic)
        {
            _DBG("        Checking Static/Enums for [" + actualKey + "]");
            for (auto &m : lc.staticMethods)
            {
                if (prefix.isNotEmpty() && !m.name.startsWithIgnoreCase(prefix))
                    continue;

                SuggestionItem item;
                // Polish: No parameters = Enum/Constant style (Icon P)
                if (m.parameters.isEmpty() || m.parameters == " ")
                {
                    item.text = m.name;
                    item.type = TypeProperty;
                }
                else
                {
                    item.text = m.name + " " + m.parameters;
                    item.type = TypeStatic;
                }

                if (!addedExactSignatures.contains(item.text))
                {
                    suggestions.push_back(item);
                    addedExactSignatures.add(item.text);
                    classMatches++;
                }
            }

            // Constructors for Path.Path() style access
            for (auto &cParams : lc.constructors)
            {
                // If the prefix is empty, or the user started typing the class name as a method
                if (prefix.isNotEmpty() && !lc.name.startsWithIgnoreCase(prefix))
                    continue;

                // Change 'lc.name' to 'new' or just format it so it's clearly a constructor
                // Or, if you want it to behave like a standard static call:
                juce::String sig = lc.name + " " + (cParams.isEmpty() ? "()" : cParams);

                if (!addedExactSignatures.contains(sig))
                {
                    suggestions.push_back({sig, TypeMethod});
                    addedExactSignatures.add(sig);
                    classMatches++;
                }
            }
        }

        // --- 2. Instance Methods (The ':' trigger) ---
        if (includeInstance)
        {
            _DBG("        Checking Instance Methods for [" + actualKey + "]");
            for (auto &m : lc.methods)
            {
                if (prefix.isNotEmpty() && !m.name.startsWithIgnoreCase(prefix))
                    continue;

                juce::String sig = m.name + " " + m.parameters;
                if (!addedExactSignatures.contains(sig))
                {
                    suggestions.push_back({sig, TypeMethod});
                    addedExactSignatures.add(sig);
                    classMatches++;
                }
            }
        }

        _DBG("    Found " + juce::String(classMatches) + " matches in [" + actualKey + "]");

        // Move to parent class for inheritance support
        currentClass = lc.parentClass;

        // Safety: prevent infinite loops if a class points to itself as a parent
        if (currentClass.equalsIgnoreCase(actualKey) || currentClass.isEmpty())
            break;
    }

    _DBG("--- SEARCH COMPLETE: Total " + juce::String(suggestions.size()) + " matches ---");
    return suggestions;
}

juce::String CtrlrLuaMethodAutoCompleteManager::getMethodParams(const juce::String &className, const juce::String &methodNameOrClass)
{
    // 1. HARDCODED FALLBACKS (Absolute Priority)
    if (methodNameOrClass == "setHeight")
        return "int newHeight";
    if (methodNameOrClass == "setWidth")
        return "int newWidth";
    if (methodNameOrClass == "setProperty")
        return "String propertyName, var newValue";
    if (methodNameOrClass == "getProperty")
        return "String propertyName";
    if (methodNameOrClass == "getPropertyInt")
        return "String propertyName";
    if (methodNameOrClass == "getPropertyString")
        return "String propertyName";
    if (methodNameOrClass == "getModulatorByName")
        return "String name";
    if (methodNameOrClass == "setBounds")
        return "int x, int y, int width, int height";
    if (methodNameOrClass == "setSize")
        return "int width, int height";

    // --- NEW COLOUR STATICS ---
    if (methodNameOrClass == "fromRGBA")
        return "int red, int green, int blue, int alpha";
    if (methodNameOrClass == "fromRGB")
        return "int red, int green, int blue";
    if (methodNameOrClass == "fromFloatRGBA")
        return "float red, float green, float blue, float alpha";
    if (methodNameOrClass == "greyLevel")
        return "float brightness";
    if (methodNameOrClass == "fromHSV")
        return "float hue, float saturation, float brightness, float alpha";

    // --- OTHER USEFUL STATICS ---
    if (methodNameOrClass == "translation")
        return "float x, float y";
    if (methodNameOrClass == "rotation")
        return "float angleInRadians";
    if (methodNameOrClass == "scale")
        return "float scaleFactor";

    // Helper lambda to clean up "(arg1, arg2)" into "arg1, arg2" for the bubble
    auto sanitizeForBubble = [](juce::String p) -> juce::String
    {
        p = p.trim();
        if (p.startsWith("(") && p.endsWith(")"))
            p = p.substring(1, p.length() - 1).trim();

        if (p == "()" || p.isEmpty() || p == "( )")
            return "";
        return p;
    };

    // Helper lambda for Global Search
    auto performGlobalSearch = [&](const juce::String &methodName)
    {
        juce::StringArray results;
        for (juce::HashMap<juce::String, LuaClass>::Iterator it(classes); it.next();)
        {
            const auto &lc = it.getValue();

            // Check Instance, Static, and Constructors
            juce::Array<LuaMethod> allMethods;
            allMethods.addArray(lc.methods);
            allMethods.addArray(lc.staticMethods);

            for (auto &m : allMethods)
            {
                if (m.name == methodName)
                {
                    juce::String cleaned = sanitizeForBubble(m.parameters);
                    if (!results.contains(cleaned))
                        results.add(cleaned);
                }
            }

            // Match Class Name (Constructor)
            if (lc.name == methodName)
            {
                for (auto &c : lc.constructors)
                {
                    juce::String cleaned = sanitizeForBubble(c);
                    if (!results.contains(cleaned))
                        results.add(cleaned);
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

        // --- UPDATED: RECURSIVE INHERITANCE SEARCH ---
        while (currentClass.isNotEmpty() && classes.contains(currentClass))
        {
            const auto &lc = classes.getReference(currentClass);

            // Search Instance Methods - Return the first one found (respects our insert(0) order)
            for (auto &m : lc.methods)
            {
                if (m.name == methodNameOrClass)
                {
                    return sanitizeForBubble(m.parameters);
                }
            }

            // Search Static Methods
            for (auto &m : lc.staticMethods)
            {
                if (m.name == methodNameOrClass)
                {
                    return sanitizeForBubble(m.parameters);
                }
            }

            // Search Constructors (Only relevant if class name matches method name)
            if (methodNameOrClass == lc.name)
            {
                if (lc.constructors.size() > 0)
                {
                    // Return all constructors joined by a newline so the bubble shows overloads
                    return sanitizeForBubble(lc.constructors.joinIntoString("\n"));
                }
            }

            // THE JUMP: Move to the parent class for the next iteration of the while loop
            currentClass = lc.parentClass;
        }

        // 3. FALLBACK TO GLOBAL if specific class search and its parent chain yielded nothing
        return performGlobalSearch(methodNameOrClass);
    }

    return "";
}

juce::String CtrlrLuaMethodAutoCompleteManager::resolveReturnType(const juce::String &className, const juce::String &methodName)
{
    juce::String currentClass = className;

    // --- NEW: RECURSIVE INHERITANCE SEARCH ---
    while (currentClass.isNotEmpty() && classes.contains(currentClass))
    {
        const auto &lc = classes.getReference(currentClass);

        // 1. Context-Specific Overrides (Priority)
        if (currentClass == "CtrlrPanel")
        {
            // Handles getModulatorByName, getModulatorByIndex, and getModulator
            if (methodName.contains("getModulator"))
                return "CtrlrModulator";
            if (methodName == "getComponent")
                return "CtrlrComponent";
        }

        if (currentClass == "CtrlrModulator")
        {
            if (methodName == "getComponent")
                return "CtrlrComponent";
            if (methodName == "getPanel")
                return "CtrlrPanel";
        }

        if (currentClass == "CtrlrComponent")
        {
            if (methodName == "getOwner")
                return "CtrlrModulator";
        }

        // Move to parent to see if it has specific context overrides
        currentClass = lc.parentClass;
    }

    // 2. Global Method Name Mappings (Fallback)
    // These work regardless of the class context
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
        return "Graphics";

    if (methodName == "getGlobalTimer")
        return "Timer";

    if (methodName == "getLastMidiMessage" || methodName == "getMidiMessage")
        return "CtrlrMidiMessage";

    // Note: setBounds returns void in Lua, but keeping your return hint for documentation purposes
    if (methodName == "setBounds")
        return "number x, number y, number width, number height";

    return "";
}

juce::StringArray CtrlrLuaMethodAutoCompleteManager::getClassNames() const
{
    juce::StringArray names;

    // JUCE HashMap iterator usage
    for (juce::HashMap<juce::String, LuaClass>::Iterator it(classes); it.next();)
    {
        names.add(it.getKey());
    }

    return names;
}

juce::String CtrlrLuaMethodAutoCompleteManager::getClassNameForVariable(const juce::String &varName, const juce::String &code)
{
    // 1. Static/Global Shortcuts (Priority)
    if (varName == "panel")
        return "CtrlrPanel";
    if (varName == "mod")
        return "CtrlrModulator";
    if (varName == "comp")
        return "CtrlrComponent";
    if (varName == "g")
        return "Graphics";
    if (varName == "utils")
        return "CtrlrLuaUtils"; // Updated v5.6.35 12.02.26. Fixed: Points to actual class
	// if (varName == "math")
	//     return "math";
	if (varName == "table")
        return "table";
    if (varName == "string")
		return "String"; // upper case for JUCE
						 // if (varName == "MemoryBlock")
						 // 	return "MemoryBlock";
						 // if (varName == "CtrlrMidiMessage")
						 // 	return "CtrlrMidiMessage";
	// if (varName.equalsIgnoreCase("Path"))
	//     return "Path";

	// 2. CHAIN RESOLUTION (e.g., mb:someMethod():)
    if (varName.contains(":") || varName.contains("."))
    {
        juce::String normalized = varName.replace(".", ":");
        juce::StringArray tokens;
        tokens.addTokens(normalized, ":", "");

        juce::String currentType = "";
        for (int i = 0; i < tokens.size(); ++i)
        {
            juce::String segment = tokens[i].upToFirstOccurrenceOf("(", false, false).trim();
            if (i == 0)
                currentType = getClassNameForVariable(segment, code);
            else
                currentType = resolveReturnType(currentType, segment);

            if (currentType.isEmpty())
                break;
        }
        if (currentType.isNotEmpty())
            return currentType;
    }

    // 3. DYNAMIC LOOK-BACK (Assignments: mb = MemoryBlock())
    // We look for the variable name followed by an equals sign, ignoring spaces
    int assignmentPos = code.lastIndexOf(varName + " =");
    if (assignmentPos == -1)
        assignmentPos = code.lastIndexOf(varName + "=");

    if (assignmentPos != -1)
    {
        juce::String rhs = code.substring(assignmentPos + varName.length()).trimStart();
        if (rhs.startsWith("="))
            rhs = rhs.substring(1).trimStart();

        int endOfLine = rhs.indexOfAnyOf(";\n\r");
        if (endOfLine != -1)
            rhs = rhs.substring(0, endOfLine).trim();

        // --- CONSTRUCTOR DETECTION (FIXED) ---
        // Strip everything after the first '(' to handle MemoryBlock(1024)
        juce::String potentialClass = rhs.upToFirstOccurrenceOf("(", false, false).trim();

        // 1. Check classNames case-insensitively
        for (auto &name : classNames)
        {
            if (name.equalsIgnoreCase(potentialClass))
            {
                // Redirection logic: ensure we always return the key that has the methods
                // if (name.equalsIgnoreCase("MemoryBlock") || name.equalsIgnoreCase("LMemoryBlock"))
                //     return "LMemoryBlock";

                return name;
            }
        }

        // 2. Explicit check for manual aliases
        // if (potentialClass.equalsIgnoreCase("MemoryBlock") || potentialClass.equalsIgnoreCase("LMemoryBlock"))
        //     return "LMemoryBlock";

        // 3. RECURSIVE ALIAS/CHAIN RESOLUTION
        // Only run this if we didn't find a direct class match above
        if (rhs != varName && rhs.isNotEmpty())
        {
            // Handle Path.Path() style statics
            if (rhs.contains("."))
            {
                juce::String base = rhs.upToFirstOccurrenceOf(".", false, false).trim();
                for (auto &name : classNames)
                {
                    if (name.equalsIgnoreCase(base))
                        return name;
                }
            }
            return getClassNameForVariable(rhs, code);
        }
    }

    // 4. Static Fallbacks (If no assignment found, guess by name)
    // if (varName == "m" || varName == "mb" || varName == "mem" || varName == "memoryBlock")
    //     return "LMemoryBlock"; // Updated v5.6.35 12.02.26
    // if (varName == "f")
    //     return "File";

    return "";
}
