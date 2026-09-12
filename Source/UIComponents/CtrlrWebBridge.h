/*
  ==============================================================================

    CtrlrWebBridge.h
    Created: 11 Sep 2026 7:29:59am
    Author:  zan64

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "CtrlrMacros.h"
#include "CtrlrManager/CtrlrManager.h" // Access to modulators and core state

class CtrlrWebBridge : public juce::Component 
{
public:
    // Pass CtrlrManager to interface directly with the existing backend
    CtrlrWebBridge (CtrlrManager& ownerManager) : owner (ownerManager)
    {
        juce::WebBrowserComponent::Options options;
        
        options = options.withNativeFunction ("postToNative", 
            [this] (const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion)
            {
                if (args.size() > 0 && args[0].isString())
                    handleJavaScriptMessage (args[0].toString());
                completion (juce::var());
            });

        webView = std::make_unique<juce::WebBrowserComponent> (options);
        addAndMakeVisible (*webView);
    }
    ~CtrlrWebBridge() override;
    void resized() override { webView->setBounds (getLocalBounds()); }
    void loadURL (const juce::String& url) { webView->goToURL (url); }

    void handleJavaScriptMessage (const juce::String& jsonString)
    {
        auto json = juce::JSON::parse (jsonString);
        if (json.isObject())
        {
            auto paramId = json.getProperty ("paramId", "").toString();
            auto value   = (int) json.getProperty ("value", 0);

            // Hook straight into Ctrlr's core manager or active panel
            if (auto* activePanel = owner.getActivePanel())
            {
                if (auto* mod = activePanel->getModulator (paramId))
                {
                    // Triggers the standard internal pipeline, luabind, and MIDI CC maps!
                    mod->setModulatorValue (value, true, true, false); 
                }
            }
        }
    }

    void updateUi (const juce::String& paramId, int value)
    {
        if (webView != nullptr)
        {
            juce::String jsCode = "if(window.updateBootstrapUi){window.updateBootstrapUi('" + paramId + "'," + juce::String(value) + ");}";
            webView->goToURL ("javascript:" + jsCode);
        }
    }

private:
    CtrlrManager& owner;
    std::unique_ptr<juce::WebBrowserComponent> webView;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CtrlrWebBridge)
};
