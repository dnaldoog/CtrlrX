#pragma once
#include <JuceHeader.h>

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
                g.setColour(juce::Colours::darkblue);
            }
            else
            {
                text = ownerRef->attributeList[rowNumber - ownerRef->methodList.size()];
                prefix = "[A] ";
                g.setColour(juce::Colours::darkgreen);
            }

            g.drawText(prefix + text, 5, 0, width - 10, height,
                juce::Justification::centredLeft, true);
        }

        void listBoxItemClicked(int row, const juce::MouseEvent& e) override
        {
            if (e.mods.isRightButtonDown())
            {
                // Right-click context menu
                juce::String itemName;
                bool isMethod = (row < ownerRef->methodList.size());

                if (isMethod)
                    itemName = ownerRef->methodList[row];
                else
                    itemName = ownerRef->attributeList[row - ownerRef->methodList.size()];

                juce::PopupMenu menu;
                menu.addItem(1, "Copy Usage Code");
                menu.addItem(2, "Copy Example Function");
                menu.addSeparator();
                menu.addItem(3, "Copy Method Name Only");

                menu.showMenuAsync(juce::PopupMenu::Options(), [this, itemName](int result)
                    {
                        if (result == 1)
                        {
                            ownerRef->copyMethodUsageToClipboard(ownerRef->currentClassName, itemName);
                        }
                        else if (result == 2)
                        {
                            ownerRef->copyExampleToClipboard(ownerRef->currentClassName, itemName);
                        }
                        else if (result == 3)
                        {
                            juce::SystemClipboard::copyTextToClipboard(itemName);
                        }
                    });
            }
            else
            {
                // Left-click behavior
                juce::String itemName;
                bool isMethod = (row < ownerRef->methodList.size());

                if (isMethod)
                    itemName = ownerRef->methodList[row];
                else
                    itemName = ownerRef->attributeList[row - ownerRef->methodList.size()];

                // Show description with introspection info
                juce::String description = ownerRef->getMethodDescription(itemName);
                ownerRef->infoDisplay->setText(description, false);

                // Copy usage on click
                ownerRef->copyMethodUsageToClipboard(ownerRef->currentClassName, itemName);
            }
        }

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

    std::unique_ptr<MethodListModel> methodListModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaClassBrowser)
};