#include "stdafx.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrMIDICalculator.h"

CtrlrMIDICalculator::CtrlrMIDICalculator(CtrlrManager &_owner)
    : owner(_owner)
{
    hexDisplay = std::make_unique<TextEditor>("hexDisplay");
    addAndMakeVisible(hexDisplay.get());

    hexDisplay->setMultiLine(false);
    hexDisplay->setReturnKeyStartsNewLine(false);
    hexDisplay->setReadOnly(false);
    hexDisplay->setScrollbarsShown(true);
    hexDisplay->setCaretVisible(true);
    hexDisplay->setPopupMenuEnabled(true);
    hexDisplay->setText("F0 00 01 40 A3 F7");

    calcLabel = std::make_unique<Label>("calcLabel", "Hexadecimal");
    addAndMakeVisible(calcLabel.get());

    calcLabel->setFont(Font(24.00f, Font::plain));
    calcLabel->setJustificationType(Justification::centredLeft);
    calcLabel->setEditable(false, false, false);
    calcLabel->setColour(TextEditor::textColourId, Colours::black);
    calcLabel->setColour(TextEditor::backgroundColourId, Colour(0x00000000));

    calcLabel2 = std::make_unique<Label>("calcLabel2", "Binary");
    addAndMakeVisible(calcLabel2.get());
    calcLabel2->setFont(Font(24.00f, Font::plain));
    calcLabel2->setJustificationType(Justification::centredLeft);
    calcLabel2->setEditable(false, false, false);
    calcLabel2->setColour(TextEditor::textColourId, Colours::black);
    calcLabel2->setColour(TextEditor::backgroundColourId, Colour(0x00000000));

    binDisplay = std::make_unique<TextEditor>("binDisplay");
    addAndMakeVisible(binDisplay.get());

    binDisplay->setMultiLine(true);
    binDisplay->setReturnKeyStartsNewLine(false);
    binDisplay->setReadOnly(false);
    binDisplay->setScrollbarsShown(true);
    binDisplay->setCaretVisible(true);
    binDisplay->setPopupMenuEnabled(true);
    binDisplay->setText("01100000 01100000");

    calcLabel3 = std::make_unique<Label>("calcLabel3", "Decimal");
    addAndMakeVisible(calcLabel3.get());
    calcLabel3->setFont(Font(24.00f, Font::plain));
    calcLabel3->setJustificationType(Justification::centredLeft);
    calcLabel3->setEditable(false, false, false);
    calcLabel3->setColour(TextEditor::textColourId, Colours::black);
    calcLabel3->setColour(TextEditor::backgroundColourId, Colour(0x00000000));

    decDisplay = std::make_unique<TextEditor>("decDisplay");
    addAndMakeVisible(decDisplay.get());

    decDisplay->setMultiLine(false);
    decDisplay->setReturnKeyStartsNewLine(false);
    decDisplay->setReadOnly(false);
    decDisplay->setScrollbarsShown(true);
    decDisplay->setCaretVisible(true);
    decDisplay->setPopupMenuEnabled(true);
    decDisplay->setText("");

    hexFormat = std::make_unique<ComboBox>("hexFormat");
    addAndMakeVisible(hexFormat.get());
    hexFormat->setEditableText(false);
    hexFormat->setJustificationType(Justification::centredLeft);
    hexFormat->setTextWhenNothingSelected("Plain text");
    hexFormat->setTextWhenNoChoicesAvailable("(no choices)");
    hexFormat->addItem("Plain text", 1);
    hexFormat->addItem("LUA table format", 2);
    hexFormat->addItem("HEX string format", 3);
    hexFormat->addListener(this);

    bit16Switch = std::make_unique<ToggleButton>("bit16Switch");
    addAndMakeVisible(bit16Switch.get());
    bit16Switch->setButtonText("16bit");
    bit16Switch->addListener(this);

    //[UserPreSize]
    hexDisplay->clear();
    hexDisplay->setFont(Font(owner.getFontManager().getDefaultMonoFontName(), 24, Font::plain));
    hexDisplay->setText(L"F0");
    hexDisplay->addListener(this);

    binDisplay->clear();
    binDisplay->setFont(Font(owner.getFontManager().getDefaultMonoFontName(), 16, Font::plain));
    binDisplay->setText(L"11110000");
    binDisplay->addListener(this);

    decDisplay->clear();
    decDisplay->setFont(Font(owner.getFontManager().getDefaultMonoFontName(), 24, Font::plain));
    decDisplay->setText(L"240");
    decDisplay->addListener(this);
    setSize(400, 256);
}

CtrlrMIDICalculator::~CtrlrMIDICalculator()
{
    // hexDisplay = nullptr;
    // calcLabel = nullptr;
    // calcLabel2 = nullptr;
    // binDisplay = nullptr;
    // calcLabel3 = nullptr;
    // decDisplay = nullptr;
    // hexFormat = nullptr;
    // bit16Switch = nullptr;
}

void CtrlrMIDICalculator::paint(Graphics &g)
{
}

void CtrlrMIDICalculator::resized()
{
    hexDisplay->setBounds(8, 40, getWidth() - 16, 32);
    calcLabel->setBounds(8, 8, 150, 24);
    calcLabel2->setBounds(8, 80, 150, 24);
    binDisplay->setBounds(8, 112, getWidth() - 16, 64);
    calcLabel3->setBounds(8, 184, 150, 24);
    decDisplay->setBounds(8, 216, getWidth() - 16, 32);
    hexFormat->setBounds(168, 8, 150, 24);
    bit16Switch->setBounds((168) + (150), 8, 64, 24);
}

void CtrlrMIDICalculator::comboBoxChanged(ComboBox *comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == hexFormat.get())
    {
        formatData(binDisplay->getText(), false, false, true);
    }
}

void CtrlrMIDICalculator::buttonClicked(Button *buttonThatWasClicked)
{
    if (buttonThatWasClicked == bit16Switch.get())
    {
        formatData(hexDisplay->getText(), true, false, false);
    }
}

void CtrlrMIDICalculator::textEditorTextChanged(TextEditor &editor)
{
    if (&editor == decDisplay.get())
    {
        formatData(editor.getText(), false, true, false);
    }

    if (&editor == binDisplay.get())
    {
        formatData(editor.getText(), false, false, true);
    }

    if (&editor == hexDisplay.get())
    {
        formatData(editor.getText(), true, false, false);
    }
}

void CtrlrMIDICalculator::formatData(const String &data, const bool isHex, const bool isDec, const bool isBin)
{
    const bool bitsLen = bit16Switch->getToggleState();
    StringArray ar;
    ar.addTokens(data, " ;:\t", "\"\'");
    String bin, dec, hex;
    BigInteger bits(0);
    int d = 0;
    for (int i = 0; i < ar.size(); i++)
    {
        if (isHex)
        {
            d = ar[i].getHexValue32();
        }
        if (isDec)
        {
            d = ar[i].getIntValue();
        }
        if (isBin)
        {
            bits.parseString(ar[i], 2);
            d = bits.getBitRangeAsInt(0, bitsLen ? 16 : 8);
        }

        BigInteger bi(d);
        bin << bi.toString(2, bitsLen ? 16 : 8) + " ";
        dec << String::formatted("%.3d ", d);
        hex << formatHex(d);
    }
    if (!isBin)
        binDisplay->setText(bin.trim(), false);
    if (!isDec)
        decDisplay->setText(dec.trim(), false);
    if (!isHex)
        hexDisplay->setText(makeHexPretty(hex.trim()), false);
}

String CtrlrMIDICalculator::formatHex(const int d)
{
    if (hexFormat->getSelectedId() == 1)
    {
        return (String::formatted("%.3x ", d));
    }

    if (hexFormat->getSelectedId() == 2)
    {
        return (String::formatted("0x%.2x, ", d));
    }

    if (hexFormat->getSelectedId() == 3)
    {
        return (String::formatted("%.2x ", d));
    }

    return (String::formatted("%.3x ", d));
}

String CtrlrMIDICalculator::makeHexPretty(const String &hex)
{
    if (hexFormat->getSelectedId() == 2)
    {
        return ("{" + hex.substring(0, hex.length() - 1) + "}");
    }

    return (hex);
}
