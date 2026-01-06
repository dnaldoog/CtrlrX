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
            if (type == LookupInstance && !m.isStatic) match = true;
            if (type == LookupStatic && m.isStatic) match = true;

            if (match)
            {
                results.push_back({ m.name, TypeMethod });
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
    // 1. Static/Global Shortcuts (Priority)
    if (varName == "panel" || varName == "pan")   return "panel";
    if (varName == "mod")                         return "mod";
    if (varName == "comp")                        return "comp";
    if (varName == "g")                           return "g";
    if (varName == "utils")                       return "utils";
    if (varName == "math")                        return "math";
    if (varName == "table")                       return "table";
    if (varName == "string")                      return "string";
    if (varName == "MemoryBlock")                 return "MemoryBlock";

    // 2. CHAIN RESOLUTION (e.g., panel:getModulatorByName():)
    if (varName.contains(":") || varName.contains("."))
    {
        DBG("=== CHAIN RESOLUTION ===");
        DBG("varName: " + varName);

        juce::String normalized = varName.replace(".", ":");
        juce::StringArray tokens;
        tokens.addTokens(normalized, ":", "");

        DBG("tokens count: " + juce::String(tokens.size()));

        juce::String currentType = "";
        for (int i = 0; i < tokens.size(); ++i)
        {
            // Extract just the method name (remove parentheses and everything after)
            juce::String segment = tokens[i].trim();
            int parenPos = segment.indexOf("(");
            if (parenPos > 0)
            {
                segment = segment.substring(0, parenPos);
            }

            DBG("  [" + juce::String(i) + "] segment: " + segment);

            if (i == 0)
            {
                currentType = getClassNameForVariable(segment, code);
                DBG("      initial type: " + currentType);
            }
            else
            {
                juce::String returnType = resolveReturnType(currentType, segment);
                DBG("      resolveReturnType(\"" + currentType + "\", \"" + segment + "\") = " + returnType);
                currentType = returnType;
            }

            if (currentType.isEmpty())
            {
                DBG("      EMPTY! Breaking.");
                break;
            }
        }

        DBG("final type: " + (currentType.isEmpty() ? "EMPTY" : currentType));

        if (currentType.isNotEmpty()) return currentType;
    }

    // 3. DYNAMIC LOOK-BACK (Assignments: mb = MemoryBlock())
    int assignmentPos = code.lastIndexOf(varName + " =");
    if (assignmentPos == -1) assignmentPos = code.lastIndexOf(varName + "=");

    if (assignmentPos != -1)
    {
        juce::String rhs = code.substring(assignmentPos + varName.length()).trimStart();
        if (rhs.startsWith("=")) rhs = rhs.substring(1).trimStart();

        int endOfLine = rhs.indexOfAnyOf(";\n\r");
        if (endOfLine != -1) rhs = rhs.substring(0, endOfLine).trim();

        juce::String potentialClass = rhs.upToFirstOccurrenceOf("(", false, false).trim();

        if (classNames.contains(potentialClass) || potentialClass == "MemoryBlock")
        {
            return potentialClass;
        }

        if (rhs != varName && rhs.isNotEmpty())
        {
            return getClassNameForVariable(rhs, code);
        }
    }

    // 4. Static Fallbacks 
    if (varName == "m" || varName == "mb" || varName == "mem") return "MemoryBlock";
    if (varName == "f") return "File";

    return "";
}
juce::String CtrlrLuaMethodAutoCompleteManager::resolveReturnType(const juce::String& className, const juce::String& methodName)
{
    // Helper lambda: Map cpp class names to their aliases (what's actually in the classes map)
    auto mapToAlias = [](const juce::String& cppName) -> juce::String
        {
            if (cppName == "CtrlrModulator")    return "mod";
            if (cppName == "CtrlrComponent")    return "comp";
            if (cppName == "CtrlrPanel")        return "panel";
            if (cppName == "Graphics")          return "g";
            if (cppName == "MouseEvent")        return "event";
            return cppName;  // Return as-is if no mapping
        };

    // 1. Context-Specific Return Type Mappings
    // These take priority and are context-aware
    if (className == "panel" || className == "CtrlrPanel")
    {
        if (methodName.contains("getModulator")) return "mod";
        if (methodName == "getComponent") return "comp";
    }

    if (className == "mod" || className == "CtrlrModulator")
    {
        if (methodName == "getComponent") return "comp";
        if (methodName == "getPanel") return "panel";
    }

    if (className == "comp" || className == "CtrlrComponent")
    {
        if (methodName == "getOwner") return "mod";
    }

    // 2. Global Method Name Mappings (Fallback)
    // These work regardless of context, returns are mapped to aliases
    juce::String returnType = "";

    if (methodName == "getModulatorByName" || methodName == "getModulator" || methodName == "getModulatorByIndex")
        returnType = "CtrlrModulator";
    else if (methodName == "getComponent" || methodName == "getOwner" || methodName == "getControl")
        returnType = "CtrlrComponent";
    else if (methodName == "getPanel" || methodName == "getOwnerPanel")
        returnType = "CtrlrPanel";
    else if (methodName == "getMemoryBlock" || methodName == "getData")
        returnType = "MemoryBlock";
    else if (methodName == "getLuaManager")
        returnType = "CtrlrLuaManager";
    else if (methodName == "getCanvas")
        returnType = "Graphics";
    else if (methodName == "getGlobalTimer")
        returnType = "Timer";

    // Map the return type to its alias before returning
    if (returnType.isNotEmpty())
    {
        return mapToAlias(returnType);
    }

    return "";
}
