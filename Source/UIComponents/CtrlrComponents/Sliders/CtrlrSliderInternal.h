#ifndef __CTRLR_SLIDER_INTERNAL__
#define __CTRLR_SLIDER_INTERNAL__

#include "../CtrlrComponentTypeManager.h"
#include "CtrlrMacros.h"
#include "CtrlrUtilities.h"
#include "CtrlrUtilitiesGUI.h"
#include "CtrlrPanel/CtrlrPanel.h"

class CtrlrSliderLookAndFeelBase : public juce::LookAndFeel_V4 
{
public:
    CtrlrSliderLookAndFeelBase (CtrlrComponent& _owner, juce::ValueTree& _ownerTree)
        : ownerTree (_ownerTree), owner (_owner)
    {
    }

~CtrlrSliderLookAndFeelBase() override
    {
        // Tells the underlying LookAndFeel engine to clear out its local style maps
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    }

protected:
    juce::ValueTree& ownerTree;
    CtrlrComponent& owner;
};
#if 0
class CtrlrSliderLookAndFeelBase : public juce::LookAndFeel_V4 // Default to V4 base plumbing
{
public:
    CtrlrSliderLookAndFeelBase (CtrlrComponent& _owner, juce::ValueTree& _ownerTree)
        : ownerTree (_ownerTree), owner (_owner)
    {}

    ~CtrlrSliderLookAndFeelBase() override = default;

    // --- RECOVERED FUNCTIONALITY 1: Custom Property Textboxes ---
    juce::Label* createSliderTextBox (juce::Slider& slider) override
    {
        auto* const l = new juce::Label();
        
        // Safe font fetching from manager
        l->setFont (owner.getOwner().getOwnerPanel().getCtrlrManagerOwner().getFontManager().getFontFromString (ownerTree.getProperty(Ids::uiSliderValueFont)));
        l->setJustificationType (justificationFromProperty(ownerTree.getProperty(Ids::uiSliderValueTextJustification)));
        
        l->setColour (juce::Label::textColourId, slider.findColour (juce::Slider::textBoxTextColourId));
        l->setColour (juce::Label::backgroundColourId, (slider.getSliderStyle() == juce::Slider::LinearBar) ? juce::Colours::transparentBlack : slider.findColour (juce::Slider::textBoxBackgroundColourId));
        l->setColour (juce::Label::outlineColourId, slider.findColour (juce::Slider::textBoxOutlineColourId));
        l->setColour (juce::TextEditor::textColourId, slider.findColour (juce::Slider::textBoxTextColourId));
        l->setColour (juce::TextEditor::backgroundColourId, slider.findColour (juce::Slider::textBoxBackgroundColourId).withAlpha (slider.getSliderStyle() == juce::Slider::LinearBar ? 0.7f : 1.0f));
        l->setColour (juce::TextEditor::outlineColourId, slider.findColour (juce::Slider::textBoxOutlineColourId));
        return l;
    }

    // --- RECOVERED FUNCTIONALITY 2: Custom Property Navigation Buttons ---
    juce::Button* createSliderButton (juce::Slider&, bool isIncrement) override
    {
        auto* tb = new juce::TextButton (isIncrement ? "+" : "-", "");
        tb->setLookAndFeel (&juce::LookAndFeel::getDefaultLookAndFeel());
        tb->setColour (juce::TextButton::buttonColourId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecButtonColour)));
        tb->setColour (juce::TextButton::textColourOffId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecTextColour)));
        return tb;
    }

protected:
    juce::ValueTree& ownerTree;
    CtrlrComponent& owner;
};

#endif // __CTRLR_SLIDER_INTERNAL__
class CtrlrSliderInternal : public Slider
{
	public:
		CtrlrSliderInternal (CtrlrComponent &_owner);
		~CtrlrSliderInternal();
		String getTextFromValue(double value); // Updated v5.6.32
		//#if !CTLRX_DISABLE_DYNAMIC_LNF
    		double getValueFromText (const String& text); // Added v5.6.32
		//#endif
		void mouseWheelMove (const MouseEvent &e, const MouseWheelDetails& wheel);
		JUCE_LEAK_DETECTOR(CtrlrSliderInternal)

	private:
		CtrlrComponent &owner;
};

#if 0
class CtrlrSliderLookAndFeel_V2 : public LookAndFeel_V2
{
	public:
        CtrlrSliderLookAndFeel_V2(CtrlrComponent &_owner, ValueTree &_ownerTree) : ownerTree(_ownerTree), owner(_owner)
		{
		    setColour (0xdeadbeed, Colour(0x00000000));
		}

		Button *createSliderButton (Slider&, bool isIncrement)
		{
			TextButton *tb = new TextButton (isIncrement ? "+" : "-", "");
			tb->setLookAndFeel (&LookAndFeel::getDefaultLookAndFeel());
			tb->setColour (TextButton::buttonColourId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecButtonColour)));
			tb->setColour (TextButton::textColourOffId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecTextColour)));
			return (tb);
		}

		Label* createSliderTextBox (Slider& slider)
		{
			Label* const l = new Label();
			l->setFont(owner.getOwner().getOwnerPanel().getCtrlrManagerOwner().getFontManager().getFontFromString (ownerTree.getProperty(Ids::uiSliderValueFont)));
			l->setJustificationType (justificationFromProperty(ownerTree.getProperty(Ids::uiSliderValueTextJustification)));
			l->setColour (Label::textColourId, slider.findColour (Slider::textBoxTextColourId));
			l->setColour (Label::backgroundColourId, (slider.getSliderStyle() == Slider::LinearBar) ? Colours::transparentBlack : slider.findColour (Slider::textBoxBackgroundColourId));
			l->setColour (Label::outlineColourId, slider.findColour (Slider::textBoxOutlineColourId));
			l->setColour (TextEditor::textColourId, slider.findColour (Slider::textBoxTextColourId));
			l->setColour (TextEditor::backgroundColourId, slider.findColour (Slider::textBoxBackgroundColourId).withAlpha (slider.getSliderStyle() == Slider::LinearBar ? 0.7f : 1.0f));
			l->setColour (TextEditor::outlineColourId, slider.findColour (Slider::textBoxOutlineColourId));
			return l;
		}

		Colour createBaseColour (const Colour& buttonColour, const bool hasKeyboardFocus, const bool isMouseOverButton, const bool isButtonDown) noexcept
		{
			const float sat = hasKeyboardFocus ? 1.3f : 0.9f;
			const Colour baseColour (buttonColour.withMultipliedSaturation (sat));

			if (isButtonDown)
	            return baseColour.contrasting (0.2f);
			else if (isMouseOverButton)
	            return baseColour.contrasting (0.1f);

			return baseColour;
		}

		int getSliderThumbRadius (Slider& slider)
		{
			return jmin (7,slider.getHeight() / 2,slider.getWidth() / 2) + 2;
		}
		Colour findColour (int colourId) const noexcept;
		int getSliderPopupPlacement(Slider &);
		Font getSliderPopupFont(Slider &);
		void drawLinearSliderBackground (Graphics& g, int x, int y, int width, int height, float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/, const Slider::SliderStyle /*style*/, Slider& slider);
		void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider);

	private:
		ValueTree &ownerTree;
		CtrlrComponent &owner;
};

class CtrlrSliderLookAndFeel_V3 : public LookAndFeel_V3
{
public:
    CtrlrSliderLookAndFeel_V3(CtrlrComponent &_owner, ValueTree &_ownerTree) : ownerTree(_ownerTree), owner(_owner)
    {
        setColour (0xdeadbeed, Colour(0x00000000));
    }

    Button *createSliderButton (Slider&, bool isIncrement)
    {
        TextButton *tb = new TextButton (isIncrement ? "+" : "-", "");
        tb->setLookAndFeel (&LookAndFeel::getDefaultLookAndFeel());
        tb->setColour (TextButton::buttonColourId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecButtonColour)));
        tb->setColour (TextButton::textColourOffId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecTextColour)));
        return (tb);
    }

    Label* createSliderTextBox (Slider& slider)
    {
        Label* const l = new Label();
        l->setFont(owner.getOwner().getOwnerPanel().getCtrlrManagerOwner().getFontManager().getFontFromString (ownerTree.getProperty(Ids::uiSliderValueFont)));
        l->setJustificationType (justificationFromProperty(ownerTree.getProperty(Ids::uiSliderValueTextJustification)));
        l->setColour (Label::textColourId, slider.findColour (Slider::textBoxTextColourId));
        l->setColour (Label::backgroundColourId, (slider.getSliderStyle() == Slider::LinearBar) ? Colours::transparentBlack : slider.findColour (Slider::textBoxBackgroundColourId));
        l->setColour (Label::outlineColourId, slider.findColour (Slider::textBoxOutlineColourId));
        l->setColour (TextEditor::textColourId, slider.findColour (Slider::textBoxTextColourId));
        l->setColour (TextEditor::backgroundColourId, slider.findColour (Slider::textBoxBackgroundColourId).withAlpha (slider.getSliderStyle() == Slider::LinearBar ? 0.7f : 1.0f));
        l->setColour (TextEditor::outlineColourId, slider.findColour (Slider::textBoxOutlineColourId));
        return l;
    }

    Colour createBaseColour (const Colour& buttonColour, const bool hasKeyboardFocus, const bool isMouseOverButton, const bool isButtonDown) noexcept
    {
        const float sat = hasKeyboardFocus ? 1.3f : 0.9f;
        const Colour baseColour (buttonColour.withMultipliedSaturation (sat));

        if (isButtonDown)
            return baseColour.contrasting (0.2f);
        else if (isMouseOverButton)
            return baseColour.contrasting (0.1f);

        return baseColour;
    }

    int getSliderThumbRadius (Slider& slider)
    {
        return jmin (7,slider.getHeight() / 2,slider.getWidth() / 2) + 2;
    }
    Colour findColour (int colourId) const noexcept;
    int getSliderPopupPlacement(Slider &);
    Font getSliderPopupFont(Slider &);
    void drawLinearSliderBackground (Graphics& g, int x, int y, int width, int height, float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/, const Slider::SliderStyle /*style*/, Slider& slider);
    void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider);

private:
    ValueTree &ownerTree;
    CtrlrComponent &owner;
};

class CtrlrSliderLookAndFeel_V4 : public LookAndFeel_V4
{
public:
    CtrlrSliderLookAndFeel_V4(CtrlrComponent &_owner, ValueTree &_ownerTree) : ownerTree(_ownerTree), owner(_owner)
    {
        //setColour (0xdeadbeed, Colour(0x00000000));
    }

    Button *createSliderButton (Slider&, bool isIncrement)
    {
        TextButton *tb = new TextButton (isIncrement ? "+" : "-", "");
        tb->setLookAndFeel (&LookAndFeel::getDefaultLookAndFeel());
        tb->setColour (TextButton::buttonColourId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecButtonColour)));
        tb->setColour (TextButton::textColourOffId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecTextColour)));
        return (tb);
    }

    Label* createSliderTextBox (Slider& slider)
    {
        Label* const l = new Label();
        l->setFont(owner.getOwner().getOwnerPanel().getCtrlrManagerOwner().getFontManager().getFontFromString (ownerTree.getProperty(Ids::uiSliderValueFont)));
        l->setJustificationType (justificationFromProperty(ownerTree.getProperty(Ids::uiSliderValueTextJustification)));
        l->setColour (Label::textColourId, slider.findColour (Slider::textBoxTextColourId));
        l->setColour (Label::backgroundColourId, (slider.getSliderStyle() == Slider::LinearBar) ? Colours::transparentBlack : slider.findColour (Slider::textBoxBackgroundColourId));
        l->setColour (Label::outlineColourId, slider.findColour (Slider::textBoxOutlineColourId));
        l->setColour (TextEditor::textColourId, slider.findColour (Slider::textBoxTextColourId));
        l->setColour (TextEditor::backgroundColourId, slider.findColour (Slider::textBoxBackgroundColourId).withAlpha (slider.getSliderStyle() == Slider::LinearBar ? 0.7f : 1.0f));
        l->setColour (TextEditor::outlineColourId, slider.findColour (Slider::textBoxOutlineColourId));
        return l;
    }

    Colour createBaseColour (const Colour& buttonColour, const bool hasKeyboardFocus, const bool isMouseOverButton, const bool isButtonDown) noexcept
    {
        const float sat = hasKeyboardFocus ? 1.3f : 0.9f;
        const Colour baseColour (buttonColour.withMultipliedSaturation (sat));

        if (isButtonDown)
            return baseColour.contrasting (0.2f);
        else if (isMouseOverButton)
            return baseColour.contrasting (0.1f);

        return baseColour;
    }

    int getSliderThumbRadius (Slider& slider)
    {
        return jmin (7,slider.getHeight() / 2,slider.getWidth() / 2) + 2;
    }
    
    Colour findColour (int colourId) const noexcept;
    
    int getSliderPopupPlacement(Slider &);
    
    Font getSliderPopupFont(Slider &);
    
    void drawLinearSliderBackground (Graphics& g, int x, int y, int width, int height, float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/, const Slider::SliderStyle /*style*/, Slider& slider);
    
    void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider);
    
private:
    
    ValueTree &ownerTree;
    CtrlrComponent &owner;
};
#else
#if CTLRX_DISABLE_DYNAMIC_LNF

class CtrlrSliderLookAndFeel_V2 : public juce::LookAndFeel_V2
{
public:
    // Keep the original constructor signature and initializer list intact
    CtrlrSliderLookAndFeel_V2(CtrlrComponent& _owner, ValueTree& _ownerTree)
        : ownerTree(_ownerTree),
        owner(_owner)
    {
    }

    // Explicitly clean up virtually 
    ~CtrlrSliderLookAndFeel_V2() override = default;

    // By not declaring any drawLinearSliderThumb or drawRotarySlider methods here,
    // this class acts as a transparent window straight down to juce::LookAndFeel_V2.

private:
    CtrlrComponent& owner;
    ValueTree& ownerTree;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrSliderLookAndFeel_V2)
};

#else // ==================== DYNAMIC PATH ====================

class CtrlrSliderLookAndFeel_V1 : public CtrlrSliderLookAndFeelBase
{
public:
    CtrlrSliderLookAndFeel_V1 (CtrlrComponent& _owner, ValueTree& _ownerTree)
        : CtrlrSliderLookAndFeelBase (_owner, _ownerTree) {}

    ~CtrlrSliderLookAndFeel_V1() 
        {
    // Force the internal LookAndFeel structures to drop cached shapes/cursors
    // before the destructor exits
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}

    // Route drawing safely to a single shared static instance
    void drawLinearSliderBackground (juce::Graphics& g, int x, int y, int w, int h, float pos, float min, float max, const juce::Slider::SliderStyle s, juce::Slider& sl) override
    { getNativeV1().drawLinearSliderBackground(g, x, y, w, h, pos, min, max, s, sl); }
    
    void drawLinearSliderThumb (juce::Graphics& g, int x, int y, int w, int h, float pos, float min, float max, const juce::Slider::SliderStyle s, juce::Slider& sl) override
    { getNativeV1().drawLinearSliderThumb(g, x, y, w, h, pos, min, max, s, sl); }

private:
    // Lazy-loaded single instance shared across all V1 sliders
    static juce::LookAndFeel_V1& getNativeV1() {
        static juce::LookAndFeel_V1 instance;
        return instance;
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CtrlrSliderLookAndFeel_V1)
};

class CtrlrSliderLookAndFeel_V2 : public CtrlrSliderLookAndFeelBase
{
public:
    CtrlrSliderLookAndFeel_V2 (CtrlrComponent& _owner, ValueTree& _ownerTree)
        : CtrlrSliderLookAndFeelBase (_owner, _ownerTree) {}

    ~CtrlrSliderLookAndFeel_V2() override
    {
    // Force the internal LookAndFeel structures to drop cached shapes/cursors
    // before the destructor exits
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}
    void drawLinearSliderBackground (juce::Graphics& g, int x, int y, int w, int h, float pos, float min, float max, const juce::Slider::SliderStyle s, juce::Slider& sl) override
    { getNativeV2().drawLinearSliderBackground(g, x, y, w, h, pos, min, max, s, sl); }
    
    void drawLinearSliderThumb (juce::Graphics& g, int x, int y, int w, int h, float pos, float min, float max, const juce::Slider::SliderStyle s, juce::Slider& sl) override
    { getNativeV2().drawLinearSliderThumb(g, x, y, w, h, pos, min, max, s, sl); }

private:
    static juce::LookAndFeel_V2& getNativeV2() {
        static juce::LookAndFeel_V2 instance;
        return instance;
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CtrlrSliderLookAndFeel_V2)
};

class CtrlrSliderLookAndFeel_V3 : public CtrlrSliderLookAndFeelBase
{
public:
    CtrlrSliderLookAndFeel_V3 (CtrlrComponent& _owner, ValueTree& _ownerTree)
        : CtrlrSliderLookAndFeelBase (_owner, _ownerTree) {}

    ~CtrlrSliderLookAndFeel_V3() override
        {
    // Force the internal LookAndFeel structures to drop cached shapes/cursors
    // before the destructor exits
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}

    void drawLinearSliderBackground (juce::Graphics& g, int x, int y, int w, int h, float pos, float min, float max, const juce::Slider::SliderStyle s, juce::Slider& sl) override
    { getNativeV3().drawLinearSliderBackground(g, x, y, w, h, pos, min, max, s, sl); }
    
    void drawLinearSliderThumb (juce::Graphics& g, int x, int y, int w, int h, float pos, float min, float max, const juce::Slider::SliderStyle s, juce::Slider& sl) override
    { getNativeV3().drawLinearSliderThumb(g, x, y, w, h, pos, min, max, s, sl); }

private:
    static juce::LookAndFeel_V3& getNativeV3() {
        static juce::LookAndFeel_V3 instance;
        return instance;
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CtrlrSliderLookAndFeel_V3)
};

class CtrlrSliderLookAndFeel_V4 : public CtrlrSliderLookAndFeelBase
{
public:
    CtrlrSliderLookAndFeel_V4 (CtrlrComponent& _owner, ValueTree& _ownerTree)
        : CtrlrSliderLookAndFeelBase (_owner, _ownerTree) {}

    ~CtrlrSliderLookAndFeel_V4() override
        {
    // Force the internal LookAndFeel structures to drop cached shapes/cursors
    // before the destructor exits
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CtrlrSliderLookAndFeel_V4)
};


#endif
#endif
#endif
