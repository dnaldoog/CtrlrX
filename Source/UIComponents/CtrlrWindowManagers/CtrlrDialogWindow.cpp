#include "stdafx.h"
#include "CtrlrDialogWindow.h"
#include "CtrlrInlineUtilitiesGUI.h"

// ============================================================================
// 1. Helper Fade-In Animator for Linux
// ============================================================================
#if JUCE_LINUX
class LinuxFadeInWindow : public juce::Timer {
	public:
		explicit LinuxFadeInWindow(juce::Component *c) : comp(c), alpha(0.0f) {
			if (comp != nullptr) {
				comp->setAlpha(alpha);
				startTimer(5); // 5ms interval for smooth updates
			}
		}

		void timerCallback() override {
			if (comp == nullptr) {
				stopTimer();
				delete this; // Auto-cleanup animator instance
				return;
			}

			alpha += 0.25f;
			if (alpha >= 1.0f) {
				alpha = 1.0f;
				comp->setAlpha(alpha);
				stopTimer();
				delete this; // Auto-cleanup animator instance
				return;
			}

			comp->setAlpha(alpha);
		}

	private:
		juce::Component *comp;
		float alpha;
};
#endif

// ============================================================================
// 2. Custom Temporary Dialog Window Class (Cross-Platform)
// ============================================================================
class CtrlrTempDialogWindow : public juce::DialogWindow {
	public:
		CtrlrTempDialogWindow(const juce::String &title, juce::Component *contentComponent_,
							  juce::Component *componentToCentreAround, const juce::Colour &colour,
							  const bool escapeKeyTriggersCloseButton_, const bool shouldBeResizable,
							  const bool useBottomRightCornerResizer)
			: juce::DialogWindow(title, colour, escapeKeyTriggersCloseButton_, true) {
			setUsingNativeTitleBar(true);

#if JUCE_LINUX
			setBounds(-10000, -10000, 400, 300);
			setAlpha(0.0f);

			if (contentComponent_ != nullptr) {
				int w = contentComponent_->getWidth();
				if (w <= 0)
					w = 400;
				int h = contentComponent_->getHeight();
				if (h <= 0)
					h = 300;
				contentComponent_->setSize(w, h);
				setSize(w, h);
			}

			setContentNonOwned(contentComponent_, true);

			addToDesktop(juce::ComponentPeer::windowHasTitleBar | juce::ComponentPeer::windowAppearsOnTaskbar);

			centreWithSize(getWidth(), getHeight());
			setResizable(shouldBeResizable, useBottomRightCornerResizer);

			new LinuxFadeInWindow(this);
#else
			setContentNonOwned(contentComponent_, true);
			centreAroundComponent(componentToCentreAround, getWidth(), getHeight());
			setResizable(shouldBeResizable, useBottomRightCornerResizer);
#endif
		}

		void closeButtonPressed() override { setVisible(false); }

	private:
		juce::TooltipWindow tooltip;
		JUCE_DECLARE_NON_COPYABLE(CtrlrTempDialogWindow);
};

// ============================================================================
// 3. CtrlrDialogWindow Member Implementations
// ============================================================================

void CtrlrDialogWindow::showCustomDialogAsync(const juce::String &title, juce::Component *content,
											  const juce::Colour &backgroundColour, bool resizable,
											  std::function<void(int)> callback) {
	auto *dw = new CtrlrTempDialogWindow(title, content, nullptr, backgroundColour, true, resizable, false);

#if JUCE_VERSION >= 0x070000
	dw->enterModalState(true, juce::ModalCallbackFunction::create([dw, callback](int result) {
							if (callback != nullptr)
								callback(result);

							delete dw;
						}),
						true);
#else
	int result = dw->runModalLoop();
	if (callback != nullptr)
		callback(result);
	delete dw;
#endif
}

void CtrlrDialogWindow::showModalDialog(const juce::String &title, juce::Component *content, const bool resizable,
										juce::Component *parent, std::function<void(int)> callback) {
#if JUCE_VERSION >= 0x070000
	AW::showCustomDialogAsync(title, content, juce::Colours::lightgrey, resizable, callback);
#else
	CtrlrTempDialogWindow dw(title, content, parent, juce::Colours::lightgrey, true, resizable, false);
	int result = dw.runModalLoop();
	if (callback != nullptr)
		callback(result);
#endif
}

juce::DialogWindow *CtrlrDialogWindow::showNonModalDialog(const juce::String &title, juce::Component *content,
														  const juce::Colour &backgroundColour, bool escapeKeyCloses,
														  bool resizable) {
	return new CtrlrTempDialogWindow(title, content, nullptr, backgroundColour, escapeKeyCloses, resizable, false);
}