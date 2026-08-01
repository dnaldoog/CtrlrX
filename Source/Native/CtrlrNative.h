#ifndef CTRLR_NATIVE_H
#define CTRLR_NATIVE_H

#include <JuceHeader.h>

class CtrlrPanel;
class CtrlrManager;

#define CTRLR_NEW_INSTANCE_DIALOG_TITLE "Write new instance here"

#define CTRLR_INTERNAL_PANEL_RESID 1040
#define CTRLR_INTERNAL_RESOURCES_RESID 1041
#define CTRLR_INTERNAL_SIGNATURE_RESID 1042
#define CTRLR_INTERNAL_SIGNATURE_MASTER_RESID 1043

#define CTRLR_MAC_PANEL_FILE "PanelZ"
#define CTRLR_MAC_RESOURCES_FILE "ResourcesZ"
#define CTRLR_MAC_SIGNATURE_FILE "Signature"
#define CTRLR_MAC_SIGNATURE_MASTER_FILE "MasterSignature"

#define CTRLR_INTERNAL_PANEL_SECTION "ctrlr_panel"
#define CTRLR_INTERNAL_RESOURCES_SECTION "ctrlr_panel_resources"
#define CTRLR_INTERNAL_SIGNATURE_SECTION "ctrlr_panel_signature"
#define CTRLR_INTERNAL_SIGNATURE_MASTER_SECTION "ctrlr_panel_signature_master"

class CtrlrNative
{
public:
    virtual ~CtrlrNative() = default;

    static CtrlrNative* getNativeObject(CtrlrManager& owner);

    // Modernized async signature for instance exports
    virtual void exportWithDefaultPanel(CtrlrPanel* panel, 
                                        const bool isRestricted, 
                                        const bool signPanel,
                                        std::function<void(juce::Result)> callback)
    {
        if (callback)
            callback(juce::Result::fail("Native, implement me"));
    }

    virtual juce::Result getDefaultPanel(juce::MemoryBlock&) 
    { 
        return juce::Result::fail("Native, implement me"); 
    }

    virtual juce::Result getDefaultResources(juce::MemoryBlock&) 
    { 
        return juce::Result::fail("Native, implement me"); 
    }

    virtual juce::Result registerFileHandler() 
    { 
        return juce::Result::fail("Native, implement me"); 
    }

    virtual juce::Result sendKeyPressEvent(const juce::KeyPress&) 
    { 
        return juce::Result::fail("Native, implement me"); 
    }

    virtual juce::Result sendKeyPressEvent(const juce::KeyPress&, const juce::String&) 
    { 
        return juce::Result::fail("Native, implement me"); 
    }
};

#endif // CTRLR_NATIVE_H