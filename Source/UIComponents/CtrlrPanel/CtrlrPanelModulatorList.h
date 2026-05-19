#ifndef __CTRLR_PANEL_MODULATOR_LIST__
#define __CTRLR_PANEL_MODULATOR_LIST__

#include "CtrlrPanelModulatorListTree.h"

class CtrlrModulatorListSorter
{
    public:
        CtrlrModulatorListSorter (CtrlrPanel &_owner, const Identifier &attributeToSort_, bool forwards);
        int compareElements (CtrlrModulator *first, CtrlrModulator *second) const;

    private:
        CtrlrPanel &owner;
        Identifier attributeToSort;
        int direction;
};

class CtrlrPanelModulatorList  : public CtrlrChildWindowContent,
                                 public TableListBoxModel,
                                 public CtrlrPanel::Listener,
                                 public juce::Timer,
                                 public juce::TableHeaderComponent::Listener // <-- FIX: Inherit Listener
{
    public:
        CtrlrPanelModulatorList (CtrlrPanel &_owner);
        ~CtrlrPanelModulatorList();
        enum ColumnId
        {
            CNone,
            CVstIndex,
            CName,
            CVName,
            CMidiType,
            CUIType,
            CPositionedOffPanel
        };

        void copyModulatorList();
        void timerCallback() override;
        int getNumRows();
        void paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected);
        void paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override {}
        void sortOrderChanged (int newSortColumnId, bool isForwards) override;
        void cellDoubleClicked (int rowNumber, int columnId, const MouseEvent &e) override {}
        void cellClicked (int rowNumber, int columnId, const MouseEvent &e) override {}
        Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;
        void refresh();
        const bool isComponentOffPanel(const int indexInModulatorCopy);
        File getModListFile(const String &suffix);
        void selectedRowsChanged (int lastRowSelected) override;
        String getContentName()                 { return ("Panel modulator list"); }
        uint8 getType()                         { return (CtrlrPanelWindowManager::ModulatorList); }
        void makeVisibleItem();
        void exportListItem(const int format);
        void deleteSelected();
        const int getColumnIdForIdentifier (const String &columnName);
        const Identifier getColumnCtrlrId(const int columnId);
        static const String getValueStringForColumn (CtrlrModulator *m, const Identifier columnName);
        static Value getValueForColumn (CtrlrModulator *m, const Identifier columnName);

        void modulatorChanged (CtrlrModulator *modulatorThatChanged) override;
        void modulatorAdded (CtrlrModulator *modulatorThatWasAdded) override;
        void modulatorRemoved (CtrlrModulator *modulatorRemoved) override;
        void restoreColumns(const String &columnState);
        ValueTree &getIdTree();
        void paint (Graphics& g) override;
        void resized() override;
        void visibilityChanged() override;
        void mouseDown (const MouseEvent& e) override;
        void mouseUp (const MouseEvent& e) override;
        void switchView();
        void resetToDefaults();
        StringArray getMenuBarNames() override;
        PopupMenu getMenuForIndex(int topLevelMenuIndex, const String &menuName) override;
        void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
        void handleColumnSelection(const int itemId);
        void handleSortSelection(const int itemId);


	void tableColumnsChanged (TableHeaderComponent* tableHeader) override;
	void tableColumnsResized (TableHeaderComponent* tableHeader) override;
	void tableSortOrderChanged (TableHeaderComponent* tableHeader) override;

        static const String getPropertyCategory(const String& propertyName);
        static const Colour getCategoryColour(const String& category);
        static const String generateLuaUsage(const String& propertyName, bool includeGetter, bool includeSetter);
        void showClipboardBubble(const String& text);

        JUCE_LEAK_DETECTOR(CtrlrPanelModulatorList)

    private:
        CtrlrPanel &owner;
        Array <WeakReference<CtrlrModulator> > copyOfModulatorList;
        int sortColumnId;
        bool isSortedForward;
        CtrlrPanelModulatorListTree modulatorListTree;
        TableListBox* modulatorList;
};

#endif