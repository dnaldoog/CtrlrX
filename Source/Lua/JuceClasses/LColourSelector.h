/*
  ==============================================================================

    LColourSelector.h
    Created: 21 Nov 2026 10:05:46am
    Author:  zan64

  ==============================================================================
*/

// #pragma once
// LColourSelector.h
#ifndef __L_COLOUR_SELECTOR__
#define __L_COLOUR_SELECTOR__

#include "JuceHeader.h"
extern "C" {
#include "lua.h"
}
#include "luabind/luabind.hpp"

void showColourPickerPopupCb(Component *anchorComp, Component *targetLabelComp, Component *repaintTargetComp,
							 const Colour &initialColour, bool liveUpdate, luabind::object cb);
void colourSelector(Component *anchorComp, Component *targetLabelComp, Component *repaintTargetComp,
								 const Colour &initialColour, bool liveUpdate, luabind::object cb);
class LColourSelector
{
    public:
        static void wrapForLua (lua_State *L);
};

#endif
