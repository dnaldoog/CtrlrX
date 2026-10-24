/*
  ==============================================================================

	CtrlrLuaApiInspector.cpp
	Created: 19 Sep 2026 4:01:10pm
	Author:  zan64

  ==============================================================================
*/
#include "CtrlrLuaApiInspector.h"
#include "CtrlrLuaApiLookup.h"
#include "CtrlrLuaManager.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "luabind/luabind.hpp"
#include "stdafx.h"

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
clearButton.onClick = [this] {
	hideSuggestions();
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
// addAndMakeVisible(docsButton);
// docsButton.onClick = [this] { openGithubDocs(); };

// Output Display (Monospaced Read-Only)
addAndMakeVisible(outputDisplay);
outputDisplay.addMouseListener(this, false);
outputDisplay.setMultiLine(true, true);
outputDisplay.setReadOnly(true);
outputDisplay.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
outputDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
outputDisplay.setColour(juce::TextEditor::textColourId, juce::Colours::black);
outputDisplay.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
updateClassCache();
suggestionList.onPick = [this](int row) { acceptSuggestion(row); };
classInput.onKey = [this](const juce::KeyPress &key) { return handleClassInputKey(key); };
addChildComponent(suggestionList); // hidden until there are matches
}

void CtrlrLuaApiInspector::resized()
{
	hideSuggestions();
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
    // docsButton.setBounds(filterRow.removeFromLeft(100));

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
		updateSuggestions();
	}
}

void CtrlrLuaApiInspector::textEditorFocusLost(juce::TextEditor &editor) {
	if (&editor == &classInput)
		hideSuggestions();
}

void CtrlrLuaApiInspector::updateSuggestions() {
	const juce::String query = classInput.getText().trim();

	if (query.length() < 2) {
		hideSuggestions();
		return;
	}

	if (classNamesCache.isEmpty())
		updateClassCache();

	struct Match {
			juce::String name;
			double score;
	};
	std::vector<Match> matches;
	const std::string queryStd = query.toStdString();

	for (const auto &className : classNamesCache) {
		double score = rapidfuzz::fuzz::partial_ratio(queryStd, className.toStdString());

		if (className.equalsIgnoreCase(query))
			score += 50.0; // exact name always wins, so Enter inspects what was typed
		else if (className.startsWithIgnoreCase(query))
			score += 20.0;

		if (score > 60.0)
			matches.push_back({className, score});
	}

	if (matches.empty()) {
		hideSuggestions();
		return;
	}

	std::stable_sort(matches.begin(), matches.end(), [](const Match &a, const Match &b) { return a.score > b.score; });

	juce::StringArray items;
	for (size_t i = 0; i < std::min<size_t>(matches.size(), 8); ++i)
		items.add(matches[i].name);

	suggestionList.setItems(items);
	suggestionList.setBounds(classInput.getX(), classInput.getBottom() + 1, juce::jmax(classInput.getWidth(), 220),
							 items.size() * CtrlrApiSuggestionList::rowHeight + 2);
	suggestionList.setVisible(true);
	suggestionList.toFront(false); // false: raise it without grabbing keyboard focus
}

void CtrlrLuaApiInspector::hideSuggestions() {
	suggestionList.setVisible(false);
}

bool CtrlrLuaApiInspector::handleClassInputKey(const juce::KeyPress &key) {
	if (!suggestionList.isVisible())
		return false; // normal typing, and Enter falls through to inspectClass

	if (key == juce::KeyPress::downKey) {
		suggestionList.moveSelection(1);
		return true;
	}
	if (key == juce::KeyPress::upKey) {
		suggestionList.moveSelection(-1);
		return true;
	}
	if (key == juce::KeyPress::returnKey || key == juce::KeyPress::tabKey) {
		acceptSuggestion(suggestionList.getSelectedRow());
		return true;
	}
	if (key == juce::KeyPress::escapeKey) {
		hideSuggestions();
		return true;
	}
	return false;
}

void CtrlrLuaApiInspector::acceptSuggestion(int row) {
	if (!juce::isPositiveAndBelow(row, suggestionList.getNumItems()))
		return;

	const juce::String selectedClass = suggestionList.getItem(row);
	hideSuggestions();

	isAutoCompleting = true;
	classInput.setText(selectedClass, juce::dontSendNotification);
	isAutoCompleting = false;
	classInput.moveCaretToEnd();

	inspectClass(selectedClass);
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
    classNamesCache = CtrlrLuaApiLookup::getInstance().getAllClassNames();
#if JUCE_DEBUG // Check which luabind class names have been added to Ctrlr but are not present in classMap
	DBG("CHECK MISSING");
	const juce::String luaScript = "local names = class_names()\n"
								   "if names == nil then return '' end\n"
								   "local ret = ''\n"
								   "for _, v in ipairs(names) do ret = ret .. tostring(v) .. '\\n' end\n"
								   "return ret";

	juce::String result = runLuaAndGetResult(luaScript);
	juce::StringArray luaClasses;
	luaClasses.addLines(result);
	luaClasses.removeEmptyStrings();

	// 2. Populate your autocomplete cache from C++ DB as normal
	classNamesCache = CtrlrLuaApiLookup::getInstance().getAllClassNames();

	// 3. AUDIT: Find which Lua classes are MISSING from your hardcoded C++ database
	auto &db = CtrlrLuaApiLookup::getInstance();
	int missingCount = 0;

	for (const auto &luaClass : luaClasses) {
		if (!db.hasClass(luaClass)) {
			DBG("⚠️ LUA API AUDIT: Class '" << luaClass
										   << "' exists in Lua/luabind but is MISSING from CtrlrLuaApiLookup!");
			missingCount++;
		}
	}

	if (missingCount > 0) {
		DBG("⚠️ LUA API AUDIT SUMMARY: " << missingCount << " classes are missing from your C++ database.");
	} else {
		DBG("✅ LUA API AUDIT: All runtime classes are successfully covered in the C++ database!");
	}

#endif
}

#if JUCE_DEBUG
void CtrlrLuaApiInspector::auditClassMethods(const juce::String& className) {
    auto& db = CtrlrLuaApiLookup::getInstance();
    if (!db.hasClass(className))
        return;

    const ApiClass* apiClass = db.getClassInfo(className);

    juce::String luaScript;
    luaScript << "if " << className << " ~= nil then return what(" << className << ") else return '' end";
    juce::String runtimeWhatOutput = runLuaAndGetResult(luaScript);

    if (runtimeWhatOutput.isEmpty() || runtimeWhatOutput.startsWith("Lua Error"))
        return;

    std::set<juce::String> cppMethods;
    for (const auto& m : apiClass->staticMethods)
        cppMethods.insert(m.name);
    for (const auto& m : apiClass->methods)
        cppMethods.insert(m.name);

    juce::StringArray runtimeLines;
    runtimeLines.addLines(runtimeWhatOutput);

    for (const auto& line : runtimeLines) {
        juce::String trimmedLine = line.trim();
        
        // Skip headers, metadata, or empty lines
        if (trimmedLine.isEmpty() || trimmedLine.contains("---") || 
            trimmedLine.startsWith("Object type") || trimmedLine.startsWith("Members:") ||
            trimmedLine.startsWith("Attributes:")) 
        {
            continue;
        }

        juce::String methodName;
        int colonIndex = trimmedLine.indexOf(":");
        int parenIndex = trimmedLine.indexOf("(");

        if (colonIndex != -1) {
            methodName = trimmedLine.substring(0, colonIndex).trim();
        } else if (parenIndex != -1) {
            methodName = trimmedLine.substring(0, parenIndex).trim();
        } else {
            methodName = trimmedLine.upToFirstOccurrenceOf(" ", false, false).trim();
        }

        // Ignore luabind's internal constructor hook
        if (methodName == "__init")
            continue;

        if (methodName.isNotEmpty()) {
            if (cppMethods.find(methodName) == cppMethods.end()) {
                DBG("⚠️ METHOD AUDIT [" << className << "]: Method '" << methodName << "' exists in Lua/luabind but is MISSING from CtrlrLuaApiLookup!");
            }
        }
    }
}
#endif

void CtrlrLuaApiInspector::inspectClass(const juce::String &className) {
    if (className.isEmpty())
        return;
#if JUCE_DEBUG
	auditClassMethods(className); // Debug only
#endif
	auto& db = CtrlrLuaApiLookup::getInstance();
    if (db.hasClass(className)) {
        const ApiClass* apiClass = db.getClassInfo(className);
        
        juce::String formattedOutput;
        formattedOutput.preallocateBytes(2048); // Prevent reallocation stutter

        formattedOutput << "Object type [" << apiClass->name << "]\n";
        formattedOutput << "-----------------------------------------------------------------\n\n";

        formattedOutput << "Static / Class Methods (Call as " << apiClass->name << ".method()):\n";
        if (apiClass->staticMethods.empty()) {
            formattedOutput << "    (None)\n";
        } else {
            for (const auto& m : apiClass->staticMethods) {
                formattedOutput << "    [Static] " << m.name << m.args << "\n";
                // formattedOutput << "⚡ [Static] " << m.name << m.args << "\n";
            }
        }

        formattedOutput << "\n";

        formattedOutput << "Instance Methods (Call as instance:method()):\n";
        if (apiClass->methods.empty()) {
            formattedOutput << "    (None)\n";
        } else {
            for (const auto& m : apiClass->methods) {
                formattedOutput << "    [Instance] " << m.name << m.args << "\n";
                // formattedOutput << "🔹 [Instance] " << m.name << m.args << "\n";
            }
        }

        formattedOutput << "\n-----------------------------------------------------------------";
        rawOutput = formattedOutput;
        applyFilter();
        return;
    }

    // Fallback reflection
    juce::String luaScript;
    luaScript << "if " << className << " ~= nil then "
              << "return what(" << className << ") "
              << "else return 'Error: Global symbol [" << className << "] is nil or uninitialized.' end";

    rawOutput = runLuaAndGetResult(luaScript);
    applyFilter();
}