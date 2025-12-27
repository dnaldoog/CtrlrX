#include "CtrlrLuaClassBrowser.h"
#include "CtrlrLuaManager.h"

CtrlrLuaClassBrowser::CtrlrLuaClassBrowser(CtrlrLuaManager* luaManager)
    : luaManagerRef(luaManager)
{
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
    infoDisplay->setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
    infoDisplay->setText("Click on a class to see its methods and attributes.\n"
        "Click on a method/attribute to copy Lua usage code.", false);
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
    g.drawText("Available Classes", 5, 5, 200, 20, juce::Justification::left);

    const int splitPos = getWidth() / 3;
    g.drawText("Methods & Attributes", splitPos + 5, 5, 300, 20, juce::Justification::left);

    // Draw divider line
    g.setColour(juce::Colours::grey);
    g.drawLine(splitPos, 0, splitPos, getHeight(), 1.0f);
}

void CtrlrLuaClassBrowser::resized()
{
    auto bounds = getLocalBounds();
    const int splitPos = bounds.getWidth() / 3;
    const int topMargin = 30;
    const int infoHeight = 150;

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
        classList.sort(true);
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
    // TODO: Load from CtrlrIDs.xml if available
    // For now, return basic info
    return "Method: " + methodName + "\n"
        "Class: " + currentClassName + "\n\n"
        "Lua usage code has been copied to clipboard.\n"
        "Paste into the Lua editor.";
}

void CtrlrLuaClassBrowser::copyMethodUsageToClipboard(const juce::String& className,
    const juce::String& methodName)
{
    juce::String luaCode = generateLuaUsageForMethod(className, methodName);

    juce::SystemClipboard::copyTextToClipboard(luaCode);

    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "Lua Usage Copied",
        "Copied to clipboard:\n\n" + luaCode.substring(0, 100) + "...\n\n"
        "Paste into the Lua editor.\n"
        "Guide only – please double-check syntax."
    );
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