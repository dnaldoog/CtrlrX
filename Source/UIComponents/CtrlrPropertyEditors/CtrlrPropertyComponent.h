#ifndef __CTRLR_PROPERTY_COMPONENT__
#define __CTRLR_PROPERTY_COMPONENT__

#include "CtrlrIDManager.h"
#include "CtrlrLog.h" // Case sensitive on LINUX
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelCanvas.h"
#include "CtrlrPanel/CtrlrPanelCanvasLayer.h"
#include "CtrlrPanel/CtrlrPanelEditorIcons.h" // Added v5.6.34.
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

class CtrlrFloatingWindow;

class CtrlrPropertyChild : public ChangeBroadcaster {
	public:
		virtual ~CtrlrPropertyChild() {}
		virtual void refresh() = 0;
};

class CtrlrPropertyComponent : public PropertyComponent, public ValueTree::Listener {
	public:
		CtrlrPropertyComponent(const Identifier &_propertyName, const ValueTree &_propertyElement,
							   const ValueTree &_identifierDefinition, CtrlrPanel *_panel = nullptr,
							   StringArray *_possibleChoices = nullptr, Array<var> *_possibleValues = nullptr);

		virtual ~CtrlrPropertyComponent();
		int getPreferredHeight() const noexcept { return preferredHeight; }
		void setPreferredHeight(int newHeight) noexcept { preferredHeight = newHeight; }
		Component *getPropertyComponent();
		void resized();
		void paint(Graphics &g);
		void refresh();
		const String getPropertyName();
		const String getVisibleText();
		const String getElementSubType();
		const String getElementType();
		void valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override;
		void valueTreeChildAdded(ValueTree &parentTree, ValueTree &childWhichHasBeenAdded) override {}
		void valueTreeChildRemoved(ValueTree &parentTree, ValueTree &childWhichHasBeenRemoved,
								   int indexFromWhichChildWasRemoved) override {}
		void valueTreeChildOrderChanged(ValueTree &parentTreeWhoseChildrenHaveMoved, int oldIndex,
										int newIndex) override {}
		void valueTreeParentChanged(ValueTree &treeWhoseParentHasChanged) override {}
				juce::String getComponentTitle() const;

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrPropertyComponent);
		String visibleText;
		String buttonText;
		ValueTree propertyElement;
		Identifier propertyName;
		ValueTree identifierDefinition;
		Font currentFont;
		CtrlrIDManager::PropertyType propertyType;
		CtrlrPanel *panel;
		StringArray possibleChoices;
		Array<var> possibleValues;
		URL url;
		String urlString;
};

class CtrlrBooleanPropertyComponent : public Component, public Button::Listener, public CtrlrPropertyChild {
	public:
		CtrlrBooleanPropertyComponent(const Value &_valueToControl, const String &_trueText, const String &_falseText);
		CtrlrBooleanPropertyComponent(const Value &_valueToControl, const String &_stateText);
		~CtrlrBooleanPropertyComponent();
		void paint(Graphics &g);
		void refresh();
		void resized();
		void buttonClicked(Button *);

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrBooleanPropertyComponent);

		Value valueToControl;
		String stateText;
		String onText;		  // This is what shows when true
		String offText;		  // This is what shows when false
		String trueText;	  // ADD THIS if not there
		String falseText;	  // ADD THIS if not there
		bool useCustomLabels; // ADD THIS if not there
		ToggleButton button;
};

class CtrlrButtonPropertyComponent : public Component, public Button::Listener, public CtrlrPropertyChild {
	public:
		CtrlrButtonPropertyComponent(const Value &_valueToControl, const String &_propertyName);
		~CtrlrButtonPropertyComponent();
		void refresh();
		void resized();
		void buttonClicked(Button *button);

	private:
		Value valueToControl;
		String propertyName;
		TextButton button;
};

// Added v5.6.32
class CtrlrButtonTextPropertyComponent : public Component, public Button::Listener, public CtrlrPropertyChild {
	public:
		CtrlrButtonTextPropertyComponent(const Value &_valueToControl, const String &_propertyName,
										 const String &_buttonText);
		~CtrlrButtonTextPropertyComponent();
		void refresh();
		void resized();
		void buttonClicked(Button *button);

	private:
		Value valueToControl;
		String propertyName;
		TextButton button;
		String buttonText;
};

class CtrlrChoicePropertyComponent : public Component, public ComboBox::Listener, public CtrlrPropertyChild {
	public:
		CtrlrChoicePropertyComponent(const Value &_valueToControl, const StringArray *_choices,
									 const Array<var> *_values, const bool _numeric);
		~CtrlrChoicePropertyComponent();
		void refresh();
		void resized();
		void comboBoxChanged(ComboBox *comboBoxThatHasChanged);
		void changed();
		// void changeListenerCallback (ChangeBroadcaster* source);

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrChoicePropertyComponent)
		Value valueToControl;
		Array<var> values;
		StringArray choices;
		std::unique_ptr<ComboBox> combo;
		CtrlrPropertyComponent *owner;
		bool numeric;
};

/* Colour Property */
class CtrlrColourLabel : public Label {
		TextEditor *createEditorComponent() {
			TextEditor *const ed = new TextEditor(getName());
			ed->setColour(TextEditor::highlightColourId, findColour(Label::backgroundColourId).contrasting(0.5f));
			ed->setInputRestrictions(8, "ABCDEFabcdef01234567890");
			ed->applyFontToAllText(getLookAndFeel().getLabelFont(*this));
			copyAllExplicitColoursTo(*ed);
			return ed;
		}
};

// Helper class to get a callback when the modal window is dismissed
class ModalCallback : public juce::ModalComponentManager::Callback {
	public:
		std::function<void()> onDismissed;

		void modalStateFinished(int) override {
			if (onDismissed)
				onDismissed();
		}
};

// Added v5.6.34. Required extra class for the colour picker button so that it follows the lookAndFeel colourScheme in
// the panel windows as well as the others (layer manager etc)
class ColourPickerButton : public juce::Button {
	public:
		ColourPickerButton(const juce::String &name = juce::String()) : juce::Button(name) {
			setWantsKeyboardFocus(false);
		}

		void paintButton(juce::Graphics &g, bool isMouseOver, bool isMouseDown) override {
			// Start with the default color.
			auto buttonColour = getLookAndFeel().findColour(juce::TextButton::buttonColourId);

			if (getToggleState() || isMouseDown) // I could not get it to work properly, I was unable to get it reset
												 // when clicking outside the oclour selector popup. the only way to
												 // reset was to click again in the button itself.
			{
				buttonColour = getLookAndFeel().findColour(juce::TextButton::buttonColourId);
			} else if (isMouseOver) // Not always reset propertly if we open the colour selector
			{
				buttonColour = getLookAndFeel().findColour(juce::TextButton::buttonColourId).contrasting(0.02f);
			}

			// Now, draw the button with the determined color.
			g.setColour(buttonColour);
			g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

			// 3. Draw the button's outline
			g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
			g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 1.0f);

			// 4. Draw the eyedropper icon.
			const juce::String eyedropperSVG = SvgIconManager::getSvgString(IconType::EyeDropper);

			std::unique_ptr<juce::Drawable> icon =
				juce::Drawable::createFromImageData(eyedropperSVG.toRawUTF8(), strlen(eyedropperSVG.toRawUTF8()));

			if (icon) {
				// Use a contrasting color for the icon so it's always visible.
				auto iconColour = buttonColour.contrasting();
				icon->replaceColour(juce::Colours::black, iconColour);

				// Calculate a new, smaller rectangle for the icon to be drawn in.
				const float iconSizeFactor = 0.4f; // % of the button's size
				auto iconBounds =
					getLocalBounds().toFloat().reduced(getLocalBounds().getWidth() * (1.0f - iconSizeFactor) / 2.0f);

				icon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
			}
		}
};

class CtrlrColourEditorComponent : public Component,
								   public ChangeListener,
								   public ChangeBroadcaster,
								   public Label::Listener,
								   public Button::Listener {
	public:
		CtrlrColourEditorComponent(ChangeListener *defaultListener = 0);
		~CtrlrColourEditorComponent();

		void updateLabel();
		void labelTextChanged(Label *labelThatHasChanged);
		void resized();
		void resetToDefault() {
			colour = Colours::black;
			sendChangeMessage();
		}
		void setColour(const Colour &newColour, const bool sendChangeMessageNow = false);
		const Colour getColour() { return (colour); }
		void refresh() { updateLabel(); }
		void buttonClicked(Button *buttonThatWasClicked) override; // Added v5.6.34.
		// void mouseDown (const MouseEvent &e);
		void changeListenerCallback(ChangeBroadcaster *source) override; // Added override
		void lookAndFeelChanged() override;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrColourEditorComponent)

	private:
		CtrlrColourLabel colourTextInput;
		// ColourPickerButton* colourPickerButton;
		std::unique_ptr<ColourPickerButton> colourPickerButton;
		Colour colour;
		bool canResetToDefault;

		void openColourPicker(); // Add this helper method
								 // void updateButtonColour(); // Helper to update button appearance
};

class CtrlrColourPropertyComponent : public Component, public ChangeListener, public CtrlrPropertyChild {
	public:
		CtrlrColourPropertyComponent(const Value &_valueToControl);
		~CtrlrColourPropertyComponent();

		void refresh() override;
		void changeListenerCallback(ChangeBroadcaster *source) override;

		void resized() override;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrColourPropertyComponent)

	private:
		Value valueToControl;
		CtrlrColourEditorComponent cs;
};

/* Read Only Property */
class CtrlrReadOnlyProperty : public Component, public CtrlrPropertyChild {
	public:
		CtrlrReadOnlyProperty(const Identifier &_propertyName, const ValueTree &_propertyElement,
							  const ValueTree &identifier, CtrlrPanel *panel = nullptr,
							  StringArray *possibleChoices = nullptr, StringArray *possibleValues = nullptr);
		~CtrlrReadOnlyProperty();
		void refresh();
		void resized();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrReadOnlyProperty)

	private:
		Label value;
		ValueTree propertyElement;
		Identifier propertyName;
		CtrlrPanel *panel; // Added v5.6.34. Thanks to @dnaldoog. Used to get the name of the layer, not the useless
						   // long id string
};

class CtrlrExpressionProperty : public Component,
								public TextEditor::Listener,
								public Button::Listener,
								public CtrlrPropertyChild {
	public:
		CtrlrExpressionProperty(const Value &_valueToControl);
		~CtrlrExpressionProperty();
		void refresh();
		void textEditorTextChanged(TextEditor &editor);
		void textEditorReturnKeyPressed(TextEditor &editor);
		void textEditorFocusLost(TextEditor &editor);
		const bool compile(const bool setPropertyIfValid);
		void resized();
		void buttonClicked(Button *buttonThatWasClicked);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrExpressionProperty);

	private:
		CtrlrFloatingWindow *externalEditorWindow;
		Value valueToControl;
		std::unique_ptr<TextEditor> text;
		std::unique_ptr<DrawableButton> apply;
};

class CtrlrFileProperty : public Component, public Label::Listener, public Button::Listener, public CtrlrPropertyChild {
	public:
		CtrlrFileProperty(const Value &_valueToControl);
		~CtrlrFileProperty();
		void refresh();
		void resized();
		void buttonClicked(Button *buttonThatWasClicked);
		void labelTextChanged(Label *labelThatHasChanged);

// 		std::unique_ptr<juce::FileChooser> fileChooser; -> Now using static FC wrapper code in CtrlrInlineUtilitiesGUI.h

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrFileProperty);
		Value valueToControl;
		std::unique_ptr<Label> path;
		std::unique_ptr<TextButton> browse;
};

class CtrlrFontPropertyComponent : public Component,
								   public ComboBox::Listener,
								   public Button::Listener,
								   public Slider::Listener,
								   public CtrlrPropertyChild
// public LookAndFeel_V2
{
	public:
		CtrlrFontPropertyComponent(const Value &_valueToControl, CtrlrPanel *_owner);
		~CtrlrFontPropertyComponent() override; // Mark destructor with override
		void refresh() override;				// Mark as override if it overrides a base class method
		Font getFont();
		void resized() override; // Mark as override

		// These are listener methods and should be marked override
		void comboBoxChanged(ComboBox *comboBoxThatHasChanged) override;
		void buttonClicked(Button *buttonThatWasClicked) override;
		void sliderValueChanged(Slider *sliderThatWasMoved) override;

		// This function seems to be part of an old LookAndFeel, so it's best to remove it if you're not using it.
		// Label* createSliderTextBox (Slider& slider);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrFontPropertyComponent);

		class SliderLabelComp : public Label {
			public:
				SliderLabelComp() : Label("", "") {}
				void mouseWheelMove(const MouseEvent &, const MouseWheelDetails &) override {} // Mark as override
		};

	private:
		Value valueToControl;
		StringArray choices;
		CtrlrPanel *owner;

		// Pointers for components whose memory will be managed by the parent
		std::unique_ptr<ComboBox> typeface;
		std::unique_ptr<ComboBox> fontSizeComboBox;		   // Replaces the Slider
		std::unique_ptr<ComboBox> horizontalScaleComboBox; // Replaces the Slider
		std::unique_ptr<ComboBox> kerningComboBox;		   // Replaces the Slider
		std::unique_ptr<DrawableButton> fontBold;
		std::unique_ptr<DrawableButton> fontItalic;
		std::unique_ptr<DrawableButton> fontUnderline;

		std::unique_ptr<Label> fontSizeLabel;		 // Added v5.6.34. Thanks to @dnaldoog
		std::unique_ptr<Label> horizontalScaleLabel; // Added v5.6.34. Thanks to @dnaldoog
		std::unique_ptr<Label> kerningLabel;		 // Added v5.6.34. Thanks to @dnaldoog
};

class CtrlrLuaMethodProperty : public Component,
							   public ComboBox::Listener,
							   public Button::Listener,
							   public CtrlrPropertyChild {
	public:
		CtrlrLuaMethodProperty(const Value &_valueToControl, const Identifier &_id, CtrlrPanel *_owner);
		~CtrlrLuaMethodProperty();
		void refresh();
		void resized();
		void comboBoxChanged(ComboBox *comboBoxThatHasChanged);
		void buttonClicked(Button *buttonThatWasClicked);

	private:
		Value valueToControl;
		Identifier id;
		String propertyName;
		CtrlrPanel *owner;
		std::unique_ptr<ComboBox> methodSelectorCombo;
		std::unique_ptr<DrawableButton> editMethodButton;
		std::unique_ptr<DrawableButton> newMethodButton;
		std::unique_ptr<DrawableButton> deleteMethodButton;
};

class CtrlrModulatorListProperty : public CtrlrPropertyChild,
								   public Component,
								   public ComboBox::Listener,
								   public CtrlrPanel::Listener {
	public:
		CtrlrModulatorListProperty(const Value &_valueToControl, CtrlrPanel *_owner);
		~CtrlrModulatorListProperty();
		void refresh();
		void comboBoxChanged(ComboBox *comboBoxThatHasChanged);
		void resized();
		void modulatorChanged(CtrlrModulator *modulatorThatChanged);
		void modulatorAdded(CtrlrModulator *modulatorThatWasAdded);
		void modulatorRemoved(CtrlrModulator *modulatorRemoved);
		void listChanged();

	private:
		CtrlrPanel *owner;
		Value valueToControl;
		StringArray choices;
		std::unique_ptr<ComboBox> combo;
		bool numeric;
};

class MultiMidiAlert : public AlertWindow 
{
public:
    MultiMidiAlert()
        : AlertWindow("Add Custom MIDI Message",
                      String(), // JUCE 7/8: Custom label layout below
                      AlertWindow::QuestionIcon) 
    {
        addTextEditor("customMidi", "F0 00 xx F7", "MIDI Message", false);

        // --- JUCE 7/8 Setup ---
        messageLabel.setText(getHelpText(), dontSendNotification);
        messageLabel.setFont(Font(15.0f));
        messageLabel.setColour(Label::textColourId, findColour(AlertWindow::textColourId));
        messageLabel.setSize(460, 200);
        addCustomComponent(&messageLabel);

        addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
        addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));
        setSize(560, 400);
        
        AW::layoutButtonsJUCE8(this, "OK", "Cancel", 80, 40);
    }

    static const String getHelpText() {
        return "Enter a Raw MIDI message:\n\n"
               "Examples:\n"
               "  Bn,-2,-1           (CC using component number & value)\n"
               "  Cn,-1   (Program change using component value)\n"
               "  SysEx,F0 00 xx F7  (SysEx with tokens)\n\n"
               "  B4 03 67           (Raw MIDI hex bytes)\n\n"
               "Tokens: -2=component number, -1=component value, xx etc = SysEx tokens";
    }


    const String getValue() {
        if (auto* ed = getTextEditor("customMidi")) {
            String userInput = ed->getText().trim();
            if (userInput.isNotEmpty())
                return "Custom," + userInput;
        }
        return String();
    }

private:
    Label messageLabel;

};

class BubbleConfigAlert : public AlertWindow 
{
public:
    BubbleConfigAlert(const String& currentTitle, const String& currentText, int currentTimeout)
        : AlertWindow("Configure Tooltip Bubble", String(), AlertWindow::NoIcon) 
    {
        addTextEditor("bubbleTitle", currentTitle, "Bubble Header/Title:", false);
        addTextEditor("bubbleText", currentText, "Help text description:", false);
        
        if (auto* textEd = getTextEditor("bubbleText")) {
            textEd->setMultiLine(true, true);
            textEd->setReturnKeyStartsNewLine(true);
        }

        addTextEditor("bubbleTimeout", String(currentTimeout), "Display timeout (ms):", false);


        addButton("Save Changes", 1, KeyPress(KeyPress::returnKey, 0, 0));
        addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));
        setSize(500, 350);
        
        AW::layoutButtonsJUCE8(this, "Save Changes", "Cancel", 120, 35);
    }

};

class CtrlrMultiMidiPropertyComponent : public Component,
										public ListBoxModel,
										public Label::Listener,
										public Button::Listener,
										public CtrlrPropertyChild {
	public:
		CtrlrMultiMidiPropertyComponent(const Value &_valueToControl);
		~CtrlrMultiMidiPropertyComponent();
		void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected);
		int getNumRows();
		void refresh();
		void loadAdditionalTemplates(const File &templateFile);
		Component *refreshComponentForRow(int rowNumber, bool isRowSelected, Component *existingComponentToUpdate);
		void labelTextChanged(Label *labelThatHasChanged);
		void paint(Graphics &g);
		void resized();
		void buttonClicked(Button *buttonThatWasClicked);
		void mouseDown(const MouseEvent &e);
		void mouseDoubleClick(const MouseEvent &e);
		void lookAndFeelChanged() override; // Updated v5.6.35. For Multi MIDI Message. Thanks to @dnaldoog

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrMultiMidiPropertyComponent)

		void updateButtonIcons(); // Updated v5.6.35. Helper function for Multi MIDI Message. Thanks to @dnaldoog

		MultiMidiAlert questionWindow;
		StringArray values;
		Value valueToControl;
		StringPairArray templates;
		std::unique_ptr<DrawableButton> addMulti;	 // Updated v5.6.35. For Multi MIDI Message. Thanks to @dnaldoog
		std::unique_ptr<DrawableButton> removeMulti; // Updated v5.6.35. For Multi MIDI Message. Thanks to @dnaldoog
		std::unique_ptr<ListBox> listMulti;			 // Updated v5.6.35. For Multi MIDI Message. Thanks to @dnaldoog
		std::unique_ptr<DrawableButton> copy;
		std::unique_ptr<DrawableButton> paste;
		std::unique_ptr<DrawableButton> helpMmidi; // Updated v5.6.35. For Multi MIDI Message. Thanks to @dnaldoog
};

class CtrlrSliderPropertyComponent : public Component, private Slider::Listener, public CtrlrPropertyChild {
	public:
		CtrlrSliderPropertyComponent(const Value &_valueToControl, double rangeMin, double rangeMax, double interval);
		~CtrlrSliderPropertyComponent();
		virtual void setValue(double newValue);
		virtual double getValue() const;
		void refresh();
		void sliderValueChanged(Slider *sliderThatChanged);
		void resized();

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrSliderPropertyComponent);
		Slider slider;
		Value valueToControl;
};

class SysExRow : public Component {
	public:
		SysExRow(const int _n = 1, const int _w = 32, const int _gap = 4);
		void paint(Graphics &g);
		void resized();

	private:
		int n, w, gap;
};

class CtrlrSysExEditor : public Component, public Slider::Listener, public Label::Listener, public ChangeBroadcaster {
	public:
		CtrlrSysExEditor(Value &_val, CtrlrPanel *_panel);
		~CtrlrSysExEditor();
		void setLength(const int newLength);
		// NOTE: Keeps a raw pointer here because ownership is handled by the JUCE
		// component hierarchy (via addAndMakeVisible). Returning a raw pointer
		// allows callers to modify the label without accidentally taking ownership.
		Label *addByte(const String &byteAsString);
		const String getValue();
		const PopupMenu getVendorIdMenu();
		void paint(Graphics &g);
		void resized();
		void sliderValueChanged(Slider *sliderThatWasMoved);
		void labelTextChanged(Label *labelThatHasChanged);
		void mouseDown(const MouseEvent &e);
		void showTokenMenuForLabel(Label *l);
		void updateLabelHighlights(Label *focusedLabel);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrSysExEditor)

	private:
		Value val;
		CtrlrPanel *owner;
		StringArray splitMessage;
		OwnedArray<Label> byteValueLabels;
		OwnedArray<SysExRow> rows;
		int currentMessageLength;
		std::unique_ptr<Slider> messageLength;
		std::unique_ptr<Label> lengthLabel;
		std::unique_ptr<TextButton> addTokenButton;
		// NOTE: Keeps a raw pointer here because ownership is handled by the JUCE
		// component hierarchy (via addAndMakeVisible). Returning a raw pointer
		// allows callers to modify the label without accidentally taking ownership.
		Label *lastFocusedLabel = nullptr; // Added v5.6.35. For Multi MIDI Message. Thanks to @dnaldoog
};

class CtrlrSysExFormulaEditor : public Component {
	public:
		//==============================================================================
		CtrlrSysExFormulaEditor();
		~CtrlrSysExFormulaEditor();

		//==============================================================================
		//[UserMethods]     -- You can add your own custom methods in this section.
		//[/UserMethods]

		void paint(Graphics &g);
		void resized();

		//==============================================================================
	juce_UseDebuggingNewOperator

		private :
		//[UserVariables]   -- You can add your own custom variables in this section.
		CodeDocument forwardFormulaDocument,
		reverseFormulaDocument;
		//[/UserVariables]

		//==============================================================================
		std::unique_ptr<CodeEditorComponent> forwardFormula;
		std::unique_ptr<CodeEditorComponent> reverseFormula;
		std::unique_ptr<Label> forwardLabel;
		std::unique_ptr<Label> reverseLabel;
		std::unique_ptr<Label> label;

		//==============================================================================
		// (prevent copy constructor and operator= being generated..)
		CtrlrSysExFormulaEditor(const CtrlrSysExFormulaEditor &);
		const CtrlrSysExFormulaEditor &operator=(const CtrlrSysExFormulaEditor &);
};

class CtrlrSysExPropertyComponent : public Component,
									public Label::Listener,
									public Button::Listener,
									public CtrlrPropertyChild,
									public ChangeListener {
	public:
		CtrlrSysExPropertyComponent(const Value &_valueToControl, const ValueTree &_propertyTree,
									const Identifier &_propertyName, CtrlrPanel *_owner);
		~CtrlrSysExPropertyComponent();
		void refresh();
		void resized();
		void labelTextChanged(Label *labelThatHasChanged);
		void buttonClicked(Button *buttonThatWasClicked);
		void changeListenerCallback(ChangeBroadcaster *cb);
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrSysExPropertyComponent)

	private:
		Value valueToControl;
		std::unique_ptr<Label> sysexPreview;
		std::unique_ptr<TextButton> editButton;
		std::unique_ptr<DrawableButton> copy;
		std::unique_ptr<DrawableButton> paste;
		ValueTree propertyTree;
		Identifier propertyName;
		CtrlrPanel *owner;
};

class CtrlrTextPropertyComponent : public Component, public CtrlrPropertyChild {
	public:
		CtrlrTextPropertyComponent(const Value &_valueToControl, const int maxNumChars, const bool isMultiLine,
								   const bool isReadOnly = false,
								   const bool useImprovedLegibility = false); // Added v5.6.34.

		~CtrlrTextPropertyComponent();
		void refresh();
		void setText(const String &newText);
		String getText() const;
		void textWasEdited();
		void resized();

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrTextPropertyComponent);

		std::unique_ptr<Label> textEditor;
		void createEditor(int maxNumChars, bool isMultiLine);

		Value valueToControl;
		bool isReadOnly;			// Added v5.6.34
		bool useImprovedLegibility; // Added v5.6.34
};

class CtrlrTimestampProperty : public Component, public CtrlrPropertyChild {
	public:
		CtrlrTimestampProperty(const Value &_valueToControl);
		~CtrlrTimestampProperty();
		void refresh();
		void resized();

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrTimestampProperty);
		std::unique_ptr<Label> textEditor;
		Value valueToControl;
};

class CtrlrUnknownPropertyComponent : public Component, public CtrlrPropertyChild {
	public:
		CtrlrUnknownPropertyComponent(const Identifier &_propertyName, const ValueTree &_propertyElement,
									  const ValueTree &identifier, CtrlrPanel *panel = nullptr,
									  StringArray *possibleChoices = nullptr, StringArray *possibleValues = nullptr);
		~CtrlrUnknownPropertyComponent();
		void resized();
		void refresh();

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CtrlrUnknownPropertyComponent);
		Label l;
		Identifier propertyName;
		ValueTree propertyElement;
};

#endif
