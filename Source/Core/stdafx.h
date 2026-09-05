#ifndef __STDAFX_H__
#define __STDAFX_H__

#ifdef __cplusplus

// 1. Force standard C++ types to resolve BEFORE anything else
#include <cstddef>
#include <cstdint>
#include <array>
#include <vector>
#include <string>

// 2. Include JUCE Header for both pure C++ and Objective-C++ (.mm) files
#include "JuceHeader.h"

// 3. Include Lua cleanly wrapped in extern "C"
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#endif // __cplusplus
#endif // __STDAFX_H__