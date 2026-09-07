#ifndef __STDAFX_H__
#define __STDAFX_H__

#ifdef __cplusplus
// Ensure standard C/C++ types exist before JUCE or system headers
#include <stddef.h>
#include <stdint.h>
#include <cstddef>
#include <cstdint>

#include "JuceHeader.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#endif // __cplusplus
#endif // __STDAFX_H__