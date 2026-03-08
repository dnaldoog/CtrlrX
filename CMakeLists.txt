cmake_minimum_required(VERSION 3.25)

# Suppress CMake 3.31+ policy warnings
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.31.0") 
    cmake_policy(SET CMP0175 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(PamplejuceVersion)

# -----------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------
option(CTRLRX_ENABLE_CPM "Enable CPM-based optional dependencies" OFF)
option(CTRLRX_USE_SYSTEM_JUCE "Use system-installed JUCE instead of vendored JUCE submodule" OFF)
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
set(CMAKE_CXX_STANDARD 14) # the lua bindings prevent newer C++ standard usage
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PLUGIN_DISPLAY_NAME "CtrlrX")
set(PRODUCT_NAME "CtrlrX")
set(COMPANY_NAME "CtrlrX Project")
set(BUNDLE_ID "com.${BASE_ID_NAME}.${PROJECT_NAME}")
set(FORMATS Standalone AU VST3)

project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

# -----------------------------------------------------------------------
# Linux: find system packages early so JUCE can see them
# -----------------------------------------------------------------------
if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libudev REQUIRED libudev)
    pkg_check_modules(x11 REQUIRED x11)
    pkg_check_modules(zlib REQUIRED zlib)
    pkg_check_modules(gtk3 REQUIRED gtk+-x11-3.0)
    pkg_check_modules(webkit REQUIRED webkit2gtk-4.1)

    message(STATUS "libudev version: ${libudev_VERSION}")
    message(STATUS "libX11 version: ${x11_VERSION}")
    message(STATUS "libz version: ${zlib_VERSION}")

    # Global headers for JUCE modules to see GTK/WebKit
    include_directories(SYSTEM
        ${gtk3_INCLUDE_DIRS}
        ${webkit_INCLUDE_DIRS}
        ${x11_INCLUDE_DIRS}
    )

    set(JUCE_USE_WEBKIT_4_1 ON CACHE INTERNAL "")
endif()

# -----------------------------------------------------------------------
# JUCE + LuaJIT subdirectories
# -----------------------------------------------------------------------
if(CTRLRX_USE_SYSTEM_JUCE)
    find_package(JUCE CONFIG REQUIRED)
    message(STATUS "Using system JUCE package")
else()
    add_subdirectory(JUCE)
    message(STATUS "Using vendored JUCE submodule")
endif()

add_subdirectory(Source/Misc/luajit)

# -----------------------------------------------------------------------
# Optional SDK paths
# -----------------------------------------------------------------------
if("VST" IN_LIST FORMATS)
    juce_set_vst2_sdk_path("${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/vst2sdk")
endif()

if("AAX" IN_LIST FORMATS)
    juce_set_aax_sdk_path("${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/aax-sdk-2-8-1")
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
# Format-specific overrides
# -----------------------------------------------------------------------
if("VST3" IN_LIST FORMATS)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        JUCE_PLUGIN_NAME "CtrlrX                          "
        JUCE_COMPANY_NAME "CtrlrX Project                  "
    )
endif()

if("AU" IN_LIST FORMATS)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        JUCE_AU_MAIN_TYPE kAudioUnitType_MusicDevice
        JUCE_PLUGIN_NAME "CtrlrX                          "
        JUCE_COMPANY_NAME "CtrlrX Project                  "
    )
endif()

# -----------------------------------------------------------------------
# SharedCode interface library
# -----------------------------------------------------------------------
add_library(SharedCode INTERFACE)
include(SharedCodeDefaults)

# FIX: Disable optimization for the crashing LuaBind file in Release builds
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/src/object_rep.cpp"
    PROPERTIES COMPILE_FLAGS "-O0"
)

# -----------------------------------------------------------------------
# Source files
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

# Exclude Linux-only files on non-Linux platforms
if(NOT LINUX)
    list(FILTER SourceFiles EXCLUDE REGEX "CtrlrLinux\\.cpp$")
    list(FILTER SourceFiles EXCLUDE REGEX "CtrlrLinux\\.h$")
endif()

# Exclude pre-built/generated library sources — linked as libs, not compiled
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/luajit/.*")
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/lua/src/.*")
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/luabind/(examples|build[^/]*|test|tests)/.*")
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Lua/Deprecated/CtrlrLuaFile\\.cpp$")

# -----------------------------------------------------------------------
# SharedCode: sources, includes, compile options, definitions
# -----------------------------------------------------------------------
target_sources(SharedCode INTERFACE ${SourceFiles})

target_include_directories(SharedCode INTERFACE
    # LuaJIT MUST come before Source/Misc/lua/include so its headers win
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luajit/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/lua/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/libusb/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/boost"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/vst2sdk"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/MIDI"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/CtrlrManager"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/CtrlrModulator"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/CtrlrPanel"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Native"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Plugin"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/UIComponents"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua"
    "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode"
)

if(UNIX)
    target_compile_options(SharedCode INTERFACE
        -w -Wno-implicit-fallthrough -Wno-maybe-uninitialized
        -Wno-missing-field-initializers -Wno-free-nonheap-object)
endif()

target_compile_definitions(SharedCode INTERFACE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    BOOST_BIND_GLOBAL_PLACEHOLDERS=1
    CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    VERSION="${CURRENT_VERSION}"
    $<$<PLATFORM_ID:Linux>:_GLIBCXX_USE_DEPRECATED=1>
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

# -----------------------------------------------------------------------
# Assets + Xcode prettify
# -----------------------------------------------------------------------
include(Assets)
include(XcodePrettify)

# -----------------------------------------------------------------------
# SharedCode: link JUCE libraries (all platforms)
# -----------------------------------------------------------------------
target_link_libraries(SharedCode INTERFACE
    Assets
    juce_audio_basics
    juce_audio_devices
    juce_audio_formats
    juce_audio_plugin_client
    juce_audio_processors
    juce_audio_utils
    juce_core
    juce_cryptography
    juce_data_structures
    juce_events
    juce_graphics
    juce_gui_basics
    juce_gui_extra
    juce_product_unlocking
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# -----------------------------------------------------------------------
# Linux-only system libraries
# -----------------------------------------------------------------------
if(LINUX)
    target_link_libraries(SharedCode INTERFACE
        ${libudev_LIBRARIES}
        ${zlib_LIBRARIES}
        ${x11_LIBRARIES}
        ${gtk3_LIBRARIES}
        ${webkit_LIBRARIES}
        dl
    )

    if(CTRLRX_USE_SYSTEM_JUCE)
        pkg_check_modules(flac QUIET flac)
        pkg_check_modules(vorbisfile QUIET vorbisfile)
        pkg_check_modules(vorbisenc QUIET vorbisenc)
        pkg_check_modules(vorbis QUIET vorbis)
        pkg_check_modules(ogg QUIET ogg)

        if(flac_FOUND AND vorbisfile_FOUND AND vorbisenc_FOUND AND vorbis_FOUND AND ogg_FOUND)
            message(STATUS "Using system FLAC/Vorbis/Ogg codec libs with system JUCE")
            target_link_libraries(SharedCode INTERFACE
                ${flac_LIBRARIES}
                ${vorbisfile_LIBRARIES}
                ${vorbisenc_LIBRARIES}
                ${vorbis_LIBRARIES}
                ${ogg_LIBRARIES}
            )
        else()
            message(WARNING "System JUCE selected, but FLAC/Vorbis/Ogg libs were not all found. Disabling OGG/FLAC support.")
            target_compile_definitions(SharedCode INTERFACE
                JUCE_USE_FLAC=0
                JUCE_USE_OGGVORBIS=0
            )
        endif()
    endif()
endif()

# -----------------------------------------------------------------------
# Final plugin target linking
# On Windows: lua51.lib linked by full path to avoid MSVC/Ninja IMPORTED
# target propagation issues. Must be built as x64 — see LUAJIT_BUILD.md
# On Linux/Mac: link libluajit by name via the INTERFACE target
# -----------------------------------------------------------------------
if(WIN32)
    target_link_libraries("${PROJECT_NAME}" PRIVATE
        SharedCode
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luajit/src/lua51.lib"
    )
else()
    target_link_libraries("${PROJECT_NAME}" PRIVATE
        SharedCode
        libluajit
    )
endif()

# -----------------------------------------------------------------------
# IPP support (comment out to disable)
# -----------------------------------------------------------------------
include(PamplejuceIPP)

# Everything related to the tests target
#include(Tests) # enable once we have tests...

# A separate target for Benchmarks (keeps the Tests target fast)
#include(Benchmarks) # enable once we have benchmarks...

# Output some config for CI (like our PRODUCT_NAME)
include(GitHubENV)