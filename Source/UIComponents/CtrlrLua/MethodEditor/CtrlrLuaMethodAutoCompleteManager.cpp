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
    classes.clear();
    classNames.clear();
    allMethodNames.clear();

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

                    // Process Instance Methods
                    if (auto* mList = classXml->getChildByName("methods"))
                    {
                        forEachXmlChildElement(*mList, methodXml)
                        {
                            LuaMethod lm;
                            lm.name = methodXml->getStringAttribute("name");
                            lm.parameters = methodXml->getStringAttribute("args");
                            lm.isStatic = false;

                            lc.methods.add(lm);
                            allMethodNames.addIfNotAlreadyThere(lm.name);
                        }
                    }
                    // Process Enums (Add them as static methods so they show up with .)
                    if (auto* eList = classXml->getChildByName("enums"))
                    {
                        forEachXmlChildElement(*eList, enumGroupXml)
                        {
                            forEachXmlChildElement(*enumGroupXml, valueXml)
                            {
                                LuaMethod em;
                                em.name = valueXml->getStringAttribute("name");
                                em.parameters = ""; // Enums have no params
                                em.isStatic = true; // Essential for dot-lookup

                                lc.methods.add(em);
                                allMethodNames.addIfNotAlreadyThere(em.name);
                            }
                        }
                    }
                    // Process Static Methods
                    if (auto* sList = classXml->getChildByName("static_methods"))
                    {
                        forEachXmlChildElement(*sList, staticXml)
                        {
                            LuaMethod sm;
                            sm.name = staticXml->getStringAttribute("name");
                            sm.parameters = staticXml->getStringAttribute("args");
                            sm.isStatic = true;

                            lc.methods.add(sm);
                            allMethodNames.addIfNotAlreadyThere(sm.name);
                        }
                    }

                    classes.set(lc.name, lc);
                    classNames.add(lc.name);
                }
            }
        }
    }
}

std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getGlobalSuggestions(const juce::String& prefix)
{
    std::vector<SuggestionItem> results;

    juce::StringArray globals = { "panel", "mod", "comp", "utils", "midi" };
    for (auto& g : globals) {
        if (g.startsWithIgnoreCase(prefix))
            results.push_back({ g, TypeGlobal });
    }

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
    juce::String currentClass = className;
    juce::StringArray addedNames;

    while (currentClass.isNotEmpty())
    {
        if (!classes.contains(currentClass)) break;

        const auto& lc = classes.getReference(currentClass);

        for (const auto& m : lc.methods)
        {
            if (addedNames.contains(m.name)) continue;
            if (prefix.isNotEmpty() && !m.name.startsWithIgnoreCase(prefix)) continue;

            bool match = false;
            SuggestionType st = TypeMethod; // Default to 'M'

            if (type == LookupInstance && !m.isStatic)
            {
                match = true;
                st = TypeMethod; // Icon 'M'
            }
            else if (type == LookupStatic && m.isStatic)
            {
                match = true;

                // --- Logic to differentiate Static Method vs Enum ---
                // In our XML, Enums have no arguments, whereas static methods 
                // usually have "()" or "(args)".
                if (m.parameters.isEmpty() || m.parameters == "()")
                    st = TypeEnum;          // Icon 'E'
                else
                    st = TypeStaticMethod;  // Icon 'S'
            }

            if (match)
            {
                results.push_back({ m.name, st });
                addedNames.add(m.name);
            }
        }

        currentClass = lc.parentClass;
    }
    return results;
}

juce::String CtrlrLuaMethodAutoCompleteManager::getMethodParams(const juce::String& className, const juce::String& methodName)
{
    // Search through inheritance tree
    juce::String currentClass = className;

    while (currentClass.isNotEmpty() && classes.contains(currentClass))
    {
        const auto& lc = classes.getReference(currentClass);

        for (const auto& m : lc.methods)
        {
            if (m.name == methodName)
            {
                juce::String rawParams = m.parameters;

                // NEW: Simple format from XML: "(int,bool,string)" or empty
                if (rawParams.isNotEmpty())
                {
                    // Strip parentheses if present
                    if (rawParams.startsWithChar('(') && rawParams.endsWithChar(')'))
                    {
                        return rawParams.substring(1, rawParams.length() - 1);
                    }
                    // Return as-is if already clean
                    return rawParams;
                }

                // Fallback: empty, return empty string
                return "";
            }
        }
        currentClass = lc.parentClass;
    }

    return "";
}

juce::String CtrlrLuaMethodAutoCompleteManager::resolveClass(const juce::String& symbol, const juce::String& fullDocumentText)
{
    if (classes.contains(symbol)) return symbol;
    // Now hard coded in xml file
    //if (symbol == "panel") return "panel";
    //if (symbol == "mod")   return "mod";
    //if (symbol == "comp")  return "comp";
    //if (symbol == "utils") return "utils";

    int assignPos = fullDocumentText.lastIndexOf(symbol + "=");
    if (assignPos == -1) assignPos = fullDocumentText.lastIndexOf(symbol + " =");

    if (assignPos != -1)
    {
        // Skip past the symbol and "=" to get to the class name
        int startOfClass = assignPos + symbol.length() + 1;  // Position right after "m="

        // Skip whitespace
        while (startOfClass < fullDocumentText.length() &&
            juce::CharacterFunctions::isWhitespace(fullDocumentText[startOfClass]))
            startOfClass++;

        int endOfClass = startOfClass;
        while (endOfClass < fullDocumentText.length() &&
            (juce::CharacterFunctions::isLetterOrDigit(fullDocumentText[endOfClass]) || fullDocumentText[endOfClass] == '_'))
            endOfClass++;

        juce::String foundClass = fullDocumentText.substring(startOfClass, endOfClass);
        if (classes.contains(foundClass)) return foundClass;
    }
    return "";
}

juce::String CtrlrLuaMethodAutoCompleteManager::getClassNameForVariable(const juce::String& varName, const juce::String& code)
{
    const juce::String v = varName.trim();
    if (v.isEmpty()) return "";

    // 1. DIRECT CLASS HIT
    // If user types 'Justification' or 'CtrlrModulator', return it immediately
    if (classNames.contains(v)) return v;

    // 2. SHORTHAND TRANSLATION
    // Maps your Lua convenience variables to the actual XML class keys
    if (v == "panel" || v == "pan") return "CtrlrPanel";
    if (v == "mod")                 return "CtrlrModulator";
    if (v == "comp")                return "CtrlrComponent";
    if (v == "g")                   return "Graphics";
    if (v == "utils")               return "CtrlrLuaUtils";
    if (v == "math" || v == "table" || v == "string") return v;

    // 3. CHAIN RESOLUTION (e.g., panel:getModulatorByName("x"): )
    if (v.contains(":") || v.contains("."))
    {
        juce::StringArray tokens;
        // Replace dots with colons so we can split everything at once
        tokens.addTokens(v.replace(".", ":"), ":", "");

        juce::String currentType = "";

        for (int i = 0; i < tokens.size(); ++i)
        {
            // Clean the segment: remove whitespace and everything from "(" onwards
            juce::String segment = tokens[i].upToFirstOccurrenceOf("(", false, false).trim();
            if (segment.isEmpty()) break;

            if (i == 0) {
                // Recursively find the start of the chain
                currentType = getClassNameForVariable(segment, code);
            }
            else {
                // Follow the return types defined in your resolveReturnType function
                currentType = resolveReturnType(currentType, segment);
            }

            if (currentType.isEmpty()) break;
        }

        if (currentType.isNotEmpty()) return currentType;
    }

    // 4. DYNAMIC ASSIGNMENT LOOK-BACK (e.g., myMod = panel:getModulator("x"))
    // Look for where this variable was last assigned in the code
    int assignPos = code.lastIndexOf(v + "=");
    if (assignPos == -1) assignPos = code.lastIndexOf(v + " =");

    if (assignPos != -1)
    {
        juce::String rhs = code.substring(assignPos + v.length()).trimStart();
        if (rhs.startsWith("=")) rhs = rhs.substring(1).trimStart();

        // Extract the expression on the right side of the equals sign
        int endOfLine = rhs.indexOfAnyOf(";\n\r");
        if (endOfLine != -1) rhs = rhs.substring(0, endOfLine).trim();

        // If it's a constructor: myMod = CtrlrModulator()
        juce::String potentialClass = rhs.upToFirstOccurrenceOf("(", false, false).trim();
        if (classNames.contains(potentialClass)) return potentialClass;

        // If it's a result of another call: myMod = otherVar:getSomething()
        if (rhs != v && rhs.isNotEmpty()) return getClassNameForVariable(rhs, code);
    }

    // 5. HARDCODED FALLBACKS
    if (v == "m" || v == "mb" || v == "mem" || v == "MemoryBlock") return "MemoryBlock";
    if (v == "f" || v == "File") return "File";

    return "";
}

juce::String CtrlrLuaMethodAutoCompleteManager::resolveReturnType(const juce::String& className, const juce::String& methodName)
{
    // Map internal names to our searchable aliases
    auto mapToAlias = [](const juce::String& name) -> juce::String {
        if (name.contains("Modulator")) return "mod";
        if (name.contains("Component")) return "comp";
        if (name.contains("Panel"))     return "panel";
        return name;
        };

    // 1. Context-Specific
    if (className == "panel" || className == "mod")
    {
        // If it looks like it returns a modulator, tell the manager it's a 'mod'
        if (methodName.contains("getModulator")) return "mod";
        if (methodName.contains("getComponent")) return "comp";
    }

    // 2. Fallback: If we can't find a specific rule, check the method name itself
    return mapToAlias(methodName);
}