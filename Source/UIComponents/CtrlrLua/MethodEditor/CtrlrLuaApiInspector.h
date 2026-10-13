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

// TextEditor that lets its owner intercept keys (Up/Down/Enter/Esc) before normal handling
class CtrlrApiSuggestEditor : public juce::TextEditor {
	public:
		std::function<bool(const juce::KeyPress &)> onKey;

		bool keyPressed(const juce::KeyPress &key) override {
			if (onKey && onKey(key))
				return true;
			return juce::TextEditor::keyPressed(key);
		}
};

// Lightweight, non-modal suggestion dropdown. Never takes keyboard focus.
class CtrlrApiSuggestionList : public juce::Component {
	public:
		static constexpr int rowHeight = 22;
		std::function<void(int)> onPick;

		CtrlrApiSuggestionList() {
			setWantsKeyboardFocus(false);
			setMouseClickGrabsKeyboardFocus(false); // clicking a row must not steal focus from the text field
		}

		void setItems(const juce::StringArray &newItems) {
			items = newItems;
			selected = 0;
			repaint();
		}
		int getNumItems() const {
			return items.size();
		}
		juce::String getItem(int row) const {
			return items[row];
		}
		int getSelectedRow() const {
			return selected;
		}
		void moveSelection(int delta) {
			if (items.isEmpty())
				return;
			selected = (selected + delta + items.size()) % items.size();
			repaint();
		}

		void paint(juce::Graphics &g) override {
			g.fillAll(juce::Colours::white);
			g.setFont(juce::FontOptions(14.0f));

			for (int i = 0; i < items.size(); ++i) {
				auto r = juce::Rectangle<int>(1, 1 + i * rowHeight, getWidth() - 2, rowHeight);
				if (i == selected) {
					g.setColour(juce::Colours::lightsteelblue);
					g.fillRect(r);
				}
				g.setColour(juce::Colours::black);
				g.drawText(items[i], r.reduced(6, 0), juce::Justification::centredLeft, true);
			}
			g.setColour(juce::Colours::grey);
			g.drawRect(getLocalBounds(), 1);
		}

		void mouseMove(const juce::MouseEvent &e) override {
			const int row = (e.y - 1) / rowHeight;
			if (juce::isPositiveAndBelow(row, items.size()) && row != selected) {
				selected = row;
				repaint();
			}
		}
		void mouseDown(const juce::MouseEvent &e) override {
			const int row = (e.y - 1) / rowHeight;
			if (juce::isPositiveAndBelow(row, items.size()) && onPick)
				onPick(row);
		}

	private:
		juce::StringArray items;
		int selected = 0;
};

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
	CtrlrApiSuggestionList suggestionList;

	void textEditorFocusLost(juce::TextEditor &editor) override;
	void updateSuggestions();
	void hideSuggestions();
	void acceptSuggestion(int row);
	bool handleClassInputKey(const juce::KeyPress &key);

	juce::StringArray classNamesCache;
	bool isAutoCompleting = false; // Guard flag to prevent feedback loops
	void applyFilter();
    void openGithubDocs();

    CtrlrPanel& owner;

    // Control UI
    juce::Label classLabel       { {}, "Class / Object:" };
	CtrlrApiSuggestEditor classInput; // was: juce::TextEditor classInput;
	juce::TextButton inspectButton{"Inspect"};
	juce::TextButton listAllButton { "List All Classes" };
    juce::TextButton clearButton   { "Clear" };
    juce::Label filterLabel      { {}, "Filter:" };
    juce::TextEditor filterInput;
	// juce::TextButton docsButton    { "GitHub Docs" };

	// Output UI
    juce::TextEditor outputDisplay;

    // Raw unfiltered output cache for client-side filtering
    juce::String rawOutput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaApiInspector)
};
