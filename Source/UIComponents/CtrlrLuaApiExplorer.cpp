#include "CtrlrLuaApiExplorer.h"
#include "CtrlrLuaManager.h"
#include "CtrlrLuaApiDatabase.h"

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

    // Details label setup
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

    refreshClassList();
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
    {
        currentClassName = filteredClasses[row];
        loadMethodsForClass(currentClassName);
        updateDetailsLabel("Class: " + currentClassName);
    }
}

void CtrlrLuaApiExplorer::loadMethodsForClass(const juce::String& className)
{
    currentMethods.clear();

    // Get the database from CtrlrLuaManager
    const CtrlrLuaApiDatabase* database = luaManager.getDatabase();
    
    if (database && database->isLoaded())
    {
        const auto* classData = database->getClass(className);
        
        if (classData != nullptr)
        {
            // Get XML root to parse args
            const juce::XmlElement* xmlRoot = database->getXmlRoot();
            const juce::XmlElement* classNode = nullptr;
            
            if (xmlRoot)
            {
                forEachXmlChildElementWithTagName(*xmlRoot, node, "class")
                {
                    if (node->getStringAttribute("name") == className)
                    {
                        classNode = node;
                        break;
                    }
                }
            }

            // Add instance methods
            for (const auto& method : classData->instanceMethods)
            {
                MethodInfo info;
                info.name = method.name;
                info.type = "instance";
                info.args = "";
                
                // Try to find args from XML
                if (classNode)
                {
                    if (auto* methodsNode = classNode->getChildByName("methods"))
                    {
                        forEachXmlChildElementWithTagName(*methodsNode, mNode, "method")
                        {
                            if (mNode->getStringAttribute("name") == method.name)
                            {
                                info.args = mNode->getStringAttribute("args");
                                break;
                            }
                        }
                    }
                }
                
                currentMethods.add(info);
            }

            // Add static methods
            for (const auto& method : classData->staticMethods)
            {
                MethodInfo info;
                info.name = method.name;
                info.type = "static";
                info.args = "";
                
                // Try to find args from XML
                if (classNode)
                {
                    if (auto* methodsNode = classNode->getChildByName("static_methods"))
                    {
                        forEachXmlChildElementWithTagName(*methodsNode, mNode, "method")
                        {
                            if (mNode->getStringAttribute("name") == method.name)
                            {
                                info.args = mNode->getStringAttribute("args");
                                break;
                            }
                        }
                    }
                }
                
                currentMethods.add(info);
            }

            // Add enums
            for (const auto& enumData : classData->enums)
            {
                MethodInfo info;
                info.name = enumData.name;
                info.type = "enum";
                info.args = "";
                currentMethods.add(info);
            }
        }
    }

    methodsList.updateContent();
    methodsList.deselectAllRows();
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
    
    // Show method type
    if (method.type.isNotEmpty())
        details += "  [" + method.type + "]";
    
    updateDetailsLabel(details);
}

void CtrlrLuaApiExplorer::updateDetailsLabel(const juce::String& text)
{
    detailsLabel.setText("  " + text, juce::dontSendNotification);
}

// MethodsListBoxModel implementation
int CtrlrLuaApiExplorer::MethodsListBoxModel::getNumRows()
{
    return explorer.currentMethods.size();
}

void CtrlrLuaApiExplorer::MethodsListBoxModel::paintListBoxItem(
    int row, juce::Graphics& g, int w, int h, bool selected)
{
    if (selected)
        g.fillAll(juce::Colours::lightblue);

    g.setColour(juce::Colours::black);
    
    if (row >= 0 && row < explorer.currentMethods.size())
    {
        const auto& method = explorer.currentMethods[row];
        juce::String text = method.name;
        
        if (method.type == "static")
            text += " (static)";
        else if (method.type == "enum")
            text += " (enum)";
        
        g.drawText(text, 6, 0, w, h, juce::Justification::centredLeft);
    }
}

void CtrlrLuaApiExplorer::MethodsListBoxModel::selectedRowsChanged(int row)
{
    explorer.showMethodDetails(row);
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