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
            forEachXmlChildElementWithTagName(*root, classXml, "class")
            {
                LuaClass lc;
                lc.name = classXml->getStringAttribute("name");
                lc.parentClass = classXml->getStringAttribute("inherits");

                // 1. Process Instance Methods
                if (auto* mList = classXml->getChildByName("methods"))
                {
                    forEachXmlChildElementWithTagName(*mList, methodXml, "method")
                    {
                        LuaMethod lm;
                        lm.name = methodXml->getStringAttribute("name");
                        lm.parameters = methodXml->getStringAttribute("args");
                        lm.isStatic = false;
                        lc.methods.add(lm);
                        allMethodNames.addIfNotAlreadyThere(lm.name);
                    }
                }

                // 2. Process Static Methods
                if (auto* sList = classXml->getChildByName("static_methods"))
                {
                    forEachXmlChildElementWithTagName(*sList, staticXml, "method")
                    {
                        LuaMethod sm;
                        sm.name = staticXml->getStringAttribute("name");
                        sm.parameters = staticXml->getStringAttribute("args");
                        sm.isStatic = true;
                        lc.methods.add(sm);
                        allMethodNames.addIfNotAlreadyThere(sm.name);
                    }
                }
// 3. Process Enums (Flattened)
if (auto* eList = classXml->getChildByName("enums"))
{
    // Handle the new flattened structure (values direct children of enums)
    forEachXmlChildElementWithTagName(*eList, valXml, "value")
    {
        LuaMethod em;
        em.name = valXml->getStringAttribute("name");
        em.parameters = ""; 
        em.isStatic = true; // MUST BE TRUE for '.' to work
        lc.methods.add(em);
        allMethodNames.addIfNotAlreadyThere(em.name);
    }
    // Keep the old nested loop for backward compatibility if needed
                    forEachXmlChildElementWithTagName(*eList, enumGroup, "enum")
                    {
                        juce::String prefix = enumGroup->getStringAttribute("name") + ".";
                        forEachXmlChildElementWithTagName(*enumGroup, valXml, "value")
                        {
                            LuaMethod em;
                            em.name = prefix + valXml->getStringAttribute("name");
                            em.parameters = "";
                            em.isStatic = true;
                            lc.methods.add(em);
                            allMethodNames.addIfNotAlreadyThere(em.name);
                        }
                    }
                }

                classes.set(lc.name, lc);
                classNames.add(lc.name);
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
std::vector<SuggestionItem> CtrlrLuaMethodAutoCompleteManager::getMethodSuggestions(const juce::String& className, const juce::String& partial, LookupType type)
{
    std::vector<SuggestionItem> results;
    
    // 1. Handle Class-level lookups (Enums/Static)
    // If we are looking at "Justification.", we want the constants, not methods.
    if (type == LookupStatic) {
        // Look for child constants/enums in your class map
        // results = getStaticMembers(className, partial); 
    }

    // 2. Fetch methods from your definitions
    // Ensure className is normalized (e.g., "mod" -> "CtrlrModulator")
    juce::String realClass = className;
    if (className == "panel") realClass = "CtrlrPanel";
    if (className == "mod")   realClass = "CtrlrModulator";

    // ... Your existing loop that populates results ...
    // IMPORTANT: Ensure you set the Type correctly:
    // item.type = TypeMethod; or item.type = TypeClass;
    
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
juce::String CtrlrLuaMethodAutoCompleteManager::resolveReturnType(const juce::String& className, const juce::String& methodName)
{
    // 1. Normalize shortcuts
    juce::String c = className;
    if (c == "panel") c = "CtrlrPanel";
    if (c == "mod")   c = "CtrlrModulator";

    // 2. Hardcoded logic for known Ctrlr returns
    if (methodName.contains("getModulator")) return "mod";
    if (methodName.contains("getComponent")) return "comp";
    if (methodName == "getPanel")            return "panel";
    if (methodName == "getMidiMessage")      return "CtrlrMidiMessage";
    
    // 3. Fallback: If you have an XML database of methods, look up the return type here
    return ""; 
}
juce::String CtrlrLuaMethodAutoCompleteManager::getClassNameForVariable(const juce::String& varName, const juce::String& code)
{
    juce::String v = varName.trim();
    if (v.isEmpty()) return "";

    // 0. DIRECT CLASS HIT (e.g. "Justification" or "MemoryBlock")
    if (classes.contains(v)) return v;

    // 1. STATIC/GLOBAL SHORTCUTS
    if (v == "panel" || v == "pan") return "panel";
    if (v == "mod")                return "mod";
    if (v == "comp")               return "comp";
    if (v == "g")                  return "g";
    if (v == "utils")              return "utils";

    // 2. CHAIN RESOLUTION (e.g., panel:getModulatorByName("test"): )
    if (v.contains(":") || v.contains("."))
    {
        juce::String normalized = v.replace(".", ":");
        juce::StringArray tokens;
        tokens.addTokens(normalized, ":", "");

        juce::String currentType = "";
        for (int i = 0; i < tokens.size(); ++i)
        {
            // Extract method name, ignoring arguments/brackets
            juce::String segment = tokens[i].trim().upToFirstOccurrenceOf("(", false, false);

            if (i == 0) {
                currentType = getClassNameForVariable(segment, code);
            } else {
                currentType = resolveReturnType(currentType, segment);
            }

            if (currentType.isEmpty()) break;
        }
        if (currentType.isNotEmpty()) return currentType;
    }

    // 3. DYNAMIC LOOK-BACK (Assignments: mb = MemoryBlock())
    int assignmentPos = code.lastIndexOf(v + " =");
    if (assignmentPos == -1) assignmentPos = code.lastIndexOf(v + "=");

    if (assignmentPos != -1)
    {
        juce::String rhs = code.substring(assignmentPos + v.length()).trimStart();
        if (rhs.startsWith("=")) rhs = rhs.substring(1).trimStart();

        int endOfLine = rhs.indexOfAnyOf(";\n\r");
        if (endOfLine != -1) rhs = rhs.substring(0, endOfLine).trim();

        juce::String potentialClass = rhs.upToFirstOccurrenceOf("(", false, false).trim();

        if (classes.contains(potentialClass))
            return potentialClass;

        if (rhs != v && rhs.isNotEmpty())
            return getClassNameForVariable(rhs, code);
    }

    return "";
}
