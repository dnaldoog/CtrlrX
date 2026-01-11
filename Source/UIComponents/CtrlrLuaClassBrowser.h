#pragma once
#include <JuceHeader.h>

class CtrlrLuaManager;
class CtrlrLuaApiDatabase;

namespace juce { class XmlElement; }

class CtrlrLuaClassBrowser : public juce::Component,
    public juce::ListBoxModel
{
public:
    CtrlrLuaClassBrowser(CtrlrLuaManager* luaManager);
    ~CtrlrLuaClassBrowser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // ListBoxModel methods for the class list
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g,
        int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;

    // Set the XML data source (this triggers loading the class list)
    void setLuaApiXml(const juce::XmlElement* xml);

    // Method info structure - public so it can be used in implementations
    struct MethodInfo
    {
        juce::String name;
        juce::String args;
        bool isStatic;
    };

    // Nested class for displaying method details
    class MethodListModel : public juce::ListBoxModel
    {
    public:
        MethodListModel(CtrlrLuaClassBrowser* owner) : ownerRef(owner) {}

        int getNumRows() override
        {
            return ownerRef ? (ownerRef->methodList.size() + ownerRef->attributeList.size()) : 0;
        }

        void paintListBoxItem(int rowNumber, juce::Graphics& g,
            int width, int height, bool rowIsSelected) override
        {
            if (!ownerRef) return;

            if (rowIsSelected)
                g.fillAll(juce::Colours::lightblue);
            else
                g.fillAll(rowNumber % 2 == 0 ? juce::Colours::white : juce::Colour(0xfff0f0f0));

            juce::String text;
            juce::String prefix;

            if (rowNumber < ownerRef->methodList.size())
            {
                const auto& method = ownerRef->methodList[rowNumber];
                text = method.name;

                // Show args for overloads and constructors
                if (method.args.isNotEmpty() &&
                    (method.name.contains("[") || method.name == ownerRef->currentClassName))
                {
                    text += " " + method.args;
                }
                if (method.isStatic) {
                prefix = "[ST] ";
                g.setColour(juce::Colour(0xffc00000)); // Red for static
                }
                else {
                prefix = "[ME] ";
                g.setColour(juce::Colours::darkblue); // Blue for instance methods
                }
            
                  
            }
            else
            {
                text = ownerRef->attributeList[rowNumber - ownerRef->methodList.size()];
                prefix = "[A] ";
                g.setColour(juce::Colours::darkgreen); // Green for attributes
            }

            g.drawText(prefix + text, 5, 0, width - 10, height,
                juce::Justification::centredLeft, true);
        }

        void listBoxItemClicked(int row, const juce::MouseEvent& e) override;

    private:
        CtrlrLuaClassBrowser* ownerRef;
    };

private:
    void loadClassList();
    void loadMethodsForClass(const juce::String& className);
    void filterClassList(const juce::String& searchText);

    juce::String getMethodDescription(const juce::String& methodName);
    juce::String introspectMethod(const juce::String& className, const juce::String& methodName);

    void copyMethodUsageToClipboard(const juce::String& className, const juce::String& methodName);
    void copyExampleToClipboard(const juce::String& className, const juce::String& methodName);

    juce::String generateLuaUsageForMethod(const juce::String& className, const juce::String& methodName);
    juce::String generateExampleFunction(const juce::String& className, const juce::String& methodName, bool isStatic);

    // Helper to get args for a method
    juce::String getMethodArgs(const juce::String& methodName) const;

    CtrlrLuaManager* luaManagerRef;

    // Data
    juce::StringArray classList;
    juce::StringArray fullClassList;
    juce::Array<MethodInfo> methodList;
    juce::StringArray attributeList;
    juce::String currentClassName;

    // UI Components
    std::unique_ptr<juce::TextEditor> searchBox;
    std::unique_ptr<juce::TextButton> refreshButton;
    std::unique_ptr<juce::TextEditor> infoDisplay;
    std::unique_ptr<juce::ListBox> classListBox;
    std::unique_ptr<juce::ListBox> methodListBox;
    std::unique_ptr<MethodListModel> methodListModel;

    // XML data (non-owning)
    const juce::XmlElement* luaApiXml = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaClassBrowser)
};