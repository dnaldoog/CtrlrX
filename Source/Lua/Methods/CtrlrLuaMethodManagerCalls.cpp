#include "CtrlrComponents/CtrlrComponent.h"
#include "CtrlrComponents/CtrlrCustomComponent.h"
#include "CtrlrComponents/Specials/CtrlrWaveform.h"
#include "CtrlrLog.h"
#include "CtrlrLua/MethodEditor/CtrlrLuaMethodEditor.h"
#include "CtrlrLuaManager.h"
#include "CtrlrLuaMethodManager.h"
#include "CtrlrLuaObjectWrapper.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelCanvas.h"
#include "JuceClasses/LMemoryBlock.h"
#include "stdafx.h"
#include "stdafx_luabind.h"
#include <luabind/luabind.hpp>

// Reads the error text a failed Lua call left on top of the Lua stack, without ever throwing.
// (The old macros did object_cast<std::string>() on the stack top; if that value was not a string it
// threw cast_failed from INSIDE the catch block, which nothing could catch.)
static juce::String describeLuaError(lua_State *L) {
	if (L == nullptr || lua_gettop(L) < 1)
		return "Unknown Lua error (no message available)";

	const int t = lua_type(L, -1);

	if (t == LUA_TSTRING || t == LUA_TNUMBER) {
		if (const char *msg = lua_tostring(L, -1))
			return juce::String::createStringFromData(msg, (int)strlen(msg));
	}

	return juce::String("Lua error (") + lua_typename(L, t) + " value, no message)";
}

void CtrlrLuaMethodManager::handleMethodException(CtrlrLuaMethod *o, const juce::String &message,
												  const bool trimLogMessage) {
	try {
		if (o)
			o->setValid(false);

		const juce::String name = o ? o->getName() : juce::String("Unknown");

		// The _NO_DIALOG variant logged only the text after "Error message:\n"; the editor output always
		// got the full text.
		const juce::String logText =
			trimLogMessage ? message.fromLastOccurrenceOf("Error message:\n", false, false) : message;

		_LERR("Callback error: [" + name + "]\n" + logText + ".\nMethod disabled");

		if (currentMethodEditor && currentMethodEditor->getMethodEditArea())
			currentMethodEditor->getMethodEditArea()->insertOutput(message, juce::Colours::red);
	} catch (...) {
		// reporting an error must never raise another one
	}
}

// One shared body. luabind::error first; cast_failed and other std exceptions fall into the second clause.
#define CATCH_METHOD_EXCEPTION_IMPL(trimLog, returnStatement)                                                          \
	catch (luabind::error & e) {                                                                                       \
		handleMethodException(o, describeLuaError(e.state()), trimLog);                                                \
		returnStatement                                                                                                \
	}                                                                                                                  \
	catch (const std::exception &e) {                                                                                  \
		handleMethodException(o, juce::String::createStringFromData(e.what(), (int)strlen(e.what())), trimLog);        \
		returnStatement                                                                                                \
	}                                                                                                                  \
	catch (...) {                                                                                                      \
		handleMethodException(o, "Unknown exception while calling the Lua method", trimLog);                           \
		returnStatement                                                                                                \
	}

#define CATCH_METHOD_EXCEPTION CATCH_METHOD_EXCEPTION_IMPL(false, return (false);)
#define CATCH_METHOD_EXCEPTION_STR CATCH_METHOD_EXCEPTION_IMPL(false, return ("");)
#define CATCH_METHOD_EXCEPTION_DAD CATCH_METHOD_EXCEPTION_IMPL(false, (void)0;)
#define CATCH_METHOD_EXCEPTION_NO_DIALOG CATCH_METHOD_EXCEPTION_IMPL(true, return (false);)

#define LUA_DEBUG                                                                                                      \
	if (getDebug() && o) {                                                                                             \
		_INF("CtrlrLuaMethodManager::call " + o->getName());                                                           \
	}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o) {
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	if (!isMethodValid(o)) // also covers o == nullptr
		return (true);

	try {
		luabind::call_function<void>(o->getObject().getObject());
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}
const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrModulator *param1, const int param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2);
		}
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrModulator *param1, const double param2,
									   const uint8 param3) // Updated v5.6.32. int to double. Callback on value Change
{
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2, param3);
		}
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrModulator *param1, const String param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2.toUTF8().getAddress());
		}
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrComponent *param1, const String param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2.toUTF8().getAddress());
		}
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrComponent *param1, const MouseEvent &param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2);
		}
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const MidiMessage &param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			CtrlrMidiMessage mm(param1);
			const luabind::object &obj = o->getObject().getLuabindObject();
			if (obj.is_valid()) {
				luabind::call_function<void>(obj, mm);
			}
		}
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const CtrlrMidiMessage &param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			const luabind::object &obj = o->getObject().getLuabindObject();
			if (obj.is_valid()) {
				luabind::call_function<void>(obj, param1);
			}
		}
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrCustomComponent *param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1);
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrCustomComponent *param1, Graphics &param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1, boost::ref(param2));
	}
	CATCH_METHOD_EXCEPTION_NO_DIALOG

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrPanelCanvas *param1, Graphics &param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1,
										 boost::reference_wrapper<Graphics>(param2));
		}
	}
	CATCH_METHOD_EXCEPTION_NO_DIALOG

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrPanel *param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1);
	}
	CATCH_METHOD_EXCEPTION_NO_DIALOG

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrCustomComponent *param1, const MouseEvent &param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1, boost::ref(param2));
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrCustomComponent *param1, const MouseEvent &param2,
									   float param3, float param4) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1, boost::ref(param2), param3, param4);
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const int param1, const int param2, const int param3) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1, param2, param3);
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const int param1, const int param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1, param2);
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const int param1, const int param2, const int param3,
									   CtrlrLuaObjectWrapper const &param4) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1, param2, param3, param4);
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const int param1, const File &param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1, param2);
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrModulator *param1, const File &param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1, param2);
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrWaveform *param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o))
			luabind::call_function<void>(o->getObject().getObject(), param1);
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const ValueTree &param1, CtrlrLuaObjectWrapper &param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2);
		}
	}
	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const String &param1, const CtrlrNotificationType param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

int CtrlrLuaMethodManager::callWithRet(CtrlrLuaMethod *o, CtrlrModulator *param1, const int param2) {
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			return (luabind::call_function<int>(o->getObject().getObject(), param1, param2));
		}
	}
	CATCH_METHOD_EXCEPTION

	return (param2);
}

int CtrlrLuaMethodManager::callWithRet(CtrlrLuaMethod *o, CtrlrModulator *param1, const CtrlrMidiMessage &param2,
									   const int param3) {
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			return (luabind::call_function<int>(o->getObject().getObject(), param1, param2, param3));
		}
	}
	CATCH_METHOD_EXCEPTION

	return (param3);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const StringArray &param1, const int param2,
									   const int param3) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2, param3);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const StringArray &param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

int CtrlrLuaMethodManager::callWithRet(CtrlrLuaMethod *o, ValueTree valueTree1, ValueTree valueTree2) {
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (-2);

	try {
		if (isMethodValid(o)) {
			return (luabind::call_function<int>(o->getObject().getObject(), valueTree1, valueTree2));
		}
	}
	CATCH_METHOD_EXCEPTION

	return (-1);
}

String CtrlrLuaMethodManager::callWithRetString(CtrlrLuaMethod *o, CtrlrCustomComponent *param1) {
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return ("");

	try {
		if (isMethodValid(o)) {
			return (luabind::call_function<String>(o->getObject().getObject(), param1));
		}
	}
	CATCH_METHOD_EXCEPTION_STR

	return ("");
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrCustomComponent *param1, const KeyPress &param2,
									   Component *param3) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2, param3);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrCustomComponent *param1, const bool param2,
									   Component *param3) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2, param3);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrCustomComponent *param1, const double param2,
									   const bool param3) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2, param3);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, CtrlrCustomComponent *param1,
									   DragAndDropSourceDetails param2) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

int CtrlrLuaMethodManager::callWithRet(CtrlrLuaMethod *o, CtrlrCustomComponent *param1,
									   DragAndDropSourceDetails param2) {
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (-2);

	try {
		if (isMethodValid(o)) {
			return (luabind::call_function<int>(o->getObject().getObject(), param1, param2));
		}
	}
	CATCH_METHOD_EXCEPTION

	return (-1);
}

DragAndDropSourceDetails CtrlrLuaMethodManager::callWithRet(CtrlrLuaMethod *o, CtrlrCustomComponent *param1,
															const MouseEvent &param2) {
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (DragAndDropSourceDetails());

	try {
		if (isMethodValid(o)) {
			return (luabind::call_function<DragAndDropSourceDetails>(o->getObject().getObject(), param1, param2));
		}
	}
	CATCH_METHOD_EXCEPTION_DAD

	return (DragAndDropSourceDetails());
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, LMemoryBlock *param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, ValueTree &param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, MidiBuffer *param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const ValueTree &param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::callAudio(CtrlrLuaMethod *o, MidiBuffer &param1,
											const AudioPlayHead::CurrentPositionInfo &param2) {
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const String &param1, const String &param2,
									   CtrlrLuaObjectWrapper const &param3) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1, param2, param3);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}

const bool CtrlrLuaMethodManager::call(CtrlrLuaMethod *o, const uint8 param1) {
	if (!isMethodValid(o))
		return (false);
	const ScopedLock sl(methodManagerCriticalSection);

	LUA_DEBUG

	if (isLuaDisabled())
		return (true);

	try {
		if (isMethodValid(o)) {
			luabind::call_function<void>(o->getObject().getObject(), param1);
		}
	}

	CATCH_METHOD_EXCEPTION

	return (true);
}
