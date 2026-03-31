cmake_minimum_required(VERSION 3.25)
#To build with LuaJIT: cmake -B build -DCTRLRX_USE_LUAJIT=ON
#To build without (default): cmake -B build
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.31.0") 
  cmake_policy(SET CMP0175 NEW)
endif()

# This tells cmake we have goodies in the /cmake folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include (PamplejuceVersion)

# CPM is only needed for optional targets (tests/benchmarks) and should not
# be required for normal local/package builds that rely on system libs.
option(CTRLRX_ENABLE_CPM "Enable CPM-based optional dependencies" OFF)
option(CTRLRX_USE_LUAJIT "Build with LuaJIT instead of standard Lua" OFF)  # <-- NEW
if(CTRLRX_ENABLE_CPM)
    include (CPM)
endif()

# Configures universal binaries and decides which version of macOS to support
include(PamplejuceMacOS)

# Couple tweaks that IMO should be JUCE defaults
include(JUCEDefaults)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(BASE_ID_NAME "CtrlrX")
set(PROJECT_NAME "${BASE_ID_NAME}")
set(CMAKE_CXX_STANDARD 14) # the lua bindings prevent newer C++ standard usage.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PLUGIN_DISPLAY_NAME "CtrlrX")
set(PRODUCT_NAME "CtrlrX")
set(COMPANY_NAME "CtrlrX Project")
set(BUNDLE_ID "com.${BASE_ID_NAME}.${PROJECT_NAME}")
set(FORMATS Standalone AU VST3)
option(CTRLRX_COPY_PLUGIN_AFTER_BUILD "Copy plugin artefacts to user/system plugin folders after build" OFF)

project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

# JUCE is setup as a submodule in the /JUCE folder
add_subdirectory(JUCE)

# <-- NEW: Only add LuaJIT subdirectory if requested
if(CTRLRX_USE_LUAJIT)
    add_subdirectory(Source/Misc/luajit)
endif()

# ---------------------------------------------------
if ("VST" IN_LIST FORMATS)
    juce_set_vst2_sdk_path("${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/vst2sdk")
endif()

if ("AAX" IN_LIST FORMATS)
    juce_set_aax_sdk_path("${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/aax-sdk-2-8-1")
endif()
# ---------------------------------------------------

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

if ("VST3" IN_LIST FORMATS)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        JUCE_PLUGIN_NAME "CtrlrX                          "
        JUCE_COMPANY_NAME "CtrlrX Project                  "
    )
endif()

if ("AU" IN_LIST FORMATS)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        JUCE_AU_MAIN_TYPE kAudioUnitType_MusicDevice
        JUCE_PLUGIN_NAME "CtrlrX                          "
        JUCE_COMPANY_NAME "CtrlrX Project                  "
    )
endif()

add_library(SharedCode INTERFACE)
include(SharedCodeDefaults)

set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/src/object_rep.cpp"
    PROPERTIES COMPILE_FLAGS "-O0"
)

file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/libusb/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/libusb/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/*.c*" "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/*.h*"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/MIDI/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Source/MIDI/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Native/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Source/Native/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Plugin/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Source/Plugin/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/UIComponents/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Source/UIComponents/*.h"
)

if (NOT LINUX)
    list(FILTER SourceFiles EXCLUDE REGEX "CtrlrLinux\\.cpp$")
    list(FILTER SourceFiles EXCLUDE REGEX "CtrlrLinux\\.h$")
endif()

# <-- NEW: Exclude luajit sources from glob — always linked as lib, never compiled directly
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/luajit/.*")
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/lua/src/.*")
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/luabind/(examples|build[^/]*|test|tests)/.*")
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Lua/Deprecated/CtrlrLuaFile\\.cpp$")

target_sources(SharedCode INTERFACE ${SourceFiles})

target_include_directories(SharedCode INTERFACE
    # <-- NEW: LuaJIT MUST come before lua/include so its headers win when enabled
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luajit/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/rapidfuzz"
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
    -w -Wno-implicit-fallthrough -Wno-maybe-uninitialized -Wno-missing-field-initializers -Wno-free-nonheap-object)
endif()

include(Assets)
include(XcodePrettify)

target_compile_definitions(SharedCode
    INTERFACE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    BOOST_BIND_GLOBAL_PLACEHOLDERS=1
    CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    VERSION="${CURRENT_VERSION}"
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    # <-- NEW: Tell the code whether LuaJIT is active
    $<$<BOOL:${CTRLRX_USE_LUAJIT}>:CTRLRX_USE_LUAJIT=1>
)

target_link_libraries(SharedCode
    INTERFACE
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
    juce::juce_recommended_warning_flags)

if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libudev REQUIRED libudev)
    pkg_check_modules(x11 REQUIRED x11)
    pkg_check_modules(zlib REQUIRED zlib)

    message(STATUS "libudev version: ${libudev_VERSION}")
    message(STATUS "libX11 version: ${x11_VERSION}")
    message(STATUS "libz version: ${zlib_VERSION}")

    target_link_libraries(SharedCode
        INTERFACE
        ${libudev_LIBRARIES}
        ${zlib_LIBRARIES}
        ${x11_LIBRARIES}
    )
endif()

# <-- NEW: Link LuaJIT if enabled, otherwise link SharedCode only as before
if(CTRLRX_USE_LUAJIT)
    if(WIN32)
        target_link_libraries("${PROJECT_NAME}" PRIVATE
            SharedCode
            "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luajit/src/lua51.lib"
        )
    elseif(APPLE)
        target_link_libraries("${PROJECT_NAME}" PRIVATE
            SharedCode
            "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luajit/src/libluajit.a"
        )
    else()
        target_link_libraries("${PROJECT_NAME}" PRIVATE
            SharedCode
            libluajit
        )
    endif()
else()
    target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode)
endif()

include(PamplejuceIPP)

#include(Tests)
#include(Benchmarks)

include(GitHubENV)