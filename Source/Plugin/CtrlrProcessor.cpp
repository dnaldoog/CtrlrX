#include "CtrlrProcessor.h"
#include "CtrlrApplicationWindow/CtrlrEditor.h"
#include "CtrlrLog.h"
#include "CtrlrMacros.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelMIDIInputThread.h"
#include "CtrlrProcessorEditorForLive.h"
#include "CtrlrUtilities.h"
#include "stdafx.h"

const uint32 magicXmlNumber = 0x00001040;

CtrlrProcessor::CtrlrProcessor()
	:
#ifndef JucePlugin_PreferredChannelConfigurations
	  AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
						 // #if ! JucePlugin_IsSynth // Removed v5.6.32. Was disabling all required Inputs for Audio
						 // Channel Insert FX Mode
						 .withInput("Input", AudioChannelSet::stereo(), true)
						 // #endif
						 .withOutput("Output", AudioChannelSet::stereo(), true)
#endif
						 ),
#endif

	  overridesTree(Ids::ctrlrOverrides),
	  advertisedParameterCount(CTRLR_MAX_EXPORTED_VST_PARAMETERS)
	  //ctrlrManager(nullptr),
	  //ctrlrLog(nullptr) // Added v5.6.34. Could be useful
{
	_DBG("CtrlrProcessor::ctor");

	midiCollector.reset(SAMPLERATE);

	currentExec = File::getSpecialLocation(File::currentApplicationFile);
	overridesFile = currentExec.withFileExtension("overrides");

	overridesTree.setProperty(Ids::ctrlrMaxExportedVstParameters, CTRLR_MAX_EXPORTED_VST_PARAMETERS, 0);
	overridesTree.setProperty(Ids::ctrlrShutdownDelay, 512, 0);

	if (overridesFile.existsAsFile()) {
		std::unique_ptr<XmlElement> xml(XmlDocument(overridesFile).getDocumentElement().release());
		if (xml) {
			overridesTree = ValueTree::fromXml(*xml);
		}
	}

	// The host snapshots this count when it initialises the plugin. Keep it frozen
	// for the lifetime of the processor; a panel loaded later must not resize the
	// host's parameter list. The default is the public maximum, not the old 64.
	const int configuredParameterCount =
		(int) overridesTree.getProperty(Ids::ctrlrMaxExportedVstParameters,
									   CTRLR_MAX_EXPORTED_VST_PARAMETERS);
	advertisedParameterCount = jmax(CTRLR_MAX_EXPORTED_VST_PARAMETERS, configuredParameterCount);

#if JUCE_VERSION >= 0x080000
    const int numParamSlots = jmax(CTRLR_MAX_PARAMETER_SLOTS, advertisedParameterCount);
    for (int i = 0; i < numParamSlots; i++)
        addParameter (new CtrlrParameter (*this, i));
#endif

#if JUCE_DEBUG // Added v5.6.34. Will show the debug log. Was set to (false) by default from the CtrlrManager property
			   // ctrlrLogToFile.
			   // If we are in a Debug build, force logging ON
	ctrlrLog = std::make_unique<CtrlrLog>(true);
#else
	// If we are in any other build (like Release), force logging OFF
	ctrlrLog = std::make_unique<CtrlrLog>(overridesTree.getProperty(Ids::ctrlrLogToFile));
#endif
	ctrlrManager = std::make_unique<CtrlrManager>(this, *ctrlrLog);
	if (ctrlrManager == nullptr) {
		return;
	}
	if (!ctrlrManager->initEmbeddedInstance()) {
		ctrlrManager->setDefaults();
	}
	ctrlrManager->setManagerReady();

	if (ctrlrManager->getInstanceMode() == InstanceSingle ||
		ctrlrManager->getInstanceMode() == InstanceSingleRestricted) {
		if (ctrlrManager->getActivePanel()) {
			ctrlrManager->getActivePanel()->initEmbeddedInstance();
		}
	}

	// An exported instance may already contain its panel at this point, before the
	// host asks for the parameter count. Honour that panel's exported range, up to
	// the registered slot pool; a panel loaded later must still not change it.
	if (ctrlrManager->isSingleInstance() && ctrlrManager->getActivePanel() != nullptr) {
		advertisedParameterCount = jmin(CTRLR_MAX_PARAMETER_SLOTS,
									   jmax(advertisedParameterCount,
											ctrlrManager->getNumModulators(true)));
	}
}

CtrlrProcessor::~CtrlrProcessor() // Updated v5.6.34. Prevents AAX from crashing when deleting the plugin from the
								  // instrument track insert slot.
{
DBG("(Y) CtrlrProcessor DTOR");
// ***** CRITICAL AAX-SPECIFIC WORKAROUND *****
// This entire block is excluded for AAX builds because:
// 1. The MessageManager::runDispatchLoopUntil() call causes crashes on AAX plugin removal.
//    (Confirmed even when property exists and has a default value).
// 2. Explicitly deleting ctrlrLog (if it were done here) caused crashes on AAX plugin startup scan.
// The safest approach for AAX is to do minimal work for these components in the destructor.
#if JucePlugin_Build_AAX
	// For JUCE_AAX builds:
	// We ensure raw pointers are nullified without deletion to avoid crashes during scan/removal.
	// ScopedPointer will still automatically delete its content (ctrlrManager) when the CtrlrProcessor object is
	// destroyed.
	// ctrlrLog = nullptr;
#else
	// For all plugin formats *EXCEPT* AAX:
	// Perform explicit deletion for the raw pointer (ctrlrLog).
	// ScopedPointer (ctrlrManager) will handle its own deletion automatically.
	// if (ctrlrLog != nullptr) {
	// 	delete ctrlrLog;
	// 	ctrlrLog = nullptr;
	// }

#ifdef JUCE_MAC
	// This line was causing crashes on AAX removal, so it's excluded for AAX by the #ifndef above.
	// It remains here for other macOS builds if it's considered necessary for them. Mainly for panels with timer
	// process handled in the background and delayed saveState() process.
	MessageManager::getInstance()->runDispatchLoopUntil((int)overridesTree.getProperty(Ids::ctrlrShutdownDelay));
#endif
#endif
}

// NAMING FROM getName() WORKS ONLY FOR WIN VST2.
// macOS VST2 ARE NAMED AFTER CFBundleName string in /Contents/Info.plist

const String CtrlrProcessor::getName() const {
	if (ctrlrManager) {
		return (ctrlrManager->getInstanceNameForHost());
	} else {
		return ("CtrlrX"); // Updated v5.5.34
	}
}

void CtrlrProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
	preparedSampleRate = jmax(1.0, sampleRate);
	midiCollector.reset(sampleRate);
	lastHostBlockTimeMs = Time::getMillisecondCounterHiRes();
	hasHostBlockTime = false;
	leftoverBuffer.ensureSize(8192);
}

void CtrlrProcessor::releaseResources() {}

void CtrlrProcessor::processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiMessages)
{
	// 1. Clear audio buffer
	buffer.clear();

	// 2. Get playhead info (optional)
	AudioPlayHead::CurrentPositionInfo info;
	if (getPlayHead())
		getPlayHead()->getCurrentPosition(info);

	// 3. Host input already carries a block-relative sample position. It must not
	// be passed through MidiMessageCollector, which expects an absolute time in
	// seconds. Keep a copy for the host output and adjust it for the real time
	// elapsed since the previous block.
	const double nowMs = Time::getMillisecondCounterHiRes();
	const double sampleRate = preparedSampleRate;
	const double blockTimeMs = 1000.0 * (double) buffer.getNumSamples() / sampleRate;
	int hostSampleShift = 0;

	if (hasHostBlockTime)
	{
		const double elapsedMs = nowMs - lastHostBlockTimeMs;
		hostSampleShift = roundToInt((blockTimeMs - elapsedMs) * sampleRate / 1000.0);

		// A late block has already missed its nominal boundary. Applying the
		// entire negative overshoot would pull every event a full block early;
		// this controller deliberately damps only late corrections to half.
		// Positive corrections (early blocks) are applied in full. This is not
		// compatibility with the old broken sample-index path.
		if (hostSampleShift < 0)
			hostSampleShift /= 2;

		hostSampleShift = jlimit(-buffer.getNumSamples(), buffer.getNumSamples(), hostSampleShift);
	}

	lastHostBlockTimeMs = nowMs;
	hasHostBlockTime = true;

	juce::MidiBuffer hostInputCopy;
	bool hostInputRoutingEnabled = true;

	if (ctrlrManager != nullptr && ctrlrManager->getNumPanels() > 0)
	{
		hostInputRoutingEnabled = false;

		for (int i = 0; i < ctrlrManager->getNumPanels(); i++)
		{
			if (auto* panel = ctrlrManager->getPanel(i))
			{
				if (!panel->isMidiInPaused() && panel->getMidiOptionBool(panelMidiThruH2H))
				{
					hostInputRoutingEnabled = true;
					break;
				}
			}
		}
	}

	const bool hostInputAllowed = midiMessages.getNumEvents() > 0 && hostInputRoutingEnabled;

#if JUCE_LINUX
	// Preserve dev8's ALSA timestamp isolation. The old path copied the host
	// input before addMidiToOutputQueue(); this fix bypasses that mutating
	// queue, so take the pristine copy here before processPanels instead.
	juce::MidiBuffer linuxTimestampIsolationCopy = midiMessages;
	if (hostInputAllowed)
		hostInputCopy = linuxTimestampIsolationCopy;
#else
	if (hostInputAllowed)
		hostInputCopy = midiMessages;
#endif

	// 4. Call processPanels with the raw host input.
	if (midiMessages.getNumEvents() > 0)
		processPanels(midiMessages, info);

	// 5. Clear the incoming midiMessages buffer for output.
	midiMessages.clear();

	// 6. Get panel/device-generated MIDI from the collector.
	midiCollector.removeNextBlockOfMessages(midiMessages, buffer.getNumSamples());

	// 7. Host input is a direct pass-through, retaining its original timestamps
	// with the elapsed-block correction applied.
	if (hostInputAllowed)
	{
		for (const auto metadata : hostInputCopy)
		{
			const int position = jlimit(0, jmax(0, buffer.getNumSamples() - 1),
										metadata.samplePosition + hostSampleShift);
			midiMessages.addEvent(metadata.getMessage(), position);
		}
	}
}

//==============================================================================

// These methods are all deprecated in favour of using AudioProcessorParameter
// and AudioProcessorParameterGroup
//
// JUCE_DEPRECATED (virtual int getNumParameters());
// JUCE_DEPRECATED (virtual const String getParameterName (int parameterIndex));
// JUCE_DEPRECATED (virtual String getParameterID (int index));
// JUCE_DEPRECATED (virtual float getParameter (int parameterIndex));
// JUCE_DEPRECATED (virtual String getParameterName (int parameterIndex, int maximumStringLength));
// JUCE_DEPRECATED (virtual const String getParameterText (int parameterIndex));
// JUCE_DEPRECATED (virtual String getParameterText (int parameterIndex, int maximumStringLength));

int CtrlrProcessor::getNumParameters()
{
	// JUCE snapshots this at plugin initialisation. It must stay identical when a
	// panel is loaded or removed, so the wrapper never indexes a parameter map
	// that is larger or smaller than the list the host was given.
	return advertisedParameterCount;
}

String CtrlrProcessor::getParameterID(int index) // Added v5.6.33. Will pass vstIndex as VST3 ID
{
	// File debugLog = File::getSpecialLocation(File::currentApplicationFile);
	// PluginLoggerVst3 logger(debugLog); // Create logger instance

	// logger.log("getParameterID() called with index: " + String(index));

	// Check if we are currently running as an AudioUnit
	if (wrapperType == AudioProcessor::wrapperType_AudioUnit) {
		// For AU: Return the index as a string to force sequential IDs
		// This stops Logic from "shuffling" your parameters based on hashes

		// Adding 1 ensures we stay away from the '0' ID which can be reserved
		return String(index + 1);
	}

	// For VST3 / Others: Keep your descriptive IDs
	if (CtrlrModulator *m = ctrlrManager->getModulatorByVstIndex(index)) {
		int vstIndex = m->getVstIndex();
		// logger.log("  Found modulator. vstIndex: " + String(vstIndex));
		String idPrefix = "CtrlrMod_";
		String vstIndexString = String(vstIndex);
		String fullId = idPrefix + vstIndexString;
		// logger.log("  ID Prefix: " + idPrefix);
		// logger.log("  vstIndex as String: " + vstIndexString);
		// logger.log("  Constructed ID: " + fullId);
		return fullId;
	} else {
		String fallbackId = "undefined_ID_" + String(index);
		// logger.log("  No modulator found for index: " + String(index) + ". Returning fallback ID: " + fallbackId);
		return fallbackId;
	}
}

const String CtrlrProcessor::getParameterName(int index) {
	if (ctrlrManager->getModulatorByVstIndex(index)) {
		return (ctrlrManager->getModulatorByVstIndex(index)->getNameForHost());
	} else {
		// return ("undefined_"+String(index)); // Updated v5.6.35. SEE:
		// https://github.com/damiensellier/CtrlrX/issues/228#issuecomment-4030969898
		return " "; // For the Mackie Controllers with LCD, a space looks better than "undefined_XX"
	}
}

float CtrlrProcessor::getParameter(int index) // Updated v5.6.33
{
	CtrlrModulator *m = ctrlrManager->getModulatorByVstIndex(index);
	if (m) {
		return (m->getProcessor().getValueForHost());
	} else {
		return 0.0f;
	}
}

void CtrlrProcessor::setParameter(int index, float newValue) {
	CtrlrModulator *m = ctrlrManager->getModulatorByVstIndex(index);
	if (m == nullptr)
		return;

	m->getProcessor().setValueFromHost(newValue);
}

const String CtrlrProcessor::getParameterText(int index) {
	if (ctrlrManager->getModulatorByVstIndex(index)) {
		const String text = ctrlrManager->getModulatorByVstIndex(index)->getTextForHost();
		return (text);
	} else {
		return ("value_" + String(index));
	}
}

//==============================================================================

double CtrlrProcessor::getTailLengthSeconds() const { return 0.0; }

//==============================================================================

bool CtrlrProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool CtrlrProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool CtrlrProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

int CtrlrProcessor::getNumPrograms() { return 1; }

int CtrlrProcessor::getCurrentProgram() { return 0; }

void CtrlrProcessor::setCurrentProgram(int index) { _DBG("CtrlrProcessor::setCurrentProgram id:" + _STR(index)); }

const String CtrlrProcessor::getProgramName(int index) {
	_DBG("CtrlrProcessor::getProgramName id:" + _STR(index));
	return ("Default CTRLR program");
}

void CtrlrProcessor::changeProgramName(int index, const String &newName) {
	_DBG("CtrlrProcessor::changeProgramName id:" + _STR(index) + " newName:" + newName);
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool CtrlrProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const // Updated v5.6.31
{
#if JucePlugin_IsMidiEffect // Ableton Live VST3 requires Midi Effect disabled in ProJucer
	ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono() &&
		layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

//==============================================================================

// Place above the addMidiToOutputQueue functions in CtrlrProcessor.cpp
static bool fitsInMidiBuffer(const MidiMessage &m) {
	// juce::MidiBuffer stores each event's size as a uint16, so larger messages get corrupted
	if (m.getRawDataSize() > 65535) {
		Logger::writeToLog("CtrlrProcessor: dropping " + String(m.getRawDataSize()) +
						   "-byte message for host output (exceeds MidiBuffer event limit)");
		return false;
	}
	return true;
}

void CtrlrProcessor::addMidiToOutputQueue(CtrlrMidiMessage &m) {
	for (int i = 0; i < m.getNumMessages(); i++) {
		if (!fitsInMidiBuffer(m.getReference(i).m))
			continue;

		m.getReference(i).m.setTimeStamp(i + 1);
		midiCollector.addMessageToQueue(m.getReference(i).m);
	}
}

void CtrlrProcessor::addMidiToOutputQueue(const CtrlrMidiMessage &m) {
	for (int i = 0; i < m.getNumMessages(); i++) {
		if (!fitsInMidiBuffer(m.getReference(i).m))
			continue;

		m.getReference(i).m.setTimeStamp(i + 1);
		midiCollector.addMessageToQueue(m.getReference(i).m);
	}
}

void CtrlrProcessor::addMidiToOutputQueue(const MidiMessage &m) {
	if (!fitsInMidiBuffer(m))
		return;

	midiCollector.addMessageToQueue(m);
}

void CtrlrProcessor::addMidiToOutputQueue(const MidiBuffer &buffer) // Updated v5.6.34
{
	MidiBuffer::Iterator i(buffer);
	MidiMessage m;
	int time;

	while (i.getNextEvent(m, time)) {
		// WORKAROUND FOR JASSERT IN JUCE'S INTERNAL MidiMessageCollector::addMessageToQueue
		// If the message is at sample position 0, we temporarily shift it to 1
		// to bypass the strict jassert.
		if (time == 0)
			time = 1;

		// Use the only available method, applying the corrected 'time'
		// JUCE's addMessageToQueue *expects* the message's timestamp to be set.
		m.setTimeStamp(time); // Set the timestamp of the MidiMessage itself
		midiCollector.addMessageToQueue(m);
	}
}

//==============================================================================

void CtrlrProcessor::processPanels(MidiBuffer &midiMessages, const AudioPlayHead::CurrentPositionInfo &positionInfo) {
	leftoverBuffer.clear();

	for (int i = 0; i < panelProcessors.size(); i++) {
		if (!panelProcessors[i].wasObjectDeleted()) {
			panelProcessors[i]->processBlock(midiMessages, leftoverBuffer, positionInfo);
		}
	}
	midiMessages.swapWith(leftoverBuffer);
}

//==============================================================================

bool CtrlrProcessor::hasEditor() const { return true; }

//==============================================================================

const var &CtrlrProcessor::getProperty(const Identifier &name) const {
	if (ctrlrManager) {
		if (ctrlrManager->getManagerTree().hasProperty(name))
			return (ctrlrManager->getProperty(name));
		else
			return (overridesTree.getProperty(name));
	} else {
		return (overridesTree.getProperty(name));
	}
}

bool CtrlrProcessor::hasProperty(const Identifier &name) const {
	if (ctrlrManager->getManagerTree().hasProperty(name) || overridesTree.hasProperty(name))
		return (true);
	else
		return (false);
}

void CtrlrProcessor::activePanelChanged() { sendChangeMessage(); }

bool CtrlrProcessor::useWrapper() // Updated v5.6.34. JUCE 6.0.8 has option "plugin requires keyboard focus" and the
								  // keyboard focus is handled better with VST3 than VST2 so Live Wrapper seems useless
								  // nowadays.
{
	File debugLog = File::getSpecialLocation(File::currentApplicationFile);
	String fileExt = debugLog.getFileExtension();
	// PluginLoggerVst3 logger(debugLog); // Create logger instance
	// logger.log("CtrlrX source fileExtension is :" + fileExt);

	if (JUCEApplication::isStandaloneApp()) {
		return (false);
	}

	// Logic for Windows + Ableton Live
	if (((SystemStats::getOperatingSystemType() & SystemStats::Windows) != 0) && host.isAbletonLive()) {
		// logger.log("useWrapper(): Running on Windows in Ableton Live.");

		if (fileExt == ".vst3") // Check if the currently loaded plugin format is VST3
		{
			// logger.log("useWrapper(): Detected VST3 on Windows Live. Forcing native editor (no wrapper) for
			// testing.");
			return (false); // Force native VST3 editor on Windows for testing
		}

		else if (fileExt == ".dll") // Check if the currently loaded plugin format is VST (VST2)
		{
			// If it's a VST2 plugin, the original developer's logic suggests using the wrapper. We'll keep this
			// behavior for VST2. logger.log("useWrapper(): Detected VST2 on Windows Live. Using wrapper as per original
			// developer's notes.");

			// if (hasProperty(Ids::ctrlrUseEditorWrapper))
			// {
			//     return ((bool)getProperty(Ids::ctrlrUseEditorWrapper));
			// }
			// else
			// {
			//     return (true); // Default to true for VST2 on Windows Live
			// }

			return (true); // Hard coded default to true for VST2 on Windows Live. Condition based on property
						   // ctrlrUseEditorWrapper bypassed.
		}
	}
	// Keep the macOS logic as it was, the wrapper is useless on macOS.
	else if (((SystemStats::getOperatingSystemType() & SystemStats::MacOSX) != 0) && host.isAbletonLive()) {
		return (false); // Default to false for macOS
	}

	// Default to false for all other cases (other DAWs, other OSes, or unhandled plugin types)
	// logger.log("useWrapper(): Not Ableton Live on Windows, or unhandled plugin type. Returning false (no wrapper).");
	return (false);
}

AudioProcessorEditor *CtrlrProcessor::createEditor() {
	if (useWrapper()) {
		return new CtrlrProcessorEditorForLive(this, *ctrlrManager);
	} else {
		return new CtrlrEditor(this, *ctrlrManager);
	}
}

//==============================================================================

void CtrlrProcessor::getStateInformation(MemoryBlock &destData) {
	// Skip state generation if manager is restoring or shutting down
	if (ctrlrManager == nullptr || ctrlrManager->isRestoring() || ctrlrManager->isShuttingDown())
		return;

	// Optional: Ensure document panel isn't in a middle state
	if (&ctrlrManager->getCtrlrDocumentPanel() != nullptr) {
		if (ctrlrManager->getCtrlrDocumentPanel().getNumDocuments() == 0) {
			// Handle state saving safely when no tabs are open
		}
	}
	_DBG("CtrlrProcessor::getStateInformation");
	std::unique_ptr<XmlElement> xmlState(ctrlrManager->saveState());
	if (xmlState) {
		CtrlrProcessor::copyXmlToBinary(*xmlState, destData);
	}
}

void CtrlrProcessor::setStateInformation(const void *data, int sizeInBytes) {
	_DBG("CtrlrProcessor::setStateInformation");
	std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
	if (xmlState) {
		setStateInformation(xmlState.get());
		return;
	}
}

void CtrlrProcessor::setStateInformation(const XmlElement *xmlState) {
	if (KeyPress::isKeyCurrentlyDown(KeyPress::createFromDescription("ctrl + R").getKeyCode())) {
		// Modern JUCE 7/8 Asynchronous (Non-blocking) call
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Ctrlr v5",
										 "Ctrl+R key is pressed, resetting to defaults");
		return;
	}

	if (xmlState) {
		_DBG("CtrlrProcessor::setStateInformation - xmlState is valid. About to call ctrlrManager->restoreState.");

		// This will call your logMessage, which internally checks 'fileLogger'.
		if (CtrlrLog::ctrlrLog != nullptr) {
			CtrlrLog::ctrlrLog->logMessage("--- TEST: Direct Log from setStateInformation ---", CtrlrLog::Debug);
		}
		// The AAX Plugin hanging problem when initiating is subsequent to this line
		ctrlrManager->restoreState(*xmlState);

		_DBG("CtrlrProcessor::setStateInformation - ctrlrManager->restoreState returned. Check for further execution.");
	}
}

//==============================================================================

void CtrlrProcessor::setMidiOptions(const bool _thruHostToHost, const bool _thruHostToDevice, const bool _outputToHost,
									const bool _inputFromHost, const bool _thruFromHostChannelize) {
	const ScopedLock sl(getCallbackLock());

	thruHostToHost = _thruHostToHost;
	thruHostToDevice = _thruHostToDevice;
	thruFromHostChannelize = _thruFromHostChannelize;
	outputToHost = _outputToHost;
	inputFromHost = _inputFromHost;
}

void CtrlrProcessor::addPanelProcessor(CtrlrPanelProcessor *processorToAdd) {
	panelProcessors.addIfNotAlreadyThere(processorToAdd);
}

void CtrlrProcessor::removePanelProcessor(CtrlrPanelProcessor *processorToRemove) {
	panelProcessors.removeAllInstancesOf(processorToRemove);
}

void CtrlrProcessor::copyXmlToBinary(const XmlElement &xml, juce::MemoryBlock &destData) {
	const String xmlString(xml.createDocument("", true, false));
	const int stringLength = (int)xmlString.getNumBytesAsUTF8();

	destData.setSize((size_t)stringLength + 10);

	char *const d = static_cast<char *>(destData.getData());
	*(uint32 *)d = ByteOrder::swapIfBigEndian((const uint32)magicXmlNumber);
	*(uint32 *)(d + 4) = ByteOrder::swapIfBigEndian((const uint32)stringLength);

	xmlString.copyToUTF8(d + 8, stringLength + 1);
}

XmlElement *CtrlrProcessor::getXmlFromBinary(const void *data, const int sizeInBytes) {
	if (sizeInBytes > 8 && ByteOrder::littleEndianInt(data) == magicXmlNumber) {
		const int stringLength = (int)ByteOrder::littleEndianInt(addBytesToPointer(data, 4));

		if (stringLength > 0)
			return XmlDocument::parse(
					   String::fromUTF8(static_cast<const char *>(data) + 8, jmin((sizeInBytes - 8), stringLength)))
				.release();
	}

	return nullptr;
}

void CtrlrProcessor::openFileFromCli(const File &file) {
	if (ctrlrManager) {
		ctrlrManager->openPanelInternal(file);
	}
}

//==============================================================================
// This creates new instances of the plugin.

AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
	_DBG("createPluginFilter");
	return new CtrlrProcessor();
}

CtrlrParameter::CtrlrParameter (CtrlrProcessor& p, int idx)
    : owner(p), paramIndex(idx) {}

float CtrlrParameter::getValue() const
{
    if (owner.ctrlrManager)
        if (auto* m = owner.ctrlrManager->getModulatorByVstIndex (paramIndex))
            return m->getProcessor().getValueForHost();
    return 0.0f;
}

void CtrlrParameter::setValue (float newValue)
{
    if (owner.ctrlrManager)
        if (auto* m = owner.ctrlrManager->getModulatorByVstIndex (paramIndex))
            m->getProcessor().setValueFromHost (newValue);
}

float  CtrlrParameter::getDefaultValue()                 const { return 0.0f; }
String CtrlrParameter::getLabel()                        const { return {}; }

String CtrlrParameter::getName (int maxLen) const
{
    if (owner.ctrlrManager)
        if (auto* m = owner.ctrlrManager->getModulatorByVstIndex (paramIndex))
            return m->getNameForHost().substring (0, maxLen);
    return " ";
}

float CtrlrParameter::getValueForText (const String& t) const
{
    return t.getFloatValue();
}
