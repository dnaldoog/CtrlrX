#include "MainComponent.h"

#if JUCE_LINUX
// ----------------------------
// LinuxFadeInHelper - fades by controlling content alpha
// ----------------------------
class LinuxFadeInHelper : public juce::Timer
{
public:
    LinuxFadeInHelper(juce::DocumentWindow* w, juce::Component* content)
        : window(w), mainContent(content), alpha(0.0f)
    {
        // Hide content initially
        if (mainContent)
            mainContent->setAlpha(0.0f);
        startTimer(8);  // Slightly faster interval
    }

    void timerCallback() override
    {
        if (!window || !mainContent)
        {
            stopTimer();
            delete this;
            return;
        }

        alpha += 0.2f;  // Faster fade (5 steps instead of 10)
        if (alpha >= 1.0f)
        {
            alpha = 1.0f;
            mainContent->setAlpha(alpha);
            stopTimer();
            delete this;
            return;
        }

        mainContent->setAlpha(alpha);
    }

private:
    juce::DocumentWindow* window;
    juce::Component* mainContent;
    float alpha;
};
#endif // JUCE_LINUX

//==============================================================================

class GuiAppApplication : public juce::JUCEApplication
{
public:
    GuiAppApplication() {}

    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);

#if JUCE_LINUX
        mainWindow.reset(new MainWindow(getApplicationName()));
        
        // Start offscreen and tiny
        mainWindow->setTopLeftPosition(-10000, -10000);
        mainWindow->setSize(1, 1);
        
        // Hide content
        if (auto* content = mainWindow->getContentComponent())
            content->setAlpha(0.0f);
        
        mainWindow->setVisible(true);
        
        // Minimal delay - next message loop iteration
        juce::MessageManager::callAsync([this]()
        {
            if (mainWindow)
            {
                mainWindow->centreWithSize(800, 600);
                
                // Immediate fade
                if (auto* content = mainWindow->getContentComponent())
                    new LinuxFadeInHelper(mainWindow.get(), content);
            }
        });
#else
        // macOS / Windows – normal behavior
        mainWindow.reset(new MainWindow(getApplicationName()));
        mainWindow->centreWithSize(800, 600);
        mainWindow->setVisible(true);
#endif
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
    }

    //==============================================================================
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(juce::String name)
            : DocumentWindow(name,
                             juce::Desktop::getInstance().getDefaultLookAndFeel()
                                 .findColour(ResizableWindow::backgroundColourId),
                             DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(true, true);
#endif
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// Main entry point
START_JUCE_APPLICATION(GuiAppApplication)