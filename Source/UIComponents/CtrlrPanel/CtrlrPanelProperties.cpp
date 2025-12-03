#include "stdafx.h"
#include "CtrlrPanelEditor.h"
#include "CtrlrPanelUtilities.h"
#include "CtrlrLog.h"
#include "CtrlrPanelComponentProperties.h"
#include "CtrlrPanelResourceEditor.h"
#include "CtrlrPanelProperties.h"

// Helper component class for Expressions tab
class CtrlrExpressionsHelp : public Component
{
public:
    CtrlrExpressionsHelp()
    {
        addAndMakeVisible(helpText);

        helpText.setMultiLine(true);
        helpText.setReadOnly(true);
        helpText.setScrollbarsShown(true);
        helpText.setCaretVisible(false);
        helpText.setPopupMenuEnabled(true);

        // Set the help text content
        String content =
            "EXPRESSIONS HELP\n\n"

            "CONSTANTS:\n\n"

            "modulatorValue\n"
            "  The current linear value of the modulator, this is the index of the\n"
            "  array of values; is always positive.\n\n"

            "modulatorMappedValue\n"
            "  The current mapped value in case of components that have mappings.\n"
            "  This might be negative.\n\n"

            "modulatorMax\n"
            "  The maximum value the modulator can have (non mapped)\n\n"

            "modulatorMin\n"
            "  The minimum value the modulator can have (non mapped)\n\n"

            "modulatorMappedMax\n"
            "  The maximum value the modulator can have (mapped)\n\n"

            "modulatorMappedMin\n"
            "  The maximum value the modulator can have (mapped)\n\n"

            "vstIndex\n"
            "  The VST/AU index of the parameter as seen by the host program\n\n"

            "midiValue\n"
            "  The current value stored in the MIDI MESSAGE associated with the\n"
            "  modulator.\n\n"

            "midiNumber\n"
            "  The number of the MIDI MESSAGE controller if applicable\n\n\n"

            "FUNCTIONS:\n\n"

            "ceil(x)\n"
            "  Returns the smallest integral value of the parameter\n\n"

            "abs(x)\n"
            "  Returns the absolute value of the parameter\n\n"

            "floor(x)\n"
            "  Returns the largest integral value that is not greater than the parameter\n\n"

            "mod(a,b)\n"
            "  Divides two numbers and returns the result of the MODULO operation %.\n"
            "  Examples: 10 % 3 = 1, 0 % 5 = 0; 30 % 6 = 0; 32 % 5 = 2\n\n"

            "fmod(numerator,denominator)\n"
            "  Returns the floating-point remainder of the two parameters passed in\n\n"

            "pow(a,b)\n"
            "  Returns the first parameter raised to the power of the second (a^b)\n\n"

            "gte(a,b,retTrue,retFalse)\n"
            "  Return the larger or equal of the two passed parameters (a >= b).\n"
            "  Example: gte(modulatorValue, 0, modulatorValue, 128 - modulatorValue)\n"
            "  will return modulatorValue if modulatorValue is greater than 0 and\n"
            "  (128 - modulatorValue) if it is less than zero\n\n"

            "gt(a,b,retTrue,retFalse)\n"
            "  Same as gte but greater than without the equal sign (a > b)\n\n"

            "lt(a,b,retTrue,retFalse)\n"
            "  Same as gte but less than (a < b)\n\n"

            "lte(a,b,retTrue,retFalse)\n"
            "  Same as gte but less than or equal (a <= b)\n\n"

            "eq(a,b,retTrue,retFalse)\n"
            "  Equals sign true if (a == b)\n\n"

            "max(a,b)\n"
            "  Returns the bigger of two parameters.\n\n"

            "min(a,b)\n"
            "  Returns the smaller of two parameters.\n\n"

            "getBitRangeAsInt(value, startBit, numBits)\n"
            "  Gets a number of bits (numBits) starting at position startBit as an\n"
            "  Integer and returns that integer.\n\n"

            "setBitRangeAsInt(value, startBit, numBits, valueToSet)\n"
            "  Sets a range of bits in value\n\n"

            "clearBit(value, bitToClear)\n"
            "  Clears a bit at position bitToClear in the value and returns that\n"
            "  modified value.\n\n"

            "isBitSet(value, bitPosition)\n"
            "  Return true if a bit at position bitPosition in value is set,\n"
            "  false otherwise.\n\n"

            "setBit(value, bitToSet)\n"
            "  Sets one bit in an integer at position (bitToSet) and returns the\n"
            "  modified value with the bit set.\n\n"

            "setGlobal(globalIndex, newValueToSet)\n"
            "  This sets the value of one of the global variables in the panel,\n"
            "  and returns that set value so the expression can continue.\n";

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
        // Update colors when component is added to hierarchy
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrExpressionsHelp)
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

    tabbedComponent->addTab("Utility",
        getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
        new CtrlrPanelUtilities(owner),
        true
    );

    // NEW: Add Expressions tab
    tabbedComponent->addTab("Expressions",
        getLookAndFeel().findColour(TabbedComponent::backgroundColourId),
        new CtrlrExpressionsHelp(),
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