#include "stdafx.h"
#include "CtrlrPanelEditor.h"
#include "CtrlrPanelUtilities.h"
#include "CtrlrLog.h"
#include "CtrlrPanelComponentProperties.h"
#include "CtrlrPanelResourceEditor.h"
#include "CtrlrPanelProperties.h"


class CtrlrTransferDumpHelp : public Component
{
public:
    CtrlrTransferDumpHelp()
    {
        addAndMakeVisible(helpText);

        helpText.setMultiLine(true);
        helpText.setReadOnly(true);
        helpText.setScrollbarsShown(true);
        helpText.setCaretVisible(false);
        helpText.setPopupMenuEnabled(true);
        helpText.setWantsKeyboardFocus(false);

        String content =
            "HOW TO PROCESS BULK MIDI MESSAGES\n\n"

            "ENCODING TYPES:\n\n"

            "EncodeNormal\n"
            "  Single 7-bit byte 0-127\n\n"

            "EncodeMSBFirst\n"
            "  7-bit: MSB, LSB\n\n"

            "EncodeLSBFirst\n"
            "  7-bit: LSB, MSB\n\n"

            "EncodeNibbleMsbFirst\n"
            "  4-bit: MSB nibble, LSB nibble (unsigned)\n\n"

            "EncodeNibbleLsbFirst\n"
            "  4-bit: LSB nibble, MSB nibble (unsigned)\n\n"

            "EncodeSignedNibbleMsbFirst\n"
            "  4-bit: MSB nibble, LSB nibble (signed int8)\n\n"

            "EncodeSignedNibbleLsbFirst\n"
            "  4-bit: LSB nibble, MSB nibble (signed int8)\n\n"

            "Encode16bitLsbFirst\n"
            "Encodes a 16 - bit value as four 4 - bit nibbles, least significant first.\n"
            "Tokens: q0 q1 q2 q3\n"
            "Example : 51379 ? 03 0B 08 0C\n\n"

            "Encode16bitMsbFirst\n"
            "Encodes a 16 - bit value as four 4 - bit nibbles, most significant first.\n"
            "Tokens: Q0 Q1 Q2 Q3\n"
            "Example : 51379 ? 0C 08 0B 03\n\n"


            "Non mapped:\n"
            "  panel:getModulatorValuesAsData(CUSTINDEX, CtrlrPanel.EncodeNormal, 1, false)\n\n"

            "Mapped:\n"
            "  panel:getModulatorValuesAsData(CUSTINDEX, CtrlrPanel.EncodeNormal, 1, true)\n\n"

            "LSB/MSB two byte 4-bit nibble:\n"
            "  panel:getModulatorValuesAsData(CUSTINDEX, CtrlrPanel.EncodeNibbleLsbFirst,\n"
            "                                 1, true)\n\n\n"

            "EXAMPLE - RECEIVE (Where Header is 5 bytes in length):\n\n"

            "  panel:setModulatorValuesFromData(midi:getData(), \"modulatorCustomIndex\",\n"
            "                                   CtrlrPanel.EncodeMSBFirst, -5, 2, false)\n\n"

            "  panel:setModulatorValuesFromData(midi:getData(), \"modulatorCustomIndex\",\n"
            "                                   CtrlrPanel.EncodeNormal, -5, 1, false)\n\n"

            "  panel:setModulatorValuesFromData(midi:getData(), \"modulatorCustomIndex\",\n"
            "                                   CtrlrPanel.EncodeSignedNibbleMsbFirst,\n"
            "                                   -54, 2, false)\n\n\n"

            "STEP 1: CREATE TABLE OF MODULATORS IN SYSEX DUMP ORDER\n\n"

            "  listOfModulators = {\n"
            "      \"lfoDelay\",\n"
            "      \"lfoRate\",\n"
            "      \"VCF Resonance\",\n"
            "      \"VCF Cutoff\",\n"
            "      \"Delay\"\n"
            "  }\n\n"

            "List all modulators here in order of sysex message data position\n\n\n"

            "STEP 2: FILL modulatorCustomIndex WITH VALUES\n\n"

            "You can create your own custom modulator property (e.g. \"CUSTINDEX\")\n"
            "Run this in the console editor:\n\n"

            "  local t = listOfModulators\n"
            "  for i, v in ipairs(t) do\n"
            "      panel:getModulatorByName(v):setProperty(\"CUSTINDEX\", tostring(i - 1),\n"
            "                                              false)\n"
            "  end\n\n\n"

            "STEP 2b: REMOVE CUSTOM INDEX (UNDO)\n\n"

            "You can completely remove the custom index you created:\n\n"

            "  local t = listOfModulators\n"
            "  for i, v in ipairs(t) do\n"
            "      panel:getModulatorByName(v):removeProperty(\"CUSTINDEX\")\n"
            "  end\n\n\n"

            "STEP 3: SEND THE BULK MIDI MESSAGE\n\n"

            "Create a (GLOBAL) header string and an EOX string:\n\n"

            "  HEADER = \"F0 41 00 00 11\"\n"
            "  EOX = \"F7\"\n\n"

            "  local data = panel:getModulatorValuesAsData(\"CUSTINDEX\",\n"
            "                                              CtrlrPanel.EncodeNormal,\n"
            "                                              1, false)\n"
            "  panel:sendMidiMessageNow(CtrlrMidiMessage(string.format(\"%s %s %s\",\n"
            "                                                          HEADER,\n"
            "                                                          data:toHexString(1),\n"
            "                                                          EOX)))\n\n\n"

            "STEP 4: RECEIVE A MIDI MESSAGE\n\n"

            "Create a method in 'Called when a panel receives a MIDI message':\n\n"

            "  local headerSize = MemoryBlock(HEADER):getSize()\n"
            "  panel:setModulatorValuesFromData(midi:getData(), \"CUSTINDEX\",\n"
            "                                   CtrlrPanel.EncodeNormal,\n"
            "                                   -headerSize, 1, false)\n\n"

            "Note that headerSize = headerSize * -1\n\n"

            "The last argument of these methods when changed to true reads/writes\n"
            "mapped values\n";

        helpText.setText(content);

        // Use a custom font for better readability
        Font monoFont = Font(Font::getDefaultMonospacedFontName(), 13.0f, Font::plain);
        helpText.setFont(monoFont);
    }

    void resized() override
    {
        helpText.setBounds(getLocalBounds().reduced(4));
    }

    void parentHierarchyChanged() override
    {
        updateColors();
    }

    void lookAndFeelChanged() override
    {
        updateColors();
    }

    void updateColors()
    {
        Colour bgColour = findColour(TextEditor::backgroundColourId);

        // Determine if background is light or dark
        float brightness = bgColour.getBrightness();
        Colour textColour = (brightness > 0.5f) ? Colours::black : Colours::white;

        helpText.setColour(TextEditor::textColourId, textColour);
        helpText.setColour(TextEditor::backgroundColourId, bgColour);
        helpText.setColour(TextEditor::outlineColourId, findColour(Slider::textBoxOutlineColourId));

        helpText.applyColourToAllText(textColour);
        helpText.repaint();
    }

private:
    TextEditor helpText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrTransferDumpHelp)
};

CtrlrPanelProperties::CtrlrPanelProperties(CtrlrPanelEditor& _owner)
    : Component(L"Properties"),
    owner(_owner),
    tabbedComponent(0)
{
    addAndMakeVisible(tabbedComponent = new TabbedComponent(TabbedButtonBar::TabsAtRight));

    tabbedComponent->setTabBarDepth(owner.getOwner().getOwner().getProperty(Ids::ctrlrTabBarDepth));
    tabbedComponent->setCurrentTabIndex(-1);
    tabbedComponent->setOutline(1);

    CtrlrPanelComponentProperties* props = new CtrlrPanelComponentProperties(owner);

    tabbedComponent->addTab("General",
        getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
        props,
        true
    );

    tabbedComponent->addTab("Resources",
        getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
        new CtrlrPanelResourceEditor(owner),
        true
    );

    tabbedComponent->addTab("XML Viewer",
        getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
        new CtrlrPanelUtilities(owner),
        true
    );

    // NEW: Add Expressions tab
    // tabbedComponent->addTab("Expressions",
    //     getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
    //     new CtrlrExpressionsHelp(),
    //     true
    // );

    tabbedComponent->addTab("MIDI Bulk Dump",
        getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
        new CtrlrTransferDumpHelp(),
        true
    );

    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::frontTextColourId, findColour(Label::textColourId));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabTextColourId, findColour(Label::textColourId).withAlpha(0.6f));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabOutlineColourId, findColour(Slider::textBoxOutlineColourId));

    ctrlrPanelFindProperty.reset(new CtrlrPanelFindProperty(owner, props));
    addAndMakeVisible(ctrlrPanelFindProperty.get());
    setSize(216, 364);
}

CtrlrPanelProperties::~CtrlrPanelProperties()
{
    CtrlrPanelComponentProperties* p = dynamic_cast <CtrlrPanelComponentProperties*>(tabbedComponent->getTabContentComponent(0));
    if (p)
    {
        owner.getOwner().getCtrlrManagerOwner().removeListener(p);
    }
    deleteAndZero(tabbedComponent);
}

void CtrlrPanelProperties::paint(Graphics& g)
{
    g.fillAll(findColour(DocumentWindow::backgroundColourId));
}

void CtrlrPanelProperties::resized()
{
    ctrlrPanelFindProperty->setBounds(0, 0, getWidth() - (int)owner.getOwner().getOwner().getProperty(Ids::ctrlrTabBarDepth), 32);
    tabbedComponent->setBounds(0, 32, getWidth() - 0, getHeight() - 32);
    updateTabColours();
    repaint();
}

void CtrlrPanelProperties::updateTabColours()
{
    for (int i = 0; i < tabbedComponent->getNumTabs(); i++)
    {
        if (i == tabbedComponent->getTabbedButtonBar().getCurrentTabIndex())
        {
            if (owner.getProperty(Ids::uiPanelLookAndFeel) == "V3")
            {
                tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, Colour(0xffcccccc));
            }
            else
            {
                tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, getLookAndFeel().findColour(TabbedComponent::backgroundColourId).contrasting(0.2f));
            }
        }
        else
        {
            tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, getLookAndFeel().findColour(TabbedComponent::backgroundColourId));
        }
    }
}

void CtrlrPanelProperties::lookAndFeelChanged()
{
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::frontTextColourId, findColour(Label::textColourId));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabTextColourId, findColour(Label::textColourId).withAlpha(0.6f));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabOutlineColourId, findColour(Slider::textBoxOutlineColourId));
    repaint();
}

void CtrlrPanelProperties::changeListenerCallback(ChangeBroadcaster* source)
{
}

void CtrlrPanelProperties::refreshAll()
{
    for (int i = 0; i < tabbedComponent->getNumTabs(); i++)
    {
        CtrlrPanelComponentProperties* cp = dynamic_cast<CtrlrPanelComponentProperties*>(tabbedComponent->getTabContentComponent(i));
        if (cp != 0)
        {
            cp->refreshAll();
        }
    }
}

void CtrlrPanelProperties::layoutChanged()
{
    if ((bool)owner.getProperty(Ids::uiPanelPropertiesOnRight) == true)
    {
        tabbedComponent->setOrientation(TabbedButtonBar::TabsAtLeft);
    }
    else
    {
        tabbedComponent->setOrientation(TabbedButtonBar::TabsAtRight);
    }
}