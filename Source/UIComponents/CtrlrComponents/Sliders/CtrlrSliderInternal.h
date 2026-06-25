#ifndef __CTRLR_SLIDER_INTERNAL__
#define __CTRLR_SLIDER_INTERNAL__

#include "../CtrlrComponentTypeManager.h"
#include "CtrlrMacros.h"
#include "CtrlrUtilities.h"
#include "CtrlrUtilitiesGUI.h"
#include "CtrlrPanel/CtrlrPanel.h"

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

class CtrlrSliderLookAndFeel_V2 : public LookAndFeel_V2
{
	public:
        CtrlrSliderLookAndFeel_V2(CtrlrComponent &_owner, ValueTree &_ownerTree) : ownerTree(_ownerTree), owner(_owner)
		{
		    setColour (0xdeadbeed, Colour(0x00000000));
		}

		Button *createSliderButton (Slider&, bool isIncrement)
		{
            auto* tb = new juce::TextButton (isIncrement ? "+" : "-", "");
			//TextButton *tb = new TextButton (isIncrement ? "+" : "-", "");
    // Removed: tb->setLookAndFeel(&LookAndFeel::getDefaultLookAndFeel());
    // This button should inherit its LookAndFeel from its parent component,
    // not be explicitly wired to whatever JUCE's static default happens to be
    // at construction time — that line was pulling in a lazily-created JUCE
    // singleton LookAndFeel_V4 unrelated to anything CtrlrX manages.
			tb->setColour (TextButton::buttonColourId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecButtonColour)));
			tb->setColour (TextButton::textColourOffId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecTextColour)));
			return (tb);
		}

                //     ~CtrlrSliderLookAndFeel_V2() override
                // {

                //     juce::ImageCache::releaseUnusedImages();
                // }

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
    // Removed: tb->setLookAndFeel(&LookAndFeel::getDefaultLookAndFeel());
    // This button should inherit its LookAndFeel from its parent component,
    // not be explicitly wired to whatever JUCE's static default happens to be
    // at construction time — that line was pulling in a lazily-created JUCE
    // singleton LookAndFeel_V4 unrelated to anything CtrlrX manages.
        tb->setColour (TextButton::buttonColourId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecButtonColour)));
        tb->setColour (TextButton::textColourOffId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecTextColour)));
        return (tb);
    }
            // ~CtrlrSliderLookAndFeel_V3() override
            //     {
            //         // Force JUCE to drop any cached vector path data built by this skin
            //         // before the memory address is unmapped!
            //        juce::ImageCache::releaseUnusedImages();
            //     }

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
    // Removed: tb->setLookAndFeel(&LookAndFeel::getDefaultLookAndFeel());
    // This button should inherit its LookAndFeel from its parent component,
    // not be explicitly wired to whatever JUCE's static default happens to be
    // at construction time — that line was pulling in a lazily-created JUCE
    // singleton LookAndFeel_V4 unrelated to anything CtrlrX manages.
        tb->setColour (TextButton::buttonColourId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecButtonColour)));
        tb->setColour (TextButton::textColourOffId, VAR2COLOUR(ownerTree.getProperty(Ids::uiSliderIncDecTextColour)));
        return (tb);
    }
                //     ~CtrlrSliderLookAndFeel_V4() override
                //         {
                //             // Force JUCE to drop any cached vector path data built by this skin
                //             // before the memory address is unmapped!
                //             juce::ImageCache::releaseUnusedImages();
                //         }
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

#endif
