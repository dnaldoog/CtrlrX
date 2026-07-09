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
        juce::String luaWrap;
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

            // 1. Background
            if (rowIsSelected)
                g.fillAll(juce::Colours::lightblue.withAlpha(0.5f));
            else
                g.fillAll(rowNumber % 2 == 0 ? juce::Colours::white : juce::Colour(0xfff5f5f5));

            juce::String text;
            juce::Colour iconColor;
            juce::String iconLetter;

            // 2. Determine Type and Data
            if (rowNumber < ownerRef->methodList.size())
            {
                const auto& method = ownerRef->methodList[rowNumber];
                text = method.name;

                // Add args for overloads and constructors to the display text
                if (method.args.isNotEmpty() &&
                    (method.name.contains("[") || method.name == ownerRef->currentClassName))
                {
                    text += " " + method.args;
                }

                if (method.isStatic) {
                    // Check if it's a constructor
                    if (method.name == ownerRef->currentClassName) {
                        iconColor = juce::Colours::darkorange;
                        iconLetter = "C";
                    }
                    else {
                        iconColor = juce::Colours::indianred;
                        iconLetter = "S";
                    }
                }
                else {
                    iconColor = juce::Colours::cornflowerblue;
                    iconLetter = "M";
                }
            }
            else
            {
                // Attribute/Enum logic
                int attrIndex = rowNumber - ownerRef->methodList.size();
                if (attrIndex < ownerRef->attributeList.size()) {
                    text = ownerRef->attributeList[attrIndex];
                    iconColor = juce::Colours::mediumseagreen;
                    iconLetter = "E";
                }
            }

            // 3. Draw the Icon Tag
            auto iconArea = juce::Rectangle<float>(4.0f, 4.0f, (float)height - 8.0f, (float)height - 8.0f);
            g.setColour(iconColor);
            g.fillRoundedRectangle(iconArea, 3.0f);

            g.setColour(juce::Colours::white);
            g.setFont(juce::Font((float)height * 0.55f, juce::Font::bold));
            g.drawText(iconLetter, iconArea, juce::Justification::centred);

            // 4. Draw the Text (offset to the right of the icon)
            g.setColour(juce::Colours::black);
            g.setFont(juce::Font((float)height * 0.7f));
            g.drawText(text, (int)iconArea.getRight() + 8, 0, width - (int)iconArea.getRight() - 12, height,
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