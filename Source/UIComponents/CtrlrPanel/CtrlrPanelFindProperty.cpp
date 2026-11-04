/*
  ==============================================================================

	CtrlrPanelFindProperty.cpp
	Created: 4 Jan 2021 4:49:27pm
	Updated: 4 Nov 2026 9:13:00pm (Dnaldoog)
	Author:  atom

  ==============================================================================
*/

#include <JuceHeader.h>
#include "CtrlrPanelFindProperty.h"
#include "CtrlrPanelEditor.h"
#include "CtrlrPanelComponentProperties.h"

//==============================================================================
CtrlrPanelFindProperty::CtrlrPanelFindProperty(CtrlrPanelEditor &_owner, CtrlrPanelComponentProperties *_props)
	: owner(_owner), props(_props) {
	filterInput.reset(new TextEditor("Filter Input"));
	filterInput->setFont(filterInput->getFont().withHeight(24.0));
	filterInput->setTextToShowWhenEmpty("Search properties...", Colours::grey);
	filterInput->addListener(this);

	filterButton.reset(gui::createDrawableButton("Filter", BIN2STR(search_svg)));

	clearButton.reset(new TextButton(juce::String::fromUTF8("\xc3\x97"))); // ×
	clearButton->onClick = [this] {
		filterInput->clear();
		if (props)
			props->setFilter("");
		filterInput->grabKeyboardFocus();
	};

	addAndMakeVisible(filterInput.get());
	addAndMakeVisible(filterButton.get());
	addAndMakeVisible(clearButton.get());
}

CtrlrPanelFindProperty::~CtrlrPanelFindProperty()
{
}

void CtrlrPanelFindProperty::paint (juce::Graphics& g)
{
	g.fillAll(Colours::white);
}

void CtrlrPanelFindProperty::resized()
{
	filterButton->setBounds(0, 0, 32, 32);
	clearButton->setBounds(getWidth() - 32, 0, 32, 32);
	filterInput->setBounds(32, 0, getWidth() - 64, 32);
}

void CtrlrPanelFindProperty::textEditorTextChanged(TextEditor &e) {
	if (props)
		props->setFilter(e.getText().trim()); // don't write the return value back into the box
}

void CtrlrPanelFindProperty::textEditorReturnKeyPressed(TextEditor &e) {
	if (props)
		props->setFilter(e.getText().trim());
}

void CtrlrPanelFindProperty::textEditorEscapeKeyPressed (TextEditor &e)
{
	if (props)
	{
		props->setFilter("");
		e.setText("");
	}
}
