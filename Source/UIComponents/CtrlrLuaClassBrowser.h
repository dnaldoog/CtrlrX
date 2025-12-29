#pragma once
#include <JuceHeader.h>
#include "CtrlrLuaApiDatabase.h"    
namespace juce { class XmlElement; }

class CtrlrLuaClassBrowser : public juce::Component,
    public juce::ListBoxModel
{
public:
    CtrlrLuaClassBrowser(class CtrlrLuaManager* luaManager);
    ~CtrlrLuaClassBrowser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // ListBoxModel methods for the class list
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g,
        int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    void setLuaApiXml(const juce::XmlElement* xml);

    // Nested class for displaying method details
    class MethodListModel : public juce::ListBoxModel
    {
    public:
        MethodListModel(CtrlrLuaClassBrowser* owner) : ownerRef(owner) {}

        int getNumRows() override
        {
            return ownerRef->methodList.size() + ownerRef->attributeList.size();
        }

        void paintListBoxItem(int rowNumber, juce::Graphics& g,
            int width, int height, bool rowIsSelected) override
        {
            if (rowIsSelected)
                g.fillAll(juce::Colours::lightblue);
            else
                g.fillAll(rowNumber % 2 == 0 ? juce::Colours::white : juce::Colour(0xfff0f0f0));

            g.setColour(juce::Colours::black);

            juce::String text;
            juce::String prefix;

            if (rowNumber < ownerRef->methodList.size())
            {
                text = ownerRef->methodList[rowNumber];
                prefix = "[M] ";

                if (text.contains("[STATIC]"))
                    g.setColour(juce::Colour(0xffc00000)); // red hue for static
                else
                    g.setColour(juce::Colours::darkblue); // normal method
            }
            else
            {
                text = ownerRef->attributeList[rowNumber - ownerRef->methodList.size()];
                prefix = "[A] ";
                g.setColour(juce::Colours::darkgreen); // attribute
            }


            g.drawText(prefix + text, 5, 0, width - 10, height,
                juce::Justification::centredLeft, true);
        }

     void listBoxItemClicked(int row, const juce::MouseEvent & e) override;


    private:
        CtrlrLuaClassBrowser* ownerRef;
    };

private:
    void loadClassList();
    void loadMethodsForClass(const juce::String& className);
    juce::String getMethodDescription(const juce::String& methodName);
    juce::String introspectMethod(const juce::String& className, const juce::String& methodName);
    void copyMethodUsageToClipboard(const juce::String& className,
        const juce::String& methodName);
    void copyExampleToClipboard(const juce::String& className,
        const juce::String& methodName);
    juce::String generateLuaUsageForMethod(const juce::String& className,
        const juce::String& methodName);
    juce::String generateExampleFunction(const juce::String& className,
        const juce::String& methodName,
        bool isStatic);
    void filterClassList(const juce::String& searchText);

    class CtrlrLuaManager* luaManagerRef;

    juce::StringArray classList;
    juce::StringArray fullClassList;  // Unfiltered list for search
    juce::StringArray methodList;
    juce::StringArray attributeList;
    juce::String currentClassName;

    std::unique_ptr<juce::ListBox> classListBox;
    std::unique_ptr<juce::ListBox> methodListBox;
    std::unique_ptr<juce::Viewport> methodViewport;
    std::unique_ptr<juce::TextEditor> infoDisplay;
    std::unique_ptr<juce::TextEditor> searchBox;
    std::unique_ptr<juce::TextButton> refreshButton;
    const juce::XmlElement* luaApiXml = nullptr; // non-owning, const
    std::unique_ptr<MethodListModel> methodListModel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaClassBrowser)
};