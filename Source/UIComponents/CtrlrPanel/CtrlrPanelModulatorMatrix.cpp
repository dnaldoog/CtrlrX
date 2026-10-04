#include "CtrlrPanelModulatorMatrix.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrInlineUtilitiesGUI.h"

CtrlrPanelModulatorMatrix::CtrlrPanelModulatorMatrix(CtrlrPanelEditor& _owner)
    : owner(_owner)
{
    // Search bar configuration
    searchField.setTextToShowWhenEmpty("Filter modulators...", Colours::grey);
    searchField.addListener(this);
    addAndMakeVisible(searchField);

    // Batch buttons
    addAndMakeVisible(toggleAllExclude);
    addAndMakeVisible(toggleAllStatic);
    addAndMakeVisible(toggleAllSave);

    toggleAllExclude.addListener(this);
    toggleAllStatic.addListener(this);
    toggleAllSave.addListener(this);

    // Table Setup
    addAndMakeVisible(table);
    table.setModel(this);
    table.setOutlineThickness(1);
    table.setHeaderHeight(24);
    table.setRowHeight(22);

    auto& header = table.getHeader();
    header.addColumn("Modulator Name", ColName, 180, 100, 400);
    header.addColumn("Exclude Snapshot", ColExcludeFromSnapshot, 120, 80, 150);
    header.addColumn("Is Static", ColIsStatic, 100, 80, 120);
    header.addColumn("Save To File", ColValueSaveToFile, 100, 80, 120);

    refresh();
}

void CtrlrPanelModulatorMatrix::refresh()
{
    masterModulatorList.clear();
    auto& panel = owner.getOwner();
    for (int i = 0; i < panel.getModulators().size(); ++i)
    {
        if (auto* mod = panel.getModulators()[i])
            masterModulatorList.add(mod);
    }

    applyFuzzyFilter();
}

void CtrlrPanelModulatorMatrix::applyFuzzyFilter()
{
    const juce::String query = searchField.getText().trim();
    filteredModulatorList.clear();

    if (query.isEmpty())
    {
        filteredModulatorList = masterModulatorList;
    }
    else
    {
        struct ScoredModulator
        {
            WeakReference<CtrlrModulator> mod;
            double score;
        };

        std::vector<ScoredModulator> scoredList;
        const std::string queryStd = query.toStdString();

        for (int i = 0; i < masterModulatorList.size(); ++i)
        {
            auto* m = masterModulatorList[i].get();
            if (!m) continue;

            const juce::String modName = m->getName();
            double score = rapidfuzz::fuzz::partial_ratio(queryStd, modName.toStdString());

            if (modName.startsWithIgnoreCase(query))
                score += 20.0;

            if (score > 55.0)
                scoredList.push_back({ masterModulatorList[i], score });
        }

        std::sort(scoredList.begin(), scoredList.end(),
                  [](const ScoredModulator& a, const ScoredModulator& b) { return a.score > b.score; });

        for (const auto& item : scoredList)
            filteredModulatorList.add(item.mod);
    }

    table.updateContent();
    table.repaint();
}

void CtrlrPanelModulatorMatrix::textEditorTextChanged(TextEditor&)
{
    applyFuzzyFilter();
}

void CtrlrPanelModulatorMatrix::buttonClicked(Button* b)
{
    Identifier propId;
    juce::String propName;

    if (b == &toggleAllExclude)
    {
        propId = Ids::modulatorExcludeFromSnapshot;
        propName = "Exclude From Snapshot";
    }
    else if (b == &toggleAllStatic)
    {
        propId = Ids::modulatorIsStatic;
        propName = "Is Static";
    }
    else if (b == &toggleAllSave)
    {
        propId = Ids::modulatorValueSaveToFile;
        propName = "Save To File";
    }
    else
    {
        return;
    }

    if (filteredModulatorList.size() == 0)
        return;

    // Determine target state based on the first item in the list
    bool newTargetState = true;
    if (auto* firstMod = filteredModulatorList[0].get())
    {
        newTargetState = ((int)firstMod->getProperty(propId) == 0);
    }

    juce::String stateText = newTargetState ? "ENABLE" : "DISABLE";
    juce::String message = "Are you sure you want to " + stateText + " '" + propName + 
                           "' across all " + juce::String(filteredModulatorList.size()) + 
                           " visible modulators?";
AW::showOkCancelAsyncSafe(
    AW::Warning,
    "Bulk Update Warning",
    message,
    [this, propId, newTargetState](bool confirmed)
    {
        if (confirmed)
        {
            for (int i = 0; i < filteredModulatorList.size(); ++i)
            {
                if (auto* mod = filteredModulatorList[i].get())
                {
                    mod->setProperty(propId, newTargetState ? 1 : 0);
                }
            }

            table.updateContent();
            table.repaint();
        }
    },
    "Proceed",
    "Cancel"
);
}

int CtrlrPanelModulatorMatrix::getNumRows()
{
    return filteredModulatorList.size();
}

void CtrlrPanelModulatorMatrix::visibilityChanged()
{
    if (isVisible())
        refresh();
}

void CtrlrPanelModulatorMatrix::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(findColour(TextEditor::highlightColourId));
    else if (rowNumber % 2 == 1)
        g.fillAll(findColour(ListBox::backgroundColourId).darker(0.05f));
    else
        g.fillAll(findColour(ListBox::backgroundColourId));
}

void CtrlrPanelModulatorMatrix::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= filteredModulatorList.size()) return;

    auto* mod = filteredModulatorList[rowNumber].get();
    if (!mod) return;

    if (columnId == ColName)
    {
        g.setColour(findColour(ListBox::textColourId));
        g.setFont(13.0f);
        g.drawText(mod->getName(), 5, 0, width - 10, height, Justification::centredLeft, true);
    }
}

Component* CtrlrPanelModulatorMatrix::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
    if (columnId == ColName) return nullptr;

    auto* toggle = dynamic_cast<ToggleButton*>(existingComponentToUpdate);
    if (!toggle)
        toggle = new ToggleButton();

    if (rowNumber < 0 || rowNumber >= filteredModulatorList.size()) return toggle;

    auto* mod = filteredModulatorList[rowNumber].get();
    if (!mod) return toggle;

    Identifier propId;
    if (columnId == ColExcludeFromSnapshot) propId = Ids::modulatorExcludeFromSnapshot;
    else if (columnId == ColIsStatic) propId = Ids::modulatorIsStatic;
    else if (columnId == ColValueSaveToFile) propId = Ids::modulatorValueSaveToFile;

    toggle->setToggleState((int)mod->getProperty(propId) != 0, dontSendNotification);

    toggle->onClick = [mod, propId, toggle]()
    {
        if (mod)
        {
            mod->setProperty(propId, toggle->getToggleState() ? 1 : 0);
        }
    };

    return toggle;
}

void CtrlrPanelModulatorMatrix::resized()
{
    auto bounds = getLocalBounds();

    // Search bar top placement
    searchField.setBounds(bounds.removeFromTop(24).reduced(2, 2));

    // Batch control bar top placement
    auto btnArea = bounds.removeFromTop(22);
    btnArea.removeFromLeft(180); // Offset for name column width
    toggleAllExclude.setBounds(btnArea.removeFromLeft(120).reduced(2, 1));
    toggleAllStatic.setBounds(btnArea.removeFromLeft(100).reduced(2, 1));
    toggleAllSave.setBounds(btnArea.removeFromLeft(100).reduced(2, 1));

    table.setBounds(bounds);
}