#include "CtrlrLuaClassBrowser.h"
#include "CtrlrLuaManager.h"

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
    loadClassList();
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

    if (!luaManagerRef)
    {
        infoDisplay->setText("Error: Lua Manager not available.", false);
        classListBox->updateContent();
        return;
    }

    // Call the Lua how() function and get result from stack
    lua_State* L = luaManagerRef->getLuaState();
    if (!L)
    {
        infoDisplay->setText("Error: Lua state not available.", false);
        return;
    }

    juce::String luaResult;

    // Execute how() and get result
    if (luaL_dostring(L, "return how()") == 0)
    {
        // Get result from top of stack
        if (lua_isstring(L, -1))
        {
            luaResult = juce::String(lua_tostring(L, -1));
        }
        lua_pop(L, 1); // Clean up stack
    }
    else
    {
        // Error executing
        if (lua_isstring(L, -1))
        {
            juce::String error = juce::String(lua_tostring(L, -1));
            infoDisplay->setText("Error calling how():\n" + error, false);
            lua_pop(L, 1);
        }
        return;
    }

    if (luaResult.isEmpty())
    {
        infoDisplay->setText("Warning: The how() function returned no data.\n"
            "Make sure the Lua utilities are loaded.", false);
        return;
    }

    // Parse the result - extract class names between the separator lines
    juce::StringArray lines;
    lines.addLines(luaResult);

    bool inClassSection = false;

    for (const auto& line : lines)
    {
        juce::String trimmed = line.trim();

        if (trimmed.contains("---------"))
        {
            inClassSection = !inClassSection;
            continue;
        }

        if (inClassSection && trimmed.isNotEmpty() && !trimmed.startsWith("Available"))
        {
            classList.add(trimmed);
        }
    }

    if (classList.isEmpty())
    {
        infoDisplay->setText("Warning: No classes found.\n"
            "The how() function may have failed or returned unexpected format.", false);
    }
    else
    {
        classList.removeDuplicates(false);  // Remove duplicate entries
        classList.sort(true);
        fullClassList = classList;  // Store unfiltered list for search
        infoDisplay->setText("Loaded " + juce::String(classList.size()) +
            " Lua classes.\n\n"
            "Click on a class to see its methods and attributes.", false);
    }

    classListBox->updateContent();
}

void CtrlrLuaClassBrowser::loadMethodsForClass(const juce::String& className)
{
    if (!luaManagerRef || className.isEmpty())
        return;

    currentClassName = className;
    methodList.clear();
    attributeList.clear();

    lua_State* L = luaManagerRef->getLuaState();
    if (!L)
    {
        infoDisplay->setText("Error: Lua state not available.", false);
        return;
    }

    juce::String luaResult;

    // Execute what(className) and get result
    juce::String luaCode = "return what(" + className + ")";
    if (luaL_dostring(L, luaCode.toRawUTF8()) == 0)
    {
        // Get result from top of stack
        if (lua_isstring(L, -1))
        {
            luaResult = juce::String(lua_tostring(L, -1));
        }
        lua_pop(L, 1); // Clean up stack
    }
    else
    {
        // Error executing
        if (lua_isstring(L, -1))
        {
            juce::String error = juce::String(lua_tostring(L, -1));
            infoDisplay->setText("Error calling what(" + className + "):\n" + error, false);
            lua_pop(L, 1);
        }
        return;
    }

    if (luaResult.isEmpty() || luaResult.contains("nil"))
    {
        infoDisplay->setText("Warning: Class '" + className +
            "' returned no information.\n"
            "This class may not have accessible methods or attributes.", false);
        methodListBox->updateContent();
        return;
    }

    // Parse the result
    juce::StringArray lines;
    lines.addLines(luaResult);

    bool inMethodSection = false;
    bool inAttributeSection = false;

    for (const auto& line : lines)
    {
        juce::String trimmed = line.trim();

        if (trimmed.startsWith("Members:"))
        {
            inMethodSection = true;
            inAttributeSection = false;
            continue;
        }
        else if (trimmed.startsWith("Attributes:"))
        {
            inMethodSection = false;
            inAttributeSection = true;
            continue;
        }
        else if (trimmed.contains("--------") || trimmed.isEmpty())
        {
            continue;
        }

        // Parse method/attribute lines (format: "    methodName:    type")
        if ((inMethodSection || inAttributeSection) && trimmed.isNotEmpty())
        {
            juce::String itemName = trimmed.upToFirstOccurrenceOf(":", false, false).trim();

            if (itemName.isNotEmpty())
            {
                if (inMethodSection)
                    methodList.add(itemName);
                else if (inAttributeSection)
                    attributeList.add(itemName);
            }
        }
    }

    // Sort alphabetically
    methodList.sort(true);
    attributeList.sort(true);

    methodListBox->updateContent();

    // Update info display
    infoDisplay->setText("Class: " + className + "\n\n" +
        "Methods: " + juce::String(methodList.size()) + "\n" +
        "Attributes: " + juce::String(attributeList.size()) + "\n\n" +
        "Click on any method or attribute to copy its Lua usage code.",
        false);
}

juce::String CtrlrLuaClassBrowser::getMethodDescription(const juce::String& methodName)
{
    juce::String description;

    description += "Class: " + currentClassName + "\n";
    description += "Method/Attribute: " + methodName + "\n\n";

    // Use Lua introspection to get method info
    juce::String introspectionInfo = introspectMethod(currentClassName, methodName);

    if (introspectionInfo.isNotEmpty())
    {
        description += "=== Detection Info ===\n";
        description += introspectionInfo + "\n\n";
    }
    else
    {
        description += "=== Detection Info ===\n";
        description += "No introspection data available\n\n";
    }

    description += "=== Usage ===\n";
    description += "Lua code copied to clipboard.\n";
    description += "Right-click for example function.";

    DBG("Method description generated: " + description.substring(0, 100));

    return description;
}

// Introspect method using Lua
juce::String CtrlrLuaClassBrowser::introspectMethod(const juce::String& className,
    const juce::String& methodName)
{
    lua_State* L = luaManagerRef->getLuaState();
    if (!L)
        return "Lua state not available";

    juce::String result;
    bool isStatic = false;

    // Method naming heuristics for static detection
    juce::String lowerMethod = methodName.toLowerCase();

    if (lowerMethod.startsWith("from") ||
        lowerMethod.startsWith("create") ||
        lowerMethod == "new" ||
        lowerMethod.startsWith("wrap"))
    {
        isStatic = true;
        result = "Likely STATIC method (use .)\n";
        result += "Reason: Method name pattern '" + methodName + "' suggests factory/constructor\n";
    }
    else
    {
        result = "Likely INSTANCE method (use :)\n";
        result += "Reason: Standard method name pattern\n";
    }

    // Try to get type info - safer approach without string formatting
    // Push class name onto stack first
    lua_getglobal(L, className.toRawUTF8());
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        result += "Type: Class not accessible from Lua\n";
        return result;
    }

    // Get the method from the class
    lua_getfield(L, -1, methodName.toRawUTF8());

    if (lua_isnil(L, -1))
    {
        lua_pop(L, 2); // Pop method and class
        result += "Type: Method not found\n";
    }
    else
    {
        int methodType = lua_type(L, -1);
        switch (methodType)
        {
        case LUA_TFUNCTION:
            result += "Type: function (C function)\n";
            break;
        case LUA_TUSERDATA:
        {
            // Check if it's callable
            if (lua_getmetatable(L, -1))
            {
                lua_getfield(L, -1, "__call");
                if (!lua_isnil(L, -1))
                {
                    result += "Type: callable userdata (luabind bound)\n";
                }
                else
                {
                    result += "Type: userdata\n";
                }
                lua_pop(L, 2); // Pop __call and metatable
            }
            else
            {
                result += "Type: userdata\n";
            }
            break;
        }
        case LUA_TTABLE:
            result += "Type: table/object\n";
            break;
        default:
            result += "Type: " + juce::String(lua_typename(L, methodType)) + "\n";
            break;
        }
        lua_pop(L, 2); // Pop method and class
    }

    // Add usage guide
    result += "\n";
    if (isStatic)
    {
        result += "Usage: " + className + "." + methodName + "()\n";
        result += "Example:\n";
        result += "  local obj = " + className + "." + methodName + "(params)";
    }
    else
    {
        result += "Usage: instance:" + methodName + "()\n";
        result += "Example:\n";
        result += "  local m = " + className + "()  -- Create instance\n";
        result += "  local result = m:" + methodName + "(params)";
    }

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
    result += "-- " + juce::String(isMethod ? "Method" : "Attribute") + ": " + methodName + "\n\n";

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