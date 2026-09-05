cmake_minimum_required(VERSION 3.25)

# Suppress CMake 3.31+ policy warnings
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.31.0") 
    cmake_policy(SET CMP0175 NEW)
endif()

# Allows clang-based tools to work with the source:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# This tells cmake we have goodies in the /cmake folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(PamplejuceVersion)

# -----------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------
option(CTRLRX_ENABLE_CPM "Enable CPM-based optional dependencies" OFF)
option(CTRLRX_USE_SYSTEM_JUCE "Use system-installed JUCE instead of vendored JUCE submodule" OFF)
option(CTRLRX_COPY_PLUGIN_AFTER_BUILD "Copy plugin artefacts to user/system plugin folders after build" OFF)
option(CTRLRX_USE_LUAJIT "Use LuaJIT instead of standard Lua" ON)
option(CTRLRX_FORCE_JIT_OFF "Force disable LuaJIT compiler on macOS for unsigned builds" OFF)

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
set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PLUGIN_DISPLAY_NAME "CtrlrX")
set(PRODUCT_NAME "CtrlrX")
set(COMPANY_NAME "CtrlrX Project")
set(BUNDLE_ID "com.${BASE_ID_NAME}.${PROJECT_NAME}")
set(FORMATS Standalone AU VST3)

project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

# -----------------------------------------------------------------------
# Linux System Packages
# -----------------------------------------------------------------------
if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libudev REQUIRED libudev)
    pkg_check_modules(x11 REQUIRED x11)
    pkg_check_modules(zlib REQUIRED zlib)
    pkg_check_modules(gtk3 REQUIRED gtk+-3.0) 
    pkg_check_modules(webkit REQUIRED webkit2gtk-4.1)

    include_directories(SYSTEM
        ${gtk3_INCLUDE_DIRS}
        ${webkit_INCLUDE_DIRS}
        ${x11_INCLUDE_DIRS}
    )
    set(JUCE_USE_WEBKIT_4_1 ON CACHE INTERNAL "")
endif()

# -----------------------------------------------------------------------
# Subdirectories (JUCE)
# -----------------------------------------------------------------------
# Prevent JUCE from attaching header install targets to CPack
set(JUCE_BUILD_HELPER_TARGETS OFF CACHE BOOL "" FORCE)
set(JUCE_INSTALL_DESTINATION "" CACHE STRING "" FORCE)

if(CTRLRX_USE_SYSTEM_JUCE)
    find_package(JUCE CONFIG REQUIRED)
else()
    add_subdirectory(JUCE)
endif()

# -----------------------------------------------------------------------
# Lua / LuaJIT Architecture Routing (Isolated cleanly)
# -----------------------------------------------------------------------
if(CTRLRX_USE_LUAJIT)
    set(LUAJIT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luajit")
    set(LUAJIT_INCLUDE_TARGET "${LUAJIT_DIR}/src")
    
    if(MSVC)
        set(LUA_STATIC_LIB_TARGET "${LUAJIT_DIR}/src/lua51.lib")
    else()
        set(LUA_STATIC_LIB_TARGET "${LUAJIT_DIR}/src/libluajit.a")
    endif()
else()
    set(LUA_INCLUDE_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/lua/include")
endif()

# -----------------------------------------------------------------------
# macOS Hardened Runtime & JIT Entitlements
# -----------------------------------------------------------------------
if(APPLE)
    set(MACOS_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/MacOS/Entitlements.plist")

    if(TARGET "${PROJECT_NAME}")
        set_target_properties("${PROJECT_NAME}" PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${MACOS_ENTITLEMENTS}"
        )
    endif()

    if(TARGET "${PROJECT_NAME}_Standalone")
        set_target_properties("${PROJECT_NAME}_Standalone" PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${MACOS_ENTITLEMENTS}"
        )
    endif()
endif()

# -----------------------------------------------------------------------
# SDK Paths
# -----------------------------------------------------------------------
if("VST" IN_LIST FORMATS)
    juce_set_vst2_sdk_path("${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/vst2sdk")
endif()

if("AAX" IN_LIST FORMATS)
    juce_set_aax_sdk_path("${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/aax-sdk-2-8-1")
endif()

# -----------------------------------------------------------------------
# JUCE Plugin Target Configuration
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

# --- Format Specific Overrides ---
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

# -----------------------------------------------------------------------
# SharedCode Library & Source Files
# -----------------------------------------------------------------------
add_library(SharedCode INTERFACE)
include(SharedCodeDefaults)

# macOS LuaJIT JIT Force-Off Override
if(CTRLRX_FORCE_JIT_OFF)
    target_compile_definitions(SharedCode INTERFACE CTRLRX_FORCE_JIT_OFF=1)
endif()

# Optimization fix for LuaBind
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/luabind/src/object_rep.cpp"
    PROPERTIES COMPILE_FLAGS "-O0"
)

file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS
    "Source/Misc/luabind/*.cpp" "Source/Misc/luabind/*.hpp"
    "Source/Misc/luabind/*.c" "Source/Misc/luabind/*.h"
    "Source/Misc/libusb/src/*.c" "Source/Misc/libusb/src/*.h"
    "Source/Misc/*.c*" "Source/Misc/*.h*"
    "Source/Core/*.cpp" "Source/Core/*.h"
    "Source/Lua/*.cpp" "Source/Lua/*.h"
    "Source/MIDI/*.cpp" "Source/MIDI/*.h"
    "Source/Native/*.cpp" "Source/Native/*.h"
    "Source/Plugin/*.cpp" "Source/Plugin/*.h"
    "Source/UIComponents/*.cpp" "Source/UIComponents/*.h"
)

if(NOT LINUX)
    list(FILTER SourceFiles EXCLUDE REGEX "CtrlrLinux\\.(cpp|h)$")
endif()

# Exclude generated/foreign sources
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/luajit/.*")
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/lua/src/.*")
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Misc/luabind/(examples|build[^/]*|test|tests)/.*")
list(FILTER SourceFiles EXCLUDE REGEX ".*/Source/Lua/Deprecated/CtrlrLuaFile\\.cpp$")

target_sources(SharedCode INTERFACE ${SourceFiles})

# Base Include Paths
target_include_directories(SharedCode INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/rapidfuzz"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/include"
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

# Conditional Engine Include Routing
if(CTRLRX_USE_LUAJIT)
    target_include_directories(SharedCode INTERFACE "${LUAJIT_INCLUDE_TARGET}")
else()
    target_include_directories(SharedCode INTERFACE "${LUA_INCLUDE_TARGET}")
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

target_compile_options(SharedCode INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-msse2 -fPIC>
)

if(UNIX)
    target_compile_options(SharedCode INTERFACE -w -Wno-implicit-fallthrough -Wno-maybe-uninitialized -Wno-missing-field-initializers -Wno-free-nonheap-object)
endif()

if(CTRLRX_USE_LUAJIT)
    target_compile_definitions(SharedCode INTERFACE CTRLRX_USE_LUAJIT=1)
endif()

# -----------------------------------------------------------------------
# ASSETS & BINARY DATA
# -----------------------------------------------------------------------
include(Assets)
include(XcodePrettify)

# -----------------------------------------------------------------------
# Linking
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

# Explicit static engine binary targeting
if(CTRLRX_USE_LUAJIT)
    target_link_libraries(SharedCode INTERFACE "${LUA_STATIC_LIB_TARGET}")
endif()

if(LINUX)
    target_link_libraries(SharedCode INTERFACE
        ${x11_LIBRARIES}
        ${gtk3_LIBRARIES}
        ${webkit_LIBRARIES}
        ${libudev_LIBRARIES}
        ${zlib_LIBRARIES}
    )
endif()

# Final Plugin Target Linking
target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode)

# -----------------------------------------------------------------------
# Universal FreeDesktop Linux Icon, Launcher & VST3 Installation
# -----------------------------------------------------------------------
if(LINUX)
    include(GNUInstallDirs)

    set(CTRLR_SVG_ICON "${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources/Icons/ctrlrx_logo.svg")
    set(ICON_NAME "ctrlr-x")

    # 1. Install scalable SVG icon
    install(FILES "${CTRLR_SVG_ICON}"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
            RENAME "${ICON_NAME}.svg")

    # 2. Rasterize and install PNG icons across standard desktop sizes with an off-white background
    set(ICON_SIZES 16 32 48 64 128 256 512)
    find_program(RSVG_CONVERT_EXECUTABLE rsvg-convert)

    if(RSVG_CONVERT_EXECUTABLE)
        foreach(SIZE ${ICON_SIZES})
            set(PNG_OUT "${CMAKE_BINARY_DIR}/icons/${SIZE}x${SIZE}/${ICON_NAME}.png")
            
            add_custom_command(
                OUTPUT "${PNG_OUT}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/icons/${SIZE}x${SIZE}"
                COMMAND ${RSVG_CONVERT_EXECUTABLE} -w ${SIZE} -h ${SIZE} -b "#f7f8f9" -f png -o "${PNG_OUT}" "${CTRLR_SVG_ICON}"
                DEPENDS "${CTRLR_SVG_ICON}"
                COMMENT "Generating ${SIZE}x${SIZE} PNG icon with off-white background..."
            )

            add_custom_target(ctrlrx_icon_${SIZE} ALL DEPENDS "${PNG_OUT}")

            install(FILES "${PNG_OUT}"
                    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/${SIZE}x${SIZE}/apps")
        endforeach()
    endif()

    # 3. Desktop Application Entry
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources/Linux/ctrlr-x.desktop.in")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources/Linux/ctrlr-x.desktop.in"
            "${CMAKE_BINARY_DIR}/ctrlr-x.desktop"
            @ONLY
        )

        install(FILES "${CMAKE_BINARY_DIR}/ctrlr-x.desktop"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
    endif()

    # 4. Install Standalone executable target to system bin path
    if(TARGET "${PROJECT_NAME}_Standalone")
        install(TARGETS "${PROJECT_NAME}_Standalone" DESTINATION "${CMAKE_INSTALL_BINDIR}")
    elseif(TARGET "${PROJECT_NAME}")
        install(TARGETS "${PROJECT_NAME}" DESTINATION "${CMAKE_INSTALL_BINDIR}")
    endif()

    # 5. Install VST3 Plugin Bundle Directory (.vst3 folder layout)
    if(NOT DEFINED VST3_INSTALL_DIR)
        if(CPACK_GENERATOR)
            set(VST3_INSTALL_DIR "/usr/lib/vst3")
        else()
            set(VST3_INSTALL_DIR "$ENV{HOME}/.vst3")
        endif()
    endif()

    set(VST3_BUNDLE_SOURCE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}_artefacts/$<CONFIG>/VST3/${PRODUCT_NAME}.vst3")

    install(DIRECTORY "${VST3_BUNDLE_SOURCE}"
            DESTINATION "${VST3_INSTALL_DIR}"
            COMPONENT VST3
            FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    # 6. Post-install hooks for GTK Icon Cache and Desktop Database
    install(CODE "
        set(HICOLOR_DIR \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons/hicolor\")
        if(NOT EXISTS \"\${HICOLOR_DIR}/index.theme\")
            file(WRITE \"\${HICOLOR_DIR}/index.theme\"
                \"[Icon Theme]\\nName=Hicolor\\nComment=Fallback icon theme\\nHidden=true\\nDirectories=scalable/apps,16x16/apps,32x32/apps,48x48/apps,64x64/apps,128x128/apps,256x256/apps,512x512/apps\\n\")
        endif()
        
        execute_process(COMMAND gtk-update-icon-cache -f -q \"\${HICOLOR_DIR}\" ERROR_QUIET)
        execute_process(COMMAND update-desktop-database -q \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/applications\" ERROR_QUIET)
    ")
endif()

# -----------------------------------------------------------------------
# Packaging & Distribution (CPack for Linux .deb / .rpm)
# -----------------------------------------------------------------------
if(LINUX)
    # Prevent juceaide or auxiliary tools from being packaged
    set_target_properties(juceaide PROPERTIES EXCLUDE_FROM_ALL TRUE)
    set(CPACK_EXCLUDE_FILES_FROM_ALL
        "/include/"
        "/cmake/"
        "/JUCE/"
        "\\\\.h$"
        "\\\\.hpp$"
        "\\\\.cmake$"
    )
    # General CPack Information
    set(CPACK_PACKAGE_NAME "ctrlrx")
    set(CPACK_PACKAGE_VENDOR "CtrlrX Project")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MIDI Control Surface Framework & Editor")
    set(CPACK_PACKAGE_VERSION "${CURRENT_VERSION}")
    set(CPACK_PACKAGE_CONTACT "CtrlrX Team <support@ctrlrx.org>")
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/ctrlrx/ctrlrx")
    set(CPACK_PACKAGE_LICENSE "GPL-3.0")

    # Install Directory Structure
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")

    # Package all components (Standalone + VST3) into a single installer package
    set(CPACK_MONOLITHIC_INSTALL ON)

    # DEB Metadata
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "CtrlrX Team <support@ctrlrx.org>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_PACKAGE_HOMEPAGE_URL}")

    # RPM Metadata
    set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")
    set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)
    set(CPACK_RPM_PACKAGE_VENDOR "${CPACK_PACKAGE_VENDOR}")
    set(CPACK_RPM_PACKAGE_URL "${CPACK_PACKAGE_HOMEPAGE_URL}")

    include(CPack)
endif()

include(PamplejuceIPP)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(Tests)
endif()

include(GitHubENV)