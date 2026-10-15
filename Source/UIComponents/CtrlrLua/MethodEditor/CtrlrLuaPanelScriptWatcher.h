#pragma once
#include <JuceHeader.h>
#include <map> // Add standard map

class CtrlrLuaPanelScriptWatcher : public juce::Timer
{
public:
    CtrlrLuaPanelScriptWatcher(const juce::File& scriptsDirectory, std::function<void(const juce::File&)> onScriptChanged);
    ~CtrlrLuaPanelScriptWatcher() override;

    void timerCallback() override;
    void setScriptDirectory(const juce::File& newDir);
	void setExternalScriptSyncEnabled(bool enabled);

private:
    void updateTimestamps();

    juce::File scriptDir;
    std::function<void(const juce::File&)> callback;
    std::map<juce::File, int64> lastModTimes; // Changed to std::map
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaPanelScriptWatcher)
};