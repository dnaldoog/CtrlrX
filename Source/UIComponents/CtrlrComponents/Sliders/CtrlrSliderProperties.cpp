#include "CtrlrSliderProperties.h"
#include "CtrlrComponent.h"
#include "CtrlrModulator/CtrlrModulator.h"

namespace CtrlrSliderProperties
{
    bool handleCommonSliderPropertyChanged(juce::Slider& slider, 
                                            CtrlrComponent& ctrlrComp, 
                                            const juce::Identifier& property)
    {
        if (property == Ids::uiSliderInterval || property == Ids::uiSliderMax || property == Ids::uiSliderMin) 
        {
            double max = ctrlrComp.getProperty(Ids::uiSliderMax);
            double min = ctrlrComp.getProperty(Ids::uiSliderMin);
            double interval = ctrlrComp.getProperty(Ids::uiSliderInterval);
            
            if (interval == 0)
                interval = std::abs(max - min) + 1;
                
            if (max <= min)
                max = min + interval * 0.66;
                
            slider.setRange(min, max, interval);
            ctrlrComp.getOwner().setProperty(Ids::modulatorMin, slider.getMinimum());
            ctrlrComp.getOwner().setProperty(Ids::modulatorMax, slider.getMaximum());
            ctrlrComp.lookAndFeelChanged();
            return true;
        }
        else if (property == Ids::uiSliderDecimalPlaces) 
        {
            slider.setNumDecimalPlacesToDisplay((int)ctrlrComp.getProperty(Ids::uiSliderDecimalPlaces));
            slider.lookAndFeelChanged();
            return true;
        }
        else if (property == Ids::uiSliderValueSuffix) 
        {
            slider.setTextValueSuffix(ctrlrComp.getProperty(Ids::uiSliderValueSuffix).toString());
            slider.lookAndFeelChanged();
            return true;
        }
        else if (property == Ids::uiSliderValuePosition || property == Ids::uiSliderValueHeight || property == Ids::uiSliderValueWidth) 
        {
            slider.setTextBoxStyle(
                (juce::Slider::TextEntryBoxPosition)(int)ctrlrComp.getProperty(Ids::uiSliderValuePosition),
                false,
                ctrlrComp.getProperty(Ids::uiSliderValueWidth, 64),
                ctrlrComp.getProperty(Ids::uiSliderValueHeight, 12));
            slider.lookAndFeelChanged();
            return true;
        }
        else if (property == Ids::uiSliderVelocityMode || property == Ids::uiSliderVelocityModeKeyTrigger
                 || property == Ids::uiSliderVelocitySensitivity || property == Ids::uiSliderVelocityThreshold
                 || property == Ids::uiSliderVelocityOffset) 
        {
            slider.setVelocityBasedMode((bool)ctrlrComp.getProperty(Ids::uiSliderVelocityMode));
            slider.setVelocityModeParameters((double)ctrlrComp.getProperty(Ids::uiSliderVelocitySensitivity),
                                              (int)ctrlrComp.getProperty(Ids::uiSliderVelocityThreshold),
                                              (double)ctrlrComp.getProperty(Ids::uiSliderVelocityOffset),
                                              (bool)ctrlrComp.getProperty(Ids::uiSliderVelocityModeKeyTrigger));
            return true;
        }
        else if (property == Ids::uiSliderDoubleClickValue || property == Ids::uiSliderDoubleClickEnabled) 
        {
            slider.setDoubleClickReturnValue((bool)ctrlrComp.getProperty(Ids::uiSliderDoubleClickEnabled), 
                                              ctrlrComp.getProperty(Ids::uiSliderDoubleClickValue));
            return true;
        }
        
        return false; // Not a common property
    }
}