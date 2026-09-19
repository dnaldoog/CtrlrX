/*
  ==============================================================================

    CtrlrLuaApiInspector.cpp
    Created: 19 Sep 2026 4:01:10pm
    Author:  zan64

  ==============================================================================
*/
# include "stdafx.h"
#include "CtrlrLuaApiInspector.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrLuaManager.h"
#include "luabind/luabind.hpp"

CtrlrLuaApiInspector::CtrlrLuaApiInspector(CtrlrPanel& _owner)
    : owner(_owner)
{
    // Class Input
    addAndMakeVisible(classLabel);
    addAndMakeVisible(classInput);
    classInput.setTextToShowWhenEmpty("e.g. MemoryBlock", juce::Colours::grey);
    classInput.addListener(this);

    // Inspect Button
    addAndMakeVisible(inspectButton);
    inspectButton.onClick = [this] { inspectClass(classInput.getText().trim()); };

    // List All Button
    addAndMakeVisible(listAllButton);
    listAllButton.onClick = [this] { listAllClasses(); };
// Add Clear Button
addAndMakeVisible(clearButton);
clearButton.onClick = [this] 
{ 
    classInput.clear();
    filterInput.clear();
    rawOutput = "";
    outputDisplay.clear();
};
    // Filter Input
    addAndMakeVisible(filterLabel);
    addAndMakeVisible(filterInput);
    filterInput.setTextToShowWhenEmpty("Filter methods...", juce::Colours::grey);
    filterInput.addListener(this);

    // Documentation Link
    addAndMakeVisible(docsButton);
    docsButton.onClick = [this] { openGithubDocs(); };

    // Output Display (Monospaced Read-Only)
    addAndMakeVisible(outputDisplay);
    outputDisplay.addMouseListener(this, false);
    outputDisplay.setMultiLine(true, true);
    outputDisplay.setReadOnly(true);
    outputDisplay.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    outputDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    outputDisplay.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    outputDisplay.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
}

void CtrlrLuaApiInspector::resized()
{
    auto area = getLocalBounds().reduced(6);
    constexpr int rowHeight = 28;
    constexpr int gap = 6;

    // Top Row: Class Search Controls
    auto topRow = area.removeFromTop(rowHeight);
    classLabel.setBounds(topRow.removeFromLeft(90));
    classInput.setBounds(topRow.removeFromLeft(160));
    topRow.removeFromLeft(gap);
    inspectButton.setBounds(topRow.removeFromLeft(70));
    topRow.removeFromLeft(gap);
    listAllButton.setBounds(topRow.removeFromLeft(110));
    topRow.removeFromLeft(gap);
    clearButton.setBounds(topRow.removeFromLeft(60)); // Clear button added

    area.removeFromTop(gap);

    // Middle Row: Filtering and Docs
    auto filterRow = area.removeFromTop(rowHeight);
    filterLabel.setBounds(filterRow.removeFromLeft(90));
    filterInput.setBounds(filterRow.removeFromLeft(160));
    filterRow.removeFromLeft(gap);
    docsButton.setBounds(filterRow.removeFromLeft(100));

    area.removeFromTop(gap);

    // Output Display
    outputDisplay.setBounds(area);
}

void CtrlrLuaApiInspector::paint(juce::Graphics& g)
{
    g.fillAll(findColour(juce::ResizableWindow::backgroundColourId));
}

void CtrlrLuaApiInspector::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    if (&editor == &classInput)
        inspectClass(classInput.getText().trim());
}
void CtrlrLuaApiInspector::mouseDoubleClick(const juce::MouseEvent& event)
{
    if (event.eventComponent != &outputDisplay)
        return;

    // 1. Get raw selected word and trim whitespace/newlines
    juce::String selectedWord = outputDisplay.getHighlightedText().trim();

    // Clean up any non-alphanumeric/underscore characters around the word
    selectedWord = selectedWord.initialSectionContainingOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

    if (selectedWord.isEmpty())
        return;

    const juce::String currentClass = classInput.getText().trim();

    // 2. If we are currently viewing "List All Classes", treat double-click as selecting a class name
    if (currentClass.isEmpty() || rawOutput.startsWith("Available Classes"))
    {
        classInput.setText(selectedWord, juce::dontSendNotification);
        inspectClass(selectedWord);
        return;
    }

    // 3. Check if selectedWord is a method or property on the CURRENT class
    juce::String luaCheckScript;
    luaCheckScript << "if " << currentClass << " ~= nil and " << currentClass << "." << selectedWord << " ~= nil then "
                   << "return 'FOUND' "
                   << "else return 'NOT_FOUND' end";

    juce::String checkResult = runLuaAndGetResult(luaCheckScript);

    if (checkResult.contains("FOUND"))
    {
        // It's a method/attribute on this class! Filter the output to show just this item
        filterInput.setText(selectedWord, juce::sendNotification);
    }
    else
    {
        // It's not a method on the current class, so test if it's a valid top-level global class/symbol
        juce::String globalCheckScript;
        globalCheckScript << "if " << selectedWord << " ~= nil then return 'GLOBAL_EXISTS' else return 'GLOBAL_NIL' end";

        if (runLuaAndGetResult(globalCheckScript).contains("GLOBAL_EXISTS"))
        {
            classInput.setText(selectedWord, juce::dontSendNotification);
            inspectClass(selectedWord);
        }
    }
}

void CtrlrLuaApiInspector::openGithubDocs()
{
    juce::String url = "https://github.com/sgorpi/CtrlrX/blob/documentation/Doc/manual/lua/01-lua-guide.md"; // Target wiki/docs URL
    const juce::String query = classInput.getText().trim();

    if (query.isNotEmpty())
        url << "/_search?q=" << juce::URL::addEscapeChars(query, true);

    juce::URL(url).launchInDefaultBrowser();
}


juce::String CtrlrLuaApiInspector::runLuaAndGetResult(const juce::String& luaScript)
{
    lua_State* L = owner.getCtrlrLuaManager().getLuaState();
    if (!L)
        return "Error: Lua state is null.";

    // Track stack depth before execution to clean up properly
    const int topBefore = lua_gettop(L);

    // Run the Lua code string
    if (luaL_dostring(L, luaScript.toRawUTF8()) != 0)
    {
        // On error, luaL_dostring leaves error string on top of stack
        juce::String errorMsg = lua_tostring(L, -1);
        lua_settop(L, topBefore); // Restore stack
        return "Lua Error: " + errorMsg;
    }

    // Check if the script returned a value on the stack
    if (lua_gettop(L) > topBefore)
    {
        juce::String result;
        if (lua_isstring(L, -1))
        {
            result = lua_tostring(L, -1);
        }
        else
        {
            result = "Execution succeeded (non-string return value).";
        }

        lua_settop(L, topBefore); // Clean up return value from stack
        return result;
    }

    lua_settop(L, topBefore);
    return "No string returned.";
}

void CtrlrLuaApiInspector::textEditorTextChanged(juce::TextEditor& editor)
{
	// Handle the Filter input box separately
	if (&editor == &filterInput) {
		applyFilter();
		return;
	}

	// Handle Class search autocomplete
	if (&editor == &classInput) {
		if (isAutoCompleting)
			return;

		const juce::String query = classInput.getText().trim();

		// Don't show popup if query is too short
		if (query.length() < 2)
			return;

		// Ensure cache is populated
		if (classNamesCache.isEmpty())
			updateClassCache();

		// Pair each class name with its fuzzy match score
		struct Match {
				juce::String name;
				double score;
		};
		std::vector<Match> matches;

		const std::string queryStd = query.toStdString();

		for (const auto &className : classNamesCache) {
			const std::string nameStd = className.toStdString();

			// RapidFuzz score between query and class name (0.0 to 100.0)
			double score = rapidfuzz::fuzz::partial_ratio(queryStd, nameStd);

			// Boost score if the class name starts directly with the query string
			if (className.startsWithIgnoreCase(query))
				score += 20.0;

			if (score > 60.0) // Threshold for relevant matches
			{
				matches.push_back({className, score});
			}
		}

		if (matches.empty())
			return;

		// Sort matches by highest score first
		std::sort(matches.begin(), matches.end(), [](const Match &a, const Match &b) { return a.score > b.score; });

		// Construct Autocomplete Popup Menu
		juce::PopupMenu menu;
		int itemID = 1;

		// Show top 8 matches
		const size_t limit = std::min<size_t>(matches.size(), 8);
		for (size_t i = 0; i < limit; ++i) {
			menu.addItem(itemID++, matches[i].name);
		}

		// Display popup directly under classInput
		auto options = juce::PopupMenu::Options().withTargetComponent(&classInput).withItemThatMustBeVisible(1);

		menu.showMenuAsync(options, [this, matches](int result) {
			if (result > 0 && static_cast<size_t>(result - 1) < matches.size()) {
				const juce::String selectedClass = matches[result - 1].name;

				isAutoCompleting = true;
				classInput.setText(selectedClass, juce::dontSendNotification);
				isAutoCompleting = false;

				// Trigger inspection directly on selection!
				inspectClass(selectedClass);
			}
		});
	}
}

void CtrlrLuaApiInspector::applyFilter()
{
    const juce::String filterText = filterInput.getText().trim().toLowerCase();

    if (filterText.isEmpty())
    {
        outputDisplay.setText(rawOutput, false);
        return;
    }

    // Split raw output into lines and filter matching method/attribute names
    juce::StringArray lines;
    lines.addLines(rawOutput);

    juce::String filteredResult;
    for (const auto& line : lines)
    {
        // Keep structure headers and line dividers intact while filtering content lines
        if (line.contains("---") || 
            line.startsWith("Object type") || 
            line.startsWith("Members:") || 
            line.startsWith("Attributes:") || 
            line.toLowerCase().contains(filterText))
        {
            filteredResult << line << "\n";
        }
    }

    outputDisplay.setText(filteredResult, false);
}
// void CtrlrLuaApiInspector::listAllClasses() // simple no hyperlink
// {
//     // Evaluates luabind's class_names() directly and formats a plain text string
//     const juce::String luaScript = 
//         "local names = class_names()\n"
//         "if names == nil then return 'Error: class_names() returned nil' end\n"
//         "local ret = 'Available Classes:\\n-----------------------------------------------------------------\\n'\n"
//         "for i, v in ipairs(names) do\n"
//         "    ret = ret .. string.format('  %s\\n', tostring(v))\n"
//         "end\n"
//         "return ret .. '-----------------------------------------------------------------'";

//     rawOutput = runLuaAndGetResult(luaScript);
//     applyFilter();
// }

void CtrlrLuaApiInspector::listAllClasses() // create links
{
    const juce::String luaScript = 
        "local names = class_names()\n"
        "if names == nil then return 'Error: class_names() returned nil' end\n"
        "local ret = 'Available Classes (Click any to inspect):\\n-----------------------------------------------------------------\\n'\n"
        "for i, v in ipairs(names) do\n"
        "    ret = ret .. string.format('  %s\\n', tostring(v))\n"
        "end\n"
        "return ret .. '-----------------------------------------------------------------'";

    rawOutput = runLuaAndGetResult(luaScript);
    applyFilter();
}

void CtrlrLuaApiInspector::updateClassCache() {
	classNamesCache.clear();

	// Query class_names() from Lua
	const juce::String luaScript = "local names = class_names()\n"
								   "if names == nil then return '' end\n"
								   "local ret = ''\n"
								   "for _, v in ipairs(names) do ret = ret .. tostring(v) .. '\\n' end\n"
								   "return ret";

	juce::String result = runLuaAndGetResult(luaScript);
	classNamesCache.addLines(result);
	classNamesCache.removeEmptyStrings();
}

void CtrlrLuaApiInspector::inspectClass(const juce::String &className) {
	if (className.isEmpty())
		return;

	// Direct JUCE string concatenation (No printf/formatted specifiers)
	juce::String luaScript;
	luaScript << "if " << className << " ~= nil then "
			  << "return what(" << className << ") "
			  << "else return 'Error: Global symbol [" << className << "] is nil or uninitialized.' end";

	rawOutput = runLuaAndGetResult(luaScript);
	applyFilter();
}

// void CtrlrLuaApiInspector::inspectClass(const juce::String &className) {
// 	if (className.isEmpty())
// 		return;

// 	juce::String luaScript;
// 	luaScript << "local className = '" << className << "'\n"
// 			  << "local cls = _G[className]\n"
// 			  << "if cls == nil then\n"
// 			  << "    return 'Error: Global class or symbol [' .. className .. '] is nil or uninitialized.'\n"
// 			  << "end\n"
// 			  << "\n"
// 			  << "local statics = {}\n"
// 			  << "local instances = {}\n"
// 			  << "\n"
// 			  << "local info = class_info(cls)\n"
// 			  << "if info and info.methods then\n"
// 			  << "    for name, _ in pairs(info.methods) do\n"
// 			  << "        local isStatic = false\n"
// 			  << "        pcall(function()\n"
// 			  << "            if type(cls[name]) == 'function' then\n"
// 			  << "                isStatic = true\n"
// 			  << "            end\n"
// 			  << "        end)\n"
// 			  << "\n"
// 			  << "        if isStatic then\n"
// 			  << "            table.insert(statics, name)\n"
// 			  << "        else\n"
// 			  << "            table.insert(instances, name)\n"
// 			  << "        end\n"
// 			  << "    end\n"
// 			  << "end\n"
// 			  << "\n"
// 			  << "table.sort(statics)\n"
// 			  << "table.sort(instances)\n"
// 			  << "\n"
// 			  << "local ret = 'Object type [' .. (info and info.name or className) .. ']\\n'\n"
// 			  << "ret = ret .. '-----------------------------------------------------------------\\n\\n'\n"
// 			  << "\n"
// 			  << "ret = ret .. 'Static / Class Methods (Call as ' .. className .. '.method()):\\n'\n"
// 			  << "if #statics == 0 then\n"
// 			  << "    ret = ret .. '  (None)\\n'\n"
// 			  << "else\n"
// 			  << "    for _, name in ipairs(statics) do\n"
// 			  << "        ret = ret .. string.format('  [Static]   %s\\n', name)\n"
// 			  << "    end\n"
// 			  << "end\n"
// 			  << "\n"
// 			  << "ret = ret .. '\\n'\n"
// 			  << "ret = ret .. 'Instance Methods (Call as instance:method()):\\n'\n"
// 			  << "if #instances == 0 then\n"
// 			  << "    ret = ret .. '  (None)\\n'\n"
// 			  << "else\n"
// 			  << "    for _, name in ipairs(instances) do\n"
// 			  << "        ret = ret .. string.format('  [Instance] %s\\n', name)\n"
// 			  << "    end\n"
// 			  << "end\n"
// 			  << "\n"
// 			  << "if info and info.attributes and next(info.attributes) ~= nil then\n"
// 			  << "    ret = ret .. '\\nAttributes / Properties:\\n'\n"
// 			  << "    local attrs = {}\n"
// 			  << "    for name, _ in pairs(info.attributes) do table.insert(attrs, name) end\n"
// 			  << "    table.sort(attrs)\n"
// 			  << "    for _, name in ipairs(attrs) do\n"
// 			  << "        ret = ret .. string.format('  [Property] %s\\n', name)\n"
// 			  << "    end\n"
// 			  << "end\n"
// 			  << "\n"
// 			  << "ret = ret .. '-----------------------------------------------------------------'\n"
// 			  << "return ret\n";

// 	rawOutput = runLuaAndGetResult(luaScript);
// 	applyFilter();
// }