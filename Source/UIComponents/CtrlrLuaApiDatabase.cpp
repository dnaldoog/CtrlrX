#include "CtrlrLuaApiDatabase.h"

//==============================================================================
CtrlrLuaApiDatabase::CtrlrLuaApiDatabase()
{
}

CtrlrLuaApiDatabase::~CtrlrLuaApiDatabase()
{
}

//==============================================================================
void CtrlrLuaApiDatabase::clear()
{
    classes.clear();
    loaded = false;
}

//==============================================================================
bool CtrlrLuaApiDatabase::loadFromFile(const juce::File& xmlFile)
{
    clear();
    xmlRoot = juce::XmlDocument::parse(xmlFile);
    if (!xmlRoot)
        return false;
    if (!xmlFile.existsAsFile())
        return false;

    juce::XmlDocument doc(xmlFile);
    std::unique_ptr<juce::XmlElement> root(doc.getDocumentElement());

    if (root == nullptr)
        return false;

    loaded = parseXml(*root);
    return loaded;
}

//==============================================================================
bool CtrlrLuaApiDatabase::parseXml(const juce::XmlElement& root)
{
    if (!root.hasTagName("LuaAPI"))
        return false;

    forEachXmlChildElementWithTagName(root, c, "class")
    {
        Class cls;
        cls.name = c->getStringAttribute("name");
        cls.cppName = c->getStringAttribute("cpp_name");

        // Instance methods
        if (auto* methods = c->getChildByName("methods"))
        {
            forEachXmlChildElementWithTagName(*methods, m, "method")
            {
                Method mm;
                mm.name = m->getStringAttribute("name");
                cls.instanceMethods.add(mm);
            }
        }

        // Static methods
        if (auto* statics = c->getChildByName("static_methods"))
        {
            forEachXmlChildElementWithTagName(*statics, m, "method")
            {
                Method sm;
                sm.name = m->getStringAttribute("name");
                cls.staticMethods.add(sm);
            }
        }

        // Enums
        if (auto* enums = c->getChildByName("enums"))
        {
            forEachXmlChildElementWithTagName(*enums, e, "enum")
            {
                Enum en;
                en.name = e->getStringAttribute("name");

                forEachXmlChildElementWithTagName(*e, v, "value")
                {
                    EnumValue ev;
                    ev.name = v->getStringAttribute("name");
                    ev.value = v->getStringAttribute("value");
                    en.values.add(ev);
                }

                cls.enums.add(en);
            }
        }

        classes.add(cls);
    }

    return classes.size() > 0;
}

//==============================================================================
const CtrlrLuaApiDatabase::Class*
CtrlrLuaApiDatabase::getClass(const juce::String& className) const
{
    for (const auto& c : classes)
        if (c.name == className)
            return &c;

    return nullptr;
}

//==============================================================================
const juce::Array<CtrlrLuaApiDatabase::Class>&
CtrlrLuaApiDatabase::getAllClasses() const
{
    return classes;
}

//==============================================================================
bool CtrlrLuaApiDatabase::isLoaded() const
{
    return loaded;
}
//const juce::XmlElement* CtrlrLuaApiDatabase::getXmlRoot() const
//{
//    return xmlRoot.get();
//}
