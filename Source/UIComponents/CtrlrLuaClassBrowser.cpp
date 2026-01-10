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
    const int labelY = 155;  // Adjusted for new info height (120 + margins)

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
    const int infoHeight = 120;  // Increased from 60 to 120
    const int labelHeight = 25;
    const int margin = 5;

    int currentY = margin;

    // Search and Refresh at top
    searchBox->setBounds(margin, currentY, bounds.getWidth() - 90, buttonHeight);
    refreshButton->setBounds(bounds.getWidth() - 80, currentY, 75, buttonHeight);
    currentY += buttonHeight + margin;

    // Info Display: Full width below search (now taller)
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
            // Get constructors
            forEachXmlChildElementWithTagName(*cls, ctor, "constructor")
            {
                MethodInfo info;
                info.name = className;  // Constructor has same name as class
                info.args = ctor->getStringAttribute("args");
                info.isStatic = true;  // Constructors are like static methods
                methodList.add(info);
            }

            // Get instance methods from <methods> container
            if (auto* methodsElem = cls->getChildByName("methods"))
            {
                forEachXmlChildElementWithTagName(*methodsElem, m, "method")
                {
                    MethodInfo info;
                    info.name = m->getStringAttribute("name");
                    info.args = m->getStringAttribute("args");
                    info.isStatic = false;

                    // Check if it's an overload
                    juce::String overload = m->getStringAttribute("overload");
                    if (overload.isNotEmpty())
                    {
                        info.name += " [" + overload + "]";
                    }

                    methodList.add(info);
                }
            }

            // Get static methods from <static_methods> container
            if (auto* staticElem = cls->getChildByName("static_methods"))
            {
                forEachXmlChildElementWithTagName(*staticElem, m, "method")
                {
                    MethodInfo info;
                    info.name = m->getStringAttribute("name");
                    info.args = m->getStringAttribute("args");
                    info.isStatic = true;

                    // Check if it's an overload
                    juce::String overload = m->getStringAttribute("overload");
                    if (overload.isNotEmpty())
                    {
                        info.name += " [" + overload + "]";
                    }

                    methodList.add(info);
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

    // Sort methods by name
    std::sort(methodList.begin(), methodList.end(),
        [](const MethodInfo& a, const MethodInfo& b) { return a.name < b.name; });

    attributeList.sort(true);
    methodListBox->updateContent();

    int constructorCount = 0;
    for (const auto& method : methodList)
    {
        if (method.name.startsWith(className))
            constructorCount++;
    }

    infoDisplay->setText("Class: " + className + "\n\n" +
        "Constructors: " + juce::String(constructorCount) + "\n" +
        "Methods: " + juce::String(methodList.size() - constructorCount) + "\n" +
        "Enums: " + juce::String(attributeList.size()) + "\n\n" +
        "Click on any method to see usage details.",
        false);
}

juce::String CtrlrLuaClassBrowser::getMethodArgs(const juce::String& methodName) const
{
    for (const auto& method : methodList)
    {
        if (method.name == methodName)
            return method.args;
    }
    return juce::String();
}

juce::String CtrlrLuaClassBrowser::introspectMethod(
    const juce::String& className,
    const juce::String& methodName)
{
    juce::String result;

    // Find the method to check if it's static
    bool isStatic = false;
    juce::String args;

    for (const auto& method : methodList)
    {
        if (method.name == methodName)
        {
            isStatic = method.isStatic;
            args = method.args;
            break;
        }
    }

    if (isStatic)
    {
        result = "STATIC method (use .)\n";
    }
    else
    {
        result = "INSTANCE method (use :)\n";
    }

    result += "Type: Method (function)\n";

    if (args.isNotEmpty())
        result += "Args: " + args + "\n";
    else
        result += "Args: ()\n";

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
    // Find method info
    bool isStatic = false;
    juce::String args;

    for (const auto& method : methodList)
    {
        if (method.name == methodName)
        {
            isStatic = method.isStatic;
            args = method.args.isNotEmpty() ? method.args : "()";
            break;
        }
    }

    juce::String example = generateExampleFunction(className, methodName, isStatic);
    juce::SystemClipboard::copyTextToClipboard(example);

    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "Example Function Copied",
        "Example function copied to clipboard.\n\n" +
        juce::String(isStatic ? "STATIC" : "INSTANCE") + " method\n" +
        "Arguments: " + (args.isEmpty() ? "()" : args) + "\n\n" +
        "Paste into your Lua code and customize."
    );
}

juce::String CtrlrLuaClassBrowser::generateExampleFunction(const juce::String& className,
    const juce::String& methodName,
    bool isStatic)
{
    juce::String example;

    // Find method args
    juce::String args;
    for (const auto& method : methodList)
    {
        if (method.name == methodName)
        {
            args = method.args;
            isStatic = method.isStatic;
            break;
        }
    }

    // Extract parameter names from args like "(int index)" -> "index"
    juce::String paramList = args.isEmpty() ? "" : args.fromFirstOccurrenceOf("(", false, false)
        .upToLastOccurrenceOf(")", false, false);

    bool isMethod = !methodList.isEmpty();
    bool hasParams = paramList.isNotEmpty() && paramList != " ";

    example += "-- ==================================================\n";
    example += "-- Example function using " + className + "." + methodName + "\n";
    example += "-- " + juce::String(isStatic ? "Static method (use .)" : "Instance method (use :)") + "\n";
    if (hasParams)
        example += "-- Arguments: " + args + "\n";
    example += "-- ==================================================\n\n";

    juce::String functionName = "example" + methodName.substring(0, 1).toUpperCase() + methodName.substring(1);

    if (className.contains("Panel"))
    {
        if (isStatic)
        {
            example += "function " + functionName + "()\n";
            example += "    local result = " + className + "." + methodName + (hasParams ? "(" + paramList + ")" : "()") + "\n";
            example += "    console(\"Result: \" .. tostring(result))\n";
            example += "    return result\n";
            example += "end\n";
        }
        else
        {
            if (hasParams)
                example += "function " + functionName + "(" + paramList + ")\n";
            else
                example += "function " + functionName + "()\n";

            if (isMethod)
            {
                example += "    local result = panel:" + methodName + (hasParams ? "(" + paramList + ")" : "()") + "\n";
                example += "    console(\"Result: \" .. tostring(result))\n";
                example += "    return result\n";
            }
            else
            {
                example += "    local value = panel." + methodName + "\n";
                example += "    console(\"Current value: \" .. tostring(value))\n";
            }
            example += "end\n";
        }
    }
    else if (className.contains("Modulator"))
    {
        if (hasParams)
            example += "function " + functionName + "(modulatorName, " + paramList + ")\n";
        else
            example += "function " + functionName + "(modulatorName)\n";

        example += "    local modulator = panel:getModulatorByName(modulatorName)\n";
        example += "    if not modulator then\n";
        example += "        console(\"Modulator not found: \" .. modulatorName)\n";
        example += "        return\n";
        example += "    end\n\n";

        if (isStatic)
        {
            example += "    local result = " + className + "." + methodName + (hasParams ? "(" + paramList + ")" : "()") + "\n";
        }
        else if (isMethod)
        {
            example += "    local result = modulator:" + methodName + (hasParams ? "(" + paramList + ")" : "()") + "\n";
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
        if (hasParams)
            example += "function " + functionName + "(" + paramList + ")\n";
        else
            example += "function " + functionName + "()\n";

        if (isStatic)
        {
            example += "    local result = " + className + "." + methodName + (hasParams ? "(" + paramList + ")" : "()") + "\n";
            example += "    console(\"Result: \" .. tostring(result))\n";
            example += "    return result\n";
        }
        else
        {
            example += "    local m = " + className + "()\n\n";
            if (isMethod)
            {
                example += "    local result = m:" + methodName + (hasParams ? "(" + paramList + ")" : "()") + "\n";
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
    if (hasParams)
        example += "-- " + functionName + "(" + paramList + ")\n";
    else
        example += "-- " + functionName + "()\n";

    return example;
}

juce::String CtrlrLuaClassBrowser::generateLuaUsageForMethod(const juce::String& className,
    const juce::String& methodName)
{
    juce::String result;

    // Find method info
    bool isStatic = false;
    juce::String args;
    bool isMethod = false;

    for (const auto& method : methodList)
    {
        if (method.name == methodName)
        {
            isStatic = method.isStatic;
            args = method.args.isNotEmpty() ? method.args : "()";
            isMethod = true;
            break;
        }
    }

    result += "-- Class: " + className + "\n";
    result += juce::String("-- ") + (isMethod ? "Method" : "Attribute") + ": " + methodName;

    if (isMethod && args.isNotEmpty())
        result += " " + args;

    result += "\n\n";

    if (className == "CtrlrPanel" || className == "panel")
    {
        if (isMethod)
        {
            result += "local result = panel:" + methodName + args + "\n";
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
            result += "local result = modulator:" + methodName + args + "\n";
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
            result += "local result = component:" + methodName + args + "\n";
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
            result += "local result = obj:" + methodName + args + "\n";
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

    // Strip overload markers like [1], [2] for display
    juce::String cleanName = methodName.upToFirstOccurrenceOf(" [", false, false);

    description += "Class: " + currentClassName + "\n";
    description += "Method: " + cleanName + "\n";

    // Find method info
    bool isStatic = false;
    juce::String args;
    bool found = false;
    bool isConstructor = false;

    for (const auto& method : methodList)
    {
        if (method.name == methodName)
        {
            isStatic = method.isStatic;
            args = method.args;
            found = true;
            isConstructor = cleanName == currentClassName;
            break;
        }
    }

    if (found)
    {
        description += "\n=== Method Info ===\n";

        if (isConstructor)
        {
            description += "Type: CONSTRUCTOR\n";
        }
        else
        {
            description += "Type: " + juce::String(isStatic ? "STATIC (use .)" : "INSTANCE (use :)") + "\n";
        }

        if (args.isNotEmpty())
            description += "Arguments: " + args + "\n";
        else
            description += "Arguments: ()\n";

        description += "\n=== Usage ===\n";

        if (isConstructor)
        {
            description += "local obj = " + currentClassName + args + "\n";
        }
        else if (isStatic)
        {
            description += currentClassName + "." + cleanName + args + "\n";
        }
        else
        {
            description += "obj:" + cleanName + args + "\n";
        }
    }
    else
    {
        description += "\nType: Enum/Attribute\n";
    }

    description += "\nRight-click for more options.";

    return description;
}

void CtrlrLuaClassBrowser::MethodListModel::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    if (!ownerRef) return;

    juce::String itemName;
    juce::String args;
    bool isMethod = (row < ownerRef->methodList.size());
    bool isStatic = false;

    if (isMethod)
    {
        const auto& method = ownerRef->methodList[row];
        itemName = method.name;
        args = method.args;
        isStatic = method.isStatic;

        // Update info display when method is clicked
        ownerRef->infoDisplay->setText(ownerRef->getMethodDescription(itemName), false);
    }
    else
    {
        itemName = ownerRef->attributeList[row - ownerRef->methodList.size()];

        // Update info for attributes
        ownerRef->infoDisplay->setText(
            "Class: " + ownerRef->currentClassName + "\n" +
            "Attribute: " + itemName + "\n\n" +
            "Type: Enum/Attribute\n" +
            "Usage: " + ownerRef->currentClassName + "." + itemName,
            false);
    }

    if (e.mods.isRightButtonDown())
    {
        juce::PopupMenu menu;

        menu.addItem(1, "Copy Class Name");
        menu.addItem(2, "Copy Method/Attribute Name");

        if (isMethod && args.isNotEmpty())
            menu.addItem(3, "Copy with Arguments: " + itemName + args);
        else
            menu.addItem(3, "Copy Class and Method");

        menu.addItem(4, "Copy Full Usage Example");

        menu.showMenuAsync(juce::PopupMenu::Options(), [this, itemName, args, isMethod, isStatic](int result)
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
                    {
                        combined += (isStatic ? "." : ":") + itemName;
                        if (args.isNotEmpty())
                            combined += args;
                        else
                            combined += "()";
                    }
                    else
                    {
                        combined += "." + itemName;
                    }
                    juce::SystemClipboard::copyTextToClipboard(combined);
                    break;
                }

                case 4:
                {
                    juce::String example;
                    if (isMethod)
                    {
                        juce::String argsToUse = args.isNotEmpty() ? args : "()";

                        if (isStatic)
                        {
                            example = "-- Static method\n";
                            example += "local result = " + ownerRef->currentClassName + "." + itemName + argsToUse + "\n";
                        }
                        else
                        {
                            example = "-- Instance method\n";

                            if (ownerRef->currentClassName.contains("Panel"))
                            {
                                example += "local result = panel:" + itemName + argsToUse + "\n";
                            }
                            else if (ownerRef->currentClassName.contains("Modulator"))
                            {
                                example += "local mod = panel:getModulatorByName(\"modulatorName\")\n";
                                example += "local result = mod:" + itemName + argsToUse + "\n";
                            }
                            else
                            {
                                example += "local obj = " + ownerRef->currentClassName + "()\n";
                                example += "local result = obj:" + itemName + argsToUse + "\n";
                            }
                        }
                    }
                    else
                    {
                        example = "-- Attribute/Enum\n";
                        example += "local value = " + ownerRef->currentClassName + "." + itemName + "\n";
                    }

                    juce::SystemClipboard::copyTextToClipboard(example);
                    break;
                }
                }
            });
    }
}