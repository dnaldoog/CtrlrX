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
                            
                            // --- CLEANED SANITIZATION ---
                            if (args.isEmpty() || args == "()" || args.containsOnly(", ") || args.containsOnly("(, )")) {
                                args = "()";
                            } else {
                                if (!args.startsWith("(")) args = "(" + args;
                                if (!args.endsWith(")")) args = args + ")";
                                
                                // Double check if the content inside brackets is just commas
                                if (args.substring(1, args.length() - 1).trim().containsOnly(", ")) args = "()";
                            }

                            bool alreadyExists = false;
                            for (auto& existing : lc.methods) {
                                if (existing.name == name && existing.parameters == args) {
                                    alreadyExists = true; break;
                                }
                            }

                            if (!alreadyExists) {
                                LuaMethod lm;
								lm.name = name;
								lm.parameters = args;
								lm.isStatic = false;
								
								// If the class is CtrlrLuaUtils, also treat these as static
								// so they show up after a dot (utils.)
								if (lc.name == "CtrlrLuaUtils") {
									lm.isStatic = true;
									lc.staticMethods.add(lm);
								}
								
                                lc.methods.add(lm);
                                
                                // FIX: We no longer manually insert a "()" version here.
                                // The UI now handles displaying the real signature.
								
                                
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
							
							LuaMethod sm;
							sm.name = name;
							sm.parameters = args;
							sm.isStatic = true;
							lc.staticMethods.add(sm);
							
							// IMPORTANT: Also add to methods list so getMethodParams can find them!
							lc.methods.add(sm);
							
							if (name != "string" && name != "math" && name != "table")
								allMethodNames.add(name);
						}
					}
					
					if (auto* cList = classXml->getChildByName("constructors"))
					{
						forEachXmlChildElement(*cList, c) {
							juce::String args = c->getStringAttribute("args").trim();
							if (!args.startsWith("(")) args = "(" + args;
							if (!args.endsWith(")")) args = args + ")";
							
							LuaMethod lm;
							lm.name = lc.name; // Use the class name for the constructor
							lm.parameters = (args == "()" || args.isEmpty()) ? "()" : args;
							lm.isStatic = true;
							
							lc.methods.add(lm);
							lc.staticMethods.add(lm); // Add to static so it shows after a dot
						}
					}
					
					classes.set(lc.name, lc);
                    if (cppName.isNotEmpty() && cppName != lc.name) classes.set(cppName, lc);
                    classNames.add(lc.name);
                }
            }
        }
    }

    // 2. Manual Injection of Base Classes
    if (!classes.contains("MemoryBlock")) {
        LuaClass mbClass; mbClass.name = "MemoryBlock";
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
            // FIX: Removed duplicate empty () injection that was overriding the real signatures.
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
            // FIX: Removed duplicate empty () injection.
            allMethodNames.add(n);
        }
        classes.set(obj.name, obj);
        classNames.add(obj.name);
    }
	
	if (!classes.contains("CtrlrMidiMessage")) {
        LuaClass mid;
        mid.name = "CtrlrMidiMessage";
        
        // Add the three constructors for the bubble suggestions
        mid.constructors.add("(String hexData)");
        mid.constructors.add("({byteTable})");
        mid.constructors.add("(MemoryBlock data)");
        
        // Basic common methods for MIDI objects
        juce::StringArray methods = { "getSize", "getData", "getType", "getChannel", "getTimestamp", "setTimestamp" };
        for (auto& n : methods) {
            LuaMethod lm; lm.name = n; lm.parameters = "()";
            mid.methods.add(lm);
            allMethodNames.add(n);
        }

        classes.set("CtrlrMidiMessage", mid);
        classNames.add("CtrlrMidiMessage");
	}
	
	if (!classes.contains("Justification")) {
		LuaClass j; j.name = "Justification";
		juce::StringArray enums = { "left", "right", "horizontallyCentred", "top", "bottom", "verticallyCentred", "centred" };
		
		for (auto& e : enums) {
			LuaMethod m;
			m.name = e;
			m.parameters = ""; // No brackets for enums
			m.isStatic = true;
			
			j.staticMethods.add(m); // This is for Justification.left
			j.methods.add(m);       // Backup for the search logic
		}
		
		// Add the constructor so Justification() works
		j.constructors.add("()");
		
		classes.set("Justification", j);
		classNames.add("Justification");
	}
	
	if (!classes.contains("Path")) {
		LuaClass pc; pc.name = "Path";
		
		// 1. Add the specific method he was missing with its full parameters
		LuaMethod m;
		m.name = "addQuadrilateral";
		m.parameters = "(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)";
		m.isStatic = false;
		pc.methods.add(m);
		
		// 2. Add other common Path methods so the : autocomplete is useful
		juce::StringArray pathMethods = { "startNewSubPath", "lineTo", "quadraticTo", "closeSubPath", "getBounds" };
		for (auto& name : pathMethods) {
			LuaMethod pm; pm.name = name; pm.parameters = "()"; // Or add real params if known
			pc.methods.add(pm);
		}
		
		classes.set("Path", pc);
		classNames.add("Path");
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
            // --- FIX: FORCE OVERRIDE ---
            // Explicitly remove any existing versions (from XML) first.
            // This prevents the empty-param version from blocking our high-quality fix.
            for (int i = panel.methods.size(); --i >= 0;) {
                if (panel.methods.getReference(i).name == mName) {
                    panel.methods.remove(i);
                }
            }
            
            // Now add the correct version with parameters
            LuaMethod lm;
            lm.name = mName;
            
            if (mName == "sendMidiMessageNow") {
                // These names appear inside the bubble when the user opens the bracket '('
                lm.parameters = "(MidiMessage) | (hexString) | ({table})"; // If the bubble is just single line
            }
            else if (mName.contains("ByName")) {
                lm.parameters = "(String name)";
            }
            else if (mName.contains("ByIndex")) {
                lm.parameters = "(int index)";
            }
            else if (mName == "setGlobalVariable") {
                lm.parameters = "(int index, int value)";
            }
            else {
                lm.parameters = "()";
            }
            panel.methods.add(lm);
        }
    }
    
    if (classes.contains("CtrlrMidiMessage")) {
        auto& midiClass = classes.getReference("CtrlrMidiMessage");
        // Clear old constructors if any
        midiClass.constructors.clear();
        
        // Add the new overloads
        midiClass.constructors.add("(String hexData)");
        midiClass.constructors.add("({table bytes})");
        midiClass.constructors.add("(MemoryBlock data)");
    }

    // 4. Detailed Library Definitions
	if (true) {
		// We use "luaString" internally as a key to avoid JUCE String conflicts
		// but we will alias it to "string"
		LuaClass strLib;
		strLib.name = "string"; // The name remains lowercase
		
		juce::StringArray strMethods = {
			"byte(s, i, j)", "char(...)", "dump(function)", "find(s, pattern, init, plain)",
			"format(formatstring, ...)", "gmatch(s, pattern)", "gsub(s, pattern, repl, n)",
			"len(s)", "lower(s)", "match(s, pattern, init)", "rep(s, n)",
			"reverse(s)", "sub(s, i, j)", "upper(s)"
		};
		
		for (auto& mEntry : strMethods) {
			LuaMethod lm;
			lm.name = mEntry.upToFirstOccurrenceOf("(", false, false).trim();
			lm.parameters = mEntry.substring(mEntry.indexOf("(")).trim();
			lm.isStatic = true;
			strLib.staticMethods.add(lm);
			strLib.methods.add(lm); // Keep both for safety
		}
		
		// FORCE both keys to point to our Lua library
		classes.set("string", strLib);
		classes.set("String", strLib);
		
		if (!classNames.contains("string")) classNames.add("string");
	}
	
	if (!classes.contains("math")) {
		LuaClass mathLib;
		mathLib.name = "math";
		juce::StringArray mathMethods = {
			"abs(x)", "acos(x)", "asin(x)", "atan(x)", "ceil(x)", "cos(x)",
			"deg(x)", "exp(x)", "floor(x)", "fmod(x, y)", "huge", "log(x)",
			"max(x, ...)", "min(x, ...)", "modf(x)", "pi", "pow(x, y)",
			"rad(x)", "random(m, n)", "randomseed(x)", "sin(x)", "sqrt(x)", "tan(x)"
		};
		
		for (auto& mEntry : mathMethods) {
			if (mEntry == "pi" || mEntry == "huge") {
				// Add directly to the local mathLib object we are building
				mathLib.properties.add(mEntry);
				continue;
			}
			
			LuaMethod lm;
			if (mEntry.contains("(")) {
				lm.name = mEntry.upToFirstOccurrenceOf("(", false, false).trim();
				// This captures from the first '(' to the end, e.g., "(x, y)"
				lm.parameters = mEntry.substring(mEntry.indexOf("(")).trim();
			} else {
				lm.name = mEntry.trim();
				lm.parameters = "()";
			}
			
			lm.isStatic = true;
			mathLib.staticMethods.add(lm);
			mathLib.methods.add(lm);
		}
		
		// Now store the completed class in the map
		classes.set("math", mathLib);
		if (!classNames.contains("math")) classNames.add("math");
	}
	
	if (classes.contains("LMemoryBlock") && !classes.contains("MemoryBlock")) {
		classes.set("MemoryBlock", classes["LMemoryBlock"]);
	}
	
	// Force CtrlrLuaUtils instance methods to act as statics so 'utils.' works
	if (classes.contains("CtrlrLuaUtils")) {
		auto& utilsClass = classes.getReference("CtrlrLuaUtils");
		if (utilsClass.staticMethods.isEmpty()) {
			for (auto& m : utilsClass.methods) {
				utilsClass.staticMethods.add(m);
			}
		}
	}

    // 5. Inject Common Libs (Final pass to ensure presence)
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
    // Note: math and string already handled in detail above, but this catch-all is safe.

    // 6. MemoryBlock specialized fixes
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
    
    // 1. MANDATORY LIBRARIES / CLASSES (The "C" Icon)
    juce::StringArray libraries = { "math", "table", "string", "utils", "MemoryBlock" };
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

    // 3. CLASSES (Version 44 Dual-Suggestion Logic)
	for (auto& c : classNames)
	{
		if (c.startsWithIgnoreCase(prefix))
		{
			// Get the specific class data
			const auto& lc = classes.getReference(c);
			
			// SUGGESTION 1: THE CONSTRUCTOR (Icon "C")
			// If the class has any constructors defined in XML
			if (!lc.constructors.isEmpty())
			{
				results.push_back({ c + "()", TypeClass }); // e.g., "CtrlrMidiMessage()"
			}
			
			// SUGGESTION 2: THE STATIC ACCESS (Icon "S")
			// If the class has static methods (like fromHex), suggest the "." access
			if (!lc.staticMethods.isEmpty())
			{
				results.push_back({ c + ".", TypeStatic }); // e.g., "CtrlrMidiMessage."
			}
			
			added.add(c.toLowerCase());
		}
	}

    // 4. METHODS (The "M" Icon)
    for (auto& m : allMethodNames) {
        if (m.startsWithIgnoreCase(prefix)) {
            juce::String lowerM = m.toLowerCase();
            
            // Skip if we already handled this as a Class/Static entry above
            if (libraries.contains(lowerM) || added.contains(lowerM)) continue;

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
    juce::StringArray addedExactSignatures;
    juce::String currentClass = className;

    _DBG("--- START SEARCH: [" + prefix + "] in Class: [" + className + "] (Static: " + (includeStatic ? "YES" : "NO") + ") ---");

    while (currentClass.isNotEmpty())
    {
        juce::String matchedKey = "";
        for (auto& name : classNames) {
            if (name.equalsIgnoreCase(currentClass)) {
                matchedKey = name;
                break;
            }
        }
        
        if (matchedKey.isEmpty() || !classes.contains(matchedKey)) {
            _DBG("    ! Class [" + currentClass + "] not found in definitions.");
            break;
        }
        
        const auto& lc = classes.getReference(matchedKey);
        int classMatches = 0;
		
		// 1. Static Methods & Constructors (Triggered by '.')
		if (includeStatic) {
			_DBG("        Checking Static Methods for [" + matchedKey + "]:");
			
			for (auto& m : lc.staticMethods) {
				juce::String fullSignature = m.name + " " + m.parameters;
				bool match = (prefix.isEmpty() || m.name.startsWithIgnoreCase(prefix));
				bool unique = !addedExactSignatures.contains(fullSignature);
				
				if (match && unique) {
					_DBG("          + Found Static: " + m.name);
					suggestions.push_back({ fullSignature, TypeStatic });
					addedExactSignatures.add(fullSignature);
					classMatches++;
				} else if (!match) {
					_DBG("          - Skipping " + m.name + " (doesn't match prefix '" + prefix + "')");
				}
			}
			
			// Include Constructors in the static list for class-level access
			for (auto& cParams : lc.constructors) {
				juce::String fullSignature = lc.name + " " + cParams;
				bool match = (prefix.isEmpty() || lc.name.startsWithIgnoreCase(prefix));
				bool unique = !addedExactSignatures.contains(fullSignature);
				
				if (match && unique) {
					_DBG("          + Found Constructor: " + lc.name);
					suggestions.push_back({ fullSignature, TypeMethod });
					addedExactSignatures.add(fullSignature);
					classMatches++;
				}
			}
		}

        // 2. Instance Methods (Triggered by ':')
        if (includeInstance) {
            for (auto& m : lc.methods) {
                juce::String fullSignature = m.name + " " + m.parameters;
                if ((prefix.isEmpty() || m.name.startsWithIgnoreCase(prefix)) && !addedExactSignatures.contains(fullSignature)) {
                    suggestions.push_back({ fullSignature, TypeMethod });
                    addedExactSignatures.add(fullSignature);
                    classMatches++;
                }
            }
        }

        // 3. Properties
		if (includeProperties) {
			for (auto& prop : lc.properties) {
				if (prefix.isEmpty() || prop.startsWithIgnoreCase(prefix)) {
					SuggestionItem item;
					item.text = prop;
					item.type = TypeProperty; // This triggers your "P" icon logic
					suggestions.push_back(item);
				}
			}
		}

        _DBG("    Found " + juce::String(classMatches) + " matches in [" + matchedKey + "]");
        
        // Move to parent
        currentClass = lc.parentClass;
        
        // Safety: If the class is its own parent, stop (prevents infinite loop)
        if (currentClass == lc.name) break;
    }

    _DBG("--- SEARCH COMPLETE: Total " + juce::String(suggestions.size()) + " matches ---");
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
	
	// --- NEW COLOUR STATICS ---
	if (methodNameOrClass == "fromRGBA") return "int red, int green, int blue, int alpha";
	if (methodNameOrClass == "fromRGB") return "int red, int green, int blue";
	if (methodNameOrClass == "fromFloatRGBA") return "float red, float green, float blue, float alpha";
	if (methodNameOrClass == "greyLevel") return "float brightness";
	if (methodNameOrClass == "fromHSV") return "float hue, float saturation, float brightness, float alpha";
	
	// --- OTHER USEFUL STATICS ---
	if (methodNameOrClass == "translation") return "float x, float y";
	if (methodNameOrClass == "rotation") return "float angleInRadians";
	if (methodNameOrClass == "scale") return "float scaleFactor";
	
	// Helper lambda to clean up "(arg1, arg2)" into "arg1, arg2" for the bubble
	auto sanitizeForBubble = [](juce::String p) -> juce::String {
		p = p.trim();
		if (p.startsWith("(") && p.endsWith(")"))
			p = p.substring(1, p.length() - 1).trim();
		
		if (p == "()" || p.isEmpty() || p == "( )") return "";
		return p;
	};
	
	// Helper lambda for Global Search
	auto performGlobalSearch = [&](const juce::String& methodName) {
		juce::StringArray results;
		for (juce::HashMap<juce::String, LuaClass>::Iterator it (classes); it.next();) {
			const auto& lc = it.getValue();
			
			// Check Instance, Static, and Constructors
			juce::Array<LuaMethod> allMethods;
			allMethods.addArray(lc.methods);
			allMethods.addArray(lc.staticMethods);
			
			for (auto& m : allMethods) {
				if (m.name == methodName) {
					juce::String cleaned = sanitizeForBubble(m.parameters);
					if (!results.contains(cleaned)) results.add(cleaned);
				}
			}
			
			// Match Class Name (Constructor)
			if (lc.name == methodName) {
				for (auto& c : lc.constructors) {
					juce::String cleaned = sanitizeForBubble(c);
					if (!results.contains(cleaned)) results.add(cleaned);
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

            // Search Constructors (Only relevant if class name matches method name)
            if (methodNameOrClass == lc.name) {
                if (lc.constructors.size() > 0) {
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

juce::String CtrlrLuaMethodAutoCompleteManager::resolveReturnType(const juce::String& className, const juce::String& methodName)
{
    juce::String currentClass = className;

    // --- NEW: RECURSIVE INHERITANCE SEARCH ---
    while (currentClass.isNotEmpty() && classes.contains(currentClass))
    {
        const auto& lc = classes.getReference(currentClass);

        // 1. Context-Specific Overrides (Priority)
        if (currentClass == "CtrlrPanel")
        {
            // Handles getModulatorByName, getModulatorByIndex, and getModulator
            if (methodName.contains("getModulator")) return "CtrlrModulator";
            if (methodName == "getComponent") return "CtrlrComponent";
        }
        
        if (currentClass == "CtrlrModulator")
        {
            if (methodName == "getComponent") return "CtrlrComponent";
            if (methodName == "getPanel") return "CtrlrPanel";
        }

        if (currentClass == "CtrlrComponent")
        {
            if (methodName == "getOwner") return "CtrlrModulator";
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
    for (juce::HashMap<juce::String, LuaClass>::Iterator it (classes); it.next();)
    {
        names.add (it.getKey());
    }

    return names;
}

juce::String CtrlrLuaMethodAutoCompleteManager::getClassNameForVariable(const juce::String& varName, const juce::String& code)
{
    // 1. Static/Global Shortcuts (Priority)
    if (varName == "panel")                      return "CtrlrPanel";
    if (varName == "mod")                         return "CtrlrModulator";
    if (varName == "comp")                        return "CtrlrComponent";
    if (varName == "g")                           return "Graphics";
	if (varName == "utils")                       return "CtrlrLuaUtils"; // Updated v5.6.35 12.02.26. Fixed: Points to actual class
    if (varName == "math")                        return "math";
    if (varName == "table")                       return "table";
    if (varName == "string")                      return "string";
	if (varName == "MemoryBlock" || varName == "memoryBlock" || varName == "mb") return "LMemoryBlock"; //  // Updated v5.6.35 12.02.26. Redirect everything to the one with the methods
    if (varName == "CtrlrMidiMessage") return "CtrlrMidiMessage";
    
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
            
            if (currentType.isEmpty()) break;
        }
        if (currentType.isNotEmpty()) return currentType;
    }

    // 3. DYNAMIC LOOK-BACK (Assignments: mb = MemoryBlock())
    // We look for the variable name followed by an equals sign, ignoring spaces
    int assignmentPos = code.lastIndexOf(varName + " =");
    if (assignmentPos == -1) assignmentPos = code.lastIndexOf(varName + "=");

    if (assignmentPos != -1)
    {
        juce::String rhs = code.substring(assignmentPos + varName.length()).trimStart();
        if (rhs.startsWith("=")) rhs = rhs.substring(1).trimStart();
        
        int endOfLine = rhs.indexOfAnyOf(";\n\r");
        if (endOfLine != -1) rhs = rhs.substring(0, endOfLine).trim();

        // --- CONSTRUCTOR DETECTION (FIXED) ---
        // Strip everything after the first '(' to handle MemoryBlock(1024)
        juce::String potentialClass = rhs.upToFirstOccurrenceOf("(", false, false).trim();
        
        // Check if the RHS is a known class name directly
		if (classNames.contains(potentialClass) || potentialClass == "MemoryBlock" || potentialClass == "LMemoryBlock") // Updated v5.6.35 12.02.26
        {
            return potentialClass;
        }

        // --- RECURSIVE ALIAS/CHAIN RESOLUTION ---
        if (rhs != varName && rhs.isNotEmpty())
        {
            return getClassNameForVariable(rhs, code);
        }
    }
    
    // 4. Static Fallbacks (If no assignment found, guess by name)
	if (varName == "m" || varName == "mb" || varName == "mem" || varName == "memoryBlock") return "LMemoryBlock"; // Updated v5.6.35 12.02.26
    if (varName == "f") return "File";
    
    return "";
}
