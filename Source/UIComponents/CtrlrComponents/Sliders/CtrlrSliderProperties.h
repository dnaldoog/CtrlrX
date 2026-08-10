#pragma once

#include "JuceHeader.h"

class CtrlrComponent; // Forward declaration

namespace CtrlrSliderProperties
{
    /** Processes common slider property changes (ranges, text boxes, velocity, double clicks, etc.).
        Returns true if the property was handled, false if it belongs to component-specific logic. */
    bool handleCommonSliderPropertyChanged(juce::Slider& slider, 
                                            CtrlrComponent& ctrlrComp, 
                                            const juce::Identifier& property);
}