cmake_minimum_required(VERSION 3.25)

# To build with LuaJIT: cmake -B build -DCTRLRX_USE_LUAJIT=ON
# To build without (default): cmake -B build

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.31.0") 
  cmake_policy(SET CMP0175 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(PamplejuceVersion)

option(CTRLRX_ENABLE_CPM "Enable CPM-based optional dependencies" OFF)
option(CTRLRX_USE_LUAJIT "Build with LuaJIT instead of standard Lua" OFF)
option(CTRLRX_COPY_PLUGIN_AFTER_BUILD "Copy plugin artefacts to user/system plugin folders after build" OFF)

if(CTRLRX_ENABLE_CPM)
    include(CPM)
endif()

include(PamplejuceMacOS)
include(JUCEDefaults)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# -----------------------------------------------------------------------
# Project identity
# -----------------------------------------------------------------------
set(BASE_ID_NAME "CtrlrX")
set(PROJECT_NAME "${BASE_ID_NAME}")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PLUGIN_DISPLAY_NAME "CtrlrX")
set(PRODUCT_NAME "CtrlrX")
set(COMPANY_NAME "CtrlrX Project")
set(BUNDLE_ID "com.${BASE_ID_NAME}.${PROJECT_NAME}")
set(FORMATS Standalone AU VST3)

project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

# -----------------------------------------------------------------------
# Linux: find system packages early
# -----------------------------------------------------------------------
if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libudev REQUIRED libudev)
    pkg_check_modules(x11 REQUIRED x11)
    pkg_check_modules(zlib REQUIRED zlib)
    pkg_check_modules(gtk3 REQUIRED gtk+-x11-3.0)
    pkg_check_modules(webkit REQUIRED webkit2gtk-4.1)

    include_directories(SYSTEM
        ${gtk3_INCLUDE_DIRS}
        ${webkit_INCLUDE_DIRS}
        ${x11_INCLUDE_DIRS}
    )
    set(JUCE_USE_WEBKIT_4_1 ON CACHE INTERNAL "")
endif()

# -----------------------------------------------------------------------
# Subdirectories (JUCE + LuaJIT)
# -----------------------------------------------------------------------
add_subdirectory(JUCE)

if(CTRLRX_USE_LUAJIT)
    # Only add local LuaJIT build if we aren't using a manual path (GitHub) or Linux system pkg
    if(NOT LUAJIT_LIBRARY AND NOT LINUX)
        add_subdirectory(Source/Misc/luajit)
    endif()
endif()

# -----------------------------------------------------------------------
# JUCE plugin target
# -----------------------------------------------------------------------
juce_add_plugin("${PROJECT_NAME}"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources/Icons/ctrlrx_logo.svg"
    PLUGIN_NAME "${PLUGIN_DISPLAY_NAME}"
    COMPANY_NAME "${COMPANY_NAME}"
    BUNDLE_ID "${BUNDLE_ID}"
    COPY_PLUGIN_AFTER_BUILD ${CTRLRX_COPY_PLUGIN_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE cTrX
    PLUGIN_CODE cTrl
    FORMATS "${FORMATS}"
    IS_SYNTH TRUE
    IS_MIDI_EFFECT FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PRODUCT_NAME "${PRODUCT_NAME}"
)

# -----------------------------------------------------------------------
# SharedCode interface library
# -----------------------------------------------------------------------
add_library(SharedCode INTERFACE)
include(SharedCodeDefaults)

# -----------------------------------------------------------------------
# Source files & Filtering
# -----------------------------------------------------------------------
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/libusb/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/libusb/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/*.h*"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/MIDI/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/MIDI/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Native/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Native/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Plugin/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Plugin/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/UIComponents/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/UIComponents/*.h"
)

# CRITICAL FILTERS
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/luajit/.*")
if(CTRLRX_USE_LUAJIT)
    list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/lua/src/.*")
    list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/lua/include/.*")
endif()
if(NOT LINUX)
    list(FILTER SourceFiles EXCLUDE REGEX "CtrlrLinux\\.cpp$")
    list(FILTER SourceFiles EXCLUDE REGEX "CtrlrLinux\\.h$")
endif()
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/luabind/(examples|build[^/]*|test|tests)/.*")

# -----------------------------------------------------------------------
# Target configuration
# -----------------------------------------------------------------------
target_sources(SharedCode INTERFACE ${SourceFiles})

target_include_directories(SharedCode INTERFACE
    "${LUAJIT_INCLUDE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luajit/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/lua/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/boost"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua"
    "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode"
)

target_compile_definitions(SharedCode INTERFACE
    JUCE_MODAL_LOOPS_PERMITTED=1
    $<$<BOOL:${CTRLRX_USE_LUAJIT}>:CTRLRX_USE_LUAJIT=1>
)

# -----------------------------------------------------------------------
# Final Linking logic
# -----------------------------------------------------------------------
if(CTRLRX_USE_LUAJIT)
    if(LUAJIT_LIBRARY)
        message(STATUS "Linking LuaJIT via manual path: ${LUAJIT_LIBRARY}")
        target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode "${LUAJIT_LIBRARY}")
    elseif(WIN32)
        target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luajit/src/lua51.lib")
    elseif(APPLE)
        target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luajit/src/libluajit.a")
    else()
        target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode ${luajit_LIBRARIES})
        target_include_directories(SharedCode INTERFACE ${luajit_INCLUDE_DIRS})
    endif()
else()
    target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode)
endif()

target_link_libraries(SharedCode INTERFACE
    juce_audio_basics juce_audio_devices juce_audio_formats
    juce_audio_plugin_client juce_audio_processors juce_audio_utils
    juce_core juce_data_structures juce_events juce_graphics
    juce_gui_basics juce_gui_extra
)

if(LINUX)
    target_link_libraries(SharedCode INTERFACE ${libudev_LIBRARIES} ${zlib_LIBRARIES} ${x11_LIBRARIES} ${gtk3_LIBRARIES} ${webkit_LIBRARIES} dl)
endif()

include(PamplejuceIPP)
include(GitHubENV)