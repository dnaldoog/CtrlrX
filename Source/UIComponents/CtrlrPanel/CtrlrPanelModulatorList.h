#ifndef __CTRLR_PANEL_MODULATOR_LIST__
#define __CTRLR_PANEL_MODULATOR_LIST__

#include "../../Core/CtrlrPanel/CtrlrPanel.h"
#include "../CtrlrWindowManagers/CtrlrPanelWindowManager.h"
#include "CtrlrPanelModulatorListTree.h"
#include <rapidfuzz/fuzz.hpp>

namespace {
struct DefaultColumn {
		const char *identifier;
		int width;
};

// Order here == order of columns after "Reset columns to default"
const DefaultColumn kDefaultColumns[] = {
	{"name", 100},
	{"modulatorValue", 60},
	{"vstIndex", 60},
	{"uiType", 100},
	{"componentRectangle", 80},
	{"componentGroupName", 60},
	{"componentTabName", 60},
	// {"componentRadioGroupId", 60},
	{"midiMessageType", 60},
	{"midiMessageCtrlrNumber", 60},
	{"midiMessageSysExFormula", 100},
	{"modulatorCustomIndex", 60},
	// {"modulatorCustomIndexGroup", 60},
};

bool isDefaultColumn(const juce::String &identifier) {
	for (const auto &c : kDefaultColumns)
		if (identifier == c.identifier)
			return true;
	return false;
}
} // namespace

class CtrlrModulator;

class CtrlrModulatorListSorter {
	public:
		CtrlrModulatorListSorter(CtrlrPanel &_owner, const juce::Identifier &attributeToSort_, bool forwards);
		int compareElements(CtrlrModulator *first, CtrlrModulator *second) const;

	private:
		CtrlrPanel &owner;
		juce::Identifier attributeToSort;
		int direction;
};

class CtrlrPanelModulatorList : public CtrlrChildWindowContent, // Component base must be first!
								public juce::TableListBoxModel, // Added namespace
								public CtrlrPanel::Listener,
								public juce::TableHeaderComponent::Listener, // Added namespace
								public juce::Timer,
								public juce::TextEditor::Listener {

	public:
		CtrlrPanelModulatorList(CtrlrPanel &_owner);
		~CtrlrPanelModulatorList();
		enum ColumnId { CNone, CVstIndex, CName, CVName, CMidiType, CUIType, CPositionedOffPanel };

		void copyModulatorList();
		void timerCallback() override;
		int getNumRows();
		void paintRowBackground(Graphics &g, int rowNumber, int width, int height, bool rowIsSelected);
		void paintCell(Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) {}
		void sortOrderChanged(int newSortColumnId, bool isForwards);
		void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent &e) {}
		void cellClicked(int rowNumber, int columnId, const MouseEvent &e) {}
		Component *refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected,
										   Component *existingComponentToUpdate);
		void refresh();
		const bool isComponentOffPanel(const int indexInModulatorCopy);
		File getModListFile(const String &suffix);
		void selectedRowsChanged(int lastRowSelected);
		String getContentName() { return ("Panel modulator list"); }
		uint8 getType() { return (CtrlrPanelWindowManager::ModulatorList); }
		void makeVisibleItem();
		void exportListItem(const int format);
		void deleteSelected();
		const int getColumnIdForIdentifier(const String &columnName);
		const Identifier getColumnCtrlrId(const int columnId);
		static const String getValueStringForColumn(CtrlrModulator *m, const Identifier columnName);
		static Value getValueForColumn(CtrlrModulator *m, const Identifier columnName);
		void textEditorTextChanged(juce::TextEditor &editor) override; // relating to fuzzy search
		void modulatorChanged(CtrlrModulator *modulatorThatChanged) override;
		void modulatorAdded(CtrlrModulator *modulatorThatWasAdded);
		void modulatorRemoved(CtrlrModulator *modulatorRemoved);
		void restoreColumns(const String &columnState);
		ValueTree &getIdTree();
		void paint(Graphics &g);
		void resized();
		void visibilityChanged();
		void mouseDown(const MouseEvent &e);
		void mouseUp(const MouseEvent &e);
		void switchView();
		void resetToDefaults();
		StringArray getMenuBarNames();
		PopupMenu getMenuForIndex(int topLevelMenuIndex, const String &menuName);
		void menuItemSelected(int menuItemID, int topLevelMenuIndex);
		void handleColumnSelection(const int itemId);
		void handleSortSelection(const int itemId);
		
/* I don't know what these are, but they don't seem to be used anywhere
https://github.com/damiensellier/CtrlrX/issues/295#issuecomment-4960450879
		static const String getPropertyCategory(const String &propertyName);
		static const Colour getCategoryColour(const String &category);
		static const String generateLuaUsage(const String &propertyName, bool includeGetter, bool includeSetter);
		void showClipboardBubble(const String &text);
*/
		void tableColumnsChanged(TableHeaderComponent *) override;
		void tableColumnsResized(TableHeaderComponent *) override;
		void tableSortOrderChanged(TableHeaderComponent *) override;
		void tableColumnDraggingChanged(TableHeaderComponent *, int) override;

		void saveColumnState();

		JUCE_LEAK_DETECTOR(CtrlrPanelModulatorList)

	private:
		CtrlrPanel &owner;
		juce::Label searchLabel{"searchLabel", "Search Modulators:"};
		juce::TextEditor searchField;
		juce::TextButton clearSearchButton{"Reset"};

		void applyFuzzyFilter();
		Array<WeakReference<CtrlrModulator>> masterModulatorList;
		Array<WeakReference<CtrlrModulator>> copyOfModulatorList;
		int sortColumnId;
		bool isSortedForward;
		void showColumnPicker();
		CtrlrPanelModulatorListTree modulatorListTree;
		std::unique_ptr<TableListBox> modulatorList;
		std::unique_ptr<juce::DocumentWindow> columnPickerWindow;
};

#endif
