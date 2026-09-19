/*
  ==============================================================================

    CtrlrLuaApiInspector.h
    Created: 19 Sep 2026 4:01:10pm
    Author:  zan64

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <rapidfuzz/fuzz.hpp>

class CtrlrPanel;

class CtrlrLuaApiInspector  : public juce::Component,
                              private juce::TextEditor::Listener
{
public:
    explicit CtrlrLuaApiInspector(CtrlrPanel& _owner);
    ~CtrlrLuaApiInspector() override = default;

    void resized() override;
    void paint(juce::Graphics& g) override;
	void textEditorTextChanged(juce::TextEditor &editor) override;

private:
    // TextEditor::Listener callback for live filtering

	void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    juce::String runLuaAndGetResult(const juce::String& luaScript);
    void mouseDoubleClick(const juce::MouseEvent& event);
    // Execution Helpers
    void inspectClass(const juce::String& className);
    void listAllClasses();
	void updateClassCache(); // Populates classNamesCache from Lua
	void showAutocompletePopup();

	juce::StringArray classNamesCache;
	bool isAutoCompleting = false; // Guard flag to prevent feedback loops
	void applyFilter();
    void openGithubDocs();

    CtrlrPanel& owner;

    // Control UI
    juce::Label classLabel       { {}, "Class / Object:" };
    juce::TextEditor classInput;
    juce::TextButton inspectButton { "Inspect" };
    juce::TextButton listAllButton { "List All Classes" };
    juce::TextButton clearButton   { "Clear" };
    juce::Label filterLabel      { {}, "Filter:" };
    juce::TextEditor filterInput;
    juce::TextButton docsButton    { "GitHub Docs" };

    // Output UI
    juce::TextEditor outputDisplay;

    // Raw unfiltered output cache for client-side filtering
    juce::String rawOutput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaApiInspector)
};
