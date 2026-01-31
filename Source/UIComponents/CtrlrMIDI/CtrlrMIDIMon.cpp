#include "stdafx.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrUtilities.h"
#include "CtrlrProcessor.h"
#include "CtrlrMIDIMon.h"

CtrlrMIDIMon::CtrlrMIDIMon(CtrlrManager& _owner)
    : owner(_owner), logIn(false), logOut(false),
    resizer(0),
    outMon(0),
    inMon(0)
{
    addAndMakeVisible(resizer = new StretchableLayoutResizerBar(&layoutManager, 1, false));

    addAndMakeVisible(outMon = new CodeEditorComponent(outputDocument, 0));
    outMon->setName(L"outMon");

    addAndMakeVisible(inMon = new CodeEditorComponent(inputDocument, 0));
    inMon->setName(L"inMon");

    layoutManager.setItemLayout(0, -0.001, -1.0, -0.49);
    layoutManager.setItemLayout(1, -0.001, -0.01, -0.01);
    layoutManager.setItemLayout(2, -0.001, -1.0, -0.49);

    owner.getCtrlrLog().addListener(this);

    inMon->setFont(Font(owner.getFontManager().getDefaultMonoFontName(), 12, Font::plain));
    inMon->setColour(CodeEditorComponent::backgroundColourId, Colour(0xffb3ffac));
    outMon->setFont(Font(owner.getFontManager().getDefaultMonoFontName(), 12, Font::plain));
    outMon->setColour(CodeEditorComponent::backgroundColourId, Colour(0xffffacac));

    setSize(500, 400);
}

CtrlrMIDIMon::~CtrlrMIDIMon()
{
    owner.getCtrlrLog().removeListener(this);
    deleteAndZero(resizer);
    deleteAndZero(outMon);
    deleteAndZero(inMon);
}

void CtrlrMIDIMon::paint(Graphics& g)
{
}

void CtrlrMIDIMon::resized()
{
    resizer->setBounds(0, proportionOfHeight(0.4900f), getWidth() - 0, proportionOfHeight(0.0100f));
    outMon->setBounds(0, 0, getWidth() - 0, proportionOfHeight(0.4900f));
    inMon->setBounds(0, proportionOfHeight(0.5000f), getWidth() - 0, proportionOfHeight(0.4900f));
    Component* comps[] = { outMon, resizer, inMon };
    layoutManager.layOutComponents(comps, 3, 0, 0, getWidth(), getHeight(), true, true);
}

void CtrlrMIDIMon::messageLogged(CtrlrLog::CtrlrLogMessage _message) // Updated v5.6.35. MIDI filters Support. Thanks to @dnaldoog
{
    if (!isVisible())
        return;

    int filters = (int)owner.getProperty(Ids::ctrlrMidiFilters);

    // Only filter if we have actual MIDI data
    if (_message.hasMidiData && (_message.level == CtrlrLog::MidiIn || _message.level == CtrlrLog::MidiOut))
    {
        if (shouldFilterMessage(_message.midiData, filters))
        {
            DBG("Message filtered out!");
            return;
        }
    }

    if (_message.level == CtrlrLog::MidiIn)
    {
        inMon->insertTextAtCaret(_message.message + "\n");
        inMon->scrollToKeepCaretOnScreen(); // Keep the new data in view
    }
    else if (_message.level == CtrlrLog::MidiOut)
    {
        outMon->insertTextAtCaret(_message.message + "\n");
        outMon->scrollToKeepCaretOnScreen(); // Keep the new data in view
    }
}

bool CtrlrMIDIMon::shouldFilterMessage(const MidiMessage& m, int filterMask) // Added v5.6.35. MIDI filters Support. Thanks to @dnaldoog
{
    DBG("Checking message against filter mask: " + String(filterMask));
    if (filterMask == 0) return false;

    if ((filterMask & Filter_NoteOn) && m.isNoteOn())           return true;
    if ((filterMask & Filter_NoteOff) && m.isNoteOff())         return true;
    if ((filterMask & Filter_AftertouchPoly) && m.isAftertouch()) return true;
    if ((filterMask & Filter_Control) && m.isController())      return true;
    if ((filterMask & Filter_Program) && m.isProgramChange())   return true;
    if ((filterMask & Filter_Sysex) && m.isSysEx())             return true;
    if ((filterMask & Filter_ChannelPressure) && m.isChannelPressure()) return true;
    if ((filterMask & Filter_PitchWheel) && m.isPitchWheel())   return true;
    if ((filterMask & Filter_ActiveSense) && m.isActiveSense()) return true;
    if ((filterMask & Filter_Clock) && m.isMidiClock()) return true;

    return false;
}

StringArray CtrlrMIDIMon::getMenuBarNames()
{
    const char* const names[] = { "Window", "View", "Filter", nullptr }; // Updated v5.6.35. MIDI Message Type Filters Support. Thanks to @dnaldoog
    return StringArray(names);
}

PopupMenu CtrlrMIDIMon::getMenuForIndex(int topLevelMenuIndex, const String& menuName)
{
    PopupMenu menu;

    int opts = (int)owner.getProperty(Ids::ctrlrLogOptions);
    int filters = (int)owner.getProperty(Ids::ctrlrMidiFilters);

    if (topLevelMenuIndex == 0)
    {
        menu.addItem(2, "Clear Input");
        menu.addItem(3, "Clear Output");
        menu.addSeparator();
        menu.addItem(1, "Close");
    }
    else if (topLevelMenuIndex == 1)
    {
        // Existing log options...
        menu.addItem(10 + 2, "Show name", true, getBitOption(opts, midiLogName));
        menu.addItem(10 + 4, "Show channel", true, getBitOption(opts, midiLogChannel));
        menu.addItem(10 + 8, "Show number", true, getBitOption(opts, midiLogNumber));
        menu.addItem(10 + 16, "Show value", true, getBitOption(opts, midiLogValue));
        menu.addItem(10 + 32, "Show RAW data", true, getBitOption(opts, midiLogRawData));
        menu.addItem(10 + 64, "Show timestamp", true, getBitOption(opts, midiLogTimestamp));
        menu.addItem(10 + 128, "RAW data in decimal", true, getBitOption(opts, midiLogRawDecimal));
        menu.addItem(10 + 1024, "Show device name", true, getBitOption(opts, midiLogDevice));
        menu.addItem(10 + 4096, "Show RAW data size", true, getBitOption(opts, midiLogDataSize));
        menu.addSeparator();
        menu.addColouredItem(10 + 256, "Monitor input", Colour(0xff21c630), true, getBitOption(opts, midiLogInput));
        menu.addColouredItem(10 + 512, "Monitor output", Colour(0xffc62121), true, getBitOption(opts, midiLogOutput));
    }
    else if (topLevelMenuIndex == 2)
    {
        menu.addSectionHeader("Active filters");

        // We add 10000 to the enum value to create a unique Menu ID
        menu.addItem(10000 + Filter_NoteOn, "Note On", true, getBitOption(filters, Filter_NoteOn));
        menu.addItem(10000 + Filter_NoteOff, "Note Off", true, getBitOption(filters, Filter_NoteOff));
        menu.addItem(10000 + Filter_AftertouchPoly, "Aftertouch (Poly)", true, getBitOption(filters, Filter_AftertouchPoly));
        menu.addItem(10000 + Filter_Control, "Control", true, getBitOption(filters, Filter_Control));
        menu.addItem(10000 + Filter_Program, "Program", true, getBitOption(filters, Filter_Program));
        menu.addItem(10000 + Filter_Sysex, "Sysex", true, getBitOption(filters, Filter_Sysex));
        menu.addItem(10000 + Filter_ChannelPressure, "Channel Pressure", true, getBitOption(filters, Filter_ChannelPressure));
        menu.addItem(10000 + Filter_PitchWheel, "Pitch Wheel", true, getBitOption(filters, Filter_PitchWheel));
        menu.addItem(10000 + Filter_ActiveSense, "Active Sense", true, getBitOption(filters, Filter_ActiveSense));
        menu.addItem(10000 + Filter_Clock, "MIDI Clock", true, getBitOption(filters, Filter_Clock));
    }

    return menu;
}

void CtrlrMIDIMon::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (topLevelMenuIndex == 0) // Updated v5.6.31. From 1 to 0
    {
        if (menuItemID == 1)
        {
            // Handle close
            // Access the window manager through the owner (CtrlrManager)
            // and tell it to hide the MIDI Monitor window.
            owner.getWindowManager().hide(CtrlrManagerWindowManager::MidiMonWindow);
        }
        if (menuItemID == 2)
        {
            // Handle clear input log
            inputDocument.replaceAllContent("");
        }
        if (menuItemID == 3)
        {
            // Handle clear output log
            outputDocument.replaceAllContent("");
        }
    }
    else if (topLevelMenuIndex == 1) // View menu
    {
        int opts = (int)owner.getProperty(Ids::ctrlrLogOptions);
        int bitToFlip = menuItemID - 10;
        setBitOption(opts, bitToFlip, !getBitOption(opts, bitToFlip));
        owner.setProperty(Ids::ctrlrLogOptions, opts);
    }
    else if (topLevelMenuIndex == 2) // Filter menu - FIXED
    {
        int filters = (int)owner.getProperty(Ids::ctrlrMidiFilters);
        int bitToToggle = menuItemID - 10000; // This gives us the enum value (1, 2, 4, 8, etc.)

        // Toggle the bit using XOR
        filters ^= bitToToggle;

        owner.setProperty(Ids::ctrlrMidiFilters, filters);

        // Debug output to verify
        DBG("Filter toggled. New filter mask: " + String(filters));
    }
}
