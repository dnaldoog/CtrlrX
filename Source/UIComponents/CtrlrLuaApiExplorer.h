#pragma once
#include <JuceHeader.h>

class CtrlrLuaManager;
class CtrlrGenericHelp;

class CtrlrLuaApiExplorer : public juce::Component,
                            private juce::ListBoxModel
{
public:
    CtrlrLuaApiExplorer(CtrlrLuaManager& lua);
    ~CtrlrLuaApiExplorer() override = default;

    void resized() override;

private:
    // ListBoxModel
    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics&, int w, int h, bool selected) override;
    void selectedRowsChanged(int row) override;

    void refreshClassList();
    void showClass(const juce::String& name);

    CtrlrLuaManager& luaManager;

    juce::TextEditor searchBox;
    juce::ListBox classList { "LuaClasses", this };
    juce::Viewport detailsViewport;

    CtrlrGenericHelp* details = nullptr;

    juce::StringArray allClasses;
    juce::StringArray filteredClasses;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaApiExplorer)
};
