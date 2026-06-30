#ifndef __JUCER_HEADER_CTRLRFIXEDSLIDER_CTRLRFIXEDSLIDER_AD4513E7__
#define __JUCER_HEADER_CTRLRFIXEDSLIDER_CTRLRFIXEDSLIDER_AD4513E7__

#include "CtrlrComponents/CtrlrComponent.h"
#include "CtrlrSliderInternal.h"

class CtrlrValueMap;
class CtrlrSliderLookAndFeel;

class CtrlrFixedSlider : public CtrlrComponent, public SettableTooltipClient, public Slider::Listener {
	public:
		//==============================================================================
		CtrlrFixedSlider(CtrlrModulator &owner);
		~CtrlrFixedSlider();

		//==============================================================================
		//[UserMethods]     -- You can add your own custom methods in this section.
		void sliderValueChanged(Slider *sliderThatWasMoved);
		void setComponentValue(const double newValue, const bool sendChangeMessage = false);
		double getComponentValue();
		int getComponentMidiValue();
		double getComponentMaxValue();
		const String getComponentText();
		void sliderContentChanged();
		const String getTextForValue(const double value);
		void valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property);
		void valueTreeChildrenChanged(ValueTree &treeWhoseChildHasChanged) {}
		void valueTreeParentChanged(ValueTree &treeWhoseParentHasChanged) {}
		void valueTreeChildAdded(ValueTree &parentTree, ValueTree &childWhichHasBeenAdded) {}
		void valueTreeChildRemoved(ValueTree &parentTree, ValueTree &childWhichHasBeenRemoved, int) {}
		void valueTreeChildOrderChanged(ValueTree &parentTreeWhoseChildrenHaveMoved, int, int) {}
		Slider *getOwnedSlider() { return (ctrlrSlider.get()); }
		CtrlrValueMap &getValueMap() { return (*valueMap); }
		void customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel = nullptr) {}; // trailing ; unnecessary
		static std::unique_ptr<juce::LookAndFeel>
		getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty);
		void resetLookAndFeelOverrides();
		void updatePropertiesPanel();
#endif
		static void wrapForLua(lua_State *L);
		//[/UserMethods]

		void paint(Graphics &g);
		void resized();
		void mouseUp(const MouseEvent &e);

		//==============================================================================
	juce_UseDebuggingNewOperator

		private :
		//[UserVariables]   -- You can add your own custom variables in this section.

		std::unique_ptr<CtrlrValueMap>
			valueMap;
		//==============================================================================
		std::unique_ptr<Slider> ctrlrSlider;
		std::unique_ptr<juce::LookAndFeel> customLF;
		//==============================================================================
		// (prevent copy constructor and operator= being generated..)
		CtrlrFixedSlider(const CtrlrFixedSlider &);
		const CtrlrFixedSlider &operator=(const CtrlrFixedSlider &);
};

// __JUCER_HEADER_CTRLRFIXEDSLIDER_CTRLRFIXEDSLIDER_AD4513E7__
