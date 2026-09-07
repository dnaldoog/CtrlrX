#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLog.h"
#include "CtrlrMacros.h"
#include "CtrlrStandaloneWindow.h"
#include "LinuxDpiScale.h"
#include "stdafx.h"

#if JUCE_LINUX
#include <cmath>
#endif

class CtrlrApplication : public JUCEApplication {
	public:
		CtrlrApplication() : filterWindow(nullptr) {}

		static void crashHandler() {
			if (JUCEApplication::isStandaloneApp()) {
				MemoryBlock mb(SystemStats::getStackBacktrace().toUTF8(), SystemStats::getStackBacktrace().length());

				// Using raw string literals R"(...)" means we can use normal quotes inside the string safely!
				File::getSpecialLocation(File::currentApplicationFile)
					.startAsProcess(String(R"(--crashReport=")") +
									File::getSpecialLocation(File::currentApplicationFile).getFullPathName() +
									String(R"(" --stackTrace=")") + mb.toBase64Encoding() + String(R"(")"));
			} else {
				const String stackTrace = SystemStats::getStackBacktrace();
				File crashFile(File::getSpecialLocation(File::currentApplicationFile).getFileExtension() + ".crash");

				AW::showNativeDialogBox(
					AW::Warning, "Ctrlr has crashed",
					"Looks like Ctrlr has crashed, since this is not a standalone instance, we won't do anything. "
					"A crash log will be written to " +
						crashFile.getFullPathName() + "\n\n" + stackTrace,
					"OK", "", false);

				crashFile.replaceWithText("Ctrlr crash at: " + Time::getCurrentTime().toString(true, true, true, true) +
										  "\nStack trace:\n" + stackTrace);
			}
		}

		const StringArray getParameters(const String &cli) {
			StringArray tokens;
			StringArray ret;
			tokens.addTokens(cli, " ", "\'\"");

			for (int i = 0; i < tokens.size(); i++) {
				ret.add(tokens[i].fromFirstOccurrenceOf("--", false, false).upToFirstOccurrenceOf("=", false, true));
			}

			return ret;
		}

		const StringArray getParameterValues(const String &cli) {
			StringArray tokens;
			StringArray ret;
			tokens.addTokens(cli, " ", "\'\"");

			for (int i = 0; i < tokens.size(); i++) {
				ret.add(tokens[i].fromFirstOccurrenceOf("=", false, false).unquoted().trim());
			}

			return ret;
		}

		// --- REFACTOR: initialise avoiding runModalLoop() ---
		void initialise(const String &commandLineParameters) override {
			Logger::writeToLog("CTRLR:initialise params \"" + commandLineParameters + "\"");

#if JUCE_LINUX
    // 1. Set global scale factor
    const double linuxScale = ctrlrx_get_linux_scale_factor();
    if (std::abs(linuxScale - 1.0) > 0.001)
        Desktop::getInstance().setGlobalScaleFactor((float)linuxScale);

    Logger::writeToLog("CTRLR:linux scale factor \"" + String(linuxScale, 3) + "\"");
#endif

			if (!commandLineParameters.isEmpty()) {
				StringArray parameters = getParameters(commandLineParameters);
				StringArray parameterValues = getParameterValues(commandLineParameters);

				if (parameters.contains("crashReport")) {
					File crashReportForExec(parameterValues[parameters.indexOf("crashReport")]);

					// Allocated with shared_ptr to ensure variables remain alive inside the async callback
					auto crashReportFile = std::make_shared<File>(
						crashReportForExec.withFileExtension(crashReportForExec.getFileExtension() + ".crash"));
					auto stackTrace = std::make_shared<String>("?");

					// Allocated on the heap so the dialog window persists past this function's scope
					auto *crashReport =
						new AlertWindow("Ctrlr has crashed",
										"This is a crash indicator, it means that Ctrlr has crashed for some "
										"reason. Some crash information will be written to: " +
											crashReportFile->getFullPathName(),
										AlertWindow::WarningIcon);

					if (parameters.contains("stackTrace")) {
						if (!parameterValues[parameters.indexOf("stackTrace")].isEmpty()) {
							MemoryBlock mb;
							mb.fromBase64Encoding(parameterValues[parameters.indexOf("stackTrace")]);
							*stackTrace = mb.toString();
							crashReport->addTextBlock(*stackTrace);
						}
					}

					crashReport->addButton("OK", 1, KeyPress(KeyPress::returnKey));

					// Use AW async engine to break out of modal execution loops safely
					AW::runCustomAlertAsyncSafe(crashReport, [crashReport, crashReportFile, stackTrace](int result) {
						crashReportFile->replaceWithText(
							"Ctrlr crash at: " + Time::getCurrentTime().toString(true, true, true, true) +
							"\nStack trace:\n" + *stackTrace);

						// Cleanup the heap allocation
						delete crashReport;

						// Tear down the application loop
						JUCEApplication::quit();
					});

					// Terminate normal initialization execution immediately.
					// This stops the main application window from opening while the crash window is up.
					return;
				}
			}

			// --- Normal Application Startup Path ---
			SystemStats::setApplicationCrashHandler(
				(juce::SystemStats::CrashHandlerFunction)&CtrlrApplication::crashHandler);

			filterWindow = new CtrlrStandaloneWindow(
				ProjectInfo::projectName + String("/") + ProjectInfo::versionString, Colours::lightgrey);

			if (File::isAbsolutePath(commandLineParameters.unquoted()))
				filterWindow->openFileFromCli(File(commandLineParameters.unquoted()));
		}

		void shutdown() override {
			juce::PopupMenu::dismissAllActiveMenus();

			if (filterWindow != nullptr) {
				// 1. Force the window to flush open panel states to XML / Properties
				filterWindow->saveStateNow();

				// 2. Delete the window to trigger proper destruction / cleanup
				delete filterWindow;
				filterWindow = nullptr;
			}
		}

const String getApplicationName() override 
{ 
#if JUCE_LINUX
    // Lowercase string ensures GNOME maps the window to ctrlr-x.desktop
    return "ctrlr-x"; 
#else
    // Windows and macOS continue using the standard clean name ("CtrlrX")
    return ProjectInfo::projectName; 
#endif
}
const String getApplicationVersion() override { return ProjectInfo::versionString; }
bool moreThanOneInstanceAllowed() override { return true; }

	private:
		Component::SafePointer<CtrlrStandaloneWindow> filterWindow;
};

// Main macro hook to launch the application
START_JUCE_APPLICATION(CtrlrApplication)
