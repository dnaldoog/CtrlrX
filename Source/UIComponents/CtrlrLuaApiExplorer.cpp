#include "CtrlrLuaApiExplorer.h"
#include "CtrlrLuaManager.h"
#include "CtrlrGenericHelp.h"

CtrlrLuaApiExplorer::CtrlrLuaApiExplorer(CtrlrLuaManager& lua)
    : luaManager(lua), methodsModel(*this)
{
    // Search box setup
    addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search classes…", juce::Colours::grey);
    searchBox.onTextChange = [this]
    {
        filteredClasses.clear();
        for (auto& c : allClasses)
            if (c.containsIgnoreCase(searchBox.getText()))
                filteredClasses.add(c);
        classList.updateContent();
    };

    // Details label setup (replaces the viewport at top)
    addAndMakeVisible(detailsLabel);
    detailsLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xff2c2c2c));
    detailsLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    detailsLabel.setColour(juce::Label::outlineColourId, juce::Colours::darkgrey);
    detailsLabel.setJustificationType(juce::Justification::centredLeft);
    detailsLabel.setText("  Select a class or method to view details", juce::dontSendNotification);
    detailsLabel.setFont(juce::Font(14.0f, juce::Font::plain));

    // Class list setup
    addAndMakeVisible(classList);
    classList.setModel(this);
    classList.setRowHeight(22);

    // Methods list setup
    addAndMakeVisible(methodsList);
    methodsList.setModel(&methodsModel);
    methodsList.setRowHeight(22);

    // Load XML data
    loadXMLData();
    
    refreshClassList();
}

void CtrlrLuaApiExplorer::loadXMLData()
{
    // Try to load XML file - adjust path as needed
    juce::File xmlFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                            .getParentDirectory()
                            .getChildFile("LuaAPI.xml");
    
    // Alternative: try relative to application data
    if (!xmlFile.existsAsFile())
    {
        xmlFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                     .getParentDirectory()
                     .getChildFile("LuaAPI.xml");
    }

    if (xmlFile.existsAsFile())
    {
        auto xml = juce::XmlDocument::parse(xmlFile);
        
        if (xml != nullptr)
        {
            // Parse <class> elements
            for (auto* classNode : xml->getChildWithTagNameIterator("class"))
            {
                ClassData classData;
                classData.name = classNode->getStringAttribute("name");
                classData.cppName = classNode->getStringAttribute("cpp_name");
                classData.alias = classNode->getStringAttribute("alias");
                
                // Parse <methods> child
                if (auto* methodsNode = classNode->getChildByName("methods"))
                {
                    for (auto* methodNode : methodsNode->getChildWithTagNameIterator("method"))
                    {
                        Method method;
                        method.name = methodNode->getStringAttribute("name");
                        method.type = methodNode->getStringAttribute("type");
                        method.args = methodNode->getStringAttribute("args");
                        
                        classData.methods.add(method);
                    }
                }
                
                classDataArray.add(classData);
            }
        }
    }
}

void CtrlrLuaApiExplorer::refreshClassList()
{
    lua_State* L = luaManager.getLuaState();

    lua_getglobal(L, "class_names");
    if (lua_pcall(L, 0, 1, 0) == 0) // Lua 5.1
    {
        if (lua_istable(L, -1))
        {
            const int n = (int)lua_objlen(L, -1);
            for (int i = 1; i <= n; ++i)
            {
                lua_rawgeti(L, -1, i);
                allClasses.add(lua_tostring(L, -1));
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);
    }

    filteredClasses = allClasses;
    classList.updateContent();
}

int CtrlrLuaApiExplorer::getNumRows()
{
    return filteredClasses.size();
}

void CtrlrLuaApiExplorer::paintListBoxItem(
    int row, juce::Graphics& g, int w, int h, bool selected)
{
    if (selected)
        g.fillAll(juce::Colours::lightblue);

    g.setColour(juce::Colours::black);
    g.drawText(filteredClasses[row], 6, 0, w, h, juce::Justification::centredLeft);
}

void CtrlrLuaApiExplorer::selectedRowsChanged(int row)
{
    if (row >= 0 && row < filteredClasses.size())
        showClass(filteredClasses[row]);
}

void CtrlrLuaApiExplorer::showClass(const juce::String& className)
{
    if (className.isEmpty())
        return;

    currentClassName = className;
    currentMethods.clear();

    // Find the class data from XML
    for (auto& classData : classDataArray)
    {
        if (classData.name == className || 
            classData.cppName == className || 
            classData.alias == className)
        {
            currentMethods = classData.methods;
            break;
        }
    }

    // Update methods list
    methodsList.updateContent();
    methodsList.deselectAllRows();

    // Update details label
    updateDetailsLabel("Class: " + className);
}

void CtrlrLuaApiExplorer::showMethodDetails(int methodIndex)
{
    if (methodIndex < 0 || methodIndex >= currentMethods.size())
        return;

    const auto& method = currentMethods[methodIndex];
    
    juce::String details = currentClassName + " -> " + method.name;
    
    // Add args
    if (method.args.isNotEmpty())
        details += " -> " + method.args;
    else
        details += " -> ()";
    
    // Optionally show method type
    if (method.type.isNotEmpty())
        details += "  [" + method.type + "]";
    
    updateDetailsLabel(details);
}

void CtrlrLuaApiExplorer::updateDetailsLabel(const juce::String& text)
{
    detailsLabel.setText("  " + text, juce::dontSendNotification);
}

void CtrlrLuaApiExplorer::resized()
{
    auto r = getLocalBounds();

    // Search box at top
    searchBox.setBounds(r.removeFromTop(28));
    r.removeFromTop(4); // spacing

    // Details label below search (100% width)
    detailsLabel.setBounds(r.removeFromTop(40));
    r.removeFromTop(4); // spacing

    // Split the remaining space: classes | methods
    auto halfWidth = r.getWidth() / 2;
    classList.setBounds(r.removeFromLeft(halfWidth - 2));
    r.removeFromLeft(4); // spacing between lists
    methodsList.setBounds(r);
}
