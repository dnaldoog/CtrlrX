#ifndef __JUCER_HEADER_CTRLRLUAMETHODCODEEDITORSETTINGS_CTRLRLUAMETHODCODEEDITORSETTINGS_FC2CDFB3__
#define __JUCER_HEADER_CTRLRLUAMETHODCODEEDITORSETTINGS_CTRLRLUAMETHODCODEEDITORSETTINGS_FC2CDFB3__

#include "CtrlrLuaCodeTokeniser.h"
#include "CtrlrLuaMethodCodeEditorSettingsColourLnF.h"
#include "CtrlrPropertyEditors/CtrlrPropertyComponent.h"
#include "CtrlrTextEditor.h"
#include "CtrlrWindowManagers/CtrlrChildWindowContent.h"
#include "CtrlrWindowManagers/CtrlrPanelWindowManager.h"
#include "Methods/CtrlrLuaMethod.h"
#include "stdafx.h"

class CtrlrLuaMethodEditor;

class CtrlrLuaMethodCodeEditorSettings : public Component,
										 public ChangeListener,
										 public ComboBox::Listener,
										 public Button::Listener,
										 public Slider::Listener
{
public:
	CtrlrLuaMethodCodeEditorSettings(CtrlrLuaMethodEditor &_owner, juce::Value &sharedSearchTabsValue_);
	~CtrlrLuaMethodCodeEditorSettings();

	void changeListenerCallback(ChangeBroadcaster *source);
	const Font getFont();
	const Colour getBgColour();
	const Colour getLineNumbersBgColour();
	const Colour getLineNumbersColour();

	// Moved these from local functions inside the constructor
	void populateColourCombo(ColourComboBox *combo);
	int findColourIndex(const Colour &colour);
	Colour getColourFromCombo(ComboBox *combo);

	void paint(Graphics &g);
	void resized();
	void comboBoxChanged(ComboBox *comboBoxThatHasChanged);
	void buttonClicked(Button *buttonThatWasClicked);
	void sliderValueChanged(Slider *sliderThatWasMoved);
	void loadSyntaxColorsFromSettings();
	void saveSyntaxColorsToSettings();
	void populateSyntaxTokenCombo();
	void updateSyntaxColors();
	String getCurrentSelectedTokenType();
	void populateColourComboWithThumbnails(ColourComboBox *combo);
	void updateTokenColorDisplay(const String &tokenType);
	void clearSyntaxColorSettings();

	bool hasUnsavedChanges() const;
	void markAsChanged();
	void markAsSaved();
#if JUCE_VERSION < 0x070000
	bool promptToSaveChanges();
#else
	void promptToSaveChanges(std::function<void(bool proceedWithClose)> onCompletion);
#endif
	void applySettings();
	void closeWindow();

	const char *getDefaultFont() const { return defaultFont; };

private:
	bool hasChanges;
	static constexpr const char *defaultFont = "<Monospaced>";

	struct ColourItem
	{
		String name;
		Colour colour;
	};
	static const ColourItem availableColours[];

	CtrlrLuaCodeTokeniser luaTokeniser;
	CodeDocument codeDocument;
	CtrlrLuaMethodEditor &owner;
	Font codeFont;
	Font previousFont;
	int marginLeft;
	int marginTop;
	int sampleWidth;
	int sampleHeight;

	std::unique_ptr<ComboBox> fontTypeface;
	std::unique_ptr<ColourComboBox> bgColour;
	std::unique_ptr<ColourComboBox> lineNumbersBgColour;
	std::unique_ptr<ColourComboBox> lineNumbersColour;
	std::unique_ptr<ComboBox> syntaxTokenType;
	std::unique_ptr<ColourComboBox> syntaxTokenColor;
	std::unique_ptr<ToggleButton> fontBold;
	std::unique_ptr<ToggleButton> fontItalic;
	std::unique_ptr<ToggleButton> openSearchTabs;
	std::unique_ptr<TextButton> applyButton;
	std::unique_ptr<TextButton> cancelButton;
	std::unique_ptr<TextButton> resetButton;
	std::unique_ptr<TextButton> resetToPreviousButton;
	std::unique_ptr<Slider> fontSize;
	std::unique_ptr<Label> label0;		// Updated v5.6.34. Thanks to @dnaldoog
	std::unique_ptr<Label> label1;		// Updated v5.6.34. Thanks to @dnaldoog
	std::unique_ptr<Label> label2;		// Updated v5.6.34. Thanks to @dnaldoog
	std::unique_ptr<Label> label3;		// Updated v5.6.34. Thanks to @dnaldoog
	std::unique_ptr<Label> syntaxLabel; // Updated v5.6.34. Thanks to @dnaldoog
	std::unique_ptr<CodeEditorComponent> fontTest;

	static CodeEditorComponent::ColourScheme &getSharedScheme();
	HashMap<String, Colour> customSyntaxColors;

	Font originalFont;
	Colour originalBgColour;
	Colour originalLineNumbersBgColour;
	Colour originalLineNumbersColour;
	HashMap<String, Colour> originalSyntaxColors;
	bool originalOpenSearchTabs;

	juce::Value &sharedSearchTabsValue;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrLuaMethodCodeEditorSettings);
};

#endif // __JUCER_HEADER_CTRLRLUAMETHODCODEEDITORSETTINGS_CTRLRLUAMETHODCODEEDITORSETTINGS_FC2CDFB3__
