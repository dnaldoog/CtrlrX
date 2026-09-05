#ifndef __STDAFX_H__
#define __STDAFX_H__

#ifdef __cplusplus

// Force standard C/C++ type definitions into global scope BEFORE JUCE/Lua
#include <stddef.h>
#include <stdint.h>
#include <cstddef>
#include <cstdint>
#include <array>
#include <vector>

// Include JUCE
#include "JuceHeader.h"

// Include Lua inside extern "C"
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#endif // __cplusplus
#endif // __STDAFX_H__