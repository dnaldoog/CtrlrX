#include "CtrlrLuaPanelScriptWatcher.h"

CtrlrLuaPanelScriptWatcher::CtrlrLuaPanelScriptWatcher(const juce::File& scriptsDirectory, std::function<void(const juce::File&)> onScriptChanged)
    : scriptDir(scriptsDirectory), callback(onScriptChanged)
{
    updateTimestamps();
    startTimer(1000); // Poll every 1 second
}

CtrlrLuaPanelScriptWatcher::~CtrlrLuaPanelScriptWatcher()
{
    stopTimer();
}

void CtrlrLuaPanelScriptWatcher::timerCallback()
{
    if (!scriptDir.isDirectory()) 
        return;

    juce::Array<juce::File> luaFiles = scriptDir.findChildFiles(juce::File::findFiles, true, "*.lua");

    for (const auto& file : luaFiles)
    {
        int64 currentModTime = file.getLastModificationTime().toMilliseconds();
        
        auto it = lastModTimes.find(file);
        if (it != lastModTimes.end())
        {
            if (it->second != currentModTime)
            {
                it->second = currentModTime; // Updates correctly
                
                if (callback)
                {
                    callback(file); // Trigger the hot-reload callback
                }
            }
        }
        else
        {
            // Track newly added scripts on disk
            lastModTimes[file] = currentModTime; // Works natively now!
        }
    }
}

void CtrlrLuaPanelScriptWatcher::setScriptDirectory(const juce::File& newDir)
{
    scriptDir = newDir;
    updateTimestamps();
}

void CtrlrLuaPanelScriptWatcher::updateTimestamps()
{
    lastModTimes.clear();
    if (!scriptDir.isDirectory()) 
        return;

    juce::Array<juce::File> luaFiles = scriptDir.findChildFiles(juce::File::findFiles, true, "*.lua");
    for (const auto& file : luaFiles)
    {
        lastModTimes[file] = file.getLastModificationTime().toMilliseconds();
    }
}