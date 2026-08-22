#ifndef __JUCER_HEADER_CTRLRFIXEDSLIDER_CTRLRFIXEDSLIDER_AD4513E7__
#define __JUCER_HEADER_CTRLRFIXEDSLIDER_CTRLRFIXEDSLIDER_AD4513E7__

#include "CtrlrComponents/CtrlrComponent.h"
#include "CtrlrSliderInternal.h"

class CtrlrValueMap;
class CtrlrSliderLookAndFeel;
struct lua_State;

class CtrlrFixedSlider : public CtrlrComponent, public juce::SettableTooltipClient, public juce::Slider::Listener {
	public:
		//==============================================================================
		CtrlrFixedSlider(CtrlrModulator &owner);
		~CtrlrFixedSlider() override;

		//==============================================================================
		// Methods
		void sliderValueChanged(juce::Slider *sliderThatWasMoved) override;
		void setComponentValue(const double newValue, const bool sendChangeMessage = false);
		double getComponentValue();
		int getComponentMidiValue();
		double getComponentMaxValue();
		const juce::String getComponentText();
		void sliderContentChanged();
		const juce::String getTextForValue(const double value);

		// ValueTree Property Listener
		void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
									  const juce::Identifier &property) override;

		juce::Slider *getOwnedSlider() {
			return ctrlrSlider.get();
		}
		CtrlrValueMap &getValueMap() {
			return *valueMap;
		}

		// LookAndFeel & Properties
		static std::unique_ptr<juce::LookAndFeel>
		getLookAndFeelFromComponentProperty(const juce::String &lookAndFeelComponentProperty);
		void resetLookAndFeelOverrides();
		void updatePropertiesPanel();
		void lookAndFeelChanged() override;
		void customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel = nullptr);
		void updateComponentColors();
		// void updateComponentFonts();
		const juce::String getCurrentLF();

		static void wrapForLua(lua_State *L);

		void paint(juce::Graphics &g) override;
		void resized() override;
		void mouseUp(const juce::MouseEvent &e) override;

	private:
		std::unique_ptr<CtrlrValueMap> valueMap;
		std::unique_ptr<juce::LookAndFeel> customLF;
		std::unique_ptr<CtrlrSliderInternal> ctrlrSlider;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrFixedSlider)
};

#endif // __JUCER_HEADER_CTRLRFIXEDSLIDER_CTRLRFIXEDSLIDER_AD4513E7__