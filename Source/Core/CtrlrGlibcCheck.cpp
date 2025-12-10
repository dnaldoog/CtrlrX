/*
  ==============================================================================

    CtrlrLinuxGlibcCheck.cpp
    Created: 10 Dec 2025 10:26:08am
    Author:  zan64

  ==============================================================================
*/

/**************************************************************
   LinuxGlibcCheck.cpp
   --------------------
   This file injects a GLIBC version check into CtrlrX startup
   WITHOUT requiring access to JUCE's main() or the App class.

   It triggers *before* the GUI fully loads, warning the user
   if their Linux system GLIBC is older than 2.38 (Wayland fix).
 **************************************************************/

#if JUCE_LINUX

#include <gnu/libc-version.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

 // ----------------------------------------------------------------------
 // Helper: Check if GLIBC version is less than required (2.38)
 // ----------------------------------------------------------------------
static bool glibcTooOld()
{
    const char* ver = gnu_get_libc_version();
    auto versionString = juce::String(ver);

    // Parse "MAJOR.MINOR"
    int major = versionString.upToFirstOccurrenceOf(".", false, false).getIntValue();
    int minor = versionString.fromFirstOccurrenceOf(".", false, false).getIntValue();

    return (major < 2) || (major == 2 && minor < 38);
}

// ----------------------------------------------------------------------
// Static initializer: runs before JUCEApplication::initialise()
// ----------------------------------------------------------------------
struct GlibcStartupChecker
{
    GlibcStartupChecker()
    {
        if (glibcTooOld())
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Incompatible GLIBC Version",
                "Your system's GLIBC version is too old to support the "
                "Wayland fixes used by CtrlrX.\n\n"
                "Required: GLIBC 2.38 or newer\n"
                "Detected: " + juce::String(gnu_get_libc_version()) + "\n\n"
                "If you experience GUI flicker or crashes, please upgrade "
                "your Linux distribution or GLIBC package.");
        }
    }
};

// Instantiate the static checker
static GlibcStartupChecker glibcChecker;

#endif // JUCE_LINUX

