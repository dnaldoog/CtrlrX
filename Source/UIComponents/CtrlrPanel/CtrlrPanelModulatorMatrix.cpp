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
    if (b == &toggleAllExclude)      propId = Ids::modulatorExcludeFromSnapshot;
    else if (b == &toggleAllStatic)  propId = Ids::modulatorIsStatic;
    else if (b == &toggleAllSave)    propId = Ids::modulatorValueSaveToFile;
    else return;

    // Determine target toggle state from first visible item
    bool newTargetState = true;
    if (filteredModulatorList.size() > 0)
    {
        if (auto* firstMod = filteredModulatorList[0].get())
            newTargetState = ((int)firstMod->getProperty(propId) == 0);
    }

    // Apply batch updates adhering to mutual rules
    for (int i = 0; i < filteredModulatorList.size(); ++i)
    {
        if (auto* mod = filteredModulatorList[i].get())
        {
            if (b == &toggleAllStatic)
            {
                mod->setProperty(Ids::modulatorIsStatic, newTargetState ? 1 : 0);
                if (newTargetState)
                {
                    // Making static forces exclusion from snapshots
                    mod->setProperty(Ids::modulatorExcludeFromSnapshot, 1);
                }
            }
            else if (b == &toggleAllExclude)
            {
                mod->setProperty(Ids::modulatorExcludeFromSnapshot, newTargetState ? 1 : 0);
                if (!newTargetState)
                {
                    // Including in snapshots forces non-static
                    mod->setProperty(Ids::modulatorIsStatic, 0);
                }
            }
            else if (b == &toggleAllSave)
            {
                mod->setProperty(Ids::modulatorValueSaveToFile, newTargetState ? 1 : 0);
            }
        }
    }

    table.updateContent();
    table.repaint();
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

    // Clear previous callback to prevent cell recycling bugs
    toggle->onClick = nullptr;

    if (rowNumber < 0 || rowNumber >= filteredModulatorList.size()) return toggle;

    auto* mod = filteredModulatorList[rowNumber].get();
    if (!mod) return toggle;

    const bool isStatic   = ((int)mod->getProperty(Ids::modulatorIsStatic) != 0);
    const bool isExcluded = ((int)mod->getProperty(Ids::modulatorExcludeFromSnapshot) != 0);
    const bool isSave     = ((int)mod->getProperty(Ids::modulatorValueSaveToFile) != 0);

    if (columnId == ColIsStatic)
    {
        toggle->setEnabled(true);
        toggle->setToggleState(isStatic, dontSendNotification);

        toggle->onClick = [this, mod, toggle]()
        {
            if (!mod) return;
            const bool active = toggle->getToggleState();
            mod->setProperty(Ids::modulatorIsStatic, active ? 1 : 0);

            if (active)
            {
                // Force Exclude Snapshot property to 0 when made Static
                mod->setProperty(Ids::modulatorExcludeFromSnapshot, 0);
            }

            table.updateContent();
            table.repaint();
        };
    }
    else if (columnId == ColExcludeFromSnapshot)
    {
        if (isStatic)
        {
            // STATIC MODE: Explicitly uncheck AND disable
            toggle->setToggleState(false, dontSendNotification);
            toggle->setEnabled(false);
        }
        else
        {
            // DYNAMIC MODE: Enable and show actual property state
            toggle->setEnabled(true);
            toggle->setToggleState(isExcluded, dontSendNotification);
        }

        toggle->onClick = [this, mod, toggle]()
        {
            if (!mod) return;
            const bool active = toggle->getToggleState();
            mod->setProperty(Ids::modulatorExcludeFromSnapshot, active ? 1 : 0);

            if (active)
            {
                // If checked Exclude Snapshot, force IsStatic to 0
                mod->setProperty(Ids::modulatorIsStatic, 0);
            }

            table.updateContent();
            table.repaint();
        };
    }
    else if (columnId == ColValueSaveToFile)
    {
        toggle->setEnabled(true);
        toggle->setToggleState(isSave, dontSendNotification);

        toggle->onClick = [mod, toggle]()
        {
            if (mod)
                mod->setProperty(Ids::modulatorValueSaveToFile, toggle->getToggleState() ? 1 : 0);
        };
    }

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