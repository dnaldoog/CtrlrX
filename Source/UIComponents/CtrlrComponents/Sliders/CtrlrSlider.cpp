#include "stdafx.h"
#include "CtrlrSlider.h"
#include "CtrlrLuaManager.h"
#include "CtrlrProcessor.h"
#include "../CtrlrComponentTypeManager.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrModulator/CtrlrModulator.h"
#include "Lua/JuceClasses/LLookAndFeel.h"


#if CTLRX_DISABLE_DYNAMIC_LNF
// ============================================================================
// LIGHTWEIGHT 5.3 FORK PATHWAY
// ============================================================================

CtrlrSlider::CtrlrSlider (CtrlrModulator &owner)
    :   CtrlrComponent(owner),
        lf(*this, componentTree), // Ensure this matches your 5.3 header variable name & constructor signature
        ctrlrSlider (*this)
{
    setColour (TooltipWindow::textColourId, Colours::red);
    addAndMakeVisible (&ctrlrSlider);

    ctrlrSlider.setRange (1, 127, 1);
    ctrlrSlider.setSliderStyle (Slider::RotaryVerticalDrag);
    ctrlrSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 64, 12);
    ctrlrSlider.addListener (this);
    ctrlrSlider.setLookAndFeel (&lf);
    componentTree.addListener (this);

    setProperty (Ids::uiSliderStyle, "RotaryVerticalDrag");
    setProperty (Ids::uiSliderMin, 0);
    setProperty (Ids::uiSliderMax, 127);
    setProperty (Ids::uiSliderInterval, 1);
    setProperty (Ids::uiSliderDoubleClickEnabled, true);
    setProperty (Ids::uiSliderDoubleClickValue, 0);
    setProperty (Ids::uiSliderValuePosition, (int)Slider::TextBoxBelow);
    setProperty (Ids::uiSliderValueHeight, 12);
    setProperty (Ids::uiSliderValueWidth, 64);
    setProperty (Ids::uiSliderTrackCornerSize, 5);
    setProperty (Ids::uiSliderThumbCornerSize, 3);
    setProperty (Ids::uiSliderThumbWidth, 0);
    setProperty (Ids::uiSliderThumbHeight, 0);
    setProperty (Ids::uiSliderThumbFlatOnLeft, false);
    setProperty (Ids::uiSliderThumbFlatOnRight, false);
    setProperty (Ids::uiSliderThumbFlatOnTop, false);
    setProperty (Ids::uiSliderThumbFlatOnBottom, false);
    setProperty (Ids::uiSliderValueTextColour, "0xff000000");
    setProperty (Ids::uiSliderValueBgColour, "0xffffffff");
    setProperty (Ids::uiSliderRotaryOutlineColour, "0xff0000ff");
    setProperty (Ids::uiSliderRotaryFillColour, "0xff0000ff");
    setProperty (Ids::uiSliderThumbColour, "0xffff0000");
    setProperty (Ids::uiSliderValueHighlightColour, "0xff0000ff");
    setProperty (Ids::uiSliderValueOutlineColour, "0xffffffff");
    setProperty (Ids::uiSliderTrackColour, "0xff0f0f0f");
    setProperty (Ids::uiSliderIncDecButtonColour, "0xff0000ff");
    setProperty (Ids::uiSliderIncDecTextColour, "0xffffffff");
    setProperty (Ids::uiSliderValueFont, FONT2STR (Font(12)));
    setProperty (Ids::uiSliderValueTextJustification, "centred");
    setProperty (Ids::uiSliderVelocitySensitivity, 1.0);
    setProperty (Ids::uiSliderVelocityThreshold, 1);
    setProperty (Ids::uiSliderVelocityOffset, 0.0);
    setProperty (Ids::uiSliderVelocityMode, false);
    setProperty (Ids::uiSliderVelocityModeKeyTrigger, true);
    setProperty (Ids::uiSliderSpringMode, false);
    setProperty (Ids::uiSliderSpringValue, 0);
    setProperty (Ids::uiSliderMouseWheelInterval, 1);
    setProperty (Ids::uiSliderPopupBubble, false);

    setSize (64, 64);
}

CtrlrSlider::~CtrlrSlider()
{
    componentTree.removeListener (this);
}

void CtrlrSlider::resized()
{
    if (restoreStateInProgress)
        return;
    ctrlrSlider.setBounds (getUsableRect());
}

void CtrlrSlider::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == &ctrlrSlider)
    {
        if ((bool)owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelEditMode) == true)
            return;

        setComponentValue (ctrlrSlider.getValue(), true);
    }
}

void CtrlrSlider::mouseUp (const MouseEvent& e)
{
    if ((bool)getProperty(Ids::uiSliderSpringMode) == true)
    {
        ctrlrSlider.setValue ((double)getProperty(Ids::uiSliderSpringValue), sendNotificationSync);
    }
}

double CtrlrSlider::getComponentValue()
{
    return (ctrlrSlider.getValue());
}

int CtrlrSlider::getComponentMidiValue()
{
    return ((int)ctrlrSlider.getValue());
}

double CtrlrSlider::getComponentMaxValue()
{
    return (ctrlrSlider.getMaximum());
}

void CtrlrSlider::setComponentValue (const double newValue, const bool sendChangeMessage)
{
    ctrlrSlider.setValue (newValue, dontSendNotification);
    if (sendChangeMessage)
    {
        owner.getProcessor().setValueGeneric (CtrlrModulatorValue(newValue,CtrlrModulatorValue::changedByGUI));
    }
}

const Array<Font> CtrlrSlider::getFontList()
{
    Array <Font> ret;
    Font f = STR2FONT(getProperty(Ids::uiSliderValueFont));
    if (f.getTypefaceName() != Font::getDefaultSerifFontName()
        && f.getTypefaceName() != Font::getDefaultSansSerifFontName()
        && f.getTypefaceName() != Font::getDefaultMonospacedFontName()
        && f.getTypefaceName() != "<Sans-Serif>")
    {
        ret.add (f);
    }
    return (ret);
}

void CtrlrSlider::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    if (property == Ids::uiSliderStyle)
    {
        ctrlrSlider.setSliderStyle ((Slider::SliderStyle)CtrlrComponentTypeManager::sliderStringToStyle (getProperty (Ids::uiSliderStyle)));
    }
    else if (property == Ids::uiSliderRotaryFillColour)
    {
        ctrlrSlider.setColour (Slider::rotarySliderFillColourId, VAR2COLOUR(getProperty (Ids::uiSliderRotaryFillColour)) );
    }
    else if (property == Ids::uiSliderRotaryOutlineColour)
    {
        ctrlrSlider.setColour (Slider::rotarySliderOutlineColourId, VAR2COLOUR(getProperty (Ids::uiSliderRotaryOutlineColour)) );
    }
    else if (property == Ids::uiSliderValueTextColour)
    {
        ctrlrSlider.setColour (Slider::textBoxTextColourId, VAR2COLOUR(getProperty (Ids::uiSliderValueTextColour)) );
    }
    else if (property == Ids::uiSliderValueBgColour)
    {
        ctrlrSlider.setColour (Slider::textBoxBackgroundColourId, VAR2COLOUR(getProperty (Ids::uiSliderValueBgColour)) );
    }
    else if (property == Ids::uiSliderThumbColour)
    {
        ctrlrSlider.setColour (Slider::thumbColourId, VAR2COLOUR(getProperty (Ids::uiSliderThumbColour)) );
    }
    else if (property == Ids::uiSliderValueHighlightColour)
    {
        ctrlrSlider.setColour (Slider::textBoxHighlightColourId, VAR2COLOUR(getProperty (Ids::uiSliderValueOutlineColour)) );
    }
    else if (property == Ids::uiSliderValueOutlineColour)
    {
        ctrlrSlider.setColour (Slider::textBoxOutlineColourId, VAR2COLOUR(getProperty (Ids::uiSliderValueOutlineColour)) );
    }
    else if (property == Ids::uiSliderTrackColour)
    {
        ctrlrSlider.setColour (Slider::trackColourId, VAR2COLOUR(getProperty (Ids::uiSliderTrackColour)) );
    }
    else if (property == Ids::uiSliderInterval || property == Ids::uiSliderMax || property == Ids::uiSliderMin)
    {
        double max       = getProperty (Ids::uiSliderMax);
        const double min = getProperty (Ids::uiSliderMin);
        double interval  = getProperty (Ids::uiSliderInterval);

        if (interval <= 0)
            interval = std::abs(max - min) + 1.0;

        // Strict JUCE Guard: Ensure the max bound expands by a minimum of 1 full step 
        if (max <= min) {
            max = min + interval;
        }
        
        // Final sanity catch if the interval width is wider than the range window
        if ((max - min) < interval) {
            max = min + interval;
        }

        ctrlrSlider.setRange ( min, max, interval  );
        owner.setProperty (Ids::modulatorMax, ctrlrSlider.getMaximum());
        owner.setProperty (Ids::modulatorMin, ctrlrSlider.getMinimum());
       }
    else if (property == Ids::uiSliderValuePosition || property == Ids::uiSliderValueHeight || property == Ids::uiSliderValueWidth)
    {
        ctrlrSlider.setTextBoxStyle (
            (Slider::TextEntryBoxPosition)(int)getProperty (Ids::uiSliderValuePosition),
            false,
            getProperty (Ids::uiSliderValueWidth, 64),
            getProperty (Ids::uiSliderValueHeight, 12));
    }
    else if (property == Ids::uiSliderIncDecButtonColour
            || property == Ids::uiSliderIncDecTextColour
            || property == Ids::uiSliderValueFont
            || property == Ids::uiSliderValueTextJustification)
    {
        ctrlrSlider.setLookAndFeel (nullptr);
        ctrlrSlider.setLookAndFeel (&lf);
    }
    else if (property == Ids::uiSliderVelocityMode
        || property == Ids::uiSliderVelocityModeKeyTrigger
        || property == Ids::uiSliderVelocitySensitivity
        || property == Ids::uiSliderVelocityThreshold
        || property == Ids::uiSliderVelocityOffset
        )
    {
        ctrlrSlider.setVelocityBasedMode((bool)getProperty(Ids::uiSliderVelocityMode));
        ctrlrSlider.setVelocityModeParameters ((double)getProperty(Ids::uiSliderVelocitySensitivity),
                                                (int)getProperty(Ids::uiSliderVelocityThreshold),
                                                (double)getProperty(Ids::uiSliderVelocityOffset),
                                                (bool)getProperty(Ids::uiSliderVelocityModeKeyTrigger));
    }
    else if (property == Ids::uiSliderSpringValue)
    {
        ctrlrSlider.setValue (getProperty(property), dontSendNotification);
    }
    else if (property == Ids::uiSliderDoubleClickValue
            || property == Ids::uiSliderDoubleClickEnabled)
    {
        ctrlrSlider.setDoubleClickReturnValue ((bool)getProperty(Ids::uiSliderDoubleClickEnabled), getProperty(Ids::uiSliderDoubleClickValue));
    }
    else if (property == Ids::uiSliderSpringMode)
    {
        if ((bool)getProperty(property) == true)
        {
            ctrlrSlider.setValue (getProperty(Ids::uiSliderSpringValue), dontSendNotification);
        }
    }
    else if (property == Ids::uiSliderPopupBubble)
    {
        ctrlrSlider.setPopupDisplayEnabled ((bool)getProperty(property), (bool)getProperty(property), owner.getOwnerPanel().getEditor());
    }
    else
    {
        CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
    }

    if (restoreStateInProgress == false)
    {
        resized();
    }
}

const String CtrlrSlider::getComponentText()
{
    return (String(getComponentValue()));
}

void CtrlrSlider::customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel)
{
    if (customLookAndFeel == nullptr)
        ctrlrSlider.setLookAndFeel (&lf);
    else
        ctrlrSlider.setLookAndFeel (customLookAndFeel);
}

#else
// ============================================================================
// UPSTREAM CTRLRX PATHWAY (WITH EXPERIMENTAL LNF ENGINE)
// ============================================================================
#include "CtrlrPanel/CtrlrPanelProperties.h"
#include "CtrlrPanel/CtrlrPanelComponentProperties.h"
// A single shared null shield for all slider instances
// Inherit from V2 to avoid the V4 vector composite caching subsystem
struct SliderDestructionShield : public juce::LookAndFeel_V2 {};
static SliderDestructionShield globalDestructionShield;

CtrlrSlider::CtrlrSlider(CtrlrModulator & owner)
        : CtrlrComponent(owner),
        ctrlrSlider(*this)
        {
        // Use the static global shield immediately
        ctrlrSlider.setLookAndFeel(&globalDestructionShield);

       // 2. Now safely determine and allocate your actual dynamic style
        juce::String initialLnf = getProperty(Ids::uiSliderLookAndFeel).toString();

        if (initialLnf == "V2")
            sliderCustomLnf = std::make_unique<CtrlrSliderLookAndFeel_V2>(*this, componentTree);
        else if (initialLnf == "V3")
            sliderCustomLnf = std::make_unique<CtrlrSliderLookAndFeel_V3>(*this, componentTree);
        else
            sliderCustomLnf = std::make_unique<CtrlrSliderLookAndFeel_V4>(*this, componentTree);

        // Apply it to the slider component
        if (sliderCustomLnf != nullptr)
        {
            ctrlrSlider.setLookAndFeel(sliderCustomLnf.get());
        }

    setColour (TooltipWindow::textColourId, findColour(Label::textColourId));
    setColour (TooltipWindow::backgroundColourId, findColour(TooltipWindow::backgroundColourId));
    setColour (TooltipWindow::outlineColourId, findColour(TooltipWindow::outlineColourId));
    
    addAndMakeVisible (&ctrlrSlider);
    
    ctrlrSlider.setRange (0, 127, 1);
    ctrlrSlider.setSliderStyle (Slider::RotaryVerticalDrag);
    ctrlrSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 64, 12);
    
    ctrlrSlider.addListener (this);
    componentTree.addListener (this);
        
    setProperty (Ids::uiSliderMin, 0);
    setProperty (Ids::uiSliderMax, 127);
    setProperty (Ids::uiSliderInterval, 1);
    setProperty (Ids::uiSliderDecimalPlaces, 0);
    setProperty (Ids::uiSliderValueSuffix, "");
    setProperty (Ids::uiSliderSetNotificationOnlyOnRelease, false);
    setProperty (Ids::uiSliderDoubleClickEnabled, true);
    setProperty (Ids::uiSliderDoubleClickValue, 0);
    
    setProperty (Ids::uiSliderVelocitySensitivity, 1.0);
    setProperty (Ids::uiSliderVelocityThreshold, 1);
    setProperty (Ids::uiSliderVelocityOffset, 0.0);
    setProperty (Ids::uiSliderVelocityMode, false);
    setProperty (Ids::uiSliderVelocityModeKeyTrigger, true);
    
    setProperty (Ids::uiSliderSpringMode, false);
    setProperty (Ids::uiSliderSpringValue, 0);
    
    setProperty (Ids::uiSliderMouseWheelInterval, 1);
    
    setProperty (Ids::uiSliderLookAndFeel, "Default");
    setProperty (Ids::uiSliderLookAndFeelIsCustom, false);
    
    setProperty (Ids::uiSliderPopupBubble, false);
    setProperty (Ids::uiSliderStyle, "RotaryVerticalDrag");
    
    bool LegacyMode = owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLegacyMode);  
    String panelLnF = owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelLookAndFeel);
    
    if (LegacyMode || panelLnF == "V3") 
    {
        setLookAndFeel(new LookAndFeel_V3());
        setProperty(Ids::uiSliderLookAndFeel, "V3");
    }
    else if (panelLnF == "V2") 
    {
        setLookAndFeel(new LookAndFeel_V2());
        setProperty(Ids::uiSliderLookAndFeel, "V2");
    }
    else if (panelLnF == "V1") 
    {
        setLookAndFeel(new LookAndFeel_V1());
        setProperty(Ids::uiSliderLookAndFeel, "V1");
    }
    
    if ( panelLnF == "V3" || panelLnF == "V2" || panelLnF == "V1" )
    {
        setSize (64, 64);
        setProperty (Ids::uiSliderRotaryOutlineColour, "0xff0000ff");  
        setProperty (Ids::uiSliderRotaryFillColour, "0xff0000ff"); 
        setProperty (Ids::uiSliderThumbColour, "0xffff0000"); 
        setProperty (Ids::uiSliderTrackColour, "0xff0f0f0f"); 
        setProperty (Ids::uiSliderLookAndFeelIsCustom, false);
    }
    else
    {
        setSize (72, 96); 
        setProperty (Ids::uiSliderRotaryOutlineColour, (String)findColour(Slider::rotarySliderOutlineColourId).toString());
        setProperty (Ids::uiSliderRotaryFillColour, (String)findColour(Slider::rotarySliderFillColourId).toString());
        setProperty (Ids::uiSliderThumbColour, (String)findColour(Slider::thumbColourId).toString());
        setProperty (Ids::uiSliderTrackColour, (String)findColour(Slider::rotarySliderFillColourId).toString());
        setProperty (Ids::uiSliderLookAndFeelIsCustom, false);
    }
    
    setProperty (Ids::uiSliderIncDecButtonColour, (String)findColour(Slider::backgroundColourId).toString());
    setProperty (Ids::uiSliderIncDecTextColour, (String)findColour(Label::textColourId).toString());
    
    setProperty (Ids::uiSliderTrackCornerSize, 5);
    setProperty (Ids::uiSliderThumbCornerSize, 3);
    setProperty (Ids::uiSliderThumbWidth, 0);
    setProperty (Ids::uiSliderThumbHeight, 0);
    setProperty (Ids::uiSliderThumbFlatOnLeft, false);
    setProperty (Ids::uiSliderThumbFlatOnRight, false);
    setProperty (Ids::uiSliderThumbFlatOnTop, false);
    setProperty (Ids::uiSliderThumbFlatOnBottom, false);
    
    setProperty (Ids::uiSliderValuePosition, (int)Slider::TextBoxBelow);
    setProperty (Ids::uiSliderValueWidth, 64);
    setProperty (Ids::uiSliderValueHeight, 10);
    setProperty (Ids::uiSliderValueTextJustification, "centred");
    setProperty (Ids::uiSliderValueFont, FONT2STR (Font(12)));
    setProperty (Ids::uiSliderValueTextColour, (String)findColour(Slider::textBoxTextColourId).toString());
    setProperty (Ids::uiSliderValueHighlightColour, (String)findColour(Slider::textBoxHighlightColourId).toString());
    setProperty (Ids::uiSliderValueBgColour, "0x00ffffff"); 
    setProperty (Ids::uiSliderValueOutlineColour, "0x00ffffff"); 
    
    setProperty (Ids::uiSliderLookAndFeelIsCustom, false);
}

CtrlrSlider::~CtrlrSlider()
{
    // 1. Create a local, sterile LookAndFeel instance right on the stack.
    // LookAndFeel_V2 uses basic rendering with zero shared vector caches.
    juce::LookAndFeel_V2 temporaryShield;
    
    // 2. Temporarily point the internal slider to this local stack instance.
    // This safely breaks the link to your custom heap-allocated LookAndFeel.
    ctrlrSlider.setLookAndFeel (&temporaryShield);
    
    // 3. Force the slider to flush its internal geometry layout right now
    ctrlrSlider.lookAndFeelChanged();
    
    // 4. Safely delete your custom heap instance while the component is decoupled
    sliderCustomLnf.reset();
    
    // 5. Explicitly clear the pointer before the temporaryShield goes out of scope.
    ctrlrSlider.setLookAndFeel (nullptr);
}

void CtrlrSlider::resized()
{
    if (restoreStateInProgress)
        return;
    ctrlrSlider.setBounds (getUsableRect());
}

void CtrlrSlider::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == &ctrlrSlider)
    {
        if ((bool)owner.getOwnerPanel().getEditor()->getProperty(Ids::uiPanelEditMode) == true)
            return;

        setComponentValue (ctrlrSlider.getValue(), true);
    }
}

void CtrlrSlider::mouseUp (const MouseEvent& e)
{
    if (mouseUpCbk && !mouseUpCbk.wasObjectDeleted())
    {
        if (mouseUpCbk->isValid())
        {
            owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().call (mouseUpCbk, this, e);
        }
    }
    if ((bool)getProperty(Ids::uiSliderSpringMode) == true)
    {
        ctrlrSlider.setValue ((double)getProperty(Ids::uiSliderSpringValue), sendNotificationSync);
    }
}

double CtrlrSlider::getComponentValue()
{
    return (ctrlrSlider.getValue());
}

int CtrlrSlider::getComponentMidiValue()
{
    return ((int)ctrlrSlider.getValue());
}

double CtrlrSlider::getComponentMaxValue()
{
    return (ctrlrSlider.getMaximum());
}

void CtrlrSlider::setComponentValue (const double newValue, const bool sendChangeMessage)
{
    ctrlrSlider.setValue (newValue, dontSendNotification);
    if (sendChangeMessage)
    {
        owner.getProcessor().setValueGeneric (CtrlrModulatorValue(newValue,CtrlrModulatorValue::changedByGUI));
    }
}

const Array<Font> CtrlrSlider::getFontList()
{
    Array <Font> ret;
    Font f = STR2FONT(getProperty(Ids::uiSliderValueFont));
    if (f.getTypefaceName() != Font::getDefaultSerifFontName()
        && f.getTypefaceName() != Font::getDefaultSansSerifFontName()
        && f.getTypefaceName() != Font::getDefaultMonospacedFontName()
        && f.getTypefaceName() != "<Sans-Serif>")
    {
        ret.add (f);
    }
    return (ret);
}

void CtrlrSlider::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    if (property == Ids::uiSliderStyle)
    {
        ctrlrSlider.setSliderStyle ((Slider::SliderStyle)CtrlrComponentTypeManager::sliderStringToStyle (getProperty (Ids::uiSliderStyle)));
    }
    else if (property == Ids::uiSliderLookAndFeel)
    {
        String LookAndFeelType = getProperty(property);
        setLookAndFeel(CtrlrSlider::getLookAndFeelFromComponentProperty(LookAndFeelType)); 
        
        if (LookAndFeelType == "Default")
        {
            setProperty(Ids::uiSliderLookAndFeelIsCustom, false); 
        }
        
        if (!getProperty(Ids::uiSliderLookAndFeelIsCustom))
        {
            CtrlrSlider::resetLookAndFeelOverrides(); 
        }
    }
    else if (property == Ids::uiSliderRotaryFillColour)
    {
        ctrlrSlider.setColour (Slider::rotarySliderFillColourId, VAR2COLOUR(getProperty (Ids::uiSliderRotaryFillColour)) );
        setProperty(Ids::uiSliderLookAndFeelIsCustom, true); 
    }
    else if (property == Ids::uiSliderRotaryOutlineColour)
    {
        ctrlrSlider.setColour (Slider::rotarySliderOutlineColourId, VAR2COLOUR(getProperty (Ids::uiSliderRotaryOutlineColour)) );
        setProperty(Ids::uiSliderLookAndFeelIsCustom, true); 
    }
    else if (property == Ids::uiSliderTrackColour)
    {
        ctrlrSlider.setColour (Slider::trackColourId, VAR2COLOUR(getProperty (Ids::uiSliderTrackColour)) );
        setProperty(Ids::uiSliderLookAndFeelIsCustom, true); 
    }
    else if (property == Ids::uiSliderThumbColour)
    {
        ctrlrSlider.setColour (Slider::thumbColourId, VAR2COLOUR(getProperty (Ids::uiSliderThumbColour)) );
        setProperty(Ids::uiSliderLookAndFeelIsCustom, true); 
    }
    else if (property == Ids::uiSliderValueTextColour)
    {
        ctrlrSlider.setColour (Slider::textBoxTextColourId, VAR2COLOUR(getProperty (Ids::uiSliderValueTextColour)) );
        setProperty(Ids::uiSliderLookAndFeelIsCustom, true); 
    }
    else if (property == Ids::uiSliderValueHighlightColour)
    {
        ctrlrSlider.setColour (Slider::textBoxHighlightColourId, VAR2COLOUR(getProperty (Ids::uiSliderValueHighlightColour)) );
        setProperty(Ids::uiSliderLookAndFeelIsCustom, true); 
    }
    else if (property == Ids::uiSliderValueBgColour)
    {
        ctrlrSlider.setColour (Slider::textBoxBackgroundColourId, VAR2COLOUR(getProperty (Ids::uiSliderValueBgColour)) );
        setProperty(Ids::uiSliderLookAndFeelIsCustom, true); 
    }
    else if (property == Ids::uiSliderValueOutlineColour)
    {
        ctrlrSlider.setColour (Slider::textBoxOutlineColourId, VAR2COLOUR(getProperty (Ids::uiSliderValueOutlineColour)) );
        setProperty(Ids::uiSliderLookAndFeelIsCustom, true); 
    }
    else if (property == Ids::uiSliderInterval || property == Ids::uiSliderMax || property == Ids::uiSliderMin)
    {
    double max       = getProperty (Ids::uiSliderMax);
    }
    else if (property == Ids::uiSliderDecimalPlaces) 
    {
        ctrlrSlider.setNumDecimalPlacesToDisplay((int)getProperty(Ids::uiSliderDecimalPlaces));
        ctrlrSlider.lookAndFeelChanged();
    }
    else if (property == Ids::uiSliderValueSuffix) 
    {
        ctrlrSlider.setTextValueSuffix(getProperty(Ids::uiSliderValueSuffix).toString());
        ctrlrSlider.lookAndFeelChanged();
    }
    else if (property == Ids::uiSliderValuePosition || property == Ids::uiSliderValueHeight || property == Ids::uiSliderValueWidth)
    {
        ctrlrSlider.setTextBoxStyle (
            (Slider::TextEntryBoxPosition)(int)getProperty(Ids::uiSliderValuePosition),
            false,
            getProperty (Ids::uiSliderValueWidth, 64),
            getProperty (Ids::uiSliderValueHeight, 12));
        
        ctrlrSlider.lookAndFeelChanged();
    }
    else if (property == Ids::uiSliderSetNotificationOnlyOnRelease)
    {
        ctrlrSlider.setChangeNotificationOnlyOnRelease((bool)getProperty(Ids::uiSliderSetNotificationOnlyOnRelease));
    }
    else if (property == Ids::uiSliderIncDecButtonColour
        || property == Ids::uiSliderIncDecTextColour
        || property == Ids::uiSliderValueFont
        || property == Ids::uiSliderValueTextJustification)
    {
        // 1. Unlink the slider temporarily so it cleans its internal caches
        ctrlrSlider.setLookAndFeel(nullptr);

        // 2. Safely apply the properties to whatever active LookAndFeel is inside your unique_ptr
        if (sliderCustomLnf != nullptr)
        {
            // Re-apply the pointer to the slider component
            ctrlrSlider.setLookAndFeel(sliderCustomLnf.get());
        }
        else
        {
            // Fallback: If for some reason the unique_ptr wasn't allocated yet, 
            // you can initialize it on the fly based on the property string
            juce::String currentLnfType = getProperty(Ids::uiSliderLookAndFeel).toString();

            if (currentLnfType == "V3" || currentLnfType == "V2" || currentLnfType == "V1")
                sliderCustomLnf = std::make_unique<juce::LookAndFeel_V3>(); // Or V2/V1 depending on preference
            else
                sliderCustomLnf = std::make_unique<juce::LookAndFeel_V4>();

            ctrlrSlider.setLookAndFeel(sliderCustomLnf.get());
        }
    
        setProperty(Ids::uiSliderLookAndFeelIsCustom, true); 
        ctrlrSlider.lookAndFeelChanged(); 
    }
    else if (property == Ids::uiSliderVelocityMode
        || property == Ids::uiSliderVelocityModeKeyTrigger
        || property == Ids::uiSliderVelocitySensitivity
        || property == Ids::uiSliderVelocityThreshold
        || property == Ids::uiSliderVelocityOffset
        )
    {
        ctrlrSlider.setVelocityBasedMode((bool)getProperty(Ids::uiSliderVelocityMode));
        ctrlrSlider.setVelocityModeParameters ((double)getProperty(Ids::uiSliderVelocitySensitivity),
                                                (int)getProperty(Ids::uiSliderVelocityThreshold),
                                                (double)getProperty(Ids::uiSliderVelocityOffset),
                                                (bool)getProperty(Ids::uiSliderVelocityModeKeyTrigger));
    }
    else if (property == Ids::uiSliderSpringValue)
    {
        ctrlrSlider.setValue (getProperty(property), dontSendNotification);
    }
    else if (property == Ids::uiSliderDoubleClickValue
            || property == Ids::uiSliderDoubleClickEnabled)
    {
        ctrlrSlider.setDoubleClickReturnValue ((bool)getProperty(Ids::uiSliderDoubleClickEnabled), getProperty(Ids::uiSliderDoubleClickValue));
    }
    else if (property == Ids::uiSliderSpringMode)
    {
        if ((bool)getProperty(property) == true)
        {
            ctrlrSlider.setValue (getProperty(Ids::uiSliderSpringValue), dontSendNotification);
        }
    }
    else if (property == Ids::uiSliderPopupBubble)
    {
        ctrlrSlider.setPopupDisplayEnabled ((bool)getProperty(property), (bool)getProperty(property), owner.getOwnerPanel().getEditor());
    }
    else
    {
        CtrlrComponent::valueTreePropertyChanged(treeWhosePropertyHasChanged, property);
    }
    if (restoreStateInProgress == false)
    {
        resized();
    }
}

const String CtrlrSlider::getComponentText()
{
    return (String(getComponentValue()));
}

void CtrlrSlider::customLookAndFeelChanged(LookAndFeelBase* customLookAndFeel)
{
    // 1. Always safely sever the existing relationship first to clear caches
    ctrlrSlider.setLookAndFeel(nullptr);

    if (customLookAndFeel == nullptr)
    {
        juce::String currentLnfType = getProperty(Ids::uiSliderLookAndFeel).toString();

        // 2. Check if our unique_ptr matches what the component property currently demands.
        // If the pointer is null, or if the type shifted, re-allocate the specific subclass.
        if (currentLnfType == "V2")
        {
            if (sliderCustomLnf == nullptr || dynamic_cast<CtrlrSliderLookAndFeel_V2*>(sliderCustomLnf.get()) == nullptr)
                sliderCustomLnf = std::make_unique<CtrlrSliderLookAndFeel_V2>(*this, componentTree);
        }
        else if (currentLnfType == "V3" || currentLnfType == "V1")
        {
            if (sliderCustomLnf == nullptr || dynamic_cast<CtrlrSliderLookAndFeel_V3*>(sliderCustomLnf.get()) == nullptr)
                sliderCustomLnf = std::make_unique<CtrlrSliderLookAndFeel_V3>(*this, componentTree);
        }
        else // Fallback or explicit "V4"
        {
            if (sliderCustomLnf == nullptr || dynamic_cast<CtrlrSliderLookAndFeel_V4*>(sliderCustomLnf.get()) == nullptr)
                sliderCustomLnf = std::make_unique<CtrlrSliderLookAndFeel_V4>(*this, componentTree);
        }

        // 3. Re-seat the slider against our safely managed heap instance
        if (sliderCustomLnf != nullptr)
        {
            ctrlrSlider.setLookAndFeel(sliderCustomLnf.get());
        }
    }
    else
    {
        // If an external look-and-feel pointer was passed explicitly, honor it.
        // Note: We cast it down to a regular juce::LookAndFeel since setLookAndFeel expects that.
        if (auto* lnf = dynamic_cast<juce::LookAndFeel*> (customLookAndFeel))
        {
            ctrlrSlider.setLookAndFeel(lnf);
        }
    }
}

const String CtrlrSlider::getCurrentLF()
{
    return getProperty(Ids::uiSliderLookAndFeel);
}

LookAndFeel *CtrlrSlider::getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty) 
{
    if (lookAndFeelComponentProperty == "Default")
    {
        return nullptr;
    }
    return gui::createLookAndFeelFromDescription(lookAndFeelComponentProperty, false);
}

void CtrlrSlider::resetLookAndFeelOverrides()
{
    if (restoreStateInProgress == false) 
    {
        setProperty (Ids::componentLabelColour, (String)findColour(Label::textColourId).toString());
        
        setProperty (Ids::uiSliderRotaryOutlineColour, (String)findColour(Slider::rotarySliderOutlineColourId).toString());
        setProperty (Ids::uiSliderRotaryFillColour, (String)findColour(Slider::rotarySliderFillColourId).toString());
        setProperty (Ids::uiSliderThumbColour, (String)findColour(Slider::thumbColourId).toString());
        
        setProperty (Ids::uiSliderTrackColour, (String)findColour(Slider::rotarySliderFillColourId).toString());

        setProperty (Ids::uiSliderIncDecTextColour, (String)findColour(Slider::textBoxTextColourId).toString());
        setProperty (Ids::uiSliderIncDecButtonColour, (String)findColour(Slider::backgroundColourId).toString());
        
        setProperty (Ids::uiSliderValueTextColour, (String)findColour(Slider::textBoxTextColourId).toString());
        setProperty (Ids::uiSliderValueHighlightColour, (String)findColour(Slider::textBoxHighlightColourId).toString());
        setProperty (Ids::uiSliderValueBgColour, "0x00ffffff"); 
        setProperty (Ids::uiSliderValueOutlineColour, "0x00ffffff"); 
        
        setProperty (Ids::uiSliderLookAndFeelIsCustom, false); 
        
        updatePropertiesPanel(); 
    }
}

void CtrlrSlider::updatePropertiesPanel()
{
    CtrlrPanelProperties *props = owner.getCtrlrManagerOwner().getActivePanel()->getEditor(false)->getPropertiesPanel();
    if (props)
    {
        props->refreshAll(); 
    }
}
#endif // CTLRX_DISABLE_DYNAMIC_LNF
