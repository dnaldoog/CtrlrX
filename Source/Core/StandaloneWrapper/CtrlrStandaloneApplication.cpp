#include "CtrlrLog.h"
#include "CtrlrMacros.h"
#include "CtrlrStandaloneWindow.h"
#include "LinuxDpiScale.h"
#include "stdafx.h"

#if JUCE_LINUX
#include <cmath>
#endif
#if JUCE_LINUX
class LinuxKeypadFixer : public juce::KeyListener {
	public:
		LinuxKeypadFixer() {}

		bool keyPressed(const juce::KeyPress &key, juce::Component *originatingComponent) override {
			// In JUCE 6, if the keycode is unmapped, we can check its raw code.
			// If JUCE's X11 layer passed through the raw 82:
			if (key.getKeyCode() == 82) {
				// Trigger your zoom out command directly here via the global command manager
				// e.g., CtrlrProcessorEditor::getCommandManager().invokeDirectly(doZoomOut, true);
				return true; // Return true to signal that we completely handled the event
			}
			return false; // Let all other keys pass through normally
		}
};
#endif
class CtrlrApplication : public JUCEApplication {
	public:
		CtrlrApplication() : filterWindow(nullptr) {}

		static void crashHandler() {
			if (JUCEApplication::isStandaloneApp()) {
				MemoryBlock mb(SystemStats::getStackBacktrace().toUTF8(), SystemStats::getStackBacktrace().length());
				File::getSpecialLocation(File::currentApplicationFile)
					.startAsProcess("--crashReport=\"" +
									File::getSpecialLocation(File::currentApplicationFile).getFullPathName() +
									"\" --stackTrace=\"" + mb.toBase64Encoding() + "\"");
			} else {
				const String stackTrace = SystemStats::getStackBacktrace();
				File crashFile(File::getSpecialLocation(File::currentApplicationFile).getFileExtension() + ".crash");

				AlertWindow::showMessageBox(
					AlertWindow::WarningIcon, "Ctrlr has crashed",
					"Looks like Ctrlr has crashed, since this is not a standalone instance, we won't do anything.\
														A crash log will be written to " +
						crashFile.getFullPathName() + "\n\n" + stackTrace);

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

			return (ret);
		}

		const StringArray getParameterValues(const String &cli) {
			StringArray tokens;
			StringArray ret;
			tokens.addTokens(cli, " ", "\'\"");

			for (int i = 0; i < tokens.size(); i++) {
				ret.add(tokens[i].fromFirstOccurrenceOf("=", false, false).unquoted().trim());
			}

			return (ret);
		}

		void initialise(const String &commandLineParameters) {
			Logger::writeToLog("CTRLR:initialise params \"" + commandLineParameters + "\"");

#if JUCE_LINUX

			std::unique_ptr<LinuxKeypadFixer> linuxKeyFixer;

			// Inside the constructor:
			linuxKeyFixer = std::make_unique<LinuxKeypadFixer>();
			juce::Desktop::getInstance().addGlobalMouseListener(linuxKeyFixer.get());
			const double linuxScale = ctrlrx_get_linux_scale_factor();
			if (std::abs(linuxScale - 1.0) > 0.001)
				Desktop::getInstance().setGlobalScaleFactor((float)linuxScale);

			Logger::writeToLog("CTRLR:linux scale factor \"" + String(linuxScale, 3) + "\"");
#endif

			{
				bool setcrashhandler = true;
				if (!commandLineParameters.isEmpty()) {
					String stackTrace = "?";
					StringArray parameters = getParameters(commandLineParameters);
					StringArray parameterValues = getParameterValues(commandLineParameters);

					if (parameters.contains("crashReport")) {
						File crashReportForExec(parameterValues[parameters.indexOf("crashReport")]);
						File crashReportFile(
							crashReportForExec.withFileExtension(crashReportForExec.getFileExtension() + ".crash"));
						AlertWindow crashReport("Ctrlr has crashed",
												"This is a crash indicator, it means that Ctrlr has crashed for some "
												"reason. Some crash information will be written to: " +
													crashReportFile.getFullPathName(),
												AlertWindow::WarningIcon);

						if (parameters.contains("stackTrace")) {
							if (!parameterValues[parameters.indexOf("stackTrace")].isEmpty()) {
								MemoryBlock mb;
								mb.fromBase64Encoding(parameterValues[parameters.indexOf("stackTrace")]);
								stackTrace = mb.toString();
								crashReport.addTextBlock(stackTrace);
							}
						}
						crashReport.addButton("OK", 1, KeyPress(KeyPress::returnKey));
						crashReport.runModalLoop();

						crashReportFile.replaceWithText(
							"Ctrlr crash at: " + Time::getCurrentTime().toString(true, true, true, true) +
							"\nStack trace:\n" + stackTrace);

						JUCEApplication::quit();
					}
				}
				// Set the crash handler only, if no crash is reported.
				if (setcrashhandler)
					SystemStats::setApplicationCrashHandler(
						(juce::SystemStats::CrashHandlerFunction)&CtrlrApplication::crashHandler);
			}

			filterWindow = new CtrlrStandaloneWindow(
				ProjectInfo::projectName + String("/") + ProjectInfo::versionString, Colours::lightgrey);

			if (File::isAbsolutePath(commandLineParameters.unquoted()))
				filterWindow->openFileFromCli(File(commandLineParameters.unquoted()));
		}

		void shutdown() {
			if (filterWindow) {
				deleteAndZero(filterWindow);
			}
		}

		const String getApplicationName() { return (ProjectInfo::projectName); }

		const String getApplicationVersion() { return (ProjectInfo::versionString); }

		void unhandledException(const std::exception *e, const String &sourceFilename, int lineNumber) {
			_DBG("CtrlrApplication::unhandledException");
			_DBG("\tfile: " + sourceFilename + ":" + _STR(lineNumber));
			_DBG("\t" + _STR(e->what()));
		}

	private:
		CtrlrStandaloneWindow *filterWindow;
};
START_JUCE_APPLICATION(CtrlrApplication)
