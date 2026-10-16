#include "CtrlrModulatorProcessor.h"
#include "CtrlrComponents/CtrlrComponent.h"
#include "CtrlrIDs.h"
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLog.h"
#include "CtrlrLuaManager.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrModulator.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrUtilities.h"
#include "Plugin/CtrlrProcessor.h"
#include "stdafx.h"

CtrlrModulatorProcessor::CtrlrModulatorProcessor(CtrlrModulator &_owner)
	: owner(_owner),
	  valueChangedCbk(0),
	  usingForwardProcess(false),
	  usingReverseProcess(false),
	  minValue(0),
	  maxValue(127),
	  usingValueMap(false),
	  linkedToGlobal(false),
	  isMute(false) {

	ctrlrMidiMessage = std::make_unique<CtrlrOwnedMidiMessage>(*this);
	ctrlrMidiControllerMessage = std::make_unique<CtrlrOwnedMidiMessage>(*this, Identifier(Ids::controllerMIDI));
}

CtrlrModulatorProcessor::~CtrlrModulatorProcessor()
{
}

void CtrlrModulatorProcessor::handleAsyncUpdate()
{
	/* update the GUI and the ValueTree from the value provided by the HOST or the MIDI subsystem */
	if (owner.isStatic() && (currentValue.lastChangeSource == CtrlrModulatorValue::changedByMidiIn ||
							 currentValue.lastChangeSource == CtrlrModulatorValue::changedByHost)) {
		return;
	}
	{
		const ScopedReadLock sl(processorLock);

		if ((double)owner.getProperty(Ids::modulatorValue) == currentValue.value) {
			owner.removeProperty(Ids::modulatorValue);
		}

		const double sliderDecPlace = 4.0;
		const double multiplier = std::pow(10, sliderDecPlace);
		const double roundedValue = std::round(currentValue.value * multiplier) / multiplier;

		owner.setProperty(Ids::modulatorValue, roundedValue);
	}

	// --- SAFETY HELPER: Ensure callback pointer is safe, active, and has a valid Lua name ---
	auto isCallbackSafe = [this](CtrlrLuaMethod *cbk) {
		if (!cbk || !cbk->isValid())
			return false;

		// Ensure the method's name is a valid Lua identifier (prevents crashes from legacy/malformed names like "1")
		String methodName = cbk->getName();
		if (methodName.isEmpty())
			return false;

		return owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().isValidMethodName(methodName);
	};
	// ------------------------------------------------------------------------------------------

	const bool warningInBoostrapState = owner.getCtrlrManagerOwner().getProperty(Ids::ctrlrWarningInBootstrapState);

	if (warningInBoostrapState == (true)) {
		if (isCallbackSafe(valueChangedCbk.get()) && !owner.getOwnerPanel().getRestoreState()) {
			owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().call(
				valueChangedCbk.get(), &owner, currentValue.value, (uint8)currentValue.lastChangeSource);
		} else if (valueChangedCbk.get() && !owner.getOwnerPanel().getRestoreState()) {
			AW::showMessageBox(AW::Warning, "Callback error",
							   owner.getName() + "\n" +
								   "LUA script not valid or has an illegal name: luaModulatorValueChange");
		}
	} else {
		if (isCallbackSafe(valueChangedCbk.get()) && !owner.getOwnerPanel().getBootstrapState() &&
			!owner.getOwnerPanel().getRestoreState() &&
			currentValue.lastChangeSource != CtrlrModulatorValue::changedByProgram) {
			owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().call(
				valueChangedCbk.get(), &owner, currentValue.value, (uint8)currentValue.lastChangeSource);
		} else if (valueChangedCbk.get() && !owner.getOwnerPanel().getBootstrapState() &&
				   !owner.getOwnerPanel().getRestoreState() &&
				   currentValue.lastChangeSource != CtrlrModulatorValue::changedByProgram) {
			AW::showMessageBox(AW::Warning, "Callback error",
							   owner.getName() +
								   "\n LUA script not valid or has an illegal name: luaModulatorValueChange");
		}
	}

	if (linkedToGlobal) {
		owner.getOwnerPanel().setGlobalVariable(getLinkedToGlobalIndex(), currentValue.value);
	}
}

void CtrlrModulatorProcessor::setValue(const double value)  // Updated v5.6.32. int to double
{
}

void CtrlrModulatorProcessor::setModulatorMaxValue (const double newMaxValue) // Updated v5.6.32. int to double
{
	const ScopedWriteLock sl (processorLock);
	maxValue = newMaxValue;
}

void CtrlrModulatorProcessor::setModulatorMinValue (const double newMinValue) // Updated v5.6.32. int to double
{
	const ScopedWriteLock sl (processorLock);
	minValue = newMinValue;
}

void CtrlrModulatorProcessor::setForwardExpression (const String &forwardExpressionString)
{
	if (forwardExpressionString == "" || forwardExpressionString == EXP_MODULATOR_FORWARD)
	{
		const ScopedWriteLock sl (processorLock);

		usingForwardProcess = false;
		return;
	}

	{
		const ScopedWriteLock sl (processorLock);
		String parseError;

		forwardProcess = Expression(forwardExpressionString, parseError);
		if (!parseError.isEmpty())
		{
			_ERR("CtrlrModulatorProcessor::setForwardExpression parse error: " + parseError);
			usingForwardProcess = false;

			return;
		}

		usingForwardProcess = true;
	}
}

void CtrlrModulatorProcessor::setReverseExpression (const String &reverseExpressionString)
{
	if (reverseExpressionString == "" || reverseExpressionString == EXP_MODULATOR_REVERSE)
	{
		const ScopedWriteLock sl (processorLock);

		usingReverseProcess = false;
		return;
	}

	{
		const ScopedWriteLock sl (processorLock);
		String parseError;
		reverseProcess = Expression(reverseExpressionString, parseError);
		if (!parseError.isEmpty())
		{
			_ERR("CtrlrModulatorProcessor::setReverseExpression parse error: " + parseError);
			usingReverseProcess = false;

			return;
		}

		usingReverseProcess = true;
	}
}

void CtrlrModulatorProcessor::setValueChangedCallback (const String &methodName)
{
	if (methodName.isEmpty()) {
		valueChangedCbk = nullptr;
		return;
	}

	auto &methodManager = owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager();
	if (!methodManager.isValidMethodName(methodName)) {
		DBG("Rejected invalid callback name: " + methodName);
		valueChangedCbk = nullptr;
		return;
	}

	valueChangedCbk = methodManager.getMethod(methodName);
}

void CtrlrModulatorProcessor::setGetValueForMidiCallback (const String &methodName)
{
	if (methodName.isEmpty())
		return;

	getValueForMidiCbk = owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().getMethod(methodName);
}

void CtrlrModulatorProcessor::setGetValueFromMidiCallback (const String &methodName)
{
	if (methodName.isEmpty())
		return;

	getValueFromMidiCbk = owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().getMethod(methodName);
}

void CtrlrModulatorProcessor::setMappedValue (const CtrlrModulatorValue mappedValue, const bool force, const bool mute)
{
	setValueGeneric (CtrlrModulatorValue (valueMap.getIndexForValue (mappedValue.value), mappedValue.lastChangeSource), force, mute);
}

void CtrlrModulatorProcessor::setValueGeneric(const CtrlrModulatorValue inValue, const bool force, const bool mute)
{
	/* there are 3 sources of value changes

		- gui
		- midi
		- host

		- gui should affect the MIDI first and then send the value to host, when it comes back from host
			and is the same as the value set before sending it to host, nothing should happen, if theyre
			different it should be set to the MIDI and a async update should be triggered

		- midi should affect the host first

		- host update has to check what's different from the value received, if it is different an update
			should be triggered (midi sent and/or async update done)


		** problem, when a value comes back from host it needs to be re-mapped to a midi value that lives
			in the UI component, this will be the audio thread so asking the component will require locking
			it might be better to keep a copy of value mapping (just the numeric part, we don't need the text, or do we?)
			internaly in the processor so we can avoid locking at runtime
	*/
	{
		/* if the values are already the same, and the force flag is set to false
			don't do anything the modulator is already in the state it should be */

		const ScopedReadLock sl(processorLock);

		if (currentValue.value == inValue.value && force == false)
		{
			return;
		}
	}

	{
		/* set the current value and the midi messages value,
			send the MIDI out and notify the host about the parameter change */
		const ScopedWriteLock sl(processorLock);

		/* it's the same value, but it's forced, send out MIDI and triggerAsyncUpdate
			don't inform the host, it already knows about it

			if mute is true, no midi goes out */
		if (currentValue.value == inValue.value && force == true)
		{
			if (!mute)
				sendMidiMessage();

			triggerAsyncUpdate();

			return;
		}

		/* first we set the currentValue to the new value comming from the gui, it's needed for the
			expressions evaluations to work */

		currentValue = inValue;

		/* send the midi message, this is done using a special thread so it won't wait until it's actualy sent */
		if (!mute)
			sendMidiMessage();

		triggerAsyncUpdate();
	}

	/* notify the pluginHost about the change */
	setParameterNotifyingHost();
}

void CtrlrModulatorProcessor::setValueFromHost(const float inValue) // Note v5.6.33. Host->CtrlrX : This is where the param value automation happens
{
    /* called from the audio thread */
    const int possibleNewValue = denormalizeValue (inValue, minValue, maxValue);

    // Quick check for early exit under a read lock
    {
        const ScopedReadLock sl (processorLock);
        if (possibleNewValue == roundDoubleToInt(currentValue.value))
        {
            return; // No need to triggerAsyncUpdate here if the value hasn't changed
        }
    }

    // If the value has changed, acquire a write lock and update
    {
        const ScopedWriteLock sl(processorLock);
        
        /* set the new value for the modulator */
        currentValue.value = possibleNewValue;
        currentValue.lastChangeSource = CtrlrModulatorValue::changedByHost;
        
        /* send a midi message */
        sendMidiMessage();
    }

    /* update the modulator ValueTree and tell the GUI about the changes */
    triggerAsyncUpdate();
}


void CtrlrModulatorProcessor::setValueFromMIDI(CtrlrMidiMessage &m, const CtrlrMIDIDeviceType source)
{
	/* called from the Panel's MIDI thread */
	{
		const ScopedWriteLock sl (processorLock);

		/* merge the incomming midi data with our message */
		mergeMidiData (m, getMidiMessage(source));

		/* fetch the value from the midi message and pass it to the host */
		const int possibleValue = getValueFromMidiMessage(source);

		if (currentValue.value != possibleValue)
		{
			if (isInValidMappedRange (possibleValue))
			{
				currentValue.value 				= possibleValue;
				currentValue.lastChangeSource	= CtrlrModulatorValue::changedByMidiIn;

				/* notify the pluginHost about the change */
				setParameterNotifyingHost();
                
                /* update the modulator ValueTree and tell the GUI about the changes */
                triggerAsyncUpdate(); // Added v5.6.32. Required to update mod value from MIDI input message
			}
		}
	}
}

void CtrlrModulatorProcessor::setParameterNotifyingHost() // CtrlrX->VST Host Updated v5.6.35. JUCE 7+
{
    // 1. Basic sanity check for export status and index
    if (owner.getVstIndex() >= 0 && owner.isExportedToVst())
    {

        // 2. Get the list of parameters currently registered with the AudioProcessor
        auto& params = getProcessor()->getParameters();
        const int index = owner.getVstIndex();

        // 3. Safety: Ensure the index actually exists in the processor
        if (index < params.size())
        {
            // 4. Get the parameter object (modern JUCE 8 approach)
            if (auto* param = params[index])
            {
                // 5. Signal the beginning of a "gesture" (important for DAW automation/undo)
                param->beginChangeGesture();

                // 6. Normalize the value and send it to the host
                float normalizedValue = normalizeValue (currentValue.value, minValue, maxValue);
                param->setValueNotifyingHost (normalizedValue);

                // 7. Signal the end of the gesture
                param->endChangeGesture();
            }
        }
        else
        {
            // Optional: Log an error if the index is out of bounds
            // This helps debug if your modulators have VST indices higher than the param count
            _DBG("CtrlrModulatorProcessor::setParameterNotifyingHost: VST Index "
                 + String(index) + " is out of bounds (Param count: "
                 + String(params.size()) + ")");
        }
    }
}

int CtrlrModulatorProcessor::getValueFromMidiMessage(const CtrlrMIDIDeviceType source)
{
	int evaluationResult = 0;
	if (usingValueMap)
	{
		const int possibleValue = valueMap.getIndexForValue(evaluateReverse (getMidiMessage(source).getValue()));

		if (possibleValue >= 0)
		{
			evaluationResult = possibleValue;
		}
		else
		{
			evaluationResult = currentValue.value;
		}
	}
	else
	{
		evaluationResult = evaluateReverse (getMidiMessage(source).getValue());
	}

	if (getValueFromMidiCbk)
	{
		if (!getValueFromMidiCbk.wasObjectDeleted() && getValueForMidiCbk->isValid())
		{
			evaluationResult = owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().callWithRet (getValueFromMidiCbk, &owner, getMidiMessage(source), evaluationResult);
		}
	}

	return (evaluationResult);
}

float CtrlrModulatorProcessor::getValueForHost() const
{
	const ScopedReadLock sl (processorLock);

	return (normalizeValue (currentValue.value, minValue, maxValue));
}

CtrlrOwnedMidiMessage *CtrlrModulatorProcessor::getMidiMessagePtr(const CtrlrMIDIDeviceType source)
{
    if (source == controllerDevice)
		return (ctrlrMidiControllerMessage.get());

	return (ctrlrMidiMessage.get());
}

CtrlrMidiMessage &CtrlrModulatorProcessor::getMidiMessage(const CtrlrMIDIDeviceType source)
{
	const ScopedReadLock sl (processorLock);
    if (source == controllerDevice)
        return (*ctrlrMidiControllerMessage);

	return (*ctrlrMidiMessage);
}

CtrlrOwnedMidiMessage &CtrlrModulatorProcessor::getOwnedMidiMessage(const CtrlrMIDIDeviceType source)
{
	const ScopedReadLock sl (processorLock);
	if (source == controllerDevice)
        return (*ctrlrMidiControllerMessage);

	return (*ctrlrMidiMessage);
}

CtrlrProcessor *CtrlrModulatorProcessor::getProcessor()
{
	return (owner.getOwnerPanel().getCtrlrManagerOwner().getProcessorOwner());
}

double CtrlrModulatorProcessor::getValue() const // Updated v5.6.32. int to double
{
	const ScopedReadLock sl (processorLock);

	return (currentValue.value);
}

double CtrlrModulatorProcessor::getValueMapped() const // Updated v5.6.32. int to double
{
	const ScopedReadLock sl (processorLock);

	return (valueMap.getMappedValue(currentValue.value));
}

double CtrlrModulatorProcessor::getMax() const // Updated v5.6.32. int to double
{
	const ScopedReadLock sl (processorLock);

	return (maxValue);
}

double CtrlrModulatorProcessor::getMin() const // Updated v5.6.32. int to double
{
	const ScopedReadLock sl (processorLock);

	return (minValue);
}

void CtrlrModulatorProcessor::sendMidiMessage()
{
	if (getMidiMessage().getMidiMessageType() == None)
		return;

	if (!owner.getRestoreState())
	{
		ctrlrMidiMessage->setValue (getValueForMidiMessage(currentValue.value));

        {
            ScopedReadLock srl(processorLock);
            if (!isMute)
            {
                owner.getOwnerPanel().sendMidi (getMidiMessage(), -1);
                // Removed v5.6.23
                // if (owner.getOwnerPanel().getMidiOptionBool(panelMidiOutputToHost))
                // {
                //    owner.getOwnerPanel().queueMessageForHostOutput (getMidiMessage());
                // }

            }
        }
	}
	else
	{
		ctrlrMidiMessage->setValue (getValueForMidiMessage(currentValue.value));
	}
}

// Used by combo
const CtrlrValueMap &CtrlrModulatorProcessor::setValueMap (const String &mapAsString)
{
	{
		const ScopedWriteLock sl (processorLock);
		valueMap.parseString (mapAsString);
		usingValueMap	= true;
	}

	{
		const ScopedReadLock sl (processorLock);

		owner.setProperty (Ids::modulatorMax, valueMap.getNonMappedMax());
		owner.setProperty (Ids::modulatorMin, valueMap.getNonMappedMin());
		return (valueMap);
	}
}

void CtrlrModulatorProcessor::setValueMap (const CtrlrValueMap &map)
{
	{
		const ScopedWriteLock sl (processorLock);
		valueMap.copyFrom (map);
		usingValueMap	= true;
	}

	{
		const ScopedReadLock sl (processorLock);

		owner.setProperty (Ids::modulatorMax, valueMap.getNonMappedMax());
		owner.setProperty (Ids::modulatorMin, valueMap.getNonMappedMin());
	}
}

int CtrlrModulatorProcessor::getValueForMidiMessage(const int value)
{
	int evaluationResult = value;

	if (usingValueMap)
	{
		if (usingForwardProcess)
		{
			evaluationResult = evaluateForward (valueMap.getMappedValue(value));
		}
		else
		{
			evaluationResult = valueMap.getMappedValue(value);
		}
	}
	else
	{
		evaluationResult = evaluateForward(value);
	}

	if (getValueForMidiCbk)
	{
		if (!getValueForMidiCbk.wasObjectDeleted() && getValueForMidiCbk->isValid())
		{
			evaluationResult = owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().callWithRet (getValueForMidiCbk, &owner, evaluationResult);
		}
	}

	return (evaluationResult);
}

bool CtrlrModulatorProcessor::isInValidMappedRange(const int possibleValue) const
{
	if (usingValueMap)
	{
		if (possibleValue >= valueMap.getMappedMin() && possibleValue <= valueMap.getMappedMax())
		{
			return (true);
		}
	}
	else
	{
		if (possibleValue >= minValue && possibleValue <= maxValue)
		{
			return (true);
		}
	}

	return (false);
}

void CtrlrModulatorProcessor::setLinkedToGlobal(const bool _linkedToGlobal, const int _globalIndex)
{
	{
		const ScopedWriteLock sl (processorLock);
		linkedToGlobal	= _linkedToGlobal;
		globalIndex		= _globalIndex;
	}
}

bool CtrlrModulatorProcessor::getLinkedToGlobal()
{
	const ScopedReadLock sl (processorLock);

	return (linkedToGlobal);
}

int CtrlrModulatorProcessor::getLinkedToGlobalIndex()
{
	const ScopedReadLock sl (processorLock);

	return (globalIndex);
}

CtrlrValueMap &CtrlrModulatorProcessor::getValueMap()
{
	return (valueMap);
}

double CtrlrModulatorProcessor::evaluateForward(const double inValue) // Updated v5.6.32. int to double
{
	if (usingForwardProcess)
	{
		String errors;
		double ret;

		ret = forwardProcess.evaluate (*this, errors);

		if (!errors.isEmpty())
		{
			_ERR("CtrlrModulatorProcessor::evaluateForward " + errors);
		}

		return (ret);
	}

	return (inValue);
}

double CtrlrModulatorProcessor::evaluateReverse(const double inValue) // Updated v5.6.32. int to double
{
	if (usingReverseProcess)
	{
		String errors;
		double ret;
		ret = reverseProcess.evaluate (*this, errors);
		if (!errors.isEmpty())
		{
			_ERR("CtrlrModulatorProcessor::evaluateReverse " + errors);
		}

		return (ret);
	}

	return (inValue);
}

String CtrlrModulatorProcessor::getScopeUID() const
{
	return ("mProcessor");
}

double CtrlrModulatorProcessor::evaluateFunction (const String& functionName, const double* parameters, int numParameters) const
{
	return (evaluateFormulaFunction (owner.getOwnerPanel(), functionName, parameters, numParameters));
}

void CtrlrModulatorProcessor::visitRelativeScope (const String &scopeName, Visitor &visitor) const
{
	if (scopeName == "panel")
	{
		visitor.visit (owner.getOwnerPanel().getPanelEvaluationScope());
	}
	else if (scopeName == "global")
	{
		visitor.visit (owner.getOwnerPanel().getGlobalEvaluationScope());
	}
}

Expression CtrlrModulatorProcessor::getSymbolValue (const String& symbol) const
{
	if (symbol == "modulatorValue")
	{
		return (Expression((double)currentValue.value));
	}
	else if (symbol == "modulatorMappedValue")
	{
		return (Expression((double)valueMap.getMappedValue(currentValue.value)));
	}
	else if (symbol == "modulatorMax")
	{
		return (Expression((double)maxValue));
	}
	else if (symbol == "modulatorMin")
	{
		return (Expression((double)minValue));
	}
	else if (symbol == "modulatorMappedMax")
	{
		return (Expression((double)valueMap.getMappedMax()));
	}
	else if (symbol == "modulatorMappedMin")
	{
		return (Expression((double)valueMap.getMappedMin()));
	}
	else if (symbol == "vstIndex")
	{
		return (Expression((int)owner.getVstIndex())); // Updated v5.5.33. double to int
	}
	else if (symbol == "midiValue")
	{
		return (Expression((double)ctrlrMidiMessage->getValue()));
	}
	else if (symbol == "midiNumber")
	{
		return (Expression((double)ctrlrMidiMessage->getNumber()));
	}

	return (Expression(1.0));
}

int CtrlrModulatorProcessor::getMidiChannelForOwnedMidiMessages()
{
	return (owner.getOwnerPanel().getProperty(Ids::panelMidiOutputChannelDevice));
}

CtrlrSysexProcessor *CtrlrModulatorProcessor::getSysexProcessor()
{
	return (&owner.getOwnerPanel().getSysExProcessor());
}

Array<int,CriticalSection> &CtrlrModulatorProcessor::getGlobalVariables()
{
	return (owner.getOwnerPanel().getGlobalVariables());
}

void CtrlrModulatorProcessor::setMute (bool _isMute)
{
    const ScopedWriteLock sl (processorLock);
    isMute = _isMute;
}
