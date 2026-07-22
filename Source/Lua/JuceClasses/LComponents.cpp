#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLog.h"
#include "LJuce.h"
#include "stdafx.h"
#include "stdafx_luabind.h"

LAlertWindow::LAlertWindow(const String &title, const String &message, AlertIconType iconType)
    : AlertWindow(title, message, iconType, nullptr)
{
}

LAlertWindow::~LAlertWindow()
{
}

// --- Static Helper Modernization ---

void LAlertWindow::showMessageBox (AlertIconType iconType, const String& title, const String& message, const String& buttonText)
{
#if JUCE_VERSION >= 0x070000
    AlertWindow::showMessageBoxAsync (iconType, title, message, buttonText, nullptr, ModalCallbackFunction::create([](int){}));
#else
    AlertWindow::showMessageBox (iconType, title, message, buttonText, nullptr);
#endif
}

void LAlertWindow::showMessageBoxAsync (AlertIconType iconType, const String& title, const String& message, const String& buttonText)
{
#if JUCE_VERSION >= 0x070000
    AlertWindow::showMessageBoxAsync (iconType, title, message, buttonText, nullptr, ModalCallbackFunction::create([](int){}));
#else
    AlertWindow::showMessageBoxAsync (iconType, title, message, buttonText, nullptr);
#endif
}

// NOTE: These legacy synchronous methods cannot return a direct value safely anymore in an async environment.
// For Lua compatibility, they now fire asynchronously. If scripts depend on the boolean return values immediately, 
// Lua scripts will need to be adapted to use the instance-based modal handlers below.
bool LAlertWindow::showOkCancelBox (AlertIconType iconType, const String& title, const String& message, const String& button1Text, const String& button2Text)
{
#if JUCE_VERSION >= 0x070000
    auto alert = std::make_unique<AlertWindow>(title, message, iconType);
    alert->addButton(button1Text, 1);
    alert->addButton(button2Text, 0);
    alert->enterModalState(true, ModalCallbackFunction::create([](int){}), true);
    return true; 
#else
    return AlertWindow::showOkCancelBox (iconType, title, message, button1Text, button2Text, nullptr, nullptr);
#endif
}

int LAlertWindow::showYesNoCancelBox (AlertIconType iconType, const String& title, const String& message, const String& button1Text, const String& button2Text, const String& button3Text)
{
#if JUCE_VERSION >= 0x070000
    auto alert = std::make_unique<AlertWindow>(title, message, iconType);
    alert->addButton(button1Text, 1);
    alert->addButton(button2Text, 2);
    alert->addButton(button3Text, 0);
    alert->enterModalState(true, ModalCallbackFunction::create([](int){}), true);
    return 0;
#else
    return AlertWindow::showYesNoCancelBox (iconType, title, message, button1Text, button2Text, button3Text, nullptr, nullptr);
#endif
}
// #if JUCE_VERSION < 0x070000
// bool LAlertWindow::showNativeDialogBox (const String& title, const String& bodyText, bool isOkCancel)
// {
//     return AlertWindow::showNativeDialogBox (title, bodyText, isOkCancel);
// }
// #else
bool LAlertWindow::showNativeDialogBox (const String& title, const String& bodyText, bool isOkCancel)
{
	AW::showNativeDialogBox(AW::Question, title, bodyText, "OK", isOkCancel ? "Cancel" : "", true,
							[](bool userClickedYes) {
								// Callback execution handling result asynchronously
							});

	return true; // Indicates the dialog was opened
}

// #endif
// --- Asynchronous Context for Luabind Queries ---
void LAlertWindow::queryText(AlertIconType iconType, const String &title, const String &textMessage, const String &textAreaContent, const String &textAreaLabel, const String &button1Text, const String &button2Text, bool isContentPassword, luabind::object const &result)
{
#if JUCE_VERSION >= 0x070000
    // Dynamic allocation managed entirely by JUCE via enterModalState's deleteWhenDismissed parameter
    auto* w = new AlertWindow(title, textMessage, iconType, nullptr);
    w->addTextEditor ("userInput", textAreaContent, textAreaLabel, isContentPassword);
    w->addButton (button1Text, 1);
    w->addButton (button2Text, 0);

    // Capture the luabind object context cleanly inside the async callback closure
    w->enterModalState(true, ModalCallbackFunction::create([w, result](int res) mutable {
        result[1] = res;
        result[2] = w->getTextEditorContents("userInput");
    }), true);
#else
    AlertWindow w(title, textMessage, iconType, 0);
    w.addTextEditor ("userInput", textAreaContent,  textAreaLabel, isContentPassword);
    w.addButton (button1Text, 1);
    w.addButton (button2Text, 0);
    result[1] = w.runModalLoop();
    result[2] = w.getTextEditorContents("userInput");
#endif
}

ComboBox* LAlertWindow::getComboBoxComponent(const String &comboName)
{
    return AlertWindow::getComboBoxComponent (comboName);
}

int LAlertWindow::runModalLoop()
{
#if JUCE_VERSION < 0x070000
    // --- Legacy JUCE 6 Path ---
    // The JUCE 8 compiler will completely ignore this block, so it won't complain!
    const int ret = AlertWindow::runModalLoop();

    if (o.is_valid())
    {
        luabind::call_member<void> (o, ret, this);
    }

    return (ret);
#else
    // --- JUCE 8 Safety Fallback ---
    // We must return something to satisfy the 'int' return type signature, 
    // even though this function is never bound or called in JUCE 8.
    jassertfalse; // Triggers a break in debug mode if somehow reached
    return 0;
#endif
}

#if JUCE_VERSION >= 0x070000
// --- Modern JUCE 7/8 Path ---
void LAlertWindow::runModalLoopAsync()
{
    Component::SafePointer<LAlertWindow> safeThis(this);

    enterModalState(true, ModalCallbackFunction::create([safeThis](int ret) {
        if (safeThis != nullptr && safeThis->o.is_valid())
        {
            try {
                luabind::call_member<void>(safeThis->o, "callback", ret, safeThis.getComponent());
            } catch (const std::exception& e) {
               // CtrlrLog::formatMessage(CtrlrLog::CtrlrLogMessage(juce::String("Error in AlertWindow modal handler callback: ") + e.what()));
            }
        }
    }), false); 
}
#endif

void LAlertWindow::setModalHandler(luabind::object const& _o)
{
    o = _o;
}

// --- Binding Mapping Layer ---
void LAlertWindow::wrapForLua(lua_State *L) {
	using namespace luabind;

	module(L)[class_<AlertWindow>("JAlertWindow"),
			  class_<LAlertWindow, bases<AlertWindow, Component>>("AlertWindow")
				  .def(constructor<const String &, const String &, AlertIconType>())
				  .def("getAlertType", &AlertWindow::getAlertType)
				  .def("setMessage", &AlertWindow::setMessage)
				  .def("addButton", &AlertWindow::addButton)
				  .def("getNumButtons", &AlertWindow::getNumButtons)
				  .def("triggerButtonClick", &AlertWindow::triggerButtonClick)
				  .def("setEscapeKeyCancels", &AlertWindow::setEscapeKeyCancels)
				  .def("addTextEditor", &AlertWindow::addTextEditor)
				  .def("getTextEditorContents", &AlertWindow::getTextEditorContents)
				  .def("getTextEditor", &AlertWindow::getTextEditor)
				  .def("addComboBox", &AlertWindow::addComboBox)
				  .def("getComboBoxComponent", &LAlertWindow::getComboBoxComponent)
				  .def("addTextBlock", &AlertWindow::addTextBlock)
				  .def("addProgressBarComponent", &AlertWindow::addProgressBarComponent)
				  .def("addCustomComponent", &AlertWindow::addCustomComponent)
				  .def("getNumCustomComponents", &AlertWindow::getNumCustomComponents)
				  .def("getCustomComponent", &AlertWindow::getCustomComponent)
				  .def("removeCustomComponent", &AlertWindow::removeCustomComponent)
				  .def("containsAnyExtraComponents", &AlertWindow::containsAnyExtraComponents)
				  .def("setModalHandler", &LAlertWindow::setModalHandler)

	// Expose the safe asynchronous engine hook to Lua bindings instead of standard blocking loops
#if JUCE_VERSION >= 0x070000

				  .def("runModalLoop", &LAlertWindow::runModalLoopAsync)
				  .def("runModalLoopAsync", &LAlertWindow::runModalLoopAsync)
#else
				  .def("runModalLoop", &LAlertWindow::runModalLoop)
#endif
				  .def("exitModalState", &Component::exitModalState)
				  .enum_("AlertIconType")[value("NoIcon", 0), value("QuestionIcon", 1), value("WarningIcon", 2),
										  value("InfoIcon", 3)]
				  .scope[def("showMessageBox", &LAlertWindow::showMessageBox),
						 def("showMessageBoxAsync", &LAlertWindow::showMessageBoxAsync),
						 def("showOkCancelBox", &LAlertWindow::showOkCancelBox),
						 def("showYesNoCancelBox", &LAlertWindow::showYesNoCancelBox),
						 def("showNativeDialogBox", &LAlertWindow::showNativeDialogBox),
						 def("queryText", &LAlertWindow::queryText)]];
}