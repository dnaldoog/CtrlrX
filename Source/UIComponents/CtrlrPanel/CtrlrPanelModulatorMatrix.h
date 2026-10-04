#pragma once

#include "CtrlrMacros.h"
#include "CtrlrModulator/CtrlrModulator.h"
#include <rapidfuzz/fuzz.hpp>

class CtrlrPanelEditor;

class CtrlrPanelModulatorMatrix : public Component,
                                  public TableListBoxModel,
                                  public TextEditor::Listener,
                                  public Button::Listener
{
public:
    CtrlrPanelModulatorMatrix(CtrlrPanelEditor& owner);
    ~CtrlrPanelModulatorMatrix() override = default;

    // Component Callbacks
    void resized() override;
    void visibilityChanged() override;

    // TableListBoxModel Callbacks
    int getNumRows() override;
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

    // Listeners
    void textEditorTextChanged(TextEditor& textEditor) override;
    void buttonClicked(Button* button) override;

    void refresh();
    void applyFuzzyFilter();

private:
    CtrlrPanelEditor& owner;
    
    TextEditor searchField;
    TextButton toggleAllExclude { "All On/Off" };
    TextButton toggleAllStatic  { "All On/Off" };
    TextButton toggleAllSave    { "All On/Off" };

    TableListBox table;
    
    Array<WeakReference<CtrlrModulator>> masterModulatorList;
    Array<WeakReference<CtrlrModulator>> filteredModulatorList;

    enum ColumnIds
    {
        ColName = 1,
        ColExcludeFromSnapshot,
        ColIsStatic,
        ColValueSaveToFile
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrPanelModulatorMatrix)
};