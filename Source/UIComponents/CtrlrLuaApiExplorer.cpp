#include "CtrlrLuaApiExplorer.h"
#include "CtrlrLuaManager.h"
#include "CtrlrGenericHelp.h"

CtrlrLuaApiExplorer::CtrlrLuaApiExplorer(CtrlrLuaManager& lua)
    : luaManager(lua)
{
  //  addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search classes…", juce::Colours::grey);
    searchBox.onTextChange = [this]
    {
        filteredClasses.clear();
        for (auto& c : allClasses)
            if (c.containsIgnoreCase(searchBox.getText()))
                filteredClasses.add(c);
        classList.updateContent();
    };

    addAndMakeVisible(classList);
    classList.setRowHeight(22);

    details = new CtrlrGenericHelp("", 0);
    details->setMarkdownContent("# Select a Lua class from the list");

    detailsViewport.setViewedComponent(details, false);
    addAndMakeVisible(detailsViewport);

    refreshClassList();
}

void CtrlrLuaApiExplorer::refreshClassList()
{
    lua_State* L = luaManager.getLuaState();

    lua_getglobal(L, "class_names");
if (lua_pcall(L, 0, 1, 0) != 0) // Lua 5.1
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
    if (row >= 0)
        showClass(filteredClasses[row]);
}
void CtrlrLuaApiExplorer::showClass(const juce::String& className)
{
    // 1. Guard against empty calls or uninitialized details
    if (className.isEmpty() || details == nullptr)
        return;

    lua_State* L = luaManager.getLuaState();
    if (L == nullptr) return;

    // 2. Safely call the Lua function
    lua_getglobal(L, "what_markdown");
    lua_pushstring(L, className.toRawUTF8());

    // Use a non-zero return check for pcall
    if (lua_pcall(L, 1, 1, 0) != 0)
    {
        const juce::String errorMessage = lua_tostring(L, -1);
        details->setMarkdownContent("# Error\n" + errorMessage);
        lua_pop(L, 1);
    }
    else
    {
        // 3. Extract the Markdown string
        const char* s = lua_tostring(L, -1);
        const juce::String md = (s != nullptr) ? s : "No documentation found for: " + className;
        lua_pop(L, 1);

        // 4. Update Content
        details->setMarkdownContent(md);
    }

    // 5. Update Layout Geometry
    // We force the content width to match the viewport's visible area
    // This triggers the word-wrapping logic inside CtrlrGenericHelp
    const int viewWidth = detailsViewport.getViewWidth();
    const int requiredHeight = juce::roundToInt(details->getRequiredHeight());

    details->setSize(viewWidth, requiredHeight);

    // 6. Navigation
    detailsViewport.setViewPosition(0, 0);
}
//void CtrlrLuaApiExplorer::showClass(const juce::String& name)
//{
//    lua_State* L = luaManager.getLuaState();
//
//    lua_getglobal(L, "what_markdown");
//    lua_pushstring(L, name.toRawUTF8());
//
//    if (lua_pcall(L, 1, 1, 0) == 0)   // Lua 5.1 ✔
//    {
//        const char* s = lua_tostring(L, -1);
//        juce::String md = s ? s : "No data returned.";
//        lua_pop(L, 1);
//
//        details->setMarkdownContent(md);   //correct call
//    }
//}

void CtrlrLuaApiExplorer::resized()
{
    auto r = getLocalBounds();

    searchBox.setBounds(r.removeFromTop(28));
    classList.setBounds(r.removeFromLeft(260));
    detailsViewport.setBounds(r);
}
