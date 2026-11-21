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

class LColourSelector
{
    public:
        static void wrapForLua (lua_State *L);
};

#endif
