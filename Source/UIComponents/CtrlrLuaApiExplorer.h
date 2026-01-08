#pragma once
#include <JuceHeader.h>

class CtrlrLuaManager;

class CtrlrLuaApiExplorer : public juce::Component,
                            public juce::ListBoxModel
{
public:
    CtrlrLuaApiExplorer(CtrlrLuaManager& lua);
    ~CtrlrLuaApiExplorer() override = default;

    void resized() override;
    void refreshClassList();

    // ListBoxModel for classList
    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics&, int w, int h, bool selected) override;
    void selectedRowsChanged(int row) override;

private:
    struct Method
    {
        juce::String name;
        juce::String type;
        juce::String args;
    };

    struct ClassData
    {
        juce::String name;
        juce::String cppName;
        juce::String alias;
        juce::Array<Method> methods;
    };

    // Inner ListBoxModel for methods list
    class MethodsListBoxModel : public juce::ListBoxModel
    {
    public:
        MethodsListBoxModel(CtrlrLuaApiExplorer& owner) : explorer(owner) {}

        int getNumRows() override;
        void paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) override;
        void selectedRowsChanged(int row) override;

   private:
        CtrlrLuaApiExplorer& explorer;
    };

    void loadXMLData();
    void showClass(const juce::String& className);
    void showMethodDetails(int methodIndex);
    void updateDetailsLabel(const juce::String& text);

    CtrlrLuaManager& luaManager;

    juce::TextEditor searchBox;
    juce::Label detailsLabel;
    juce::ListBox classList { "LuaClasses", this };
    juce::ListBox methodsList { "LuaMethods", nullptr };
    MethodsListBoxModel methodsModel;

    juce::StringArray allClasses;
    juce::StringArray filteredClasses;
    
    juce::Array<ClassData> classDataArray;
    juce::Array<Method> currentMethods;
    juce::String currentClassName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaApiExplorer)
};