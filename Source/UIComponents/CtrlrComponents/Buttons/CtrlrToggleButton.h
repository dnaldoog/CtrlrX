#ifndef __JUCER_HEADER_CTRLRTOGGLEBUTTON_CTRLRTOGGLEBUTTON_74F5E916__
#define __JUCER_HEADER_CTRLRTOGGLEBUTTON_CTRLRTOGGLEBUTTON_74F5E916__

#include "CtrlrComponents/CtrlrComponent.h"

class CtrlrValueMap;

class CtrlrToggleButton : public CtrlrComponent, public Button::Listener, public LookAndFeel_V4
{
public:
	//==============================================================================
	CtrlrToggleButton(CtrlrModulator &owner);
	~CtrlrToggleButton();

	//==============================================================================
	//[UserMethods]     -- You can add your own custom methods in this section.
	void setComponentValue(const double newValue, const bool sendChangeMessage = false);
	void setComponentMidiValue(const int newValue, const bool sendChangeMessage = false);
	double getComponentValue();
	int getComponentMidiValue();
	double getComponentMaxValue();
	bool getToggleState();
	void valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property);
	void valueTreeChildrenChanged(ValueTree &treeWhoseChildHasChanged) {}
	void valueTreeParentChanged(ValueTree &treeWhoseParentHasChanged) {}
	void valueTreeChildAdded(ValueTree &parentTree, ValueTree &childWhichHasBeenAdded) {}
	void valueTreeChildRemoved(ValueTree &parentTree, ValueTree &childWhichHasBeenRemoved, int) {}
	void valueTreeChildOrderChanged(ValueTree &parentTreeWhoseChildrenHaveMoved, int, int) {}
	void updateComponentColors();
	void click();
	bool isToggleButton();
	void setToggleState(const bool toggleState, const bool sendChangeMessage = false);
	static std::unique_ptr<juce::LookAndFeel>
	getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty);
	void resetLookAndFeelOverrides();
	void lookAndFeelChanged() override;
	void updatePropertiesPanel();
	CtrlrValueMap &getValueMap() { return (*valueMap); }
	// void customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel = nullptr) {}

	static void wrapForLua(lua_State *L);
	//[/UserMethods]

	void paint(Graphics &g);
	void resized();
	void buttonClicked(Button *buttonThatWasClicked);
	void mouseDown(const MouseEvent &e);
	std::unique_ptr<juce::LookAndFeel> customLF;

	//==============================================================================
juce_UseDebuggingNewOperator

	private :
	//[UserVariables]   -- You can add your own custom variables in this section.
	bool applyingLNFDefaults = false;
	std::unique_ptr<CtrlrValueMap>
		valueMap;
	//[/UserVariables]

	//==============================================================================
	std::unique_ptr<ToggleButton> ctrlrButton;

	//==============================================================================
	// (prevent copy constructor and operator= being generated..)
	CtrlrToggleButton(const CtrlrToggleButton &);
	const CtrlrToggleButton &operator=(const CtrlrToggleButton &);
	 void updateInspectorLabelsForRadioStyle();
};

#endif // __JUCER_HEADER_CTRLRTOGGLEBUTTON_CTRLRTOGGLEBUTTON_74F5E916__
