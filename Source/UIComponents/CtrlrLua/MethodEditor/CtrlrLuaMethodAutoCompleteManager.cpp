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

    if (symbol == "panel") return "panel";
    if (symbol == "mod")   return "mod";
    if (symbol == "comp")  return "comp";
    if (symbol == "utils") return "utils";

    int assignPos = fullDocumentText.lastIndexOf(symbol + "=");
    if (assignPos == -1) assignPos = fullDocumentText.lastIndexOf(symbol + " =");

    if (assignPos != -1)
    {
        int startOfClass = fullDocumentText.indexOf(assignPos, "=") + 1;
        while (startOfClass < fullDocumentText.length() &&
            juce::CharacterFunctions::isWhitespace(fullDocumentText[startOfClass]))
            startOfClass++;

        int endOfClass = startOfClass;
        while (endOfClass < fullDocumentText.length() &&
            (juce::CharacterFunctions::isLetterOrDigit(fullDocumentText[endOfClass])))
            endOfClass++;

        juce::String foundClass = fullDocumentText.substring(startOfClass, endOfClass);
        if (classes.contains(foundClass)) return foundClass;
    }
    return "";
}