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

    // Create info display for method descriptions
    infoDisplay = std::make_unique<juce::TextEditor>();
    infoDisplay->setMultiLine(true);
    infoDisplay->setReadOnly(true);
    infoDisplay->setScrollbarsShown(true);
    infoDisplay->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xfff8f8f8));
    infoDisplay->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    infoDisplay->setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
    infoDisplay->setText("Click on a class to see its methods and attributes.\n"
        "Click on a method/attribute to copy Lua usage code.\n"
        "Right-click for more options.", false);
    addAndMakeVisible(infoDisplay.get());

    // Load the class list
	//loadClassList(); Done in header after luaManagerRef is set
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

    const int splitPos = getWidth() / 3;

    // Draw label for class list (below search box)
    g.drawText("Available Classes", 5, 35, 200, 20, juce::Justification::left);

    // Draw label for methods list
    g.drawText("Methods & Attributes", splitPos + 5, 35, 300, 20, juce::Justification::left);

    // Draw divider line
    g.setColour(juce::Colours::grey);
    g.drawLine((float)splitPos, 0, (float)splitPos, (float)getHeight(), 1.0f);
}

void CtrlrLuaClassBrowser::resized()
{
    auto bounds = getLocalBounds();
    const int splitPos = bounds.getWidth() / 3;
    const int topMargin = 65;  // Space for search controls
    const int infoHeight = 150;  // Taller info panel for better visibility
    const int buttonHeight = 25;

    // Search and refresh controls at top
    searchBox->setBounds(5, 5, splitPos - 90, buttonHeight);
    refreshButton->setBounds(splitPos - 80, 5, 75, buttonHeight);

    // Label area
    auto labelY = 35;

    // Left side: class list
    classListBox->setBounds(5, topMargin, splitPos - 10, bounds.getHeight() - topMargin - 5);

    // Right side: method list and info display
    methodListBox->setBounds(splitPos + 5, topMargin,
        bounds.getWidth() - splitPos - 10,
        bounds.getHeight() - topMargin - infoHeight - 10);

    infoDisplay->setBounds(splitPos + 5,
        bounds.getHeight() - infoHeight - 5,
        bounds.getWidth() - splitPos - 10,
        infoHeight);
}

void CtrlrLuaClassBrowser::loadClassList()
{
    classList.clear();

    DBG("loadClassList called");
    DBG("luaApiXml is: " + juce::String(luaApiXml ? "valid" : "NULL"));

    if (!luaApiXml)
    {
        infoDisplay->setText("ERROR: No XML data provided!\n"
            "Make sure LuaAPI.xml was loaded.", false);
        return;
    }

    DBG("XML root tag: " + luaApiXml->getTagName());
    DBG("XML has " + juce::String(luaApiXml->getNumChildElements()) + " children");

    forEachXmlChildElementWithTagName(*luaApiXml, cls, "class")
    {
        juce::String className = cls->getStringAttribute("name");
        DBG("Found class: " + className);
        classList.add(className);
    }

    DBG("Total classes found: " + juce::String(classList.size()));

    classList.removeDuplicates(false);
    classList.sort(true);
    fullClassList = classList;
    classListBox->updateContent();

    if (classList.isEmpty())
    {
        infoDisplay->setText("WARNING: No classes found in XML!\n"
            "Check LuaAPI.xml file structure.", false);
    }
    else
    {
        infoDisplay->setText("Loaded " + juce::String(classList.size()) + " classes.\n"
            "Click on a class to see methods.", false);
    }
}


void CtrlrLuaClassBrowser::loadMethodsForClass(const juce::String& className)
{
    methodList.clear();
    attributeList.clear();
    currentClassName = className;

    if (!luaApiXml)
        return;

    // Find the class by name attribute
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
// Introspect method using Lua - simplified to avoid crashes
juce::String CtrlrLuaClassBrowser::introspectMethod(
    const juce::String& className,
    const juce::String& methodName)
{
    juce::String result;
    bool isStatic = false;

    // Check naming patterns
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

    // Check if it's a method or attribute
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

// Copy example function to clipboard
void CtrlrLuaClassBrowser::copyExampleToClipboard(const juce::String& className,
    const juce::String& methodName)
{
    // Use heuristic to detect static vs instance
    bool isStatic = false;
    juce::String lowerMethod = methodName.toLowerCase();

    // Method naming patterns that indicate static methods
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

// Generate example function
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

    // Create function name with first letter capitalized
    juce::String functionName = "example" + methodName.substring(0, 1).toUpperCase() + methodName.substring(1);

    // Generate appropriate example based on class type and static/instance
    if (className.contains("Panel"))
    {
        if (isStatic)
        {
            example += "function " + functionName + "()\n";
            example += "    -- Static method - call on class directly\n";
            example += "    local result = " + className + "." + methodName + "()\n";
            example += "    console(\"Result: \" .. tostring(result))\n";
            example += "    return result\n";
            example += "end\n";
        }
        else
        {
            example += "function " + functionName + "()\n";
            example += "    -- Instance method - call on panel object\n";
            if (isMethod)
            {
                example += "    local result = panel:" + methodName + "()\n";
                example += "    console(\"Result: \" .. tostring(result))\n";
                example += "    return result\n";
            }
            else
            {
                example += "    -- Get attribute\n";
                example += "    local value = panel." + methodName + "\n";
                example += "    console(\"Current value: \" .. tostring(value))\n";
                example += "    \n";
                example += "    -- Set attribute\n";
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
            example += "    -- Static method\n";
            example += "    local result = " + className + "." + methodName + "()\n";
        }
        else if (isMethod)
        {
            example += "    -- Instance method\n";
            example += "    local result = modulator:" + methodName + "()\n";
            example += "    console(\"Result: \" .. tostring(result))\n";
            example += "    return result\n";
        }
        else
        {
            example += "    -- Get/Set attribute\n";
            example += "    local value = modulator." + methodName + "\n";
            example += "    console(\"Value: \" .. tostring(value))\n";
        }
        example += "end\n";
    }
    else
    {
        // Generic example
        example += "function " + functionName + "()\n";

        if (isStatic)
        {
            example += "    -- Static method - use dot (.)\n";
            example += "    local result = " + className + "." + methodName + "()\n";
            example += "    console(\"Result: \" .. tostring(result))\n";
            example += "    return result\n";
        }
        else
        {
            example += "    -- Create instance with constructor\n";
            example += "    local m = " + className + "() -- or with params: " + className + "(param1, param2)\n\n";
            example += "    -- Instance method - use colon (:)\n";
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

    // Check if it's a method or attribute
    bool isMethod = methodList.contains(methodName);

    result += "-- Class: " + className + "\n";
    result += juce::String("-- ") + (isMethod ? "Method" : "Attribute") + ": " + methodName + "\n\n";

    // Generate appropriate usage based on common Ctrlr patterns
    if (className == "CtrlrPanel" || className == "panel")
    {
        if (isMethod)
        {
            result += "-- Example usage:\n";
            result += "local result = panel:" + methodName + "()\n";
        }
        else
        {
            result += "-- Get attribute:\n";
            result += "local value = panel." + methodName + "\n\n";
            result += "-- Set attribute:\n";
            result += "panel." + methodName + " = newValue\n";
        }
    }
    else if (className == "CtrlrModulator" || className.startsWith("Modulator"))
    {
        if (isMethod)
        {
            result += "-- Example usage:\n";
            result += "local modulator = panel:getModulatorByName(\"modulatorName\")\n";
            result += "local result = modulator:" + methodName + "()\n";
        }
        else
        {
            result += "-- Get attribute:\n";
            result += "local modulator = panel:getModulatorByName(\"modulatorName\")\n";
            result += "local value = modulator." + methodName + "\n";
        }
    }
    else if (className == "CtrlrComponent" || className.contains("Component"))
    {
        if (isMethod)
        {
            result += "-- Example usage:\n";
            result += "local component = panel:getModulatorByName(\"modulatorName\"):getComponent()\n";
            result += "local result = component:" + methodName + "()\n";
        }
        else
        {
            result += "-- Get attribute:\n";
            result += "local component = panel:getModulatorByName(\"modulatorName\"):getComponent()\n";
            result += "local value = component." + methodName + "\n";
        }
    }
    else
    {
        // Generic usage for other classes
        if (isMethod)
        {
            result += "-- Example usage:\n";
            result += "local obj = " + className + ":new() -- or obtain reference\n";
            result += "local result = obj:" + methodName + "()\n";
        }
        else
        {
            result += "-- Get attribute:\n";
            result += "local obj = " + className + ":new() -- or obtain reference\n";
            result += "local value = obj." + methodName + "\n";
        }
    }

    result += "\n-- Note: This is a guide. Check Ctrlr documentation for exact syntax.\n";

    return result;
}

// ListBoxModel implementation
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

// Filter class list based on search text
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
juce::String generateLuaSnippet(
    const CtrlrLuaApiDatabase::Class& cls,
    const juce::String& symbol,
    bool isStatic,
    bool isEnum)
{
    if (isEnum)
        return cls.name + "." + symbol;

    if (isStatic)
        return cls.name + "." + symbol + "()";

    return "obj:" + symbol + "()";
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

    // Detect static vs instance from naming patterns
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

    // Check if it's in method list
    bool isMethod = methodList.contains(methodName);
    description += "Type: " + juce::String(isMethod ? "Method (function)" : "Enum/Attribute") + "\n\n";

    description += "=== Usage ===\n";
    description += "Lua code copied to clipboard.\n";
    description += "Right-click for example function.";

    return description;
}