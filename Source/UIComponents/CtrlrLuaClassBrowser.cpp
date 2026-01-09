#include "CtrlrLuaClassBrowser.h"
#include "CtrlrLuaManager.h"
#include "CtrlrLuaApiDatabase.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

CtrlrLuaClassBrowser::CtrlrLuaClassBrowser(CtrlrLuaManager* luaManager)
    : luaManagerRef(luaManager)
{
    // Create search box
    searchBox = std::make_unique<juce::TextEditor>();
    searchBox->setTextToShowWhenEmpty("Search classes...", juce::Colours::grey);
    searchBox->onTextChange = [this]() { filterClassList(searchBox->getText()); };
    addAndMakeVisible(searchBox.get());

    // Create refresh button
    refreshButton = std::make_unique<juce::TextButton>("Refresh");
    refreshButton->onClick = [this]() { loadClassList(); };
    addAndMakeVisible(refreshButton.get());

    // Create info display (top, full width)
    infoDisplay = std::make_unique<juce::TextEditor>();
    infoDisplay->setMultiLine(true);
    infoDisplay->setReadOnly(true);
    infoDisplay->setScrollbarsShown(true);
    infoDisplay->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2c2c2c));
    infoDisplay->setColour(juce::TextEditor::textColourId, juce::Colours::white);
    infoDisplay->setFont(juce::Font(13.0f));
    infoDisplay->setText("Select a class to view its methods and attributes.", false);
    addAndMakeVisible(infoDisplay.get());

    // Create class list box
    classListBox = std::make_unique<juce::ListBox>("Classes", this);
    classListBox->setRowHeight(22);
    classListBox->setColour(juce::ListBox::backgroundColourId, juce::Colours::white);
    addAndMakeVisible(classListBox.get());

    // Create method list model and box
    methodListModel = std::make_unique<MethodListModel>(this);
    methodListBox = std::make_unique<juce::ListBox>("Methods", methodListModel.get());
    methodListBox->setRowHeight(22);
    methodListBox->setColour(juce::ListBox::backgroundColourId, juce::Colours::white);
    addAndMakeVisible(methodListBox.get());
}

CtrlrLuaClassBrowser::~CtrlrLuaClassBrowser()
{
}

void CtrlrLuaClassBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xffe0e0e0));

    // Draw labels
    g.setColour(juce::Colours::black);
    g.setFont(14.0f);

    const int splitPos = getWidth() / 2;
    const int labelY = 105;

    // Draw label for class list
    g.drawText("Available Classes", 5, labelY, 200, 20, juce::Justification::left);

    // Draw label for methods list
    g.drawText("Methods & Attributes", splitPos + 5, labelY, 300, 20, juce::Justification::left);

    // Draw divider line between class and method lists
    g.setColour(juce::Colours::grey);
    g.drawLine((float)splitPos, labelY + 25, (float)splitPos, (float)getHeight(), 1.0f);
}

void CtrlrLuaClassBrowser::resized()
{
    auto bounds = getLocalBounds();
    const int buttonHeight = 25;
    const int infoHeight = 60;
    const int labelHeight = 25;
    const int margin = 5;
    
    int currentY = margin;
    
    // Search and Refresh at top
    searchBox->setBounds(margin, currentY, bounds.getWidth() - 90, buttonHeight);
    refreshButton->setBounds(bounds.getWidth() - 80, currentY, 75, buttonHeight);
    currentY += buttonHeight + margin;
    
    // Info Display: Full width below search
    infoDisplay->setBounds(margin, currentY, bounds.getWidth() - (2 * margin), infoHeight);
    currentY += infoHeight + margin;
    
    // Labels (drawn in paint)
    currentY += labelHeight;
    
    // Split view: Classes | Methods
    const int splitPos = bounds.getWidth() / 2;
    const int remainingHeight = bounds.getHeight() - currentY - margin;
    
    classListBox->setBounds(margin, currentY, splitPos - (2 * margin), remainingHeight);
    methodListBox->setBounds(splitPos + margin, currentY, 
                            bounds.getWidth() - splitPos - (2 * margin), 
                            remainingHeight);
}

void CtrlrLuaClassBrowser::loadClassList()
{
    classList.clear();

    if (!luaApiXml)
    {
        infoDisplay->setText("XML data not available.\nLua API browser cannot load classes.", false);
        return;
    }

    forEachXmlChildElementWithTagName(*luaApiXml, cls, "class")
    {
        classList.add(cls->getStringAttribute("name"));
    }

    classList.removeDuplicates(false);
    classList.sort(true);
    fullClassList = classList;
    classListBox->updateContent();

    if (classList.isEmpty())
    {
        infoDisplay->setText("No classes found in XML.", false);
    }
    else
    {
        infoDisplay->setText("Loaded " + juce::String(classList.size()) + " classes.\n\n"
            "Click on a class to see its methods and attributes.", false);
    }
}

void CtrlrLuaClassBrowser::loadMethodsForClass(const juce::String& className)
{
    methodList.clear();
    attributeList.clear();
    currentClassName = className;

    if (!luaApiXml)
        return;

    forEachXmlChildElementWithTagName(*luaApiXml, cls, "class")
    {
        if (cls->getStringAttribute("name") == className)
        {
            // Get methods from <methods> container
            if (auto* methodsElem = cls->getChildByName("methods"))
            {
                forEachXmlChildElementWithTagName(*methodsElem, m, "method")
                {
                    methodList.add(m->getStringAttribute("name"));
                }
            }

            // Get static methods from <static_methods> container
            if (auto* staticElem = cls->getChildByName("static_methods"))
            {
                forEachXmlChildElementWithTagName(*staticElem, m, "method")
                {
                    methodList.add(m->getStringAttribute("name") + " [STATIC]");
                }
            }

            // Get enums from <enums> container
            if (auto* enumsElem = cls->getChildByName("enums"))
            {
                forEachXmlChildElementWithTagName(*enumsElem, e, "enum")
                {
                    juce::String enumName = e->getStringAttribute("name");
                    forEachXmlChildElementWithTagName(*e, v, "value")
                    {
                        attributeList.add(enumName + "." + v->getStringAttribute("name"));
                    }
                }
            }

            break;
        }
    }

    methodList.sort(true);
    attributeList.sort(true);
    methodListBox->updateContent();

    infoDisplay->setText("Class: " + className + "\n\n" +
        "Methods: " + juce::String(methodList.size()) + "\n" +
        "Enums: " + juce::String(attributeList.size()) + "\n\n" +
        "Click on any method to copy Lua usage code.",
        false);
}

juce::String CtrlrLuaClassBrowser::introspectMethod(
    const juce::String& className,
    const juce::String& methodName)
{
    juce::String result;
    bool isStatic = false;

    juce::String lowerMethod = methodName.toLowerCase();
    if (lowerMethod.startsWith("from") ||
        lowerMethod.startsWith("create") ||
        lowerMethod == "new" ||
        lowerMethod.startsWith("wrap"))
    {
        isStatic = true;
        result = "Likely STATIC method (use .)\n";
        result += "Reason: Method name pattern suggests factory/constructor\n";
    }
    else
    {
        result = "Likely INSTANCE method (use :)\n";
        result += "Reason: Standard method name pattern\n";
    }

    bool isMethod = methodList.contains(methodName);
    result += "Type: " + juce::String(isMethod ? "Method (function)" : "Attribute (property)") + "\n";

    return result;
}

void CtrlrLuaClassBrowser::copyMethodUsageToClipboard(const juce::String& className,
    const juce::String& methodName)
{
    juce::String luaCode = generateLuaUsageForMethod(className, methodName);
    juce::SystemClipboard::copyTextToClipboard(luaCode);

    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "Lua Usage Copied",
        "Usage code copied to clipboard.\n\n"
        "Paste into the Lua editor.\n"
        "Right-click for example function."
    );
}

void CtrlrLuaClassBrowser::copyExampleToClipboard(const juce::String& className,
    const juce::String& methodName)
{
    bool isStatic = false;
    juce::String lowerMethod = methodName.toLowerCase();

    if (lowerMethod.startsWith("from") ||
        lowerMethod.startsWith("create") ||
        lowerMethod == "new" ||
        lowerMethod.startsWith("wrap"))
    {
        isStatic = true;
    }

    juce::String example = generateExampleFunction(className, methodName, isStatic);
    juce::SystemClipboard::copyTextToClipboard(example);

    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "Example Function Copied",
        "Example function copied to clipboard.\n\n"
        "Detected as: " + juce::String(isStatic ? "STATIC" : "INSTANCE") + " method\n"
        "Based on naming pattern analysis.\n\n"
        "Paste into your Lua code and customize."
    );
}

juce::String CtrlrLuaClassBrowser::generateExampleFunction(const juce::String& className,
    const juce::String& methodName,
    bool isStatic)
{
    juce::String example;
    bool isMethod = methodList.contains(methodName);

    example += "-- ==================================================\n";
    example += "-- Example function using " + className + "." + methodName + "\n";
    example += "-- " + juce::String(isStatic ? "Static method (use .)" : "Instance method (use :)") + "\n";
    example += "-- ==================================================\n\n";

    juce::String functionName = "example" + methodName.substring(0, 1).toUpperCase() + methodName.substring(1);

    if (className.contains("Panel"))
    {
        if (isStatic)
        {
            example += "function " + functionName + "()\n";
            example += "    local result = " + className + "." + methodName + "()\n";
            example += "    console(\"Result: \" .. tostring(result))\n";
            example += "    return result\n";
            example += "end\n";
        }
        else
        {
            example += "function " + functionName + "()\n";
            if (isMethod)
            {
                example += "    local result = panel:" + methodName + "()\n";
                example += "    console(\"Result: \" .. tostring(result))\n";
                example += "    return result\n";
            }
            else
            {
                example += "    local value = panel." + methodName + "\n";
                example += "    console(\"Current value: \" .. tostring(value))\n";
                example += "    panel." + methodName + " = newValue\n";
            }
            example += "end\n";
        }
    }
    else if (className.contains("Modulator"))
    {
        example += "function " + functionName + "(modulatorName)\n";
        example += "    local modulator = panel:getModulatorByName(modulatorName)\n";
        example += "    if not modulator then\n";
        example += "        console(\"Modulator not found: \" .. modulatorName)\n";
        example += "        return\n";
        example += "    end\n\n";

        if (isStatic)
        {
            example += "    local result = " + className + "." + methodName + "()\n";
        }
        else if (isMethod)
        {
            example += "    local result = modulator:" + methodName + "()\n";
            example += "    console(\"Result: \" .. tostring(result))\n";
            example += "    return result\n";
        }
        else
        {
            example += "    local value = modulator." + methodName + "\n";
            example += "    console(\"Value: \" .. tostring(value))\n";
        }
        example += "end\n";
    }
    else
    {
        example += "function " + functionName + "()\n";

        if (isStatic)
        {
            example += "    local result = " + className + "." + methodName + "()\n";
            example += "    console(\"Result: \" .. tostring(result))\n";
            example += "    return result\n";
        }
        else
        {
            example += "    local m = " + className + "()\n\n";
            if (isMethod)
            {
                example += "    local result = m:" + methodName + "()\n";
                example += "    console(\"Result: \" .. tostring(result))\n";
                example += "    return result\n";
            }
            else
            {
                example += "    local value = m." + methodName + "\n";
                example += "    console(\"Value: \" .. tostring(value))\n";
            }
        }
        example += "end\n";
    }

    example += "\n-- Call the example:\n";
    example += "-- " + functionName + "()\n";

    return example;
}

juce::String CtrlrLuaClassBrowser::generateLuaUsageForMethod(const juce::String& className,
    const juce::String& methodName)
{
    juce::String result;
    bool isMethod = methodList.contains(methodName);

    result += "-- Class: " + className + "\n";
    result += juce::String("-- ") + (isMethod ? "Method" : "Attribute") + ": " + methodName + "\n\n";

    if (className == "CtrlrPanel" || className == "panel")
    {
        if (isMethod)
        {
            result += "local result = panel:" + methodName + "()\n";
        }
        else
        {
            result += "local value = panel." + methodName + "\n";
            result += "panel." + methodName + " = newValue\n";
        }
    }
    else if (className == "CtrlrModulator" || className.startsWith("Modulator"))
    {
        if (isMethod)
        {
            result += "local modulator = panel:getModulatorByName(\"modulatorName\")\n";
            result += "local result = modulator:" + methodName + "()\n";
        }
        else
        {
            result += "local modulator = panel:getModulatorByName(\"modulatorName\")\n";
            result += "local value = modulator." + methodName + "\n";
        }
    }
    else if (className == "CtrlrComponent" || className.contains("Component"))
    {
        if (isMethod)
        {
            result += "local component = panel:getModulatorByName(\"modulatorName\"):getComponent()\n";
            result += "local result = component:" + methodName + "()\n";
        }
        else
        {
            result += "local component = panel:getModulatorByName(\"modulatorName\"):getComponent()\n";
            result += "local value = component." + methodName + "\n";
        }
    }
    else
    {
        if (isMethod)
        {
            result += "local obj = " + className + ":new()\n";
            result += "local result = obj:" + methodName + "()\n";
        }
        else
        {
            result += "local obj = " + className + ":new()\n";
            result += "local value = obj." + methodName + "\n";
        }
    }

    return result;
}

int CtrlrLuaClassBrowser::getNumRows()
{
    return classList.size();
}

void CtrlrLuaClassBrowser::paintListBoxItem(int rowNumber, juce::Graphics& g,
    int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
    else
        g.fillAll(rowNumber % 2 == 0 ? juce::Colours::white : juce::Colour(0xfff0f0f0));

    g.setColour(juce::Colours::black);

    if (rowNumber < classList.size())
    {
        g.drawText(classList[rowNumber], 5, 0, width - 10, height,
            juce::Justification::centredLeft, true);
    }
}

void CtrlrLuaClassBrowser::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    if (row < classList.size())
    {
        loadMethodsForClass(classList[row]);
    }
}

void CtrlrLuaClassBrowser::filterClassList(const juce::String& searchText)
{
    if (searchText.isEmpty())
    {
        classList = fullClassList;
    }
    else
    {
        classList.clear();
        for (const auto& className : fullClassList)
        {
            if (className.containsIgnoreCase(searchText))
                classList.add(className);
        }
    }

    classListBox->updateContent();
}

void CtrlrLuaClassBrowser::setLuaApiXml(const juce::XmlElement* xml)
{
    luaApiXml = xml;
    loadClassList();
}

juce::String CtrlrLuaClassBrowser::getMethodDescription(const juce::String& methodName)
{
    juce::String description;

    description += "Class: " + currentClassName + "\n";
    description += "Method/Attribute: " + methodName + "\n\n";

    bool isStatic = false;
    juce::String lowerMethod = methodName.toLowerCase();

    if (lowerMethod.startsWith("from") ||
        lowerMethod.startsWith("create") ||
        lowerMethod == "new" ||
        lowerMethod.startsWith("wrap"))
    {
        isStatic = true;
        description += "=== Detection Info ===\n";
        description += "Likely STATIC method (use .)\n";
        description += "Reason: Method name pattern suggests factory/constructor\n\n";
    }
    else
    {
        description += "=== Detection Info ===\n";
        description += "Likely INSTANCE method (use :)\n";
        description += "Reason: Standard method name pattern\n\n";
    }

    bool isMethod = methodList.contains(methodName);
    description += "Type: " + juce::String(isMethod ? "Method (function)" : "Enum/Attribute") + "\n\n";

    description += "=== Usage ===\n";
    description += "Lua code copied to clipboard.\n";
    description += "Right-click for example function.";

    return description;
}

void CtrlrLuaClassBrowser::MethodListModel::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    if (!ownerRef) return;

    juce::String itemName;
    bool isMethod = (row < ownerRef->methodList.size());
    bool isStatic = false;

    if (isMethod)
    {
        itemName = ownerRef->methodList[row];
        if (itemName.contains("[STATIC]"))
        {
            isStatic = true;
            itemName = itemName.replace("[STATIC]", "").trim();
        }
    }
    else
    {
        itemName = ownerRef->attributeList[row - ownerRef->methodList.size()];
    }

    if (e.mods.isRightButtonDown())
    {
        juce::PopupMenu menu;

        menu.addItem(1, "Copy Class Name");
        menu.addItem(2, "Copy Method/Attribute Name");
        menu.addItem(3, "Copy Class and Method");
        menu.addItem(4, "Copy Example Function (Lua)");

        menu.showMenuAsync(juce::PopupMenu::Options(), [this, itemName, isMethod, isStatic](int result)
        {
            if (!ownerRef) return;

            switch (result)
            {
            case 1:
                juce::SystemClipboard::copyTextToClipboard(ownerRef->currentClassName);
                break;

            case 2:
                juce::SystemClipboard::copyTextToClipboard(itemName);
                break;

            case 3:
            {
                juce::String combined = ownerRef->currentClassName;
                if (isMethod)
                    combined += (isStatic ? "." : ":") + itemName;
                else
                    combined += "." + itemName;
                juce::SystemClipboard::copyTextToClipboard(combined);
                break;
            }

            case 4:
            {
                juce::String example;
                if (isMethod)
                {
                    if (isStatic)
                    {
                        example = "local result = " + ownerRef->currentClassName + "." + itemName + "(parameters)\n";
                    }
                    else
                    {
                        example = "local result = " + ownerRef->currentClassName + ":" + itemName + "(parameters)\n";
                    }
                }
                else
                {
                    example = "local result = " + ownerRef->currentClassName + "." + itemName + "\n";
                }

                juce::SystemClipboard::copyTextToClipboard(example);
                break;
            }
            }
        });
    }
}