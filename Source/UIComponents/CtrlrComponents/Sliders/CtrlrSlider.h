#ifndef __CTRLR_SLIDER__
#define __CTRL_SLIDER__

#include "../CtrlrComponent.h"
#include "CtrlrSliderInternal.h"

#if CTLRX_DISABLE_DYNAMIC_LNF
// ============================================================================
// LIGHTWEIGHT 5.3 FORK PATHWAY
// ============================================================================
class CtrlrSlider : public CtrlrComponent,
                    public SettableTooltipClient,
                    public Slider::Listener
{
public:
    CtrlrSlider (CtrlrModulator &owner);
    ~CtrlrSlider();
    
    void setComponentValue (const double newValue, const bool sendChangeMessage=false);
    double getComponentValue();
    int getComponentMidiValue();
    double getComponentMaxValue();
    const String getComponentText();
    const Array<Font> getFontList();
    
    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property);
    void valueTreeChildrenChanged (ValueTree &treeWhoseChildHasChanged){}
    void valueTreeParentChanged (ValueTree &treeWhoseParentHasChanged){}
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded){}
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int){}
    void valueTreeChildOrderChanged (ValueTree& parentTreeWhoseChildrenHaveMoved, int, int){}
    
    Slider &getOwnedSlider() { return (ctrlrSlider); }
    static void wrapForLua(lua_State *L);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void mouseUp (const MouseEvent& e);
    void customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel = nullptr);
    
    JUCE_LEAK_DETECTOR(CtrlrSlider)

private:
    CtrlrSliderLookAndFeel_V2 lf; // Ensure this matches your 5.3 name definition
    CtrlrSliderInternal ctrlrSlider;
};

#else
// ============================================================================
// UPSTREAM CTRLRX PATHWAY (WITH EXPERIMENTAL LNF ENGINE)
// ============================================================================
#include "CtrlrPanel/CtrlrPanelEditor.h"

class CtrlrSlider : public CtrlrComponent,
                    public SettableTooltipClient,
                    public Slider::Listener
{
public:
    CtrlrSlider (CtrlrModulator &owner);
    ~CtrlrSlider();
    Slider &getOwnedSlider() { return (ctrlrSlider); }
    void setComponentValue (const double newValue, const bool sendChangeMessage=false);
    double getComponentValue();
    int getComponentMidiValue();
    double getComponentMaxValue();
    const String getComponentText();
    const String getCurrentLF(); // Remember to wrap this in the .cpp file too!
    const Array<Font> getFontList();
    
    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property);
    void valueTreeChildrenChanged (ValueTree &treeWhoseChildHasChanged){}
    void valueTreeParentChanged (ValueTree &treeWhoseParentHasChanged){}
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded){}
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int){}
    void valueTreeChildOrderChanged (ValueTree& parentTreeWhoseChildrenHaveMoved, int, int){}
    
    static void wrapForLua(lua_State *L);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void mouseUp (const MouseEvent& e);
    void customLookAndFeelChanged(LookAndFeelBase *customLookAndFeel = nullptr);
    static LookAndFeel* getLookAndFeelFromComponentProperty(const String &lookAndFeelComponentProperty);
    void resetLookAndFeelOverrides();
    void updatePropertiesPanel();
    
    JUCE_LEAK_DETECTOR(CtrlrSlider)

private:
    // CtrlrSliderLookAndFeel_V2 lf;
    // CtrlrSliderLookAndFeel_V2 lfV2;
    // CtrlrSliderLookAndFeel_V3 lfV3;
    // CtrlrSliderLookAndFeel_V4 lfV4;
    CtrlrSliderInternal ctrlrSlider;
    // std::unique_ptr<LookAndFeel> legacyLookAndFeel;
};

#endif // CTLRX_DISABLE_DYNAMIC_LNF
#endif // __CTRLR_SLIDER__
