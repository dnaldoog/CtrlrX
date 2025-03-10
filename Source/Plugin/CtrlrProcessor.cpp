#include "stdafx.h"
#include "CtrlrUtilities.h"
#include "CtrlrProcessor.h"
#include "CtrlrApplicationWindow/CtrlrEditor.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrProcessorEditorForLive.h"
#include "CtrlrLog.h"
#include "CtrlrPanel/CtrlrPanelMIDIInputThread.h"
#include "CtrlrPanel/CtrlrPanel.h"

const uint32 magicXmlNumber = 0x00001040;


CtrlrProcessor::CtrlrProcessor() :
                                    #ifndef JucePlugin_PreferredChannelConfigurations
                                    AudioProcessor (BusesProperties()
                                        #if ! JucePlugin_IsMidiEffect
                                        //#if ! JucePlugin_IsSynth // Removed v5.6.32. Was disabling all Inputs
                                                    .withInput  ("Input",  AudioChannelSet::stereo(), true)
                                        //#endif
                                                    .withOutput ("Output", AudioChannelSet::stereo(), true)
                                        #endif
                                                    ),
                                        #endif

                                    overridesTree (Ids::ctrlrOverrides),
                                    ctrlrManager (nullptr)
{
	_DBG("CtrlrProcessor::ctor");

	midiCollector.reset (SAMPLERATE);
    
	currentExec		= File::getSpecialLocation(File::currentApplicationFile);
	overridesFile	= currentExec.withFileExtension("overrides");

	overridesTree.setProperty (Ids::ctrlrMaxExportedVstParameters, 64, 0);
	overridesTree.setProperty (Ids::ctrlrShutdownDelay, 512, 0);

	if (overridesFile.existsAsFile())
	{
		ScopedPointer <XmlElement> xml (XmlDocument(overridesFile).getDocumentElement().release());
		if (xml)
		{
			overridesTree = ValueTree::fromXml (*xml);
		}
	}

	ctrlrLog				= new CtrlrLog(overridesTree.getProperty (Ids::ctrlrLogToFile));
	ctrlrManager			= new CtrlrManager(this, *ctrlrLog);

	if (!ctrlrManager->initEmbeddedInstance())
	{
		ctrlrManager->setDefaults();
	}

	ctrlrManager->setManagerReady();

	if (ctrlrManager->getInstanceMode() == InstanceSingle || ctrlrManager->getInstanceMode() == InstanceSingleRestriced)
	{
		if (ctrlrManager->getActivePanel())
		{
			ctrlrManager->getActivePanel()->initEmbeddedInstance();
		}
	}
}

CtrlrProcessor::~CtrlrProcessor()
{
#ifdef JUCE_MAC // Updated v5.5.31. was JUCE_OSX
	MessageManager::getInstance()->runDispatchLoopUntil((int)overridesTree.getProperty(Ids::ctrlrShutdownDelay)); // Updated v5.6.31. Not sure if it's useful anyway
#endif
}

//==============================================================================
// NAMING FROM getName() WORKS ONLY FOR WIN VST2.
// macOS VST2 ARE NAMED AFTER CFBundleName string in /Contents/Info.plist
//==============================================================================

const String CtrlrProcessor::getName() const
{
	if (ctrlrManager)
    {
        return (ctrlrManager->getInstanceNameForHost());
    }
	else
    {
		return ("Ctrlr");
    }
}

//==============================================================================


void CtrlrProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    midiCollector.reset (sampleRate);
    leftoverBuffer.ensureSize (8192);
}

void CtrlrProcessor::releaseResources()
{
}

void CtrlrProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    AudioPlayHead::CurrentPositionInfo info;
    if (getPlayHead())
    {
        getPlayHead()->getCurrentPosition(info);
    }

    if (midiMessages.getNumEvents() > 0)
    {
        processPanels(midiMessages, info);
    }
    

    midiCollector.removeNextBlockOfMessages (midiMessages, (buffer.getNumSamples() > 0) ? buffer.getNumSamples() : 1);
    MidiBuffer::Iterator i(midiMessages);
    while (i.getNextEvent(logResult, logSamplePos))
    _MOUT("VST OUTPUT", logResult, logSamplePos);
    
    
    // Added v5.6.32. Useless
    // Loop the audio from the inputs to the corroesponding outputs
//    juce::ScopedNoDenormals noDenormals;
//    auto totalNumInputChannels  = getTotalNumInputChannels();
//    auto totalNumOutputChannels = getTotalNumOutputChannels();
//
//    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
//    buffer.clear (i, 0, buffer.getNumSamples());
//
//    double rawVolume = 1.0; // vol gain in = vol gain out 1/1
//    for (int channel = 0; channel < totalNumInputChannels; ++channel)
//    {
//        auto* channelData = buffer.getWritePointer (channel);
//
//        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
//        {
//            channelData[sample] = buffer.getSample(channel, sample) * rawVolume;
//        }
//    }
}

//==============================================================================
//
// These methods are all deprecated in favour of using AudioProcessorParameter
// and AudioProcessorParameterGroup
//
//JUCE_DEPRECATED (virtual int getNumParameters());
//JUCE_DEPRECATED (virtual const String getParameterName (int parameterIndex));
//JUCE_DEPRECATED (virtual String getParameterID (int index));
//JUCE_DEPRECATED (virtual float getParameter (int parameterIndex));
//JUCE_DEPRECATED (virtual String getParameterName (int parameterIndex, int maximumStringLength));
//JUCE_DEPRECATED (virtual const String getParameterText (int parameterIndex));
//JUCE_DEPRECATED (virtual String getParameterText (int parameterIndex, int maximumStringLength));
//

int CtrlrProcessor::getNumParameters() // Updated 5.6.31. VST Host was assigned (64) params most of the time since panels hardly have more than 64 params passed as VST controls
{
    if (ctrlrManager)
        if (ctrlrManager->isSingleInstance()) // Added v5.6.31
        {
            return (ctrlrManager->getNumModulators(true)); // Added v5.6.31. Will pass the highest vstIndex value as the total number of VST params to the host.
        }
        else
        {
            return (jmax(ctrlrManager->getNumModulators(true), (int)overridesTree.getProperty (Ids::ctrlrMaxExportedVstParameters))); // Pass jmax ctrlrMaxExportedVstParameters value or default (64) params to the host when designing or loading a panel in ctrlr.vst or ctrlr.vst3
        }
    else
        return (CTRLR_DEFAULT_PARAMETER_COUNT);
}

float CtrlrProcessor::getParameter (int index)
{
    CtrlrModulator *m = ctrlrManager->getModulatorByVstIndex (index);
    if (m)
    {
        return (m->getProcessor().getValueForHost());
    }
    else
    {
        return 0.0f;
    }
}

void CtrlrProcessor::setParameter (int index, float newValue)
{
    CtrlrModulator *m = ctrlrManager->getModulatorByVstIndex (index);
    if (m == nullptr)
    {
        return;
    }
    else if (newValue > 1.0f || newValue < 0.000000f)
    {
        return;
    }
    else
    {
        return (m->getProcessor().setValueFromHost(newValue));
    }
}

const String CtrlrProcessor::getParameterName (int index)
{
    if (ctrlrManager->getModulatorByVstIndex (index))
    {
        return (ctrlrManager->getModulatorByVstIndex (index)->getNameForHost());
    }
    else
    {
        return ("undefined_"+String(index));
    }
}

const String CtrlrProcessor::getParameterText (int index)
{
    if (ctrlrManager->getModulatorByVstIndex (index))
    {
        const String text = ctrlrManager->getModulatorByVstIndex (index)->getTextForHost();
        return (text);
    }
    else
    {
        return ("value_"+String(index));
    }
}

//==============================================================================
//
// DEPRECATED IN JUCE 5
//
// These functions are deprecated: your audio processor can inform the host
// on its bus and channel layouts and names using the AudioChannelSet and various bus classes.
//const String CtrlrProcessor::getInputChannelName (const int channelIndex) const
//
//{
//#if JucePlugin_Build_VST
//    if (channelIndex >= 1024)
//    {
//        /* used internally */
//        switch (channelIndex)
//        {
//            case VST_INDEX_UNIQUEID:
//                if (ctrlrManager)
//                {
//                    if (ctrlrManager->getActivePanel())
//                    {
//                        return (ctrlrManager->getActivePanel()->getPanelInstanceID());
//                    }
//                }
//                return ("CTRL");
//
//            case VST_INDEX_MANUFACTURER:
//                if (ctrlrManager)
//                {
//                    if (ctrlrManager->getActivePanel())
//                    {
//                        return (ctrlrManager->getActivePanel()->getPanelInstanceManufacturer());
//                    }
//                }
//                return ("Instigator");
//
//            case VST_INDEX_NAME:
//                if (ctrlrManager)
//                {
//                    if (ctrlrManager->getActivePanel())
//                    {
//                        return (ctrlrManager->getActivePanel()->getPanelInstanceName());
//                    }
//                }
//                return ("Ctrlr");
//
//            case VST_INDEX_VERSION_STRING:
//                if (ctrlrManager)
//                {
//                    if (ctrlrManager->getActivePanel())
//                    {
//                        return (ctrlrManager->getActivePanel()->getPanelInstanceVersionString());
//                    }
//                }
//                return (ProjectInfo::versionString);
//
//            case VST_INDEX_VERSION_CODE:
//                if (ctrlrManager)
//                {
//                    if (ctrlrManager->getActivePanel())
//                    {
//                        return (String(ctrlrManager->getActivePanel()->getPanelInstanceVersionInt()));
//                    }
//                }
//                return (String(ProjectInfo::versionNumber));
//
//            default:
//                break;
//        }
//    }
//#endif
//    return ("Audio Input: "+_STR(channelIndex));
//}
//
//const String CtrlrProcessor::getOutputChannelName (const int channelIndex) const
//{
//    return ("Audio Output: "+_STR(channelIndex));
//}


//==============================================================================
//
// DEPRECATED IN JUCE 5
//
// These functions are deprecated: your audio processor can inform the host
// on its bus and channel layouts and names using the AudioChannelSet and various bus classes.
//  JUCE_DEPRECATED (virtual bool isInputChannelStereoPair  (int index) const);
//  JUCE_DEPRECATED (virtual bool isOutputChannelStereoPair (int index) const);
//
//#if JucePlugin_Build_VST
//bool CtrlrProcessor::isInputChannelStereoPair (int index) const
//{
//    return true;
//}
//
//bool CtrlrProcessor::isOutputChannelStereoPair (int index) const
//{
//    return (true);
//}
//#endif

//==============================================================================

double CtrlrProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================

bool CtrlrProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CtrlrProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CtrlrProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}


int CtrlrProcessor::getNumPrograms()
{
    return 1;
}

int CtrlrProcessor::getCurrentProgram()
{
    return 0;
}

void CtrlrProcessor::setCurrentProgram (int index)
{
	_DBG("CtrlrProcessor::setCurrentProgram id:"+_STR(index));
}

const String CtrlrProcessor::getProgramName (int index)
{
	_DBG("CtrlrProcessor::getProgramName id:"+_STR(index));
    return ("Default CTRLR program");
}

void CtrlrProcessor::changeProgramName (int index, const String& newName)
{
	_DBG("CtrlrProcessor::changeProgramName id:"+_STR(index)+" newName:"+newName);
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool CtrlrProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const  // Updated v5.6.31
{
  #if JucePlugin_IsMidiEffect // Ableton Live VST3 requires Midi Effect disabled in ProJucer
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================

void CtrlrProcessor::addMidiToOutputQueue (CtrlrMidiMessage &m)
{

	for (int i=0; i<m.getNumMessages(); i++)
	{
		m.getReference(i).m.setTimeStamp(i+1);
		midiCollector.addMessageToQueue (m.getReference(i).m);
	}
}

void CtrlrProcessor::addMidiToOutputQueue (const CtrlrMidiMessage &m)
{
	for (int i=0; i<m.getNumMessages(); i++)
	{
		m.getReference(i).m.setTimeStamp(i+1);
		midiCollector.addMessageToQueue (m.getReference(i).m);
	}
}

void CtrlrProcessor::addMidiToOutputQueue (const MidiMessage &m)
{
	midiCollector.addMessageToQueue (m);
}

void CtrlrProcessor::addMidiToOutputQueue (const MidiBuffer &buffer)
{
	MidiBuffer::Iterator i(buffer);
	MidiMessage m;
	int time;

	while (i.getNextEvent (m, time))
	{
		midiCollector.addMessageToQueue (m);
	}
}

//==============================================================================

void CtrlrProcessor::processPanels(MidiBuffer &midiMessages, const AudioPlayHead::CurrentPositionInfo &positionInfo)
{
	leftoverBuffer.clear();

	for (int i=0; i<panelProcessors.size(); i++)
	{
		if (!panelProcessors[i].wasObjectDeleted())
		{
			panelProcessors[i]->processBlock (midiMessages, leftoverBuffer, positionInfo);
		}
	}
	midiMessages.swapWith(leftoverBuffer);
}

//==============================================================================

bool CtrlrProcessor::hasEditor() const
{
    return true;
}

//==============================================================================

const var &CtrlrProcessor::getProperty (const Identifier& name) const
{
	if (ctrlrManager)
	{
		if (ctrlrManager->getManagerTree().hasProperty(name))
			return (ctrlrManager->getProperty(name));
		else
			return (overridesTree.getProperty(name));
	}
	else
	{
		return (overridesTree.getProperty(name));
	}
}

bool CtrlrProcessor::hasProperty(const Identifier &name) const
{
	if (ctrlrManager->getManagerTree().hasProperty(name) || overridesTree.hasProperty(name))
		return (true);
	else
		return (false);
}

void CtrlrProcessor::activePanelChanged()
{
	sendChangeMessage();
}


bool CtrlrProcessor::useWrapper()
{
	if (JUCEApplication::isStandaloneApp())
	{
		return (false);
	}

	if (((SystemStats::getOperatingSystemType() & SystemStats::Windows) != 0) && host.isAbletonLive())
	{
		if (hasProperty(Ids::ctrlrUseEditorWrapper))
		{
			return ((bool)getProperty(Ids::ctrlrUseEditorWrapper));
		}
		else
		{
			return (true);
		}
	}
	return (false);
}

AudioProcessorEditor* CtrlrProcessor::createEditor()
{
    if (useWrapper())
    {
        return new CtrlrProcessorEditorForLive (this, *ctrlrManager);
    }
    else
    {
        return new CtrlrEditor (this, *ctrlrManager);
    }
}

//==============================================================================

void CtrlrProcessor::getStateInformation (MemoryBlock& destData)
{
	_DBG("CtrlrProcessor::getStateInformation");
	ScopedPointer <XmlElement> xmlState(ctrlrManager->saveState());
	if (xmlState)
	{
		CtrlrProcessor::copyXmlToBinary (*xmlState, destData);
	}
}

void CtrlrProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	_DBG("CtrlrProcessor::setStateInformation");
	ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
	if (xmlState)
	{
		setStateInformation (xmlState);
	}
}

void CtrlrProcessor::setStateInformation (const XmlElement *xmlState)
{
	if (KeyPress::isKeyCurrentlyDown(KeyPress::createFromDescription("ctrl + R").getKeyCode()))
	{
		AlertWindow::showMessageBox (AlertWindow::WarningIcon, "Ctrlr v5", "Ctrl+R key is pressed, resetting to defaults");
		return;
	}

	if (xmlState)
	{
		ctrlrManager->restoreState (*xmlState);
	}
}

//==============================================================================

void CtrlrProcessor::setMidiOptions(const bool _thruHostToHost, const bool _thruHostToDevice, const bool _outputToHost, const bool _inputFromHost, const bool _thruFromHostChannelize)
{
	const ScopedLock sl(getCallbackLock());

	thruHostToHost				= _thruHostToHost;
	thruHostToDevice			= _thruHostToDevice;
	thruFromHostChannelize		= _thruFromHostChannelize;
	outputToHost				= _outputToHost;
	inputFromHost				= _inputFromHost;
}

void CtrlrProcessor::addPanelProcessor (CtrlrPanelProcessor *processorToAdd)
{
	panelProcessors.addIfNotAlreadyThere (processorToAdd);
}

void CtrlrProcessor::removePanelProcessor (CtrlrPanelProcessor *processorToRemove)
{
	panelProcessors.removeAllInstancesOf (processorToRemove);
}

void CtrlrProcessor::copyXmlToBinary (const XmlElement& xml, juce::MemoryBlock& destData)
{
    const String xmlString (xml.createDocument ("", true, false));
    const int stringLength = (int)xmlString.getNumBytesAsUTF8();

    destData.setSize ((size_t) stringLength + 10);

    char* const d = static_cast<char*> (destData.getData());
    *(uint32*) d = ByteOrder::swapIfBigEndian ((const uint32) magicXmlNumber);
    *(uint32*) (d + 4) = ByteOrder::swapIfBigEndian ((const uint32) stringLength);

    xmlString.copyToUTF8 (d + 8, stringLength + 1);
}

XmlElement* CtrlrProcessor::getXmlFromBinary (const void* data, const int sizeInBytes)
{
    if (sizeInBytes > 8
         && ByteOrder::littleEndianInt (data) == magicXmlNumber)
    {
        const int stringLength = (int) ByteOrder::littleEndianInt (addBytesToPointer (data, 4));

        if (stringLength > 0)
            return XmlDocument::parse (String::fromUTF8 (static_cast<const char*> (data) + 8,
                                                         jmin ((sizeInBytes - 8), stringLength))).release();
    }

    return nullptr;
}

void CtrlrProcessor::openFileFromCli(const File &file)
{
	if (ctrlrManager)
	{
		ctrlrManager->openPanelInternal (file);
	}
}

//==============================================================================
// This creates new instances of the plugin.

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    _DBG("createPluginFilter");
    return new CtrlrProcessor();
}
