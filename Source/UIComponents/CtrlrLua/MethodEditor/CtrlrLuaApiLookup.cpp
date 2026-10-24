/*
  ==============================================================================
    CtrlrLuaApiLookup.cpp
    Auto-generated from LuaAPI.xml
  ==============================================================================
*/

#include "CtrlrLuaApiLookup.h"

CtrlrLuaApiLookup::CtrlrLuaApiLookup() {
    // --- AffineTransform ---
    {
        ApiClass cls;
        cls.name = "AffineTransform";
        cls.methods.push_back({"followedBy", "instance", "(AffineTransform)", 1});
        cls.methods.push_back({"getScaleFactor", "instance", "()", 1});
        cls.methods.push_back({"getTranslationX", "instance", "()", 1});
        cls.methods.push_back({"getTranslationY", "instance", "()", 1});
        cls.methods.push_back({"inverted", "instance", "()", 1});
        cls.methods.push_back({"isIdentity", "instance", "()", 1});
        cls.methods.push_back({"isOnlyTranslation", "instance", "()", 1});
        cls.methods.push_back({"isSingularity", "instance", "()", 1});
        cls.methods.push_back({"rotated", "instance", "(float, float, float)", 1});
        cls.methods.push_back({"rotated", "instance", "(float, float, float)", 2});
        cls.methods.push_back({"scaled", "instance", "(float, float, float, float)", 1});
        cls.methods.push_back({"scaled", "instance", "(float, float, float, float)", 2});
        cls.methods.push_back({"sheared", "instance", "(float, float)", 1});
        cls.methods.push_back({"translated", "instance", "(float, float)", 1});
        cls.staticMethods.push_back({"fromTargetPoints", "static", "(p1, Point<float>()", 1});
        cls.staticMethods.push_back({"rotation", "static", "((float)", 1});
        cls.staticMethods.push_back({"rotation", "static", "((float)", 2});
        cls.staticMethods.push_back({"scale", "static", "((float)", 1});
        cls.staticMethods.push_back({"scale", "static", "((float)", 2});
        cls.staticMethods.push_back({"scale", "static", "((float)", 3});
        cls.staticMethods.push_back({"shear", "static", "(float, float)", 1});
        cls.staticMethods.push_back({"translation", "static", "(0.0f, 1.0f)", 1});
        cls.staticMethods.push_back({"verticalFlip", "static", "(float)", 1});
        classMap[cls.name] = cls;
    }

    // --- AiffAudioFormat ---
    {
        ApiClass cls;
        cls.name = "AiffAudioFormat";
        cls.methods.push_back({"canDoMono", "instance", "()", 1});
        cls.methods.push_back({"canDoStereo", "instance", "()", 1});
        cls.methods.push_back({"canHandleFile", "instance", "(File)", 1});
        cls.methods.push_back({"createReaderFor", "instance", "(InputStream, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- AsyncUpdater ---
    {
        ApiClass cls;
        cls.name = "AsyncUpdater";
        cls.methods.push_back({"cancelPendingUpdate", "instance", "()", 1});
        cls.methods.push_back({"handleAsyncUpdate", "instance", "()", 1});
        cls.methods.push_back({"handleUpdateNowIfNeeded", "instance", "()", 1});
        cls.methods.push_back({"isUpdatePending", "instance", "()", 1});
        cls.methods.push_back({"triggerAsyncUpdate", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- AttributedString ---
    {
        ApiClass cls;
        cls.name = "AttributedString";
        cls.methods.push_back({"append", "instance", "(AttributedString)", 1});
        cls.methods.push_back({"append", "instance", "(AttributedString)", 2});
        cls.methods.push_back({"append", "instance", "(AttributedString)", 3});
        cls.methods.push_back({"append", "instance", "(AttributedString)", 4});
        cls.methods.push_back({"append", "instance", "(AttributedString)", 5});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"getAttribute", "instance", "()", 1});
        cls.methods.push_back({"getJustification", "instance", "()", 1});
        cls.methods.push_back({"getLineSpacing", "instance", "()", 1});
        cls.methods.push_back({"getNumAttributes", "instance", "()", 1});
        cls.methods.push_back({"getReadingDirection", "instance", "()", 1});
        cls.methods.push_back({"getText", "instance", "()", 1});
        cls.methods.push_back({"getWordWrap", "instance", "()", 1});
        cls.methods.push_back({"setColour", "instance", "(Colour)", 1});
        cls.methods.push_back({"setColour", "instance", "(Colour)", 2});
        cls.methods.push_back({"setFont", "instance", "(Font)", 1});
        cls.methods.push_back({"setFont", "instance", "(Font)", 2});
        cls.methods.push_back({"setJustification", "instance", "(Justification)", 1});
        cls.methods.push_back({"setLineSpacing", "instance", "(float)", 1});
        cls.methods.push_back({"setReadingDirection", "instance", "(ReadingDirection)", 1});
        cls.methods.push_back({"setText", "instance", "(String)", 1});
        cls.methods.push_back({"setWordWrap", "instance", "(WordWrap)", 1});
        classMap[cls.name] = cls;
    }

    // --- AudioFormat ---
    {
        ApiClass cls;
        cls.name = "AudioFormat";
        cls.methods.push_back({"canDoMono", "instance", "()", 1});
        cls.methods.push_back({"canDoMono", "instance", "()", 2});
        cls.methods.push_back({"canDoStereo", "instance", "()", 1});
        cls.methods.push_back({"canHandleFile", "instance", "(File)", 1});
        cls.methods.push_back({"getFileExtensions", "instance", "()", 1});
        cls.methods.push_back({"getFormatName", "instance", "()", 1});
        cls.methods.push_back({"getQualityOptions", "instance", "()", 1});
        cls.methods.push_back({"isCompressed", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- AudioFormatReader ---
    {
        ApiClass cls;
        cls.name = "AudioFormatReader";
        cls.methods.push_back({"getFormatName", "instance", "()", 1});
        cls.methods.push_back({"read", "instance", "(AudioBuffer<float>, int, int, int64, bool, bool)", 1});
        cls.methods.push_back({"read", "instance", "(AudioBuffer<float>, int, int, int64, bool, bool)", 2});
        cls.methods.push_back({"readMaxLevels", "instance", "(int64, int64, float, float, float, float)", 1});
        cls.methods.push_back({"searchForLevel", "instance", "(int64, int64, double, double, int)", 1});
        classMap[cls.name] = cls;
    }

    // --- AudioFormatWriter ---
    {
        ApiClass cls;
        cls.name = "AudioFormatWriter";
        cls.methods.push_back({"getBitsPerSample", "instance", "()", 1});
        cls.methods.push_back({"getFormatName", "instance", "()", 1});
        cls.methods.push_back({"getNumChannels", "instance", "()", 1});
        cls.methods.push_back({"getSampleRate", "instance", "()", 1});
        cls.methods.push_back({"isFloatingPoint", "instance", "()", 1});
        cls.methods.push_back({"writeFromAudioReader", "instance", "(AudioFormatReader, int64, int64)", 1});
        cls.methods.push_back({"writeFromAudioSampleBuffer", "instance", "(AudioBuffer<float>, int, int)", 1});
        cls.methods.push_back({"writeFromAudioSource", "instance", "(AudioSource, int, int)", 1});
        classMap[cls.name] = cls;
    }

    // --- AudioPlayHead ---
    {
        ApiClass cls;
        cls.name = "AudioPlayHead";
        cls.methods.push_back({"getCurrentPosition", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- AudioSampleBuffer ---
    {
        ApiClass cls;
        cls.name = "AudioSampleBuffer";
        cls.methods.push_back({"addFrom", "instance", "()", 1});
        cls.methods.push_back({"addFrom", "instance", "()", 2});
        cls.methods.push_back({"addFromWithRamp", "instance", "()", 1});
        cls.methods.push_back({"applyGain", "instance", "()", 1});
        cls.methods.push_back({"applyGain", "instance", "()", 2});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"clear", "instance", "()", 2});
        cls.methods.push_back({"clear", "instance", "()", 3});
        cls.methods.push_back({"copyFrom", "instance", "()", 1});
        cls.methods.push_back({"copyFrom", "instance", "()", 2});
        cls.methods.push_back({"copyFrom", "instance", "()", 3});
        cls.methods.push_back({"copyFromWithRamp", "instance", "()", 1});
        cls.methods.push_back({"findMinMax", "instance", "()", 1});
        cls.methods.push_back({"getArrayOfChannels", "instance", "()", 1});
        cls.methods.push_back({"getArrayOfWritePointers", "instance", "()", 1});
        cls.methods.push_back({"getMagnitude", "instance", "()", 1});
        cls.methods.push_back({"getMagnitude", "instance", "()", 2});
        cls.methods.push_back({"getNumChannels", "instance", "()", 1});
        cls.methods.push_back({"getNumSamples", "instance", "()", 1});
        cls.methods.push_back({"getRMSLevel", "instance", "()", 1});
        cls.methods.push_back({"setSize", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- AudioThumbnail ---
    {
        ApiClass cls;
        cls.name = "AudioThumbnail";
        cls.methods.push_back({"addBlock", "instance", "(int64, AudioBuffer<float>, int, int)", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"drawChannel", "instance", "(Graphics, Rectangle<int>, double, double, int, float)", 1});
        cls.methods.push_back({"drawChannels", "instance", "(Graphics, Rectangle<int>, double, double, float)", 1});
        cls.methods.push_back({"getApproximateMinMax", "instance", "(double, double, int, float, float)", 1});
        cls.methods.push_back({"getApproximatePeak", "instance", "()", 1});
        cls.methods.push_back({"getHashCode", "instance", "()", 1});
        cls.methods.push_back({"getNumChannels", "instance", "()", 1});
        cls.methods.push_back({"getNumSamplesFinished", "instance", "()", 1});
        cls.methods.push_back({"getProportionComplete", "instance", "()", 1});
        cls.methods.push_back({"getTotalLength", "instance", "()", 1});
        cls.methods.push_back({"isFullyLoaded", "instance", "()", 1});
        cls.methods.push_back({"loadFrom", "instance", "(InputStream)", 1});
        cls.methods.push_back({"reset", "instance", "(int, double, int64)", 1});
        cls.methods.push_back({"saveTo", "instance", "(OutputStream)", 1});
        cls.methods.push_back({"setReader", "instance", "(AudioFormatReader, int64)", 1});
        cls.methods.push_back({"setSource", "instance", "(InputSource)", 1});
        classMap[cls.name] = cls;
    }

    // --- AudtioThumbnailCache ---
    {
        ApiClass cls;
        cls.name = "AudtioThumbnailCache";
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"getTimeSliceThread", "instance", "()", 1});
        cls.methods.push_back({"loadThumb", "instance", "(AudioThumbnailBase, int64)", 1});
        cls.methods.push_back({"readFromStream", "instance", "(InputStream)", 1});
        cls.methods.push_back({"removeThumb", "instance", "(int64)", 1});
        cls.methods.push_back({"storeThumb", "instance", "(AudioThumbnailBase, int64)", 1});
        cls.methods.push_back({"writeToStream", "instance", "(OutputStream)", 1});
        classMap[cls.name] = cls;
    }

    // --- BigInteger ---
    {
        ApiClass cls;
        cls.name = "BigInteger";
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"clearBit", "instance", "()", 1});
        cls.methods.push_back({"compare", "instance", "()", 1});
        cls.methods.push_back({"compareAbsolute", "instance", "()", 1});
        cls.methods.push_back({"countNumberOfSetBits", "instance", "()", 1});
        cls.methods.push_back({"divideBy", "instance", "()", 1});
        cls.methods.push_back({"exponentModulo", "instance", "()", 1});
        cls.methods.push_back({"findGreatestCommonDivisor", "instance", "()", 1});
        cls.methods.push_back({"findNextClearBit", "instance", "()", 1});
        cls.methods.push_back({"findNextSetBit", "instance", "()", 1});
        cls.methods.push_back({"getBitRange", "instance", "()", 1});
        cls.methods.push_back({"getBitRangeAsInt", "instance", "()", 1});
        cls.methods.push_back({"getHighestBit", "instance", "()", 1});
        cls.methods.push_back({"insertBit", "instance", "()", 1});
        cls.methods.push_back({"inverseModulo", "instance", "()", 1});
        cls.methods.push_back({"isNegative", "instance", "()", 1});
        cls.methods.push_back({"isOne", "instance", "()", 1});
        cls.methods.push_back({"isZero", "instance", "()", 1});
        cls.methods.push_back({"loadFromMemoryBlock", "instance", "()", 1});
        cls.methods.push_back({"negate", "instance", "()", 1});
        cls.methods.push_back({"parseString", "instance", "()", 1});
        cls.methods.push_back({"setBit", "instance", "()", 1});
        cls.methods.push_back({"setBit", "instance", "()", 2});
        cls.methods.push_back({"setBitRangeAsInt", "instance", "()", 1});
        cls.methods.push_back({"setNegative", "instance", "()", 1});
        cls.methods.push_back({"setRange", "instance", "()", 1});
        cls.methods.push_back({"shiftBits", "instance", "()", 1});
        cls.methods.push_back({"swapWith", "instance", "()", 1});
        cls.methods.push_back({"toInteger", "instance", "()", 1});
        cls.methods.push_back({"toMemoryBlock", "instance", "()", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Button ---
    {
        ApiClass cls;
        cls.name = "Button";
        cls.methods.push_back({"addListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"addShortcut", "instance", "(KeyPress)", 1});
        cls.methods.push_back({"clearShortcuts", "instance", "()", 1});
        cls.methods.push_back({"getButtonText", "instance", "()", 1});
        cls.methods.push_back({"getClickingTogglesState", "instance", "()", 1});
        cls.methods.push_back({"getCommandID", "instance", "()", 1});
        cls.methods.push_back({"getConnectedEdgeFlags", "instance", "()", 1});
        cls.methods.push_back({"getMillisecondsSinceButtonDown", "instance", "()", 1});
        cls.methods.push_back({"getRadioGroupId", "instance", "()", 1});
        cls.methods.push_back({"getToggleState", "instance", "()", 1});
        cls.methods.push_back({"getToggleStateValue", "instance", "()", 1});
        cls.methods.push_back({"getTooltip", "instance", "()", 1});
        cls.methods.push_back({"isConnectedOnBottom", "instance", "()", 1});
        cls.methods.push_back({"isConnectedOnLeft", "instance", "()", 1});
        cls.methods.push_back({"isConnectedOnRight", "instance", "()", 1});
        cls.methods.push_back({"isConnectedOnTop", "instance", "()", 1});
        cls.methods.push_back({"isDown", "instance", "()", 1});
        cls.methods.push_back({"isOver", "instance", "()", 1});
        cls.methods.push_back({"isRegisteredForShortcut", "instance", "(KeyPress)", 1});
        cls.methods.push_back({"removeListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"setButtonText", "instance", "(String)", 1});
        cls.methods.push_back({"setClickingTogglesState", "instance", "(bool)", 1});
        cls.methods.push_back({"setCommandToTrigger", "instance", "()", 1});
        cls.methods.push_back({"setConnectedEdges", "instance", "(int)", 1});
        cls.methods.push_back({"setRadioGroupId", "instance", "(int, NotificationType)", 1});
        cls.methods.push_back({"setRepeatSpeed", "instance", "()", 1});
        cls.methods.push_back({"setState", "instance", "(ButtonState)", 1});
        cls.methods.push_back({"setToggleState", "instance", "(bool, bool)", 1});
        cls.methods.push_back({"setTooltip", "instance", "(String)", 1});
        cls.methods.push_back({"setTriggeredOnMouseDown", "instance", "()", 1});
        cls.methods.push_back({"triggerClick", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CharPointer_UTF8 ---
    {
        ApiClass cls;
        cls.name = "CharPointer_UTF8";
        cls.methods.push_back({"getAddress", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Colour ---
    {
        ApiClass cls;
        cls.name = "Colour";
        cls.methods.push_back({"brighter", "instance", "(float)", 1});
        cls.methods.push_back({"darker", "instance", "(float)", 1});
        cls.methods.push_back({"getARGB", "instance", "()", 1});
        cls.methods.push_back({"getAlpha", "instance", "()", 1});
        cls.methods.push_back({"getBlue", "instance", "()", 1});
        cls.methods.push_back({"getFloatAlpha", "instance", "()", 1});
        cls.methods.push_back({"getFloatBlue", "instance", "()", 1});
        cls.methods.push_back({"getFloatGreen", "instance", "()", 1});
        cls.methods.push_back({"getFloatRed", "instance", "()", 1});
        cls.methods.push_back({"getGreen", "instance", "()", 1});
        cls.methods.push_back({"getHue", "instance", "()", 1});
        cls.methods.push_back({"getRed", "instance", "()", 1});
        cls.methods.push_back({"getSaturation", "instance", "()", 1});
        cls.methods.push_back({"interpolatedWith", "instance", "(Colour, float)", 1});
        cls.methods.push_back({"isOpaque", "instance", "()", 1});
        cls.methods.push_back({"isTransparent", "instance", "()", 1});
        cls.methods.push_back({"overlaidWith", "instance", "(Colour)", 1});
        cls.methods.push_back({"toDisplayString", "instance", "(bool)", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        cls.methods.push_back({"withAlpha", "instance", "(float)", 1});
        cls.methods.push_back({"withBrightness", "instance", "(float)", 1});
        cls.methods.push_back({"withHue", "instance", "(float)", 1});
        cls.methods.push_back({"withMultipliedAlpha", "instance", "(float)", 1});
        cls.methods.push_back({"withMultipliedBrightness", "instance", "(float)", 1});
        cls.methods.push_back({"withMultipliedSaturation", "instance", "(float)", 1});
        cls.methods.push_back({"withRotatedHue", "instance", "(float)", 1});
        cls.methods.push_back({"withSaturation", "instance", "(float)", 1});
        cls.staticMethods.push_back({"contrasting", "static", "(Colour, Colour)", 1});
        cls.staticMethods.push_back({"fromFloatRGBA", "static", "(float, float, float, float)", 1});
        cls.staticMethods.push_back({"fromHSV", "static", "(float, float, float, float)", 1});
        cls.staticMethods.push_back({"fromRGB", "static", "(uint8, uint8, uint8)", 1});
        cls.staticMethods.push_back({"fromRGBA", "static", "(uint8, uint8, uint8, uint8)", 1});
        cls.staticMethods.push_back({"fromString", "static", "(x)", 1});
        cls.staticMethods.push_back({"greyLevel", "static", "(float)", 1});
        classMap[cls.name] = cls;
    }

    // --- ColourGradient ---
    {
        ApiClass cls;
        cls.name = "ColourGradient";
        cls.methods.push_back({"addColour", "instance", "(double, Colour)", 1});
        cls.methods.push_back({"clearColours", "instance", "()", 1});
        cls.methods.push_back({"getColour", "instance", "(int)", 1});
        cls.methods.push_back({"getColourAtPosition", "instance", "(double)", 1});
        cls.methods.push_back({"getColourPosition", "instance", "(int)", 1});
        cls.methods.push_back({"getNumColours", "instance", "()", 1});
        cls.methods.push_back({"isInvisible", "instance", "()", 1});
        cls.methods.push_back({"isOpaque", "instance", "()", 1});
        cls.methods.push_back({"multiplyOpacity", "instance", "(float)", 1});
        cls.methods.push_back({"removeColour", "instance", "(int)", 1});
        cls.methods.push_back({"setColour", "instance", "(int, Colour)", 1});
        classMap[cls.name] = cls;
    }

    // --- ComboBox ---
    {
        ApiClass cls;
        cls.name = "ComboBox";
        cls.methods.push_back({"addItem", "instance", "(String, int)", 1});
        cls.methods.push_back({"addItemList", "instance", "(StringArray, int)", 1});
        cls.methods.push_back({"addListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"addSectionHeading", "instance", "(String)", 1});
        cls.methods.push_back({"addSeparator", "instance", "()", 1});
        cls.methods.push_back({"changeItemText", "instance", "(int, String)", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"getItemId", "instance", "(int)", 1});
        cls.methods.push_back({"getItemText", "instance", "(int)", 1});
        cls.methods.push_back({"getJustificationType", "instance", "()", 1});
        cls.methods.push_back({"getNumItems", "instance", "()", 1});
        cls.methods.push_back({"getSelectedId", "instance", "()", 1});
        cls.methods.push_back({"getSelectedIdAsValue", "instance", "()", 1});
        cls.methods.push_back({"getSelectedItemIndex", "instance", "()", 1});
        cls.methods.push_back({"getText", "instance", "()", 1});
        cls.methods.push_back({"getTextWhenNothingSelected", "instance", "()", 1});
        cls.methods.push_back({"indexOfItemId", "instance", "(int)", 1});
        cls.methods.push_back({"isItemEnabled", "instance", "(int)", 1});
        cls.methods.push_back({"isTextEditable", "instance", "()", 1});
        cls.methods.push_back({"removeListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"setEditableText", "instance", "(bool)", 1});
        cls.methods.push_back({"setItemEnabled", "instance", "(int, bool)", 1});
        cls.methods.push_back({"setJustificationType", "instance", "(Justification)", 1});
        cls.methods.push_back({"setSelectedId", "instance", "(int, bool)", 1});
        cls.methods.push_back({"setSelectedItemIndex", "instance", "(int, bool)", 1});
        cls.methods.push_back({"setText", "instance", "(String, bool)", 1});
        cls.methods.push_back({"setTextWhenNoChoicesAvailable", "instance", "(String)", 1});
        cls.methods.push_back({"setTextWhenNothingSelected", "instance", "(String)", 1});
        cls.methods.push_back({"setTooltip", "instance", "(String)", 1});
        cls.methods.push_back({"showEditor", "instance", "()", 1});
        cls.methods.push_back({"showPopup", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Component ---
    {
        ApiClass cls;
        cls.name = "Component";
        cls.methods.push_back({"addAndMakeVisible", "instance", "(child, zOrder)", 1});
        cls.methods.push_back({"addAndMakeVisible", "instance", "(child, zOrder)", 2});
        cls.methods.push_back({"addChildAndSetID", "instance", "(Component, String)", 1});
        cls.methods.push_back({"addChildComponent", "instance", "(child, zOrder)", 1});
        cls.methods.push_back({"addChildComponent", "instance", "(child, zOrder)", 2});
        cls.methods.push_back({"addComponentListener", "instance", "()", 1});
        cls.methods.push_back({"addKeyListener", "instance", "()", 1});
        cls.methods.push_back({"addMouseListener", "instance", "(MouseListener, bool)", 1});
        cls.methods.push_back({"addToDesktop", "instance", "(windowStyleFlags, nullptr)", 1});
        cls.methods.push_back({"broughtToFront", "instance", "()", 1});
        cls.methods.push_back({"canModalEventBeSentToComponent", "instance", "(Component)", 1});
        cls.methods.push_back({"centreWithSize", "instance", "(int, int)", 1});
        cls.methods.push_back({"childBoundsChanged", "instance", "(Component)", 1});
        cls.methods.push_back({"childrenChanged", "instance", "()", 1});
        cls.methods.push_back({"colourChanged", "instance", "()", 1});
        cls.methods.push_back({"contains", "instance", "(Point<int>)", 1});
        cls.methods.push_back({"copyAllExplicitColoursTo", "instance", "(Component)", 1});
        cls.methods.push_back({"createComponentSnapshot", "instance", "(Rectangle<int>, bool, float)", 1});
        cls.methods.push_back({"createFocusTraverser", "instance", "()", 1});
        cls.methods.push_back({"deleteAllChildren", "instance", "()", 1});
        cls.methods.push_back({"enablementChanged", "instance", "()", 1});
        cls.methods.push_back({"enterModalState", "instance", "()", 1});
        cls.methods.push_back({"exitModalState", "instance", "(int)", 1});
        cls.methods.push_back({"findChildWithID", "instance", "(StringRef)", 1});
        cls.methods.push_back({"findColour", "instance", "(textColourId)", 1});
        cls.methods.push_back({"focusGained", "instance", "(cause)", 1});
        cls.methods.push_back({"focusLost", "instance", "(cause)", 1});
        cls.methods.push_back({"focusOfChildComponentChanged", "instance", "(FocusChangeType)", 1});
        cls.methods.push_back({"getAlpha", "instance", "()", 1});
        cls.methods.push_back({"getBottom", "instance", "()", 1});
        cls.methods.push_back({"getBounds", "instance", "()", 1});
        cls.methods.push_back({"getBoundsInParent", "instance", "()", 1});
        cls.methods.push_back({"getCachedComponentImage", "instance", "()", 1});
        cls.methods.push_back({"getChildComponent", "instance", "(int)", 1});
        cls.methods.push_back({"getComponentAt", "instance", "(int, int)", 1});
        cls.methods.push_back({"getComponentAt", "instance", "(int, int)", 2});
        cls.methods.push_back({"getComponentEffect", "instance", "()", 1});
        cls.methods.push_back({"getComponentID", "instance", "()", 1});
        cls.methods.push_back({"getExplicitFocusOrder", "instance", "()", 1});
        cls.methods.push_back({"getHeight", "instance", "()", 1});
        cls.methods.push_back({"getIndexOfChildComponent", "instance", "(Component)", 1});
        cls.methods.push_back({"getInterceptsMouseClicks", "instance", "(bool, bool)", 1});
        cls.methods.push_back({"getLocalBounds", "instance", "()", 1});
        cls.methods.push_back({"getLocalPoint", "instance", "(Component, Point<float>)", 1});
        cls.methods.push_back({"getLookAndFeel", "instance", "()", 1});
        cls.methods.push_back({"getMouseClickGrabsKeyboardFocus", "instance", "()", 1});
        cls.methods.push_back({"getMouseCursor", "instance", "()", 1});
        cls.methods.push_back({"getMouseXYRelative", "instance", "()", 1});
        cls.methods.push_back({"getNumChildComponents", "instance", "()", 1});
        cls.methods.push_back({"getParentComponent", "instance", "()", 1});
        cls.methods.push_back({"getParentHeight", "instance", "()", 1});
        cls.methods.push_back({"getParentMonitorArea", "instance", "()", 1});
        cls.methods.push_back({"getParentWidth", "instance", "()", 1});
        cls.methods.push_back({"getPeer", "instance", "()", 1});
        cls.methods.push_back({"getPosition", "instance", "()", 1});
        cls.methods.push_back({"getPositioner", "instance", "()", 1});
        cls.methods.push_back({"getProperties", "instance", "()", 1});
        cls.methods.push_back({"getProperties", "instance", "()", 2});
        cls.methods.push_back({"getRight", "instance", "()", 1});
        cls.methods.push_back({"getScreenBounds", "instance", "()", 1});
        cls.methods.push_back({"getScreenPosition", "instance", "()", 1});
        cls.methods.push_back({"getScreenX", "instance", "()", 1});
        cls.methods.push_back({"getScreenY", "instance", "()", 1});
        cls.methods.push_back({"getTopLevelComponent", "instance", "()", 1});
        cls.methods.push_back({"getTransform", "instance", "()", 1});
        cls.methods.push_back({"getWantsKeyboardFocus", "instance", "()", 1});
        cls.methods.push_back({"getWidth", "instance", "()", 1});
        cls.methods.push_back({"getWindowHandle", "instance", "()", 1});
        cls.methods.push_back({"getX", "instance", "()", 1});
        cls.methods.push_back({"getY", "instance", "()", 1});
        cls.methods.push_back({"grabKeyboardFocus", "instance", "()", 1});
        cls.methods.push_back({"handleCommandMessage", "instance", "(commandId)", 1});
        cls.methods.push_back({"hasKeyboardFocus", "instance", "(bool)", 1});
        cls.methods.push_back({"hitTest", "instance", "(int, int)", 1});
        cls.methods.push_back({"inputAttemptWhenModal", "instance", "()", 1});
        cls.methods.push_back({"isAlwaysOnTop", "instance", "()", 1});
        cls.methods.push_back({"isBroughtToFrontOnMouseClick", "instance", "()", 1});
        cls.methods.push_back({"isColourSpecified", "instance", "(int)", 1});
        cls.methods.push_back({"isCurrentlyBlockedByAnotherModalComponent", "instance", "()", 1});
        cls.methods.push_back({"isCurrentlyModal", "instance", "(bool)", 1});
        cls.methods.push_back({"isEnabled", "instance", "()", 1});
        cls.methods.push_back({"isFocusContainer", "instance", "()", 1});
        cls.methods.push_back({"isMouseButtonDown", "instance", "(bool)", 1});
        cls.methods.push_back({"isMouseOver", "instance", "(bool)", 1});
        cls.methods.push_back({"isMouseOverOrDragging", "instance", "(bool)", 1});
        cls.methods.push_back({"isOnDesktop", "instance", "()", 1});
        cls.methods.push_back({"isOpaque", "instance", "()", 1});
        cls.methods.push_back({"isParentOf", "instance", "(Component)", 1});
        cls.methods.push_back({"isShowing", "instance", "()", 1});
        cls.methods.push_back({"isTransformed", "instance", "()", 1});
        cls.methods.push_back({"isVisible", "instance", "()", 1});
        cls.methods.push_back({"keyPressed", "instance", "(KeyPress)", 1});
        cls.methods.push_back({"keyStateChanged", "instance", "(bool)", 1});
        cls.methods.push_back({"localPointToGlobal", "instance", "(Point<float>)", 1});
        cls.methods.push_back({"lookAndFeelChanged", "instance", "()", 1});
        cls.methods.push_back({"minimisationStateChanged", "instance", "(bool)", 1});
        cls.methods.push_back({"modifierKeysChanged", "instance", "(ModifierKeys)", 1});
        cls.methods.push_back({"mouseDoubleClick", "instance", "(MouseEvent)", 1});
        cls.methods.push_back({"mouseDown", "instance", "(MouseEvent)", 1});
        cls.methods.push_back({"mouseDrag", "instance", "(MouseEvent)", 1});
        cls.methods.push_back({"mouseEnter", "instance", "(MouseEvent)", 1});
        cls.methods.push_back({"mouseExit", "instance", "(MouseEvent)", 1});
        cls.methods.push_back({"mouseMove", "instance", "(MouseEvent)", 1});
        cls.methods.push_back({"mouseUp", "instance", "(MouseEvent)", 1});
        cls.methods.push_back({"mouseWheelMove", "instance", "(e, wheel)", 1});
        cls.methods.push_back({"moveKeyboardFocusToSibling", "instance", "(bool)", 1});
        cls.methods.push_back({"moved", "instance", "()", 1});
        cls.methods.push_back({"paint", "instance", "()", 1});
        cls.methods.push_back({"paintEntireComponent", "instance", "(Graphics, bool)", 1});
        cls.methods.push_back({"paintOverChildren", "instance", "(Graphics)", 1});
        cls.methods.push_back({"parentHierarchyChanged", "instance", "()", 1});
        cls.methods.push_back({"parentSizeChanged", "instance", "()", 1});
        cls.methods.push_back({"postCommandMessage", "instance", "(int)", 1});
        cls.methods.push_back({"proportionOfHeight", "instance", "(float)", 1});
        cls.methods.push_back({"proportionOfWidth", "instance", "(float)", 1});
        cls.methods.push_back({"reallyContains", "instance", "(Point<int>, bool)", 1});
        cls.methods.push_back({"removeAllChildren", "instance", "()", 1});
        cls.methods.push_back({"removeChildComponent", "instance", "(int, bool, bool)", 1});
        cls.methods.push_back({"removeChildComponent", "instance", "(int, bool, bool)", 2});
        cls.methods.push_back({"removeColour", "instance", "(int)", 1});
        cls.methods.push_back({"removeComponentListener", "instance", "(ComponentListener)", 1});
        cls.methods.push_back({"removeFromDesktop", "instance", "()", 1});
        cls.methods.push_back({"removeKeyListener", "instance", "(KeyListener)", 1});
        cls.methods.push_back({"removeMouseListener", "instance", "(MouseListener)", 1});
        cls.methods.push_back({"repaint", "instance", "()", 1});
        cls.methods.push_back({"repaint", "instance", "()", 2});
        cls.methods.push_back({"resized", "instance", "()", 1});
        cls.methods.push_back({"sendLookAndFeelChange", "instance", "()", 1});
        cls.methods.push_back({"setAlpha", "instance", "(float)", 1});
        cls.methods.push_back({"setAlwaysOnTop", "instance", "(bool)", 1});
        cls.methods.push_back({"setBounds", "instance", "(RelativeRectangle)", 1});
        cls.methods.push_back({"setBounds", "instance", "(RelativeRectangle)", 2});
        cls.methods.push_back({"setBoundsInset", "instance", "(BorderSize<int>)", 1});
        cls.methods.push_back({"setBoundsRelative", "instance", "(float, float, float, float)", 1});
        cls.methods.push_back({"setBoundsRelative", "instance", "(float, float, float, float)", 2});
        cls.methods.push_back({"setBoundsToFit", "instance", "(Rectangle<int>, Justification, bool)", 1});
        cls.methods.push_back({"setBroughtToFrontOnMouseClick", "instance", "(bool)", 1});
        cls.methods.push_back({"setBufferedToImage", "instance", "(bool)", 1});
        cls.methods.push_back({"setCachedComponentImage", "instance", "(CachedComponentImage)", 1});
        cls.methods.push_back({"setCentreRelative", "instance", "(float, float)", 1});
        cls.methods.push_back({"setColour", "instance", "()", 1});
        cls.methods.push_back({"setComponentEffect", "instance", "(ImageEffectFilter)", 1});
        cls.methods.push_back({"setComponentID", "instance", "()", 1});
        cls.methods.push_back({"setEnabled", "instance", "(bool)", 1});
        cls.methods.push_back({"setExplicitFocusOrder", "instance", "()", 1});
        cls.methods.push_back({"setFocusContainer", "instance", "(bool)", 1});
        cls.methods.push_back({"setInterceptsMouseClicks", "instance", "()", 1});
        cls.methods.push_back({"setLookAndFeel", "instance", "(LookAndFeel)", 1});
        cls.methods.push_back({"setMouseClickGrabsKeyboardFocus", "instance", "(bool)", 1});
        cls.methods.push_back({"setMouseCursor", "instance", "(MouseCursor)", 1});
        cls.methods.push_back({"setName", "instance", "(owner.getPanelWindowTitle()", 1});
        cls.methods.push_back({"setOpaque", "instance", "(bool)", 1});
        cls.methods.push_back({"setPaintingIsUnclipped", "instance", "(bool)", 1});
        cls.methods.push_back({"setPositioner", "instance", "(Positioner)", 1});
        cls.methods.push_back({"setRepaintsOnMouseActivity", "instance", "(bool)", 1});
        cls.methods.push_back({"setSize", "instance", "(int, int)", 1});
        cls.methods.push_back({"setTopLeftPosition", "instance", "(Point<int>)", 1});
        cls.methods.push_back({"setTopLeftPosition", "instance", "(Point<int>)", 2});
        cls.methods.push_back({"setTopRightPosition", "instance", "(int, int)", 1});
        cls.methods.push_back({"setTransform", "instance", "(AffineTransform)", 1});
        cls.methods.push_back({"setVisible", "instance", "(shouldBeVisible)", 1});
        cls.methods.push_back({"setWantsKeyboardFocus", "instance", "(bool)", 1});
        cls.methods.push_back({"toBack", "instance", "()", 1});
        cls.methods.push_back({"toBehind", "instance", "(Component)", 1});
        cls.methods.push_back({"toFront", "instance", "(bool)", 1});
        cls.methods.push_back({"updateMouseCursor", "instance", "()", 1});
        cls.methods.push_back({"userTriedToCloseWindow", "instance", "()", 1});
        cls.methods.push_back({"visibilityChanged", "instance", "()", 1});
        cls.staticMethods.push_back({"beginDragAutoRepeat", "static", "(milisecondsBetweenCallbacks)", 1});
        cls.staticMethods.push_back({"getCurrentlyFocusedComponent", "static", "()", 1});
        cls.staticMethods.push_back({"getCurrentlyModalComponent", "static", "()", 1});
        cls.staticMethods.push_back({"getNumCurrentlyModalComponents", "static", "()", 1});
        cls.staticMethods.push_back({"isMouseButtonDownAnywhere", "static", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- ComponentPeer ---
    {
        ApiClass cls;
        cls.name = "ComponentPeer";
        classMap[cls.name] = cls;
    }

    // --- CriticalSection ---
    {
        ApiClass cls;
        cls.name = "CriticalSection";
        cls.methods.push_back({"enter", "instance", "()", 1});
        cls.methods.push_back({"exit", "instance", "()", 1});
        cls.methods.push_back({"tryEnter", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrButton ---
    {
        ApiClass cls;
        cls.name = "CtrlrButton";
        cls.methods.push_back({"getToggleState", "instance", "()", 1});
        cls.methods.push_back({"getValueMap", "instance", "()", 1});
        cls.methods.push_back({"isToggleButton", "instance", "()", 1});
        cls.methods.push_back({"setToggleState", "instance", "(bool, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrCombo ---
    {
        ApiClass cls;
        cls.name = "CtrlrCombo";
        cls.methods.push_back({"getOwnedComboBox", "instance", "()", 1});
        cls.methods.push_back({"getSelectedId", "instance", "()", 1});
        cls.methods.push_back({"getSelectedItemIndex", "instance", "()", 1});
        cls.methods.push_back({"getText", "instance", "()", 1});
        cls.methods.push_back({"getValueMap", "instance", "()", 1});
        cls.methods.push_back({"setSelectedId", "instance", "(int, bool)", 1});
        cls.methods.push_back({"setSelectedItemIndex", "instance", "(int, bool)", 1});
        cls.methods.push_back({"setText", "instance", "(String, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrComponent ---
    {
        ApiClass cls;
        cls.name = "CtrlrComponent";
        cls.methods.push_back({"click", "instance", "()", 1});
        cls.methods.push_back({"getBounds", "instance", "()", 1});
        cls.methods.push_back({"getComponentMidiValue", "instance", "()", 1});
        cls.methods.push_back({"getComponentText", "instance", "()", 1});
        cls.methods.push_back({"getComponentValue", "instance", "()", 1});
        cls.methods.push_back({"getHeight", "instance", "()", 1});
        cls.methods.push_back({"getLuaBounds", "instance", "()", 1});
        cls.methods.push_back({"getMaximum", "instance", "()", 1});
        cls.methods.push_back({"getMidiValue", "instance", "()", 1});
        cls.methods.push_back({"getMinimum", "instance", "()", 1});
        cls.methods.push_back({"getOwner", "instance", "()", 1});
        cls.methods.push_back({"getRect", "instance", "()", 1});
        cls.methods.push_back({"getTextForValue", "instance", "(double)", 1});
        cls.methods.push_back({"getTransform", "instance", "()", 1});
        cls.methods.push_back({"getValue", "instance", "()", 1});
        cls.methods.push_back({"getWidth", "instance", "()", 1});
        cls.methods.push_back({"getX", "instance", "()", 1});
        cls.methods.push_back({"getY", "instance", "()", 1});
        cls.methods.push_back({"isMouseButtonDown", "instance", "()", 1});
        cls.methods.push_back({"isMouseOver", "instance", "()", 1});
        cls.methods.push_back({"isMouseOverOrDragging", "instance", "()", 1});
        cls.methods.push_back({"isVisible", "instance", "()", 1});
        cls.methods.push_back({"keyPressed", "instance", "()", 1});
        cls.methods.push_back({"mouseEnter", "instance", "(MouseEvent)", 1});
        cls.methods.push_back({"mouseExit", "instance", "(MouseEvent)", 1});
        cls.methods.push_back({"mouseUp", "instance", "(MouseEvent)", 1});
        cls.methods.push_back({"repaint", "instance", "()", 1});
        cls.methods.push_back({"setBounds", "instance", "()", 1});
        cls.methods.push_back({"setBounds", "instance", "()", 2});
        cls.methods.push_back({"setComponentMidiValue", "instance", "(int, bool)", 1});
        cls.methods.push_back({"setComponentText", "instance", "()", 1});
        cls.methods.push_back({"setComponentValue", "instance", "()", 1});
        cls.methods.push_back({"setCustomLookAndFeel", "instance", "(table)", 1});
        cls.methods.push_back({"setMidiValue", "instance", "()", 1});
        cls.methods.push_back({"setSize", "instance", "()", 1});
        cls.methods.push_back({"setTransform", "instance", "()", 1});
        cls.methods.push_back({"setValue", "instance", "()", 1});
        cls.methods.push_back({"setVisible", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrFileListBox ---
    {
        ApiClass cls;
        cls.name = "CtrlrFileListBox";
        cls.methods.push_back({"deselectAllFiles", "instance", "()", 1});
        cls.methods.push_back({"getNumSelectedFiles", "instance", "()", 1});
        cls.methods.push_back({"getSelectedFile", "instance", "(int)", 1});
        cls.methods.push_back({"refresh", "instance", "()", 1});
        cls.methods.push_back({"scrollToTop", "instance", "()", 1});
        cls.methods.push_back({"setSelectedFile", "instance", "(File)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrFixedImageSlider ---
    {
        ApiClass cls;
        cls.name = "CtrlrFixedImageSlider";
        cls.methods.push_back({"getOwnedSlider", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrFixedSlider ---
    {
        ApiClass cls;
        cls.name = "CtrlrFixedSlider";
        cls.methods.push_back({"getOwnedSlider", "instance", "()", 1});
        cls.methods.push_back({"getValueMap", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrImageButton ---
    {
        ApiClass cls;
        cls.name = "CtrlrImageButton";
        cls.methods.push_back({"getToggleState", "instance", "()", 1});
        cls.methods.push_back({"getValueMap", "instance", "()", 1});
        cls.methods.push_back({"isToggleButton", "instance", "()", 1});
        cls.methods.push_back({"setToggleState", "instance", "(bool, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrImageSlider ---
    {
        ApiClass cls;
        cls.name = "CtrlrImageSlider";
        cls.methods.push_back({"getOwnedSlider", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLCDLabel ---
    {
        ApiClass cls;
        cls.name = "CtrlrLCDLabel";
        cls.methods.push_back({"append", "instance", "()", 1});
        cls.methods.push_back({"appendText", "instance", "()", 1});
        cls.methods.push_back({"getText", "instance", "()", 1});
        cls.methods.push_back({"setText", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLabel ---
    {
        ApiClass cls;
        cls.name = "CtrlrLabel";
        cls.methods.push_back({"append", "instance", "()", 1});
        cls.methods.push_back({"appendText", "instance", "()", 1});
        cls.methods.push_back({"getText", "instance", "()", 1});
        cls.methods.push_back({"setText", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrListBox ---
    {
        ApiClass cls;
        cls.name = "CtrlrListBox";
        cls.methods.push_back({"deselectAllRows", "instance", "()", 1});
        cls.methods.push_back({"deselectRow", "instance", "(int)", 1});
        cls.methods.push_back({"flipRowSelection", "instance", "(int)", 1});
        cls.methods.push_back({"getLastRowSelected", "instance", "()", 1});
        cls.methods.push_back({"getNumRows", "instance", "()", 1});
        cls.methods.push_back({"getNumSelectedRows", "instance", "()", 1});
        cls.methods.push_back({"getSelectedRow", "instance", "(int)", 1});
        cls.methods.push_back({"getSelectedRows", "instance", "()", 1});
        cls.methods.push_back({"isRowSelected", "instance", "(int)", 1});
        cls.methods.push_back({"selectRangeOfRows", "instance", "(int, int)", 1});
        cls.methods.push_back({"selectRow", "instance", "(int, bool, bool)", 1});
        cls.methods.push_back({"setMultipleSelectionEnabled", "instance", "(bool)", 1});
        cls.methods.push_back({"updateContent", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLuaAudioConverter ---
    {
        ApiClass cls;
        cls.name = "CtrlrLuaAudioConverter";
        cls.methods.push_back({"convertInt16", "instance", "(MemoryBlock, AudioSampleBuffer, int, int, bool)", 1});
        cls.methods.push_back({"convertInt24", "instance", "(MemoryBlock, AudioSampleBuffer, int, int, bool)", 1});
        cls.methods.push_back({"convertInt32", "instance", "(MemoryBlock, AudioSampleBuffer, int, int, bool)", 1});
        cls.methods.push_back({"convertInt8", "instance", "(MemoryBlock, AudioSampleBuffer, int, int, bool)", 1});
        cls.methods.push_back({"convertToUInt8", "instance", "(AudioFormatReader, int, int, int, bool)", 1});
        cls.methods.push_back({"convertUInt16", "instance", "(MemoryBlock, AudioSampleBuffer, int, int, bool)", 1});
        cls.methods.push_back({"convertUInt8", "instance", "(MemoryBlock, AudioSampleBuffer, int, int, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLuaBigInteger ---
    {
        ApiClass cls;
        cls.name = "CtrlrLuaBigInteger";
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"clearBit", "instance", "(int)", 1});
        cls.methods.push_back({"clearRange", "instance", "(int, int)", 1});
        cls.methods.push_back({"getBit", "instance", "(int)", 1});
        cls.methods.push_back({"getBitRangeAsInt", "instance", "(int, int)", 1});
        cls.methods.push_back({"setBit", "instance", "(int, bool)", 1});
        cls.methods.push_back({"setBitRangeAsInt", "instance", "(int, int, int)", 1});
        cls.methods.push_back({"setRange", "instance", "(int, int)", 1});
        cls.methods.push_back({"shiftBits", "instance", "(int, int)", 1});
        cls.methods.push_back({"toString", "instance", "(int, int)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLuaComponentAnimator ---
    {
        ApiClass cls;
        cls.name = "CtrlrLuaComponentAnimator";
        cls.methods.push_back({"animateComponent", "instance", "(CtrlrComponent, CtrlrLuaRectangle, float, int, bool, double, double)", 1});
        cls.methods.push_back({"cancelAllAnimations", "instance", "(bool)", 1});
        cls.methods.push_back({"cancelAnimation", "instance", "(CtrlrComponent, bool)", 1});
        cls.methods.push_back({"fadeIn", "instance", "(CtrlrComponent, int)", 1});
        cls.methods.push_back({"fadeOut", "instance", "(CtrlrComponent, int)", 1});
        cls.methods.push_back({"getComponentDestination", "instance", "(CtrlrComponent)", 1});
        cls.methods.push_back({"isAnimating", "instance", "(CtrlrComponent)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLuaDebugger ---
    {
        ApiClass cls;
        cls.name = "CtrlrLuaDebugger";
        cls.methods.push_back({"read", "instance", "()", 1});
        cls.methods.push_back({"read", "instance", "()", 2});
        cls.methods.push_back({"write", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLuaMemoryBlock ---
    {
        ApiClass cls;
        cls.name = "CtrlrLuaMemoryBlock";
        cls.methods.push_back({"append", "instance", "(uint8)", 1});
        cls.methods.push_back({"append", "instance", "(uint8)", 2});
        cls.methods.push_back({"copyFrom", "instance", "(CtrlrLuaMemoryBlock, int, int)", 1});
        cls.methods.push_back({"copyTo", "instance", "(CtrlrLuaMemoryBlock, int, int)", 1});
        cls.methods.push_back({"getByte", "instance", "(int)", 1});
        cls.methods.push_back({"getRange", "instance", "(int, int)", 1});
        cls.methods.push_back({"getSize", "instance", "()", 1});
        cls.methods.push_back({"insert", "instance", "(CtrlrLuaMemoryBlock, int, int)", 1});
        cls.methods.push_back({"loadFromHexString", "instance", "(String)", 1});
        cls.methods.push_back({"removeSection", "instance", "(int, int)", 1});
        cls.methods.push_back({"setByte", "instance", "(unsigned, int)", 1});
        cls.methods.push_back({"toHexString", "instance", "(int)", 1});
        cls.methods.push_back({"toSafeString", "instance", "()", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLuaMultiTimer ---
    {
        ApiClass cls;
        cls.name = "CtrlrLuaMultiTimer";
        cls.methods.push_back({"getTimerInterval", "instance", "(int)", 1});
        cls.methods.push_back({"isRegistered", "instance", "(int)", 1});
        cls.methods.push_back({"isTimerRunning", "instance", "(int)", 1});
        cls.methods.push_back({"setCallback", "instance", "(int, table)", 1});
        cls.methods.push_back({"startTimer", "instance", "(int, int)", 1});
        cls.methods.push_back({"stopTimer", "instance", "(int)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLuaObject ---
    {
        ApiClass cls;
        cls.name = "CtrlrLuaObject";
        cls.methods.push_back({"getObjectTree", "instance", "()", 1});
        cls.methods.push_back({"getProperty", "instance", "()", 1});
        cls.methods.push_back({"getPropertyDouble", "instance", "(String)", 1});
        cls.methods.push_back({"getPropertyInt", "instance", "(String)", 1});
        cls.methods.push_back({"getPropertyString", "instance", "(String)", 1});
        cls.methods.push_back({"removeProperty", "instance", "(Identifier)", 1});
        cls.methods.push_back({"setProperty", "instance", "()", 1});
        cls.methods.push_back({"setPropertyColour", "instance", "(String, Colour)", 1});
        cls.methods.push_back({"setPropertyDouble", "instance", "(String, double)", 1});
        cls.methods.push_back({"setPropertyInt", "instance", "(String, int)", 1});
        cls.methods.push_back({"setPropertyString", "instance", "(String, String)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLuaObjectWrapper ---
    {
        ApiClass cls;
        cls.name = "CtrlrLuaObjectWrapper";
        cls.methods.push_back({"getObject", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLuaRectangle ---
    {
        ApiClass cls;
        cls.name = "CtrlrLuaRectangle";
        cls.methods.push_back({"contains", "instance", "(CtrlrLuaRectangle)", 1});
        cls.methods.push_back({"contains", "instance", "(CtrlrLuaRectangle)", 2});
        cls.methods.push_back({"enlargeIfAdjacent", "instance", "(CtrlrLuaRectangle)", 1});
        cls.methods.push_back({"expand", "instance", "(int, int)", 1});
        cls.methods.push_back({"getAspectRatio", "instance", "()", 1});
        cls.methods.push_back({"getBottom", "instance", "()", 1});
        cls.methods.push_back({"getCentreX", "instance", "()", 1});
        cls.methods.push_back({"getCentreY", "instance", "()", 1});
        cls.methods.push_back({"getHeight", "instance", "()", 1});
        cls.methods.push_back({"getIntersection", "instance", "(CtrlrLuaRectangle)", 1});
        cls.methods.push_back({"getRight", "instance", "()", 1});
        cls.methods.push_back({"getUnion", "instance", "(CtrlrLuaRectangle)", 1});
        cls.methods.push_back({"getWidth", "instance", "()", 1});
        cls.methods.push_back({"getX", "instance", "()", 1});
        cls.methods.push_back({"getY", "instance", "()", 1});
        cls.methods.push_back({"intersects", "instance", "(CtrlrLuaRectangle)", 1});
        cls.methods.push_back({"reduce", "instance", "(int, int)", 1});
        cls.methods.push_back({"reduceIfPartlyContainedIn", "instance", "(CtrlrLuaRectangle)", 1});
        cls.methods.push_back({"setBottom", "instance", "(int)", 1});
        cls.methods.push_back({"setBounds", "instance", "(int, int, int, int)", 1});
        cls.methods.push_back({"setHeight", "instance", "(int)", 1});
        cls.methods.push_back({"setLeft", "instance", "(int)", 1});
        cls.methods.push_back({"setPosition", "instance", "(int, int)", 1});
        cls.methods.push_back({"setRight", "instance", "(int)", 1});
        cls.methods.push_back({"setSize", "instance", "(int, int)", 1});
        cls.methods.push_back({"setTop", "instance", "(int)", 1});
        cls.methods.push_back({"setWidth", "instance", "(int)", 1});
        cls.methods.push_back({"setX", "instance", "(int)", 1});
        cls.methods.push_back({"setY", "instance", "(int)", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        cls.methods.push_back({"translate", "instance", "(int, int)", 1});
        cls.methods.push_back({"translated", "instance", "(int, int)", 1});
        cls.methods.push_back({"withBottom", "instance", "(int)", 1});
        cls.methods.push_back({"withHeight", "instance", "(int)", 1});
        cls.methods.push_back({"withLeft", "instance", "(int)", 1});
        cls.methods.push_back({"withPosition", "instance", "(int, int)", 1});
        cls.methods.push_back({"withRight", "instance", "(int)", 1});
        cls.methods.push_back({"withTop", "instance", "(int)", 1});
        cls.methods.push_back({"withWidth", "instance", "(int)", 1});
        cls.methods.push_back({"withX", "instance", "(int)", 1});
        cls.methods.push_back({"withY", "instance", "(int)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrLuaUtils ---
    {
        ApiClass cls;
        cls.name = "CtrlrLuaUtils";
        cls.staticMethods.push_back({"askForTextInputWindow", "static", "(String, String, String, String, bool, String, String)", 1});
        cls.staticMethods.push_back({"base64_decode", "static", "(String)", 1});
        cls.staticMethods.push_back({"base64_encode", "static", "(String)", 1});
        cls.staticMethods.push_back({"get16bitSigned", "static", "()", 1});
        cls.staticMethods.push_back({"get8bitSigned", "static", "()", 1});
        cls.staticMethods.push_back({"getDirectoryWindow", "static", "(String, File)", 1});
        cls.staticMethods.push_back({"getMidiInputDevices", "static", "()", 1});
        cls.staticMethods.push_back({"getMidiOutputDevices", "static", "()", 1});
        cls.staticMethods.push_back({"getPi", "static", "()", 1});
        cls.staticMethods.push_back({"getVersionMajor", "static", "()", 1});
        cls.staticMethods.push_back({"getVersionMinor", "static", "()", 1});
        cls.staticMethods.push_back({"getVersionRevision", "static", "()", 1});
        cls.staticMethods.push_back({"getVersionString", "static", "()", 1});
        cls.staticMethods.push_back({"infoWindow", "static", "(String, String)", 1});
        cls.staticMethods.push_back({"openFileWindow", "static", "(String, File, String, bool)", 1});
        cls.staticMethods.push_back({"openMultipleFilesWindow", "static", "(String, File, String, bool, table)", 1});
        cls.staticMethods.push_back({"packDsiData", "static", "()", 1});
        cls.staticMethods.push_back({"questionWindow", "static", "(String, String, String, String)", 1});
        cls.staticMethods.push_back({"saveFileWindow", "static", "(String, File, String, bool)", 1});
        cls.staticMethods.push_back({"unpackDsiData", "static", "()", 1});
        cls.staticMethods.push_back({"warnWindow", "static", "(String, String)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrMIDIDevice ---
    {
        ApiClass cls;
        cls.name = "CtrlrMIDIDevice";
        cls.methods.push_back({"closeDevice", "instance", "()", 1});
        cls.methods.push_back({"getName", "instance", "()", 1});
        cls.methods.push_back({"getState", "instance", "()", 1});
        cls.methods.push_back({"getType", "instance", "()", 1});
        cls.methods.push_back({"openDevice", "instance", "()", 1});
        cls.methods.push_back({"sendMidiBuffer", "instance", "(MidiBuffer, double)", 1});
        cls.methods.push_back({"sendMidiMessage", "instance", "(MidiMessage, double)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrMIDIDeviceManager ---
    {
        ApiClass cls;
        cls.name = "CtrlrMIDIDeviceManager";
        cls.methods.push_back({"getDeviceByIndex", "instance", "()", 1});
        cls.methods.push_back({"getDeviceByName", "instance", "()", 1});
        cls.methods.push_back({"getDeviceName", "instance", "(int, CtrlrMIDIDeviceType)", 1});
        cls.methods.push_back({"getManagedDevices", "instance", "(CtrlrMIDIDeviceType)", 1});
        cls.methods.push_back({"getNumDevices", "instance", "(CtrlrMIDIDeviceType)", 1});
        cls.methods.push_back({"isDeviceOpened", "instance", "(int, CtrlrMIDIDeviceType)", 1});
        cls.methods.push_back({"refreshDevices", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrMidiMessage ---
    {
        ApiClass cls;
        cls.name = "CtrlrMidiMessage";
        cls.methods.push_back({"getChannel", "instance", "()", 1});
        cls.methods.push_back({"getData", "instance", "()", 1});
        cls.methods.push_back({"getInitializationResult", "instance", "()", 1});
        cls.methods.push_back({"getLuaData", "instance", "()", 1});
        cls.methods.push_back({"getMidiMessageType", "instance", "()", 1});
        cls.methods.push_back({"getNumber", "instance", "()", 1});
        cls.methods.push_back({"getProperty", "instance", "()", 1});
        cls.methods.push_back({"getSize", "instance", "()", 1});
        cls.methods.push_back({"getType", "instance", "()", 1});
        cls.methods.push_back({"getValue", "instance", "()", 1});
        cls.methods.push_back({"setChannel", "instance", "(jlimit(1, 16, owner.getMidiChannelForOwnedMidiMessages()", 1});
        cls.methods.push_back({"setMidiMessageType", "instance", "(newType)", 1});
        cls.methods.push_back({"setNumber", "instance", "(controllerNumber)", 1});
        cls.methods.push_back({"setProperty", "instance", "()", 1});
        cls.methods.push_back({"setType", "instance", "()", 1});
        cls.methods.push_back({"setValue", "instance", "(int)", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrModulator ---
    {
        ApiClass cls;
        cls.name = "CtrlrModulator";
        cls.methods.push_back({"getComponent", "instance", "()", 1});
        cls.methods.push_back({"getLuaName", "instance", "()", 1});
        cls.methods.push_back({"getMaxMapped", "instance", "()", 1});
        cls.methods.push_back({"getMaxMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getMaxModulatorValue", "instance", "()", 1});
        cls.methods.push_back({"getMaxModulatorValueInt", "instance", "()", 1});
        cls.methods.push_back({"getMaxNonMapped", "instance", "()", 1});
        cls.methods.push_back({"getMaxNonMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getMidiMessage", "instance", "()", 1});
        cls.methods.push_back({"getMidiMessage", "instance", "()", 2});
        cls.methods.push_back({"getMinMapped", "instance", "()", 1});
        cls.methods.push_back({"getMinMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getMinModulatorValue", "instance", "()", 1});
        cls.methods.push_back({"getMinModulatorValueInt", "instance", "()", 1});
        cls.methods.push_back({"getMinNonMapped", "instance", "()", 1});
        cls.methods.push_back({"getMinNonMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getModulatorName", "instance", "()", 1});
        cls.methods.push_back({"getModulatorValue", "instance", "()", 1});
        cls.methods.push_back({"getModulatorValueInt", "instance", "()", 1});
        cls.methods.push_back({"getName", "instance", "()", 1});
        cls.methods.push_back({"getRestoreState", "instance", "()", 1});
        cls.methods.push_back({"getValue", "instance", "()", 1});
        cls.methods.push_back({"getValueInt", "instance", "()", 1});
        cls.methods.push_back({"getValueMapped", "instance", "()", 1});
        cls.methods.push_back({"getValueMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getValueNonMapped", "instance", "()", 1});
        cls.methods.push_back({"getValueNonMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getVstIndex", "instance", "()", 1});
        cls.methods.push_back({"isRestoring", "instance", "()", 1});
        cls.methods.push_back({"setModulatorValue", "instance", "(int, bool, bool, bool)", 1});
        cls.methods.push_back({"setRestoreState", "instance", "(bool)", 1});
        cls.methods.push_back({"setValue", "instance", "(int, bool, bool)", 1});
        cls.methods.push_back({"setValue", "instance", "(int, bool, bool)", 2});
        cls.methods.push_back({"setValueMapped", "instance", "(int, bool, bool)", 1});
        cls.methods.push_back({"setValueMapped", "instance", "(int, bool, bool)", 2});
        cls.methods.push_back({"setValueNonMapped", "instance", "(int, bool, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrNative ---
    {
        ApiClass cls;
        cls.name = "CtrlrNative";
        cls.methods.push_back({"sendKeyPressEvent", "instance", "()", 1});
        cls.methods.push_back({"sendKeyPressEvent", "instance", "()", 2});
        cls.staticMethods.push_back({"getNativeObject", "static", "(owner.getCtrlrManagerOwner()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrPanel ---
    {
        ApiClass cls;
        cls.name = "CtrlrPanel";
        cls.methods.push_back({"dumpDebugData", "instance", "()", 1});
        cls.methods.push_back({"getBootstrapState", "instance", "()", 1});
        cls.methods.push_back({"getButton", "instance", "()", 1});
        cls.methods.push_back({"getButtonComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getCanvas", "instance", "()", 1});
        cls.methods.push_back({"getCombo", "instance", "()", 1});
        cls.methods.push_back({"getComboComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getFileListBox", "instance", "()", 1});
        cls.methods.push_back({"getFileListBoxComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getFixedImageSlider", "instance", "()", 1});
        cls.methods.push_back({"getFixedImageSliderComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getFixedSlider", "instance", "()", 1});
        cls.methods.push_back({"getFixedSliderComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getGlobalVariable", "instance", "(int)", 1});
        cls.methods.push_back({"getImageButton", "instance", "()", 1});
        cls.methods.push_back({"getImageButtonComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getImageSlider", "instance", "()", 1});
        cls.methods.push_back({"getImageSliderComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getInputComparator", "instance", "()", 1});
        cls.methods.push_back({"getLCDLabel", "instance", "()", 1});
        cls.methods.push_back({"getLCDLabelComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getLabel", "instance", "()", 1});
        cls.methods.push_back({"getLabelComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getListBox", "instance", "()", 1});
        cls.methods.push_back({"getListBoxComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getModulator", "instance", "(String)", 1});
        cls.methods.push_back({"getModulatorByIndex", "instance", "(int)", 1});
        cls.methods.push_back({"getModulatorByName", "instance", "()", 1});
        cls.methods.push_back({"getModulatorValuesAsData", "instance", "(ValueTree, String, CtrlrByteEncoding, int, bool)", 1});
        cls.methods.push_back({"getModulatorValuesAsData", "instance", "(ValueTree, String, CtrlrByteEncoding, int, bool)", 2});
        cls.methods.push_back({"getModulatorValuesAsData", "instance", "(ValueTree, String, CtrlrByteEncoding, int, bool)", 3});
        cls.methods.push_back({"getModulatorsWildcard", "instance", "()", 1});
        cls.methods.push_back({"getModulatorsWildcard", "instance", "()", 2});
        cls.methods.push_back({"getModulatorsWithProperty", "instance", "(Identifier, var)", 1});
        cls.methods.push_back({"getNumModulators", "instance", "()", 1});
        cls.methods.push_back({"getPanelEditor", "instance", "()", 1});
        cls.methods.push_back({"getProgramState", "instance", "()", 1});
        cls.methods.push_back({"getRestoreState", "instance", "()", 1});
        cls.methods.push_back({"getSlider", "instance", "()", 1});
        cls.methods.push_back({"getSliderComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getToggleButton", "instance", "()", 1});
        cls.methods.push_back({"getToggleButtonComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getWaveform", "instance", "()", 1});
        cls.methods.push_back({"getWaveformComponent", "instance", "(String)", 1});
        cls.methods.push_back({"isRestoring", "instance", "()", 1});
        cls.methods.push_back({"sendMidi", "instance", "(CtrlrMidiMessage, double)", 1});
        cls.methods.push_back({"sendMidi", "instance", "(CtrlrMidiMessage, double)", 2});
        cls.methods.push_back({"sendMidi", "instance", "(CtrlrMidiMessage, double)", 3});
        cls.methods.push_back({"sendMidiMessageNow", "instance", "()", 1});
        cls.methods.push_back({"setGlobalVariable", "instance", "(int, int)", 1});
        cls.methods.push_back({"setModulatorValuesFromData", "instance", "(MemoryBlock, String, CtrlrByteEncoding, int, int, bool)", 1});
        cls.methods.push_back({"setProgramState", "instance", "(bool)", 1});
        cls.methods.push_back({"setRestoreState", "instance", "(bool)", 1});
        cls.methods.push_back({"getModulatorWithProperty", "instance", "(String propName, String propValue)", 1});
        cls.methods.push_back({"getModulatorWithProperty", "instance", "(String propName, int propValue)", 2});
        cls.methods.push_back({"getModulatorWithProperty", "instance", "(String propName, String propValue)", 1});
        cls.methods.push_back({"getModulatorWithProperty", "instance", "(String propName, int propValue)", 2});
        cls.methods.push_back({"getModulatorWithProperty", "instance", "(String propName, String propValue)", 1});
        cls.methods.push_back({"getModulatorWithProperty", "instance", "(String propName, int propValue)", 2});
        classMap[cls.name] = cls;
    }

    // --- CtrlrPanelCanvas ---
    {
        ApiClass cls;
        cls.name = "CtrlrPanelCanvas";
        cls.methods.push_back({"getLayer", "instance", "()", 1});
        cls.methods.push_back({"getLayerByName", "instance", "()", 1});
        cls.methods.push_back({"getLayerFromArray", "instance", "()", 1});
        cls.methods.push_back({"getLayerName", "instance", "(int)", 1});
        cls.methods.push_back({"getNumLayers", "instance", "()", 1});
        cls.methods.push_back({"setCustomLookAndFeel", "instance", "(table)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrPanelCanvasLayer ---
    {
        ApiClass cls;
        cls.name = "CtrlrPanelCanvasLayer";
        cls.methods.push_back({"moveDown", "instance", "()", 1});
        cls.methods.push_back({"moveUp", "instance", "()", 1});
        cls.methods.push_back({"setCustomLookAndFeel", "instance", "(LookAndFeelBase)", 1});
        cls.methods.push_back({"setVisible", "instance", "(bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrPanelEditor ---
    {
        ApiClass cls;
        cls.name = "CtrlrPanelEditor";
        cls.methods.push_back({"getCanvas", "instance", "()", 1});
        cls.methods.push_back({"getHeight", "instance", "()", 1});
        cls.methods.push_back({"getOwner", "instance", "()", 1});
        cls.methods.push_back({"getWidth", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrPanelResource ---
    {
        ApiClass cls;
        cls.name = "CtrlrPanelResource";
        cls.methods.push_back({"asAudioFormat", "instance", "()", 1});
        cls.methods.push_back({"asData", "instance", "()", 1});
        cls.methods.push_back({"asFont", "instance", "()", 1});
        cls.methods.push_back({"asGzipText", "instance", "()", 1});
        cls.methods.push_back({"asImage", "instance", "()", 1});
        cls.methods.push_back({"asText", "instance", "()", 1});
        cls.methods.push_back({"asXml", "instance", "()", 1});
        cls.methods.push_back({"createInputStream", "instance", "()", 1});
        cls.methods.push_back({"getFile", "instance", "()", 1});
        cls.methods.push_back({"getHashCode", "instance", "()", 1});
        cls.methods.push_back({"getName", "instance", "()", 1});
        cls.methods.push_back({"getSizeDouble", "instance", "()", 1});
        cls.methods.push_back({"getType", "instance", "()", 1});
        cls.methods.push_back({"getTypeDescription", "instance", "()", 1});
        cls.methods.push_back({"load", "instance", "()", 1});
        cls.methods.push_back({"loadIfNeeded", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrPanelResourceManager ---
    {
        ApiClass cls;
        cls.name = "CtrlrPanelResourceManager";
        cls.methods.push_back({"getNumResources", "instance", "()", 1});
        cls.methods.push_back({"getResource", "instance", "()", 1});
        cls.methods.push_back({"getResource", "instance", "()", 2});
        cls.methods.push_back({"getResourceAsFont", "instance", "(String)", 1});
        cls.methods.push_back({"getResourceAsImage", "instance", "(String)", 1});
        cls.methods.push_back({"getResourceIndex", "instance", "(String)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrSlider ---
    {
        ApiClass cls;
        cls.name = "CtrlrSlider";
        cls.methods.push_back({"getOwnedSlider", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrToggleButton ---
    {
        ApiClass cls;
        cls.name = "CtrlrToggleButton";
        cls.methods.push_back({"getToggleState", "instance", "()", 1});
        cls.methods.push_back({"getValueMap", "instance", "()", 1});
        cls.methods.push_back({"setToggleState", "instance", "(bool, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrValueMap ---
    {
        ApiClass cls;
        cls.name = "CtrlrValueMap";
        cls.methods.push_back({"addAdditionalData", "instance", "(int, String)", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"copyFrom", "instance", "(CtrlrValueMap)", 1});
        cls.methods.push_back({"decrement", "instance", "()", 1});
        cls.methods.push_back({"fillCombo", "instance", "(ComboBox, bool)", 1});
        cls.methods.push_back({"getCurrentMappedValue", "instance", "()", 1});
        cls.methods.push_back({"getCurrentNonMappedValue", "instance", "()", 1});
        cls.methods.push_back({"getCurrentText", "instance", "()", 1});
        cls.methods.push_back({"getIndexForValue", "instance", "(int)", 1});
        cls.methods.push_back({"getMap", "instance", "()", 1});
        cls.methods.push_back({"getMappedMax", "instance", "()", 1});
        cls.methods.push_back({"getMappedMin", "instance", "()", 1});
        cls.methods.push_back({"getMappedValue", "instance", "(String)", 1});
        cls.methods.push_back({"getMappedValue", "instance", "(String)", 2});
        cls.methods.push_back({"getNonMappedMax", "instance", "()", 1});
        cls.methods.push_back({"getNonMappedMin", "instance", "()", 1});
        cls.methods.push_back({"getNonMappedValue", "instance", "(String)", 1});
        cls.methods.push_back({"getNumericValues", "instance", "()", 1});
        cls.methods.push_back({"getTextForIndex", "instance", "(int)", 1});
        cls.methods.push_back({"increment", "instance", "()", 1});
        cls.methods.push_back({"mappedValueExists", "instance", "(int)", 1});
        cls.methods.push_back({"parseString", "instance", "(String)", 1});
        cls.methods.push_back({"setCurrentMappedValue", "instance", "(int)", 1});
        cls.methods.push_back({"setCurrentNonMappedValue", "instance", "(int)", 1});
        cls.methods.push_back({"setPair", "instance", "(int, int, String)", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- CtrlrWaveform ---
    {
        ApiClass cls;
        cls.name = "CtrlrWaveform";
        cls.methods.push_back({"addBlock", "instance", "(double, AudioSampleBuffer, int, int)", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"getApproximateMinMax", "instance", "(double, double, int, float, float)", 1});
        cls.methods.push_back({"getApproximatePeak", "instance", "()", 1});
        cls.methods.push_back({"getCurrentAudioBuffer", "instance", "()", 1});
        cls.methods.push_back({"getCurrentFile", "instance", "()", 1});
        cls.methods.push_back({"getHashCode", "instance", "()", 1});
        cls.methods.push_back({"getNumChannels", "instance", "()", 1});
        cls.methods.push_back({"getNumSamplesFinished", "instance", "()", 1});
        cls.methods.push_back({"getTotalLength", "instance", "()", 1});
        cls.methods.push_back({"isFullyLoaded", "instance", "()", 1});
        cls.methods.push_back({"loadFromFile", "instance", "(String)", 1});
        cls.methods.push_back({"loadFromFile", "instance", "(String)", 2});
        cls.methods.push_back({"reset", "instance", "(int, double, double)", 1});
        cls.methods.push_back({"setEnd", "instance", "(double)", 1});
        cls.methods.push_back({"setReader", "instance", "(AudioFormatReader, double)", 1});
        cls.methods.push_back({"setSourceSamplesPerThumbnailSample", "instance", "(int)", 1});
        cls.methods.push_back({"setStart", "instance", "(double)", 1});
        cls.methods.push_back({"setVerticalZoomFactor", "instance", "(float)", 1});
        classMap[cls.name] = cls;
    }

    // --- DragAndDropSourceDetails ---
    {
        ApiClass cls;
        cls.name = "DragAndDropSourceDetails";
        cls.methods.push_back({"getDescription", "instance", "()", 1});
        cls.methods.push_back({"getLocalPositionX", "instance", "()", 1});
        cls.methods.push_back({"getLocalPositionY", "instance", "()", 1});
        cls.methods.push_back({"getSourceComponent", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Drawable ---
    {
        ApiClass cls;
        cls.name = "Drawable";
        cls.methods.push_back({"draw", "instance", "(Graphics, float, AffineTransform)", 1});
        cls.methods.push_back({"drawAt", "instance", "(Graphics, float, float, float)", 1});
        cls.methods.push_back({"drawWithin", "instance", "(Graphics, Rectangle<float>, RectanglePlacement, float)", 1});
        cls.methods.push_back({"getDrawableBounds", "instance", "()", 1});
        cls.methods.push_back({"getParent", "instance", "()", 1});
        cls.methods.push_back({"setOriginWithOriginalSize", "instance", "(Point<float>)", 1});
        cls.methods.push_back({"setTransformToFit", "instance", "(Rectangle<float>, RectanglePlacement)", 1});
        cls.staticMethods.push_back({"createFromImageData", "static", "(svgData.toRawUTF8()", 1});
        cls.staticMethods.push_back({"createFromImageDataStream", "static", "(stream)", 1});
        cls.staticMethods.push_back({"createFromImageFile", "static", "(file)", 1});
        cls.staticMethods.push_back({"createFromSVG", "static", "(svgElement)", 1});
        cls.staticMethods.push_back({"toDrawableComposite", "static", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- DrawableComposite ---
    {
        ApiClass cls;
        cls.name = "DrawableComposite";
        cls.methods.push_back({"getBoundingBox", "instance", "()", 1});
        cls.methods.push_back({"getContentArea", "instance", "()", 1});
        cls.methods.push_back({"getDrawableBounds", "instance", "()", 1});
        cls.methods.push_back({"resetBoundingBoxToContentArea", "instance", "()", 1});
        cls.methods.push_back({"resetContentAreaAndBoundingBoxToFitChildren", "instance", "()", 1});
        cls.methods.push_back({"setBoundingBox", "instance", "(Parallelogram<float>)", 1});
        cls.methods.push_back({"setContentArea", "instance", "(Rectangle<float>)", 1});
        classMap[cls.name] = cls;
    }

    // --- DropShadow ---
    {
        ApiClass cls;
        cls.name = "DropShadow";
        cls.methods.push_back({"drawForImage", "instance", "(Graphics, Image)", 1});
        cls.methods.push_back({"drawForPath", "instance", "(Graphics, Path)", 1});
        cls.methods.push_back({"drawForRectangle", "instance", "(Graphics, Rectangle<int>)", 1});
        classMap[cls.name] = cls;
    }

    // --- DropShadower ---
    {
        ApiClass cls;
        cls.name = "DropShadower";
        cls.methods.push_back({"setOwner", "instance", "(Component)", 1});
        classMap[cls.name] = cls;
    }

    // --- Expression ---
    {
        ApiClass cls;
        cls.name = "Expression";
        cls.methods.push_back({"adjustedToGiveNewResult", "instance", "(double, Scope)", 1});
        cls.methods.push_back({"evaluate", "instance", "(Scope, String)", 1});
        cls.methods.push_back({"evaluate", "instance", "(Scope, String)", 2});
        cls.methods.push_back({"evaluate", "instance", "(Scope, String)", 3});
        cls.methods.push_back({"findReferencedSymbols", "instance", "(Array<Symbol>, Scope)", 1});
        cls.methods.push_back({"getInput", "instance", "(int)", 1});
        cls.methods.push_back({"getNumInputs", "instance", "()", 1});
        cls.methods.push_back({"getSymbolOrFunction", "instance", "()", 1});
        cls.methods.push_back({"getType", "instance", "()", 1});
        cls.methods.push_back({"referencesSymbol", "instance", "(Symbol, Scope)", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        cls.methods.push_back({"usesAnySymbols", "instance", "()", 1});
        cls.methods.push_back({"withRenamedSymbol", "instance", "(Symbol, String, Scope)", 1});
        classMap[cls.name] = cls;
    }

    // --- File ---
    {
        ApiClass cls;
        cls.name = "File";
        cls.methods.push_back({"appendData", "instance", "(void, size_t)", 1});
        cls.methods.push_back({"appendText", "instance", "(String, bool, bool, char)", 1});
        cls.methods.push_back({"containsSubDirectories", "instance", "()", 1});
        cls.methods.push_back({"copyFileTo", "instance", "(File)", 1});
        cls.methods.push_back({"create", "instance", "()", 1});
        cls.methods.push_back({"createDirectory", "instance", "()", 1});
        cls.methods.push_back({"deleteFile", "instance", "()", 1});
        cls.methods.push_back({"deleteRecursively", "instance", "(bool)", 1});
        cls.methods.push_back({"exists", "instance", "()", 1});
        cls.methods.push_back({"existsAsFile", "instance", "()", 1});
        cls.methods.push_back({"findChildFiles", "instance", "(files, whatToLookFor, searchRecursively, wildcardPattern)", 1});
        cls.methods.push_back({"getBytesFreeOnVolume", "instance", "()", 1});
        cls.methods.push_back({"getChildFile", "instance", "(StringRef)", 1});
        cls.methods.push_back({"getCreationTime", "instance", "()", 1});
        cls.methods.push_back({"getFileExtension", "instance", "()", 1});
        cls.methods.push_back({"getFileIdentifier", "instance", "()", 1});
        cls.methods.push_back({"getFileName", "instance", "()", 1});
        cls.methods.push_back({"getFileNameWithoutExtension", "instance", "()", 1});
        cls.methods.push_back({"getFullPathName", "instance", "()", 1});
        cls.methods.push_back({"getLastAccessTime", "instance", "()", 1});
        cls.methods.push_back({"getLastModificationTime", "instance", "()", 1});
        cls.methods.push_back({"getLinkedTarget", "instance", "()", 1});
        cls.methods.push_back({"getNonexistentChildFile", "instance", "(String, String, bool)", 1});
        cls.methods.push_back({"getNonexistentSibling", "instance", "()", 1});
        cls.methods.push_back({"getNumberOfChildFiles", "instance", "(int, String)", 1});
        cls.methods.push_back({"getParentDirectory", "instance", "()", 1});
        cls.methods.push_back({"getRelativePathFrom", "instance", "(File)", 1});
        cls.methods.push_back({"getSiblingFile", "instance", "(StringRef)", 1});
        cls.methods.push_back({"getSize", "instance", "()", 1});
        cls.methods.push_back({"getVersion", "instance", "()", 1});
        cls.methods.push_back({"getVolumeLabel", "instance", "()", 1});
        cls.methods.push_back({"getVolumeSerialNumber", "instance", "()", 1});
        cls.methods.push_back({"getVolumeTotalSize", "instance", "()", 1});
        cls.methods.push_back({"hasFileExtension", "instance", "(StringRef)", 1});
        cls.methods.push_back({"hasIdenticalContentTo", "instance", "(File)", 1});
        cls.methods.push_back({"hasWriteAccess", "instance", "()", 1});
        cls.methods.push_back({"hashCode", "instance", "()", 1});
        cls.methods.push_back({"hashCode64", "instance", "()", 1});
        cls.methods.push_back({"isAChildOf", "instance", "(File)", 1});
        cls.methods.push_back({"isDirectory", "instance", "()", 1});
        cls.methods.push_back({"isHidden", "instance", "()", 1});
        cls.methods.push_back({"isHidden", "instance", "()", 2});
        cls.methods.push_back({"isOnCDRomDrive", "instance", "()", 1});
        cls.methods.push_back({"isOnHardDisk", "instance", "()", 1});
        cls.methods.push_back({"isOnRemovableDrive", "instance", "()", 1});
        cls.methods.push_back({"isRoot", "instance", "()", 1});
        cls.methods.push_back({"isValid", "instance", "()", 1});
        cls.methods.push_back({"loadFileAsData", "instance", "(mb)", 1});
        cls.methods.push_back({"loadFileAsString", "instance", "()", 1});
        cls.methods.push_back({"moveFileTo", "instance", "(File)", 1});
        cls.methods.push_back({"moveToTrash", "instance", "()", 1});
        cls.methods.push_back({"replaceWithData", "instance", "(data.getData()", 1});
        cls.methods.push_back({"replaceWithText", "instance", "(String, bool, bool, char)", 1});
        cls.methods.push_back({"revealToUser", "instance", "()", 1});
        cls.methods.push_back({"setAsCurrentWorkingDirectory", "instance", "()", 1});
        cls.methods.push_back({"setCreationTime", "instance", "(Time)", 1});
        cls.methods.push_back({"setLastAccessTime", "instance", "(Time)", 1});
        cls.methods.push_back({"setLastModificationTime", "instance", "(Time)", 1});
        cls.methods.push_back({"setReadOnly", "instance", "(bool, bool)", 1});
        cls.methods.push_back({"startAsProcess", "instance", "(String)", 1});
        cls.methods.push_back({"withFileExtension", "instance", "(StringRef)", 1});
        cls.staticMethods.push_back({"addTrailingSeparator", "static", "(String)", 1});
        cls.staticMethods.push_back({"areFileNamesCaseSensitive", "static", "()", 1});
        cls.staticMethods.push_back({"createFileWithoutCheckingPath", "static", "(path)", 1});
        cls.staticMethods.push_back({"createLegalFileName", "static", "(owner.getInstanceName()", 1});
        cls.staticMethods.push_back({"createLegalPathName", "static", "(String)", 1});
        cls.staticMethods.push_back({"createTempFile", "static", "(StringRef)", 1});
        cls.staticMethods.push_back({"descriptionOfSizeInBytes", "static", "(fontFile.getSize()", 1});
        cls.staticMethods.push_back({"findFileSystemRoots", "static", "(files)", 1});
        cls.staticMethods.push_back({"getCurrentWorkingDirectory", "static", "()", 1});
        cls.staticMethods.push_back({"getSpecialLocation", "static", "(userDocumentsDirectory)", 1});
        cls.staticMethods.push_back({"isAbsolutePath", "static", "(panelCertificateWinPath)", 1});
        classMap[cls.name] = cls;
    }

    // --- FillType ---
    {
        ApiClass cls;
        cls.name = "FillType";
        cls.methods.push_back({"getOpacity", "instance", "()", 1});
        cls.methods.push_back({"isColour", "instance", "()", 1});
        cls.methods.push_back({"isGradient", "instance", "()", 1});
        cls.methods.push_back({"isInvisible", "instance", "()", 1});
        cls.methods.push_back({"isTiledImage", "instance", "()", 1});
        cls.methods.push_back({"setColour", "instance", "(Colour)", 1});
        cls.methods.push_back({"setGradient", "instance", "(ColourGradient)", 1});
        cls.methods.push_back({"setOpacity", "instance", "(float)", 1});
        cls.methods.push_back({"setTiledImage", "instance", "(Image, AffineTransform)", 1});
        classMap[cls.name] = cls;
    }

    // --- FlacAudioFormat ---
    {
        ApiClass cls;
        cls.name = "FlacAudioFormat";
        cls.methods.push_back({"canDoMono", "instance", "()", 1});
        cls.methods.push_back({"canDoStereo", "instance", "()", 1});
        cls.methods.push_back({"canHandleFile", "instance", "()", 1});
        cls.methods.push_back({"createReaderFor", "instance", "(InputStream, bool)", 1});
        cls.methods.push_back({"getQualityOptions", "instance", "()", 1});
        cls.methods.push_back({"isCompressed", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Font ---
    {
        ApiClass cls;
        cls.name = "Font";
        cls.methods.push_back({"boldened", "instance", "()", 1});
        cls.methods.push_back({"getAscent", "instance", "()", 1});
        cls.methods.push_back({"getAscentInPoints", "instance", "()", 1});
        cls.methods.push_back({"getAvailableStyles", "instance", "()", 1});
        cls.methods.push_back({"getDescent", "instance", "()", 1});
        cls.methods.push_back({"getDescentInPoints", "instance", "()", 1});
        cls.methods.push_back({"getExtraKerningFactor", "instance", "()", 1});
        cls.methods.push_back({"getGlyphPositions", "instance", "(String, Array<int>, Array<float>)", 1});
        cls.methods.push_back({"getHeight", "instance", "()", 1});
        cls.methods.push_back({"getHeightInPoints", "instance", "()", 1});
        cls.methods.push_back({"getHorizontalScale", "instance", "()", 1});
        cls.methods.push_back({"getStringWidth", "instance", "(String)", 1});
        cls.methods.push_back({"getStringWidthFloat", "instance", "(String)", 1});
        cls.methods.push_back({"getStyleFlags", "instance", "()", 1});
        cls.methods.push_back({"getTypeface", "instance", "()", 1});
        cls.methods.push_back({"getTypefaceName", "instance", "()", 1});
        cls.methods.push_back({"getTypefaceStyle", "instance", "()", 1});
        cls.methods.push_back({"isBold", "instance", "()", 1});
        cls.methods.push_back({"isUnderlined", "instance", "()", 1});
        cls.methods.push_back({"italicised", "instance", "()", 1});
        cls.methods.push_back({"setBold", "instance", "(bool)", 1});
        cls.methods.push_back({"setExtraKerningFactor", "instance", "(float)", 1});
        cls.methods.push_back({"setHeight", "instance", "(float)", 1});
        cls.methods.push_back({"setHeightWithoutChangingWidth", "instance", "(float)", 1});
        cls.methods.push_back({"setHorizontalScale", "instance", "(float)", 1});
        cls.methods.push_back({"setItalic", "instance", "(bool)", 1});
        cls.methods.push_back({"setSizeAndStyle", "instance", "(float, String, float, float)", 1});
        cls.methods.push_back({"setSizeAndStyle", "instance", "(float, String, float, float)", 2});
        cls.methods.push_back({"setStyleFlags", "instance", "(int)", 1});
        cls.methods.push_back({"setTypefaceName", "instance", "(String)", 1});
        cls.methods.push_back({"setTypefaceStyle", "instance", "(String)", 1});
        cls.methods.push_back({"setUnderline", "instance", "(bool)", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        cls.methods.push_back({"withExtraKerningFactor", "instance", "(float)", 1});
        cls.methods.push_back({"withHeight", "instance", "(float)", 1});
        cls.methods.push_back({"withPointHeight", "instance", "(float)", 1});
        cls.methods.push_back({"withStyle", "instance", "(int)", 1});
        cls.methods.push_back({"withTypefaceStyle", "instance", "(String)", 1});
        cls.staticMethods.push_back({"findAllTypefaceNames", "static", "()", 1});
        cls.staticMethods.push_back({"findAllTypefaceStyles", "static", "(String)", 1});
        cls.staticMethods.push_back({"fromString", "static", "(String)", 1});
        cls.staticMethods.push_back({"getDefaultMonospacedFontName ", "static", "()", 1});
        cls.staticMethods.push_back({"getDefaultSansSerifFontName", "static", "()", 1});
        cls.staticMethods.push_back({"getDefaultSerifFontName", "static", "()", 1});
        cls.staticMethods.push_back({"getDefaultStyle", "static", "()", 1});
        cls.staticMethods.push_back({"getDefaultTypefaceForFont", "static", "(f)", 1});
        cls.staticMethods.push_back({"getFallbackFontName", "static", "()", 1});
        cls.staticMethods.push_back({"getFallbackFontStyle", "static", "()", 1});
        cls.staticMethods.push_back({"setFallbackFontName", "static", "(String)", 1});
        cls.staticMethods.push_back({"setFallbackFontStyle", "static", "(String)", 1});
        classMap[cls.name] = cls;
    }

    // --- GZIPDecompressorInputStream ---
    {
        ApiClass cls;
        cls.name = "GZIPDecompressorInputStream";
        cls.staticMethods.push_back({"create", "static", "(InputStream, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- GlyphArrangement ---
    {
        ApiClass cls;
        cls.name = "GlyphArrangement";
        cls.methods.push_back({"addCurtailedLineOfText", "instance", "(Font, String, float, float, float, bool)", 1});
        cls.methods.push_back({"addFittedText", "instance", "(Font, String, float, float, float, float, Justification, int, float)", 1});
        cls.methods.push_back({"addGlyph", "instance", "(PositionedGlyph)", 1});
        cls.methods.push_back({"addGlyphArrangement", "instance", "(GlyphArrangement)", 1});
        cls.methods.push_back({"addJustifiedText", "instance", "(Font, String, float, float, float, Justification, float)", 1});
        cls.methods.push_back({"addLineOfText", "instance", "(Font, String, float, float)", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"createPath", "instance", "(Path)", 1});
        cls.methods.push_back({"draw", "instance", "(Graphics, AffineTransform)", 1});
        cls.methods.push_back({"draw", "instance", "(Graphics, AffineTransform)", 2});
        cls.methods.push_back({"findGlyphIndexAt", "instance", "(float, float)", 1});
        cls.methods.push_back({"getBoundingBox", "instance", "(int, int, bool)", 1});
        cls.methods.push_back({"getGlyph", "instance", "(int)", 1});
        cls.methods.push_back({"getNumGlyphs", "instance", "()", 1});
        cls.methods.push_back({"justifyGlyphs", "instance", "(int, int, float, float, float, float, Justification)", 1});
        cls.methods.push_back({"moveRangeOfGlyphs", "instance", "(int, int, float, float)", 1});
        cls.methods.push_back({"removeRangeOfGlyphs", "instance", "(int, int)", 1});
        cls.methods.push_back({"stretchRangeOfGlyphs", "instance", "(int, int, float)", 1});
        classMap[cls.name] = cls;
    }

    // --- Graphics ---
    {
        ApiClass cls;
        cls.name = "Graphics";
        cls.methods.push_back({"addTransform", "instance", "(AffineTransform)", 1});
        cls.methods.push_back({"beginTransparencyLayer", "instance", "(float)", 1});
        cls.methods.push_back({"clipRegionIntersects", "instance", "(Rectangle<int>)", 1});
        cls.methods.push_back({"drawArrow", "instance", "(Line<float>, float, float, float)", 1});
        cls.methods.push_back({"drawDashedLine", "instance", "(Line<float>, float, int, float, int)", 1});
        cls.methods.push_back({"drawEllipse", "instance", "(Rectangle<float>, float)", 1});
        cls.methods.push_back({"drawEllipse", "instance", "(Rectangle<float>, float)", 2});
        cls.methods.push_back({"drawFittedText", "instance", "(String, int, int, int, int, Justification, int, float)", 1});
        cls.methods.push_back({"drawFittedText", "instance", "(String, int, int, int, int, Justification, int, float)", 2});
        cls.methods.push_back({"drawHorizontalLine", "instance", "(int, float, float)", 1});
        cls.methods.push_back({"drawImage", "instance", "(Image, int, int, int, int, int, int, int, int, bool)", 1});
        cls.methods.push_back({"drawImage", "instance", "(Image, int, int, int, int, int, int, int, int, bool)", 2});
        cls.methods.push_back({"drawImageAt", "instance", "(Image, int, int, bool)", 1});
        cls.methods.push_back({"drawImageTransformed", "instance", "(Image, AffineTransform, bool)", 1});
        cls.methods.push_back({"drawImageWithin", "instance", "(Image, int, int, int, int, RectanglePlacement, bool)", 1});
        cls.methods.push_back({"drawLine", "instance", "(Line<float>, float)", 1});
        cls.methods.push_back({"drawLine", "instance", "(Line<float>, float)", 2});
        cls.methods.push_back({"drawMultiLineText", "instance", "(String, int, int, int, Justification, float)", 1});
        cls.methods.push_back({"drawRect", "instance", "(Rectangle<float>, float)", 1});
        cls.methods.push_back({"drawRect", "instance", "(Rectangle<float>, float)", 2});
        cls.methods.push_back({"drawRoundedRectangle", "instance", "(Rectangle<float>, float, float)", 1});
        cls.methods.push_back({"drawRoundedRectangle", "instance", "(Rectangle<float>, float, float)", 2});
        cls.methods.push_back({"drawSingleLineText", "instance", "(String, int, int, Justification)", 1});
        cls.methods.push_back({"drawText", "instance", "(String, int, int, int, int, Justification, bool)", 1});
        cls.methods.push_back({"drawText", "instance", "(String, int, int, int, int, Justification, bool)", 2});
        cls.methods.push_back({"drawText", "instance", "(String, int, int, int, int, Justification, bool)", 3});
        cls.methods.push_back({"drawVerticalLine", "instance", "(int, float, float)", 1});
        cls.methods.push_back({"endTransparencyLayer", "instance", "()", 1});
        cls.methods.push_back({"excludeClipRegion", "instance", "(Rectangle<int>)", 1});
        cls.methods.push_back({"fillAll", "instance", "(Colour)", 1});
        cls.methods.push_back({"fillAll", "instance", "(Colour)", 2});
        cls.methods.push_back({"fillCheckerBoard", "instance", "(Rectangle<float>, float, float, Colour, Colour)", 1});
        cls.methods.push_back({"fillEllipse", "instance", "(float, float, float, float)", 1});
        cls.methods.push_back({"fillEllipse", "instance", "(float, float, float, float)", 2});
        cls.methods.push_back({"fillPath", "instance", "(Path, AffineTransform)", 1});
        cls.methods.push_back({"fillPath", "instance", "(Path, AffineTransform)", 2});
        cls.methods.push_back({"fillRect", "instance", "(float, float, float, float)", 1});
        cls.methods.push_back({"fillRect", "instance", "(float, float, float, float)", 2});
        cls.methods.push_back({"fillRoundedRectangle", "instance", "(Rectangle<float>, float)", 1});
        cls.methods.push_back({"fillRoundedRectangle", "instance", "(Rectangle<float>, float)", 2});
        cls.methods.push_back({"getClipBounds", "instance", "()", 1});
        cls.methods.push_back({"getCurrentFont", "instance", "()", 1});
        cls.methods.push_back({"isClipEmpty", "instance", "()", 1});
        cls.methods.push_back({"isVectorDevice", "instance", "()", 1});
        cls.methods.push_back({"reduceClipRegion", "instance", "(Image, AffineTransform)", 1});
        cls.methods.push_back({"reduceClipRegion", "instance", "(Image, AffineTransform)", 2});
        cls.methods.push_back({"reduceClipRegion", "instance", "(Image, AffineTransform)", 3});
        cls.methods.push_back({"reduceClipRegion", "instance", "(Image, AffineTransform)", 4});
        cls.methods.push_back({"reduceClipRegion", "instance", "(Image, AffineTransform)", 5});
        cls.methods.push_back({"resetToDefaultState", "instance", "()", 1});
        cls.methods.push_back({"restoreState", "instance", "()", 1});
        cls.methods.push_back({"saveState", "instance", "()", 1});
        cls.methods.push_back({"setColour", "instance", "(Colour)", 1});
        cls.methods.push_back({"setFillType", "instance", "(FillType)", 1});
        cls.methods.push_back({"setFont", "instance", "(float)", 1});
        cls.methods.push_back({"setFont", "instance", "(float)", 2});
        cls.methods.push_back({"setGradientFill", "instance", "(ColourGradient)", 1});
        cls.methods.push_back({"setImageResamplingQuality", "instance", "(ResamplingQuality)", 1});
        cls.methods.push_back({"setOpacity", "instance", "(float)", 1});
        cls.methods.push_back({"setOrigin", "instance", "(int, int)", 1});
        cls.methods.push_back({"setOrigin", "instance", "(int, int)", 2});
        cls.methods.push_back({"setTiledImageFill", "instance", "(Image, int, int, float)", 1});
        cls.methods.push_back({"strokePath", "instance", "(Path, PathStrokeType, AffineTransform)", 1});
        classMap[cls.name] = cls;
    }

    // --- GroupComponent ---
    {
        ApiClass cls;
        cls.name = "GroupComponent";
        cls.methods.push_back({"getText", "instance", "()", 1});
        cls.methods.push_back({"setText", "instance", "(String)", 1});
        classMap[cls.name] = cls;
    }

    // --- Identifier ---
    {
        ApiClass cls;
        cls.name = "Identifier";
        cls.methods.push_back({"toString", "instance", "()", 1});
        cls.staticMethods.push_back({"isValidIdentifier", "static", "(String)", 1});
        classMap[cls.name] = cls;
    }

    // --- Image ---
    {
        ApiClass cls;
        cls.name = "Image";
        cls.methods.push_back({"desaturate", "instance", "()", 1});
        cls.methods.push_back({"duplicateIfShared", "instance", "()", 1});
        cls.methods.push_back({"getHeight", "instance", "()", 1});
        cls.methods.push_back({"getPixelAt", "instance", "(int, int)", 1});
        cls.methods.push_back({"getReferenceCount", "instance", "()", 1});
        cls.methods.push_back({"getWidth", "instance", "()", 1});
        cls.methods.push_back({"hasAlphaChannel", "instance", "()", 1});
        cls.methods.push_back({"isARGB", "instance", "()", 1});
        cls.methods.push_back({"isNull", "instance", "()", 1});
        cls.methods.push_back({"isRGB", "instance", "()", 1});
        cls.methods.push_back({"isSingleChannel", "instance", "()", 1});
        cls.methods.push_back({"isValid", "instance", "()", 1});
        cls.methods.push_back({"moveImageSection", "instance", "(int, int, int, int, int, int)", 1});
        cls.methods.push_back({"multiplyAllAlphas", "instance", "(float)", 1});
        cls.methods.push_back({"multiplyAlphaAt", "instance", "(int, int, float)", 1});
        cls.methods.push_back({"rescaled", "instance", "(int, int, ResamplingQuality)", 1});
        cls.methods.push_back({"setPixelAt", "instance", "(int, int, Colour)", 1});
        classMap[cls.name] = cls;
    }

    // --- ImageFileFormat ---
    {
        ApiClass cls;
        cls.name = "ImageFileFormat";
        cls.methods.push_back({"canUnderstand", "instance", "()", 1});
        cls.methods.push_back({"getFormatName", "instance", "()", 1});
        cls.methods.push_back({"loadFrom", "instance", "(f)", 1});
        classMap[cls.name] = cls;
    }

    // --- ImageType ---
    {
        ApiClass cls;
        cls.name = "ImageType";
        cls.methods.push_back({"convert", "instance", "(Image)", 1});
        cls.methods.push_back({"create", "instance", "()", 1});
        cls.methods.push_back({"getTypeID", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- InputStream ---
    {
        ApiClass cls;
        cls.name = "InputStream";
        cls.methods.push_back({"getPosition", "instance", "()", 1});
        cls.methods.push_back({"getTotalLength", "instance", "()", 1});
        cls.methods.push_back({"isExhausted", "instance", "()", 1});
        cls.methods.push_back({"readEntireStreamAsString", "instance", "()", 1});
        cls.methods.push_back({"readIntoMemoryBlock", "instance", "(MemoryBlock, ssize_t)", 1});
        cls.methods.push_back({"setPosition", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Justification ---
    {
        ApiClass cls;
        cls.name = "Justification";
        cls.methods.push_back({"getFlags", "instance", "()", 1});
        cls.methods.push_back({"getOnlyHorizontalFlags", "instance", "()", 1});
        cls.methods.push_back({"getOnlyVerticalFlags", "instance", "()", 1});
        cls.methods.push_back({"testFlags", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- KeyPress ---
    {
        ApiClass cls;
        cls.name = "KeyPress";
        cls.methods.push_back({"getKeyCode", "instance", "()", 1});
        cls.methods.push_back({"getModifiers", "instance", "()", 1});
        cls.methods.push_back({"getTextCharacter", "instance", "()", 1});
        cls.methods.push_back({"getTextDescription", "instance", "()", 1});
        cls.methods.push_back({"getTextDescriptionWithIcons", "instance", "()", 1});
        cls.methods.push_back({"isCurrentlyDown", "instance", "()", 1});
        cls.methods.push_back({"isKeyCode", "instance", "()", 1});
        cls.methods.push_back({"isValid", "instance", "()", 1});
        cls.staticMethods.push_back({"createFromDescription", "static", "(map->getStringAttribute)", 1});
        cls.staticMethods.push_back({"isKeyCurrentlyDown", "static", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Label ---
    {
        ApiClass cls;
        cls.name = "Label";
        cls.methods.push_back({"addListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"attachToComponent", "instance", "(Component, bool)", 1});
        cls.methods.push_back({"doesLossOfFocusDiscardChanges", "instance", "()", 1});
        cls.methods.push_back({"getAttachedComponent", "instance", "()", 1});
        cls.methods.push_back({"getFont", "instance", "()", 1});
        cls.methods.push_back({"getJustificationType", "instance", "()", 1});
        cls.methods.push_back({"getMinimumHorizontalScale", "instance", "()", 1});
        cls.methods.push_back({"getText", "instance", "(bool)", 1});
        cls.methods.push_back({"getTextValue", "instance", "()", 1});
        cls.methods.push_back({"hideEditor", "instance", "(bool)", 1});
        cls.methods.push_back({"isAttachedOnLeft", "instance", "()", 1});
        cls.methods.push_back({"isBeingEdited", "instance", "()", 1});
        cls.methods.push_back({"isEditable", "instance", "()", 1});
        cls.methods.push_back({"isEditableOnDoubleClick", "instance", "()", 1});
        cls.methods.push_back({"isEditableOnSingleClick", "instance", "()", 1});
        cls.methods.push_back({"removeListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"setBorderSize", "instance", "(BorderSize<int>)", 1});
        cls.methods.push_back({"setEditable", "instance", "(bool, bool, bool)", 1});
        cls.methods.push_back({"setFont", "instance", "(Font)", 1});
        cls.methods.push_back({"setJustificationType", "instance", "(Justification)", 1});
        cls.methods.push_back({"setMinimumHorizontalScale", "instance", "(float)", 1});
        cls.methods.push_back({"setText", "instance", "(String, NotificationType)", 1});
        cls.methods.push_back({"showEditor", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Line ---
    {
        ApiClass cls;
        cls.name = "Line";
        cls.methods.push_back({"applyTransform", "instance", "()", 1});
        cls.methods.push_back({"findNearestPointTo", "instance", "()", 1});
        cls.methods.push_back({"findNearestProportionalPositionTo", "instance", "()", 1});
        cls.methods.push_back({"getAngle", "instance", "()", 1});
        cls.methods.push_back({"getDistanceFromPoint", "instance", "()", 1});
        cls.methods.push_back({"getEnd", "instance", "()", 1});
        cls.methods.push_back({"getEndX", "instance", "()", 1});
        cls.methods.push_back({"getEndY", "instance", "()", 1});
        cls.methods.push_back({"getIntersection", "instance", "()", 1});
        cls.methods.push_back({"getLength", "instance", "()", 1});
        cls.methods.push_back({"getPointAlongLine", "instance", "()", 1});
        cls.methods.push_back({"getPointAlongLine", "instance", "()", 2});
        cls.methods.push_back({"getPointAlongLineProportionally", "instance", "()", 1});
        cls.methods.push_back({"getStart", "instance", "()", 1});
        cls.methods.push_back({"getStartX", "instance", "()", 1});
        cls.methods.push_back({"getStartY", "instance", "()", 1});
        cls.methods.push_back({"isHorizontal", "instance", "()", 1});
        cls.methods.push_back({"isPointAbove", "instance", "()", 1});
        cls.methods.push_back({"isVertical", "instance", "()", 1});
        cls.methods.push_back({"reversed", "instance", "()", 1});
        cls.methods.push_back({"setEnd", "instance", "()", 1});
        cls.methods.push_back({"setEnd", "instance", "()", 2});
        cls.methods.push_back({"setStart", "instance", "()", 1});
        cls.methods.push_back({"setStart", "instance", "()", 2});
        cls.methods.push_back({"withShortenedEnd", "instance", "()", 1});
        cls.methods.push_back({"withShortenedStart", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- LookAndFeel ---
    {
        ApiClass cls;
        cls.name = "LookAndFeel";
        cls.methods.push_back({"setImplementation", "instance", "(table)", 1});
        cls.methods.push_back({"setMethod", "instance", "(String, table)", 1});
        cls.staticMethods.push_back({"createDrawableComposite", "static", "()", 1});
        cls.staticMethods.push_back({"createDrawableImage", "static", "()", 1});
        cls.staticMethods.push_back({"createDrawablePath", "static", "()", 1});
        cls.staticMethods.push_back({"createDrawableRectangle", "static", "()", 1});
        cls.staticMethods.push_back({"createDrawableText", "static", "()", 1});
        cls.staticMethods.push_back({"createDropShadowEffect", "static", "()", 1});
        cls.staticMethods.push_back({"createGlowEffect", "static", "()", 1});
        cls.staticMethods.push_back({"createImageButton", "static", "()", 1});
        cls.staticMethods.push_back({"createLabel", "static", "()", 1});
        cls.staticMethods.push_back({"createTextButton", "static", "()", 1});
        cls.staticMethods.push_back({"createToggleButton", "static", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- MachineIDUtilities ---
    {
        ApiClass cls;
        cls.name = "MachineIDUtilities";
        cls.staticMethods.push_back({"getLocalMachineIDs", "static", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- MidiBuffer ---
    {
        ApiClass cls;
        cls.name = "MidiBuffer";
        cls.methods.push_back({"addEvent", "instance", "(void, int, int)", 1});
        cls.methods.push_back({"addEvents", "instance", "(MidiBuffer, int, int, int)", 1});
        cls.methods.push_back({"clear", "instance", "(int, int)", 1});
        cls.methods.push_back({"clear", "instance", "(int, int)", 2});
        cls.methods.push_back({"ensureSize", "instance", "(size_t)", 1});
        cls.methods.push_back({"getFirstEventTime", "instance", "()", 1});
        cls.methods.push_back({"getLastEventTime", "instance", "()", 1});
        cls.methods.push_back({"getNumEvents", "instance", "()", 1});
        cls.methods.push_back({"isEmpty", "instance", "()", 1});
        cls.methods.push_back({"swapWith", "instance", "(MidiBuffer)", 1});
        classMap[cls.name] = cls;
    }

    // --- MidiBufferIterator ---
    {
        ApiClass cls;
        cls.name = "MidiBufferIterator";
        cls.methods.push_back({"getNextEvent", "instance", "(MidiMessage, int)", 1});
        cls.methods.push_back({"setNextSamplePosition", "instance", "(int)", 1});
        classMap[cls.name] = cls;
    }

    // --- MidiMessage ---
    {
        ApiClass cls;
        cls.name = "MidiMessage";
        cls.methods.push_back({"addToTimeStamp", "instance", "()", 1});
        cls.methods.push_back({"getAfterTouchValue", "instance", "()", 1});
        cls.methods.push_back({"getChannel", "instance", "()", 1});
        cls.methods.push_back({"getChannelPressureValue", "instance", "()", 1});
        cls.methods.push_back({"getControllerNumber", "instance", "()", 1});
        cls.methods.push_back({"getControllerValue", "instance", "()", 1});
        cls.methods.push_back({"getFloatVelocity", "instance", "()", 1});
        cls.methods.push_back({"getKeySignatureNumberOfSharpsOrFlats", "instance", "()", 1});
        cls.methods.push_back({"getMetaEventData", "instance", "()", 1});
        cls.methods.push_back({"getMetaEventLength", "instance", "()", 1});
        cls.methods.push_back({"getMetaEventType", "instance", "()", 1});
        cls.methods.push_back({"getMidiChannelMetaEventChannel", "instance", "()", 1});
        cls.methods.push_back({"getNoteNumber", "instance", "()", 1});
        cls.methods.push_back({"getPitchWheelValue", "instance", "()", 1});
        cls.methods.push_back({"getProgramChangeNumber", "instance", "()", 1});
        cls.methods.push_back({"getRawData", "instance", "()", 1});
        cls.methods.push_back({"getRawDataSize", "instance", "()", 1});
        cls.methods.push_back({"getSongPositionPointerMidiBeat", "instance", "()", 1});
        cls.methods.push_back({"getSysExData", "instance", "()", 1});
        cls.methods.push_back({"getSysExDataSize", "instance", "()", 1});
        cls.methods.push_back({"getTempoMetaEventTickLength", "instance", "(short)", 1});
        cls.methods.push_back({"getTempoSecondsPerQuarterNote", "instance", "()", 1});
        cls.methods.push_back({"getTextFromTextMetaEvent", "instance", "()", 1});
        cls.methods.push_back({"getTimeSignatureInfo", "instance", "(int, int)", 1});
        cls.methods.push_back({"getTimeStamp", "instance", "()", 1});
        cls.methods.push_back({"getVelocity", "instance", "()", 1});
        cls.methods.push_back({"isActiveSense", "instance", "()", 1});
        cls.methods.push_back({"isAftertouch", "instance", "()", 1});
        cls.methods.push_back({"isAllNotesOff", "instance", "()", 1});
        cls.methods.push_back({"isAllSoundOff", "instance", "()", 1});
        cls.methods.push_back({"isChannelPressure", "instance", "()", 1});
        cls.methods.push_back({"isController", "instance", "()", 1});
        cls.methods.push_back({"isControllerOfType", "instance", "(int)", 1});
        cls.methods.push_back({"isEndOfTrackMetaEvent", "instance", "()", 1});
        cls.methods.push_back({"isForChannel", "instance", "(int)", 1});
        cls.methods.push_back({"isKeySignatureMetaEvent", "instance", "()", 1});
        cls.methods.push_back({"isMetaEvent", "instance", "()", 1});
        cls.methods.push_back({"isMidiChannelMetaEvent", "instance", "()", 1});
        cls.methods.push_back({"isMidiClock", "instance", "()", 1});
        cls.methods.push_back({"isMidiContinue", "instance", "()", 1});
        cls.methods.push_back({"isMidiStart", "instance", "()", 1});
        cls.methods.push_back({"isMidiStop", "instance", "()", 1});
        cls.methods.push_back({"isNoteOff", "instance", "(bool)", 1});
        cls.methods.push_back({"isNoteOn", "instance", "(bool)", 1});
        cls.methods.push_back({"isNoteOnOrOff", "instance", "()", 1});
        cls.methods.push_back({"isPitchWheel", "instance", "()", 1});
        cls.methods.push_back({"isProgramChange", "instance", "()", 1});
        cls.methods.push_back({"isSoftPedalOff", "instance", "()", 1});
        cls.methods.push_back({"isSoftPedalOn", "instance", "()", 1});
        cls.methods.push_back({"isSongPositionPointer", "instance", "()", 1});
        cls.methods.push_back({"isSostenutoPedalOff", "instance", "()", 1});
        cls.methods.push_back({"isSostenutoPedalOn", "instance", "()", 1});
        cls.methods.push_back({"isSustainPedalOff", "instance", "()", 1});
        cls.methods.push_back({"isSustainPedalOn", "instance", "()", 1});
        cls.methods.push_back({"isSysEx", "instance", "()", 1});
        cls.methods.push_back({"isTempoMetaEvent", "instance", "()", 1});
        cls.methods.push_back({"isTextMetaEvent", "instance", "()", 1});
        cls.methods.push_back({"isTimeSignatureMetaEvent", "instance", "()", 1});
        cls.methods.push_back({"isTrackMetaEvent", "instance", "()", 1});
        cls.methods.push_back({"isTrackNameEvent", "instance", "()", 1});
        cls.methods.push_back({"multiplyVelocity", "instance", "(float)", 1});
        cls.methods.push_back({"setChannel", "instance", "(int)", 1});
        cls.methods.push_back({"setNoteNumber", "instance", "(int)", 1});
        cls.methods.push_back({"setTimeStamp", "instance", "()", 1});
        cls.methods.push_back({"setVelocity", "instance", "(float)", 1});
        cls.staticMethods.push_back({"aftertouchChange", "static", "(ch, number, value)", 1});
        cls.staticMethods.push_back({"allControllersOff", "static", "(int)", 1});
        cls.staticMethods.push_back({"allNotesOff", "static", "(int)", 1});
        cls.staticMethods.push_back({"allSoundOff", "static", "(int)", 1});
        cls.staticMethods.push_back({"channelPressureChange", "static", "(ch, value)", 1});
        cls.staticMethods.push_back({"controllerEvent", "static", "(ch, number, value)", 1});
        cls.staticMethods.push_back({"createSysExMessage", "static", "(data.data()", 1});
        cls.staticMethods.push_back({"endOfTrack", "static", "()", 1});
        cls.staticMethods.push_back({"getControllerName", "static", "(int)", 1});
        cls.staticMethods.push_back({"getGMInstrumentBankName", "static", "(int)", 1});
        cls.staticMethods.push_back({"getGMInstrumentName", "static", "(int)", 1});
        cls.staticMethods.push_back({"getMessageLengthFromFirstByte", "static", "(d[0])", 1});
        cls.staticMethods.push_back({"getMidiNoteInHertz", "static", "(int, double)", 1});
        cls.staticMethods.push_back({"getMidiNoteName", "static", "(midiNoteNumber, true, true, octaveNumForMiddleC)", 1});
        cls.staticMethods.push_back({"getRhythmInstrumentName", "static", "(int)", 1});
        cls.staticMethods.push_back({"masterVolume", "static", "(float)", 1});
        cls.staticMethods.push_back({"midiChannelMetaEvent", "static", "(int)", 1});
        cls.staticMethods.push_back({"midiClock", "static", "()", 1});
        cls.staticMethods.push_back({"midiContinue", "static", "()", 1});
        cls.staticMethods.push_back({"midiStart", "static", "()", 1});
        cls.staticMethods.push_back({"midiStop", "static", "()", 1});
        cls.staticMethods.push_back({"noteOff", "static", "(ch, number, (uint8)", 1});
        cls.staticMethods.push_back({"noteOff", "static", "(ch, number, (uint8)", 2});
        cls.staticMethods.push_back({"noteOn", "static", "(ch, number, (uint8)", 1});
        cls.staticMethods.push_back({"noteOn", "static", "(ch, number, (uint8)", 2});
        cls.staticMethods.push_back({"pitchWheel", "static", "(ch, jmin<int>(getValueInternal(value)", 1});
        cls.staticMethods.push_back({"programChange", "static", "(ch, value)", 1});
        cls.staticMethods.push_back({"readVariableLengthVal", "static", "(copy.data()", 1});
        cls.staticMethods.push_back({"tempoMetaEvent", "static", "(int)", 1});
        cls.staticMethods.push_back({"timeSignatureMetaEvent", "static", "(int, int)", 1});
        classMap[cls.name] = cls;
    }

    // --- ModifierKeys ---
    {
        ApiClass cls;
        cls.name = "ModifierKeys";
        cls.methods.push_back({"getNumMouseButtonsDown", "instance", "()", 1});
        cls.methods.push_back({"getRawFlags", "instance", "()", 1});
        cls.methods.push_back({"isAltDown", "instance", "()", 1});
        cls.methods.push_back({"isAnyModifierKeyDown", "instance", "()", 1});
        cls.methods.push_back({"isCommandDown", "instance", "()", 1});
        cls.methods.push_back({"isCtrlDown", "instance", "()", 1});
        cls.methods.push_back({"isLeftButtonDown", "instance", "()", 1});
        cls.methods.push_back({"isMiddleButtonDown", "instance", "()", 1});
        cls.methods.push_back({"isPopupMenu", "instance", "()", 1});
        cls.methods.push_back({"isRightButtonDown", "instance", "()", 1});
        cls.methods.push_back({"isShiftDown", "instance", "()", 1});
        cls.methods.push_back({"testFlags", "instance", "()", 1});
        cls.staticMethods.push_back({"getCurrentModifiers", "static", "()", 1});
        cls.staticMethods.push_back({"getCurrentModifiersRealtime", "static", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- MouseCursor ---
    {
        ApiClass cls;
        cls.name = "MouseCursor";
        cls.staticMethods.push_back({"hideWaitCursor", "static", "()", 1});
        cls.staticMethods.push_back({"showWaitCursor", "static", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- MouseEvent ---
    {
        ApiClass cls;
        cls.name = "MouseEvent";
        cls.methods.push_back({"getDistanceFromDragStart", "instance", "()", 1});
        cls.methods.push_back({"getDistanceFromDragStartX", "instance", "()", 1});
        cls.methods.push_back({"getDistanceFromDragStartY", "instance", "()", 1});
        cls.methods.push_back({"getLengthOfMousePress", "instance", "()", 1});
        cls.methods.push_back({"getMouseDownPosition", "instance", "()", 1});
        cls.methods.push_back({"getMouseDownScreenPosition", "instance", "()", 1});
        cls.methods.push_back({"getMouseDownScreenX", "instance", "()", 1});
        cls.methods.push_back({"getMouseDownScreenY", "instance", "()", 1});
        cls.methods.push_back({"getMouseDownX", "instance", "()", 1});
        cls.methods.push_back({"getMouseDownY", "instance", "()", 1});
        cls.methods.push_back({"getNumberOfClicks", "instance", "()", 1});
        cls.methods.push_back({"getOffsetFromDragStart", "instance", "()", 1});
        cls.methods.push_back({"getPosition", "instance", "()", 1});
        cls.methods.push_back({"getScreenPosition", "instance", "()", 1});
        cls.methods.push_back({"getScreenX", "instance", "()", 1});
        cls.methods.push_back({"getScreenY", "instance", "()", 1});
        cls.methods.push_back({"mouseWasClicked", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- MouseInputSource ---
    {
        ApiClass cls;
        cls.name = "MouseInputSource";
        cls.methods.push_back({"setScreenPosition", "instance", "(Point<float>)", 1});
        classMap[cls.name] = cls;
    }

    // --- OggVorbisAudioFormat ---
    {
        ApiClass cls;
        cls.name = "OggVorbisAudioFormat";
        cls.methods.push_back({"canDoMono", "instance", "()", 1});
        cls.methods.push_back({"canDoStereo", "instance", "()", 1});
        cls.methods.push_back({"canHandleFile", "instance", "()", 1});
        cls.methods.push_back({"createReaderFor", "instance", "(InputStream, bool)", 1});
        cls.methods.push_back({"estimateOggFileQuality", "instance", "(File)", 1});
        cls.methods.push_back({"getQualityOptions", "instance", "()", 1});
        cls.methods.push_back({"isCompressed", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- OnlineUnlockStatus ---
    {
        ApiClass cls;
        cls.name = "OnlineUnlockStatus";
        cls.methods.push_back({"isUnlocked", "instance", "()", 1});
        cls.methods.push_back({"load", "instance", "()", 1});
        cls.methods.push_back({"save", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- OnlineUnlockStatusCheck ---
    {
        ApiClass cls;
        cls.name = "OnlineUnlockStatusCheck";
        cls.methods.push_back({"doesProductIDMatch", "instance", "(String)", 1});
        cls.methods.push_back({"getLocalMachineIDs", "instance", "()", 1});
        cls.methods.push_back({"getParsedErrorMessage", "instance", "()", 1});
        cls.methods.push_back({"getParsedInformativeMessage", "instance", "()", 1});
        cls.methods.push_back({"getParsedSuccess", "instance", "()", 1});
        cls.methods.push_back({"getParsedUrlToLaunch", "instance", "()", 1});
        cls.methods.push_back({"getProductID", "instance", "()", 1});
        cls.methods.push_back({"getPublicKey", "instance", "()", 1});
        cls.methods.push_back({"getServerAuthenticationURL", "instance", "()", 1});
        cls.methods.push_back({"getState", "instance", "()", 1});
        cls.methods.push_back({"getWebsiteName", "instance", "()", 1});
        cls.methods.push_back({"isPluginUnlocked", "instance", "()", 1});
        cls.methods.push_back({"isUnlocked", "instance", "()", 1});
        cls.methods.push_back({"readReplyFromWebserver", "instance", "(String, String)", 1});
        cls.methods.push_back({"saveState", "instance", "(String)", 1});
        cls.methods.push_back({"setUnlockCompletionCallback", "instance", "(table)", 1});
        cls.methods.push_back({"setUserEmail", "instance", "(String)", 1});
        cls.methods.push_back({"triggerWebserverUnlock", "instance", "(String, String)", 1});
        classMap[cls.name] = cls;
    }

    // --- Path ---
    {
        ApiClass cls;
        cls.name = "Path";
        cls.methods.push_back({"addArc", "instance", "(float, float, float, float, float, float, bool)", 1});
        cls.methods.push_back({"addArrow", "instance", "(Line<float>, float, float, float)", 1});
        cls.methods.push_back({"addBubble", "instance", "(Rectangle<float>, Rectangle<float>, Point<float>, float, float)", 1});
        cls.methods.push_back({"addCentredArc", "instance", "(float, float, float, float, float, float, float, bool)", 1});
        cls.methods.push_back({"addEllipse", "instance", "(Rectangle<float>)", 1});
        cls.methods.push_back({"addEllipse", "instance", "(Rectangle<float>)", 2});
        cls.methods.push_back({"addLineSegment", "instance", "(Line<float>, float)", 1});
        cls.methods.push_back({"addPath", "instance", "(Path, AffineTransform)", 1});
        cls.methods.push_back({"addPath", "instance", "(Path, AffineTransform)", 2});
        cls.methods.push_back({"addPieSegment", "instance", "(Rectangle<float>, float, float, float)", 1});
        cls.methods.push_back({"addPieSegment", "instance", "(Rectangle<float>, float, float, float)", 2});
        cls.methods.push_back({"addPolygon", "instance", "(Point<float>, int, float, float)", 1});
        cls.methods.push_back({"addQuadrilateral", "instance", "(float, float, float, float, float, float, float, float)", 1});
        cls.methods.push_back({"addRectangle", "instance", "(float, float, float, float)", 1});
        cls.methods.push_back({"addRoundedRectangle", "instance", "(float, float, float, float, float)", 1});
        cls.methods.push_back({"addRoundedRectangle", "instance", "(float, float, float, float, float)", 2});
        cls.methods.push_back({"addRoundedRectangle", "instance", "(float, float, float, float, float)", 3});
        cls.methods.push_back({"addStar", "instance", "(Point<float>, int, float, float, float)", 1});
        cls.methods.push_back({"addTriangle", "instance", "(Point<float>, Point<float>, Point<float>)", 1});
        cls.methods.push_back({"applyTransform", "instance", "(AffineTransform)", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"closeSubPath", "instance", "()", 1});
        cls.methods.push_back({"contains", "instance", "(Point<float>, float)", 1});
        cls.methods.push_back({"contains", "instance", "(Point<float>, float)", 2});
        cls.methods.push_back({"createPathWithRoundedCorners", "instance", "(float)", 1});
        cls.methods.push_back({"cubicTo", "instance", "(Point<float>, Point<float>, Point<float>)", 1});
        cls.methods.push_back({"cubicTo", "instance", "(Point<float>, Point<float>, Point<float>)", 2});
        cls.methods.push_back({"getBounds", "instance", "()", 1});
        cls.methods.push_back({"getBoundsTransformed", "instance", "(AffineTransform)", 1});
        cls.methods.push_back({"getClippedLine", "instance", "(Line<float>, bool)", 1});
        cls.methods.push_back({"getCurrentPosition", "instance", "()", 1});
        cls.methods.push_back({"getLength", "instance", "(AffineTransform, float)", 1});
        cls.methods.push_back({"getNearestPoint", "instance", "(Point<float>, Point<float>, AffineTransform, float)", 1});
        cls.methods.push_back({"getPointAlongPath", "instance", "(float, AffineTransform, float)", 1});
        cls.methods.push_back({"getTransformToScaleToFit", "instance", "(float, float, float, float, bool, Justification)", 1});
        cls.methods.push_back({"intersectsLine", "instance", "(Line<float>, float)", 1});
        cls.methods.push_back({"isEmpty", "instance", "()", 1});
        cls.methods.push_back({"isUsingNonZeroWinding", "instance", "()", 1});
        cls.methods.push_back({"lineTo", "instance", "(Point<float>)", 1});
        cls.methods.push_back({"lineTo", "instance", "(Point<float>)", 2});
        cls.methods.push_back({"loadPathFromData", "instance", "(void, size_t)", 1});
        cls.methods.push_back({"loadPathFromStream", "instance", "(InputStream)", 1});
        cls.methods.push_back({"quadraticTo", "instance", "(Point<float>, Point<float>)", 1});
        cls.methods.push_back({"quadraticTo", "instance", "(Point<float>, Point<float>)", 2});
        cls.methods.push_back({"restoreFromString", "instance", "(StringRef)", 1});
        cls.methods.push_back({"scaleToFit", "instance", "(float, float, float, float, bool)", 1});
        cls.methods.push_back({"setUsingNonZeroWinding", "instance", "(bool)", 1});
        cls.methods.push_back({"startNewSubPath", "instance", "(Point<float>)", 1});
        cls.methods.push_back({"startNewSubPath", "instance", "(Point<float>)", 2});
        cls.methods.push_back({"swapWithPath", "instance", "(Path)", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        cls.methods.push_back({"writePathToStream", "instance", "(OutputStream)", 1});
        classMap[cls.name] = cls;
    }

    // --- PathStrokeType ---
    {
        ApiClass cls;
        cls.name = "PathStrokeType";
        cls.methods.push_back({"createDashedStroke", "instance", "(destPath, sourcePath, dashLengthsFloat.getRawDataPointer()", 1});
        cls.methods.push_back({"createStrokeWithArrowheads", "instance", "(Path, Path, float, float, float, float, AffineTransform, float)", 1});
        cls.methods.push_back({"createStrokedPath", "instance", "(Path, Path, AffineTransform, float)", 1});
        cls.methods.push_back({"getEndStyle", "instance", "()", 1});
        cls.methods.push_back({"getJointStyle", "instance", "()", 1});
        cls.methods.push_back({"getStrokeThickness", "instance", "()", 1});
        cls.methods.push_back({"setEndStyle", "instance", "()", 1});
        cls.methods.push_back({"setJointStyle", "instance", "()", 1});
        cls.methods.push_back({"setStrokeThickness", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Point ---
    {
        ApiClass cls;
        cls.name = "Point";
        cls.methods.push_back({"addXY", "instance", "()", 1});
        cls.methods.push_back({"applyTransform", "instance", "()", 1});
        cls.methods.push_back({"getAngleToPoint", "instance", "()", 1});
        cls.methods.push_back({"getDistanceFrom", "instance", "()", 1});
        cls.methods.push_back({"getDistanceFromOrigin", "instance", "()", 1});
        cls.methods.push_back({"getPointOnCircumference", "instance", "()", 1});
        cls.methods.push_back({"getPointOnCircumference", "instance", "()", 2});
        cls.methods.push_back({"getX", "instance", "()", 1});
        cls.methods.push_back({"getY", "instance", "()", 1});
        cls.methods.push_back({"isOrigin", "instance", "()", 1});
        cls.methods.push_back({"setX", "instance", "()", 1});
        cls.methods.push_back({"setXY", "instance", "()", 1});
        cls.methods.push_back({"setY", "instance", "()", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        cls.methods.push_back({"transformedBy", "instance", "()", 1});
        cls.methods.push_back({"translated", "instance", "()", 1});
        cls.methods.push_back({"withX", "instance", "()", 1});
        cls.methods.push_back({"withY", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- RSAKey ---
    {
        ApiClass cls;
        cls.name = "RSAKey";
        cls.methods.push_back({"applyToValue", "instance", "(BigInteger)", 1});
        cls.methods.push_back({"isValid", "instance", "()", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- RSATools ---
    {
        ApiClass cls;
        cls.name = "RSATools";
        cls.staticMethods.push_back({"createKeyPair", "static", "()", 1});
        cls.staticMethods.push_back({"getDummy", "static", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Random ---
    {
        ApiClass cls;
        cls.name = "Random";
        cls.methods.push_back({"combineSeed", "instance", "()", 1});
        cls.methods.push_back({"fillBitsRandomly", "instance", "(BigInteger, int, int)", 1});
        cls.methods.push_back({"fillBitsRandomly", "instance", "(BigInteger, int, int)", 2});
        cls.methods.push_back({"nextBool", "instance", "()", 1});
        cls.methods.push_back({"nextDouble", "instance", "()", 1});
        cls.methods.push_back({"nextFloat", "instance", "()", 1});
        cls.methods.push_back({"nextInt", "instance", "(Range<int>)", 1});
        cls.methods.push_back({"nextInt", "instance", "(Range<int>)", 2});
        cls.methods.push_back({"nextInt64", "instance", "()", 1});
        cls.methods.push_back({"nextLargeNumber", "instance", "(BigInteger)", 1});
        cls.methods.push_back({"setSeed", "instance", "(int64)", 1});
        cls.methods.push_back({"setSeedRandomly", "instance", "()", 1});
        cls.staticMethods.push_back({"getSystemRandom", "static", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Range ---
    {
        ApiClass cls;
        cls.name = "Range";
        cls.methods.push_back({"clipValue", "instance", "()", 1});
        cls.methods.push_back({"constrainRange", "instance", "()", 1});
        cls.methods.push_back({"contains", "instance", "()", 1});
        cls.methods.push_back({"contains", "instance", "()", 2});
        cls.methods.push_back({"getEnd", "instance", "()", 1});
        cls.methods.push_back({"getIntersectionWith", "instance", "()", 1});
        cls.methods.push_back({"getLength", "instance", "()", 1});
        cls.methods.push_back({"getStart", "instance", "()", 1});
        cls.methods.push_back({"getUnionWith", "instance", "()", 1});
        cls.methods.push_back({"intersects", "instance", "()", 1});
        cls.methods.push_back({"isEmpty", "instance", "()", 1});
        cls.methods.push_back({"movedToEndAt", "instance", "()", 1});
        cls.methods.push_back({"movedToStartAt", "instance", "()", 1});
        cls.methods.push_back({"setEnd", "instance", "()", 1});
        cls.methods.push_back({"setLength", "instance", "()", 1});
        cls.methods.push_back({"setStart", "instance", "()", 1});
        cls.methods.push_back({"withEnd", "instance", "()", 1});
        cls.methods.push_back({"withLength", "instance", "()", 1});
        cls.methods.push_back({"withStart", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Rectangle ---
    {
        ApiClass cls;
        cls.name = "Rectangle";
        cls.methods.push_back({"contains", "instance", "()", 1});
        cls.methods.push_back({"contains", "instance", "()", 2});
        cls.methods.push_back({"contains", "instance", "()", 3});
        cls.methods.push_back({"contains", "instance", "()", 4});
        cls.methods.push_back({"enlargeIfAdjacent", "instance", "()", 1});
        cls.methods.push_back({"enlargeIfAdjacent", "instance", "()", 2});
        cls.methods.push_back({"expand", "instance", "()", 1});
        cls.methods.push_back({"expand", "instance", "()", 2});
        cls.methods.push_back({"expanded", "instance", "()", 1});
        cls.methods.push_back({"expanded", "instance", "()", 2});
        cls.methods.push_back({"expanded", "instance", "()", 3});
        cls.methods.push_back({"expanded", "instance", "()", 4});
        cls.methods.push_back({"findAreaContainingPoints", "instance", "()", 1});
        cls.methods.push_back({"findAreaContainingPoints", "instance", "()", 2});
        cls.methods.push_back({"fromString", "instance", "()", 1});
        cls.methods.push_back({"fromString", "instance", "()", 2});
        cls.methods.push_back({"getAspectRatio", "instance", "()", 1});
        cls.methods.push_back({"getAspectRatio", "instance", "()", 2});
        cls.methods.push_back({"getBottom", "instance", "()", 1});
        cls.methods.push_back({"getBottom", "instance", "()", 2});
        cls.methods.push_back({"getBottomLeft", "instance", "()", 1});
        cls.methods.push_back({"getBottomLeft", "instance", "()", 2});
        cls.methods.push_back({"getBottomRight", "instance", "()", 1});
        cls.methods.push_back({"getBottomRight", "instance", "()", 2});
        cls.methods.push_back({"getCentre", "instance", "()", 1});
        cls.methods.push_back({"getCentre", "instance", "()", 2});
        cls.methods.push_back({"getCentreX", "instance", "()", 1});
        cls.methods.push_back({"getCentreX", "instance", "()", 2});
        cls.methods.push_back({"getCentreY", "instance", "()", 1});
        cls.methods.push_back({"getCentreY", "instance", "()", 2});
        cls.methods.push_back({"getConstrainedPoint", "instance", "()", 1});
        cls.methods.push_back({"getConstrainedPoint", "instance", "()", 2});
        cls.methods.push_back({"getHeight", "instance", "()", 1});
        cls.methods.push_back({"getHeight", "instance", "()", 2});
        cls.methods.push_back({"getIntersection", "instance", "()", 1});
        cls.methods.push_back({"getIntersection", "instance", "()", 2});
        cls.methods.push_back({"getPosition", "instance", "()", 1});
        cls.methods.push_back({"getPosition", "instance", "()", 2});
        cls.methods.push_back({"getRight", "instance", "()", 1});
        cls.methods.push_back({"getRight", "instance", "()", 2});
        cls.methods.push_back({"getSmallestIntegerContainer", "instance", "()", 1});
        cls.methods.push_back({"getTopLeft", "instance", "()", 1});
        cls.methods.push_back({"getTopLeft", "instance", "()", 2});
        cls.methods.push_back({"getTopRight", "instance", "()", 1});
        cls.methods.push_back({"getTopRight", "instance", "()", 2});
        cls.methods.push_back({"getUnion", "instance", "()", 1});
        cls.methods.push_back({"getUnion", "instance", "()", 2});
        cls.methods.push_back({"getWidth", "instance", "()", 1});
        cls.methods.push_back({"getWidth", "instance", "()", 2});
        cls.methods.push_back({"getX", "instance", "()", 1});
        cls.methods.push_back({"getX", "instance", "()", 2});
        cls.methods.push_back({"getY", "instance", "()", 1});
        cls.methods.push_back({"getY", "instance", "()", 2});
        cls.methods.push_back({"intersectRectangles", "instance", "()", 1});
        cls.methods.push_back({"intersectRectangles", "instance", "()", 2});
        cls.methods.push_back({"isEmpty", "instance", "()", 1});
        cls.methods.push_back({"isEmpty", "instance", "()", 2});
        cls.methods.push_back({"leftTopRightBottom", "instance", "()", 1});
        cls.methods.push_back({"leftTopRightBottom", "instance", "()", 2});
        cls.methods.push_back({"reduce", "instance", "()", 1});
        cls.methods.push_back({"reduce", "instance", "()", 2});
        cls.methods.push_back({"reduceIfPartlyContainedIn", "instance", "()", 1});
        cls.methods.push_back({"reduceIfPartlyContainedIn", "instance", "()", 2});
        cls.methods.push_back({"reduced", "instance", "()", 1});
        cls.methods.push_back({"reduced", "instance", "()", 2});
        cls.methods.push_back({"reduced", "instance", "()", 3});
        cls.methods.push_back({"reduced", "instance", "()", 4});
        cls.methods.push_back({"removeFromBottom", "instance", "()", 1});
        cls.methods.push_back({"removeFromBottom", "instance", "()", 2});
        cls.methods.push_back({"removeFromLeft", "instance", "()", 1});
        cls.methods.push_back({"removeFromLeft", "instance", "()", 2});
        cls.methods.push_back({"removeFromRight", "instance", "()", 1});
        cls.methods.push_back({"removeFromRight", "instance", "()", 2});
        cls.methods.push_back({"removeFromTop", "instance", "()", 1});
        cls.methods.push_back({"removeFromTop", "instance", "()", 2});
        cls.methods.push_back({"setBottom", "instance", "()", 1});
        cls.methods.push_back({"setBottom", "instance", "()", 2});
        cls.methods.push_back({"setBounds", "instance", "()", 1});
        cls.methods.push_back({"setBounds", "instance", "()", 2});
        cls.methods.push_back({"setHeight", "instance", "()", 1});
        cls.methods.push_back({"setHeight", "instance", "()", 2});
        cls.methods.push_back({"setLeft", "instance", "()", 1});
        cls.methods.push_back({"setLeft", "instance", "()", 2});
        cls.methods.push_back({"setPosition", "instance", "()", 1});
        cls.methods.push_back({"setPosition", "instance", "()", 2});
        cls.methods.push_back({"setPosition", "instance", "()", 3});
        cls.methods.push_back({"setPosition", "instance", "()", 4});
        cls.methods.push_back({"setRight", "instance", "()", 1});
        cls.methods.push_back({"setRight", "instance", "()", 2});
        cls.methods.push_back({"setSize", "instance", "()", 1});
        cls.methods.push_back({"setSize", "instance", "()", 2});
        cls.methods.push_back({"setTop", "instance", "()", 1});
        cls.methods.push_back({"setTop", "instance", "()", 2});
        cls.methods.push_back({"setWidth", "instance", "()", 1});
        cls.methods.push_back({"setWidth", "instance", "()", 2});
        cls.methods.push_back({"setX", "instance", "()", 1});
        cls.methods.push_back({"setX", "instance", "()", 2});
        cls.methods.push_back({"setY", "instance", "()", 1});
        cls.methods.push_back({"setY", "instance", "()", 2});
        cls.methods.push_back({"toFloat", "instance", "()", 1});
        cls.methods.push_back({"toInt", "instance", "()", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        cls.methods.push_back({"toString", "instance", "()", 2});
        cls.methods.push_back({"transformedBy", "instance", "()", 1});
        cls.methods.push_back({"transformedBy", "instance", "()", 2});
        cls.methods.push_back({"translate", "instance", "()", 1});
        cls.methods.push_back({"translate", "instance", "()", 2});
        cls.methods.push_back({"translated", "instance", "()", 1});
        cls.methods.push_back({"translated", "instance", "()", 2});
        cls.methods.push_back({"withBottom", "instance", "()", 1});
        cls.methods.push_back({"withBottom", "instance", "()", 2});
        cls.methods.push_back({"withHeight", "instance", "()", 1});
        cls.methods.push_back({"withHeight", "instance", "()", 2});
        cls.methods.push_back({"withLeft", "instance", "()", 1});
        cls.methods.push_back({"withLeft", "instance", "()", 2});
        cls.methods.push_back({"withPosition", "instance", "()", 1});
        cls.methods.push_back({"withPosition", "instance", "()", 2});
        cls.methods.push_back({"withPosition", "instance", "()", 3});
        cls.methods.push_back({"withPosition", "instance", "()", 4});
        cls.methods.push_back({"withRight", "instance", "()", 1});
        cls.methods.push_back({"withRight", "instance", "()", 2});
        cls.methods.push_back({"withSize", "instance", "()", 1});
        cls.methods.push_back({"withSize", "instance", "()", 2});
        cls.methods.push_back({"withTop", "instance", "()", 1});
        cls.methods.push_back({"withTop", "instance", "()", 2});
        cls.methods.push_back({"withWidth", "instance", "()", 1});
        cls.methods.push_back({"withWidth", "instance", "()", 2});
        cls.methods.push_back({"withX", "instance", "()", 1});
        cls.methods.push_back({"withX", "instance", "()", 2});
        cls.methods.push_back({"withY", "instance", "()", 1});
        cls.methods.push_back({"withY", "instance", "()", 2});
        cls.methods.push_back({"withZeroOrigin", "instance", "()", 1});
        cls.methods.push_back({"withZeroOrigin", "instance", "()", 2});
        classMap[cls.name] = cls;
    }

    // --- RectanglePlacement ---
    {
        ApiClass cls;
        cls.name = "RectanglePlacement";
        cls.methods.push_back({"applyTo", "instance", "(double, double, double, double, double, double, double, double)", 1});
        cls.methods.push_back({"getFlags", "instance", "()", 1});
        cls.methods.push_back({"testFlags", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- RelativeParallelogram ---
    {
        ApiClass cls;
        cls.name = "RelativeParallelogram";
        cls.methods.push_back({"getBounds", "instance", "(Scope)", 1});
        cls.methods.push_back({"getPath", "instance", "(Path, Scope)", 1});
        cls.methods.push_back({"resetToPerpendicular", "instance", "(Scope)", 1});
        cls.methods.push_back({"resolveFourCorners", "instance", "(Point<float>, Scope)", 1});
        cls.methods.push_back({"resolveThreePoints", "instance", "(Point<float>, Scope)", 1});
        cls.staticMethods.push_back({"getBoundingBox", "static", "(Point<float>)", 1});
        cls.staticMethods.push_back({"getInternalCoordForPoint", "static", "(Point<float>, Point<float>)", 1});
        cls.staticMethods.push_back({"getPointForInternalCoord", "static", "(Point<float>, Point<float>)", 1});
        classMap[cls.name] = cls;
    }

    // --- RelativePoint ---
    {
        ApiClass cls;
        cls.name = "RelativePoint";
        cls.methods.push_back({"isDynamic", "instance", "()", 1});
        cls.methods.push_back({"moveToAbsolute", "instance", "(Point<float>, Scope)", 1});
        cls.methods.push_back({"resolve", "instance", "(Scope)", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Result ---
    {
        ApiClass cls;
        cls.name = "Result";
        cls.methods.push_back({"fail", "instance", "(\"Error:)", 1});
        cls.methods.push_back({"failed", "instance", "()", 1});
        cls.methods.push_back({"getErrorMessage", "instance", "()", 1});
        cls.methods.push_back({"ok", "instance", "()", 1});
        cls.methods.push_back({"wasOk", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- ScopedTryLock ---
    {
        ApiClass cls;
        cls.name = "ScopedTryLock";
        cls.methods.push_back({"isLocked", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Slider ---
    {
        ApiClass cls;
        cls.name = "Slider";
        cls.methods.push_back({"addListener", "instance", "()", 1});
        cls.methods.push_back({"getCurrentPopupDisplay", "instance", "()", 1});
        cls.methods.push_back({"getDoubleClickReturnValue", "instance", "()", 1});
        cls.methods.push_back({"getInterval", "instance", "()", 1});
        cls.methods.push_back({"getMaxValue", "instance", "()", 1});
        cls.methods.push_back({"getMaxValueObject", "instance", "()", 1});
        cls.methods.push_back({"getMaximum", "instance", "()", 1});
        cls.methods.push_back({"getMinValue", "instance", "()", 1});
        cls.methods.push_back({"getMinValueObject", "instance", "()", 1});
        cls.methods.push_back({"getMinimum", "instance", "()", 1});
        cls.methods.push_back({"getMouseDragSensitivity", "instance", "()", 1});
        cls.methods.push_back({"getPositionOfValue", "instance", "(double)", 1});
        cls.methods.push_back({"getSkewFactor", "instance", "()", 1});
        cls.methods.push_back({"getSliderSnapsToMousePosition", "instance", "()", 1});
        cls.methods.push_back({"getSliderStyle", "instance", "()", 1});
        cls.methods.push_back({"getTextBoxHeight", "instance", "()", 1});
        cls.methods.push_back({"getTextBoxPosition", "instance", "()", 1});
        cls.methods.push_back({"getTextBoxWidth", "instance", "()", 1});
        cls.methods.push_back({"getTextFromValue", "instance", "(double)", 1});
        cls.methods.push_back({"getTextValueSuffix", "instance", "()", 1});
        cls.methods.push_back({"getThumbBeingDragged", "instance", "()", 1});
        cls.methods.push_back({"getValue", "instance", "()", 1});
        cls.methods.push_back({"getValueFromText", "instance", "(String)", 1});
        cls.methods.push_back({"getValueObject", "instance", "()", 1});
        cls.methods.push_back({"getVelocityBasedMode", "instance", "()", 1});
        cls.methods.push_back({"getVelocityModeIsSwappable", "instance", "()", 1});
        cls.methods.push_back({"getVelocityOffset", "instance", "()", 1});
        cls.methods.push_back({"getVelocitySensitivity", "instance", "()", 1});
        cls.methods.push_back({"getVelocityThreshold", "instance", "()", 1});
        cls.methods.push_back({"hideTextBox", "instance", "(bool)", 1});
        cls.methods.push_back({"isHorizontal", "instance", "()", 1});
        cls.methods.push_back({"isTextBoxEditable", "instance", "()", 1});
        cls.methods.push_back({"isVertical", "instance", "()", 1});
        cls.methods.push_back({"proportionOfLengthToValue", "instance", "(double)", 1});
        cls.methods.push_back({"removeListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"setChangeNotificationOnlyOnRelease", "instance", "(bool)", 1});
        cls.methods.push_back({"setDoubleClickReturnValue", "instance", "(bool, double, ModifierKeys)", 1});
        cls.methods.push_back({"setIncDecButtonsMode", "instance", "(IncDecButtonMode)", 1});
        cls.methods.push_back({"setMaxValue", "instance", "(double, NotificationType, bool)", 1});
        cls.methods.push_back({"setMinAndMaxValues", "instance", "(double, double, NotificationType)", 1});
        cls.methods.push_back({"setMinValue", "instance", "(double, NotificationType, bool)", 1});
        cls.methods.push_back({"setMouseDragSensitivity", "instance", "(int)", 1});
        cls.methods.push_back({"setPopupDisplayEnabled", "instance", "(bool, bool, Component, int)", 1});
        cls.methods.push_back({"setPopupMenuEnabled", "instance", "(bool)", 1});
        cls.methods.push_back({"setRange", "instance", "(Range<double>, double)", 1});
        cls.methods.push_back({"setRotaryParameters", "instance", "(float, float, bool)", 1});
        cls.methods.push_back({"setScrollWheelEnabled", "instance", "(bool)", 1});
        cls.methods.push_back({"setSkewFactor", "instance", "(double, bool)", 1});
        cls.methods.push_back({"setSkewFactorFromMidPoint", "instance", "(double)", 1});
        cls.methods.push_back({"setSliderSnapsToMousePosition", "instance", "(bool)", 1});
        cls.methods.push_back({"setSliderStyle", "instance", "(SliderStyle)", 1});
        cls.methods.push_back({"setTextBoxIsEditable", "instance", "(bool)", 1});
        cls.methods.push_back({"setTextBoxStyle", "instance", "(TextEntryBoxPosition, bool, int, int)", 1});
        cls.methods.push_back({"setTextValueSuffix", "instance", "(String)", 1});
        cls.methods.push_back({"setValue", "instance", "()", 1});
        cls.methods.push_back({"setVelocityBasedMode", "instance", "(bool)", 1});
        cls.methods.push_back({"setVelocityModeParameters", "instance", "(double, int, double, bool, Flags)", 1});
        cls.methods.push_back({"showTextBox", "instance", "()", 1});
        cls.methods.push_back({"startedDragging", "instance", "()", 1});
        cls.methods.push_back({"stoppedDragging", "instance", "()", 1});
        cls.methods.push_back({"updateText", "instance", "()", 1});
        cls.methods.push_back({"valueChanged", "instance", "()", 1});
        cls.methods.push_back({"valueToProportionOfLength", "instance", "(double)", 1});
        classMap[cls.name] = cls;
    }

    // --- SparseSet ---
    {
        ApiClass cls;
        cls.name = "SparseSet";
        cls.methods.push_back({"addRange", "instance", "()", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"contains", "instance", "()", 1});
        cls.methods.push_back({"containsRange", "instance", "()", 1});
        cls.methods.push_back({"get", "instance", "(int)", 1});
        cls.methods.push_back({"getNumRanges", "instance", "()", 1});
        cls.methods.push_back({"getRange", "instance", "()", 1});
        cls.methods.push_back({"getTotalRange", "instance", "()", 1});
        cls.methods.push_back({"invertRange", "instance", "()", 1});
        cls.methods.push_back({"isEmpty", "instance", "()", 1});
        cls.methods.push_back({"overlapsRange", "instance", "()", 1});
        cls.methods.push_back({"removeRange", "instance", "()", 1});
        cls.methods.push_back({"size", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- String ---
    {
        ApiClass cls;
        cls.name = "String";
        cls.methods.push_back({"append", "instance", "(String, size_t)", 1});
        cls.methods.push_back({"compare", "instance", "(wchar_t)", 1});
        cls.methods.push_back({"compare", "instance", "(wchar_t)", 2});
        cls.methods.push_back({"compareIgnoreCase", "instance", "(String)", 1});
        cls.methods.push_back({"compareIgnoreCase", "instance", "(String)", 2});
        cls.methods.push_back({"compareNatural", "instance", "()", 1});
        cls.methods.push_back({"contains", "instance", "(StringRef)", 1});
        cls.methods.push_back({"containsAnyOf", "instance", "(StringRef)", 1});
        cls.methods.push_back({"containsIgnoreCase", "instance", "(StringRef)", 1});
        cls.methods.push_back({"containsNonWhitespaceChars", "instance", "()", 1});
        cls.methods.push_back({"containsOnly", "instance", "(StringRef)", 1});
        cls.methods.push_back({"containsWholeWord", "instance", "(StringRef)", 1});
        cls.methods.push_back({"containsWholeWordIgnoreCase", "instance", "(StringRef)", 1});
        cls.methods.push_back({"dropLastCharacters", "instance", "(int)", 1});
        cls.methods.push_back({"endsWith", "instance", "(StringRef)", 1});
        cls.methods.push_back({"equalsIgnoreCase", "instance", "(String)", 1});
        cls.methods.push_back({"equalsIgnoreCase", "instance", "(String)", 2});
        cls.methods.push_back({"fromFirstOccurrenceOf", "instance", "(StringRef, bool, bool)", 1});
        cls.methods.push_back({"fromLastOccurrenceOf", "instance", "(StringRef, bool, bool)", 1});
        cls.methods.push_back({"getDoubleValue", "instance", "()", 1});
        cls.methods.push_back({"getFloatValue", "instance", "()", 1});
        cls.methods.push_back({"getHexValue32", "instance", "()", 1});
        cls.methods.push_back({"getIntValue", "instance", "()", 1});
        cls.methods.push_back({"getLastCharacters", "instance", "(int)", 1});
        cls.methods.push_back({"getTrailingIntValue", "instance", "()", 1});
        cls.methods.push_back({"hashCode", "instance", "()", 1});
        cls.methods.push_back({"indexOf", "instance", "(int, StringRef)", 1});
        cls.methods.push_back({"indexOf", "instance", "(int, StringRef)", 2});
        cls.methods.push_back({"indexOfAnyOf", "instance", "(StringRef, int, bool)", 1});
        cls.methods.push_back({"indexOfIgnoreCase", "instance", "(int, StringRef)", 1});
        cls.methods.push_back({"indexOfWholeWord", "instance", "(StringRef)", 1});
        cls.methods.push_back({"indexOfWholeWordIgnoreCase", "instance", "(StringRef)", 1});
        cls.methods.push_back({"initialSectionContainingOnly", "instance", "(StringRef)", 1});
        cls.methods.push_back({"initialSectionNotContaining", "instance", "(StringRef)", 1});
        cls.methods.push_back({"isEmpty", "instance", "()", 1});
        cls.methods.push_back({"isNotEmpty", "instance", "()", 1});
        cls.methods.push_back({"isQuotedString", "instance", "()", 1});
        cls.methods.push_back({"lastIndexOf", "instance", "(StringRef)", 1});
        cls.methods.push_back({"lastIndexOfAnyOf", "instance", "(StringRef, bool)", 1});
        cls.methods.push_back({"lastIndexOfIgnoreCase", "instance", "(StringRef)", 1});
        cls.methods.push_back({"length", "instance", "()", 1});
        cls.methods.push_back({"matchesWildcard", "instance", "(StringRef, bool)", 1});
        cls.methods.push_back({"paddedLeft", "instance", "(juce_wchar, int)", 1});
        cls.methods.push_back({"paddedRight", "instance", "(juce_wchar, int)", 1});
        cls.methods.push_back({"quoted", "instance", "(juce_wchar)", 1});
        cls.methods.push_back({"removeCharacters", "instance", "(StringRef)", 1});
        cls.methods.push_back({"repeatedString", "instance", "(StringRef, int)", 1});
        cls.methods.push_back({"replace", "instance", "(StringRef, StringRef, bool)", 1});
        cls.methods.push_back({"replaceCharacters", "instance", "(StringRef, StringRef)", 1});
        cls.methods.push_back({"replaceSection", "instance", "(int, int, StringRef)", 1});
        cls.methods.push_back({"retainCharacters", "instance", "(StringRef)", 1});
        cls.methods.push_back({"startsWith", "instance", "(StringRef)", 1});
        cls.methods.push_back({"startsWithIgnoreCase", "instance", "(StringRef)", 1});
        cls.methods.push_back({"substring", "instance", "(int)", 1});
        cls.methods.push_back({"substring", "instance", "(int)", 2});
        cls.methods.push_back({"swapWith", "instance", "(String)", 1});
        cls.methods.push_back({"toLowerCase", "instance", "()", 1});
        cls.methods.push_back({"toUTF8", "instance", "()", 1});
        cls.methods.push_back({"toUpperCase", "instance", "()", 1});
        cls.methods.push_back({"trim", "instance", "()", 1});
        cls.methods.push_back({"trimCharactersAtEnd", "instance", "(StringRef)", 1});
        cls.methods.push_back({"trimCharactersAtStart", "instance", "(StringRef)", 1});
        cls.methods.push_back({"trimEnd", "instance", "()", 1});
        cls.methods.push_back({"unquoted", "instance", "()", 1});
        cls.methods.push_back({"upToFirstOccurrenceOf", "instance", "(StringRef, bool, bool)", 1});
        cls.methods.push_back({"upToLastOccurrenceOf", "instance", "(StringRef, bool, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- StringArray ---
    {
        ApiClass cls;
        cls.name = "StringArray";
        cls.methods.push_back({"add", "instance", "(String)", 1});
        cls.methods.push_back({"addArray", "instance", "(StringArray, int, int)", 1});
        cls.methods.push_back({"addIfNotAlreadyThere", "instance", "(String, bool)", 1});
        cls.methods.push_back({"addLines", "instance", "(StringRef)", 1});
        cls.methods.push_back({"addTokens", "instance", "(StringRef, StringRef, StringRef)", 1});
        cls.methods.push_back({"addTokens", "instance", "(StringRef, StringRef, StringRef)", 2});
        cls.methods.push_back({"appendNumbersToDuplicates", "instance", "(bool, bool, CharPointer_UTF8, CharPointer_UTF8)", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"contains", "instance", "(StringRef, bool)", 1});
        cls.methods.push_back({"get", "instance", "()", 1});
        cls.methods.push_back({"indexOf", "instance", "(StringRef, bool, int)", 1});
        cls.methods.push_back({"insert", "instance", "(int, String)", 1});
        cls.methods.push_back({"joinIntoString", "instance", "(StringRef, int, int)", 1});
        cls.methods.push_back({"minimiseStorageOverheads", "instance", "()", 1});
        cls.methods.push_back({"move", "instance", "(int, int)", 1});
        cls.methods.push_back({"remove", "instance", "(int)", 1});
        cls.methods.push_back({"removeDuplicates", "instance", "(bool)", 1});
        cls.methods.push_back({"removeEmptyStrings", "instance", "(bool)", 1});
        cls.methods.push_back({"removeRange", "instance", "(int, int)", 1});
        cls.methods.push_back({"removeString", "instance", "(StringRef, bool)", 1});
        cls.methods.push_back({"set", "instance", "(int, String)", 1});
        cls.methods.push_back({"size", "instance", "()", 1});
        cls.methods.push_back({"sort", "instance", "(bool)", 1});
        cls.methods.push_back({"trim", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- StringPairArray ---
    {
        ApiClass cls;
        cls.name = "StringPairArray";
        cls.methods.push_back({"addArray", "instance", "(StringPairArray)", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"getAllKeys", "instance", "()", 1});
        cls.methods.push_back({"getAllValues", "instance", "()", 1});
        cls.methods.push_back({"getDescription", "instance", "()", 1});
        cls.methods.push_back({"getValue", "instance", "(StringRef, String)", 1});
        cls.methods.push_back({"minimiseStorageOverheads", "instance", "()", 1});
        cls.methods.push_back({"remove", "instance", "(int)", 1});
        cls.methods.push_back({"set", "instance", "(String, String)", 1});
        cls.methods.push_back({"setIgnoresCase", "instance", "(bool)", 1});
        cls.methods.push_back({"size", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- StringRef ---
    {
        ApiClass cls;
        cls.name = "StringRef";
        cls.methods.push_back({"isEmpty", "instance", "()", 1});
        cls.methods.push_back({"isNotEmpty", "instance", "()", 1});
        cls.methods.push_back({"length", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- TextEditor ---
    {
        ApiClass cls;
        cls.name = "TextEditor";
        cls.methods.push_back({"addListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"addPopupMenuItems", "instance", "(PopupMenu, MouseEvent)", 1});
        cls.methods.push_back({"applyFontToAllText", "instance", "(Font, bool)", 1});
        cls.methods.push_back({"areScrollbarsShown", "instance", "()", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"copy", "instance", "()", 1});
        cls.methods.push_back({"copyToClipboard", "instance", "()", 1});
        cls.methods.push_back({"cut", "instance", "()", 1});
        cls.methods.push_back({"cutToClipboard", "instance", "()", 1});
        cls.methods.push_back({"deleteBackwards", "instance", "(bool)", 1});
        cls.methods.push_back({"deleteForwards", "instance", "(bool)", 1});
        cls.methods.push_back({"getBorder", "instance", "()", 1});
        cls.methods.push_back({"getCaretPosition", "instance", "()", 1});
        cls.methods.push_back({"getCaretRectangle", "instance", "()", 1});
        cls.methods.push_back({"getFont", "instance", "()", 1});
        cls.methods.push_back({"getHighlightedRegion", "instance", "()", 1});
        cls.methods.push_back({"getHighlightedText", "instance", "()", 1});
        cls.methods.push_back({"getPasswordCharacter", "instance", "()", 1});
        cls.methods.push_back({"getReturnKeyStartsNewLine", "instance", "()", 1});
        cls.methods.push_back({"getText", "instance", "()", 1});
        cls.methods.push_back({"getTextInRange", "instance", "(Range<int>)", 1});
        cls.methods.push_back({"getTextIndexAt", "instance", "(int, int)", 1});
        cls.methods.push_back({"getTextValue", "instance", "()", 1});
        cls.methods.push_back({"getTextWidth", "instance", "()", 1});
        cls.methods.push_back({"getTotalNumChars", "instance", "()", 1});
        cls.methods.push_back({"insertTextAtCaret", "instance", "(\"\n\")", 1});
        cls.methods.push_back({"isCaretVisible", "instance", "()", 1});
        cls.methods.push_back({"isEmpty", "instance", "()", 1});
        cls.methods.push_back({"isMultiLine", "instance", "()", 1});
        cls.methods.push_back({"isPopupMenuCurrentlyActive", "instance", "()", 1});
        cls.methods.push_back({"isPopupMenuEnabled", "instance", "()", 1});
        cls.methods.push_back({"isReadOnly", "instance", "()", 1});
        cls.methods.push_back({"isTabKeyUsedAsCharacter", "instance", "()", 1});
        cls.methods.push_back({"moveCaretDown", "instance", "(bool)", 1});
        cls.methods.push_back({"moveCaretLeft", "instance", "(bool, bool)", 1});
        cls.methods.push_back({"moveCaretRight", "instance", "(bool, bool)", 1});
        cls.methods.push_back({"moveCaretToEnd", "instance", "(bool)", 1});
        cls.methods.push_back({"moveCaretToEnd", "instance", "(bool)", 2});
        cls.methods.push_back({"moveCaretToEndOfLine", "instance", "(bool)", 1});
        cls.methods.push_back({"moveCaretToStartOfLine", "instance", "(bool)", 1});
        cls.methods.push_back({"moveCaretToTop", "instance", "(bool)", 1});
        cls.methods.push_back({"moveCaretUp", "instance", "(bool)", 1});
        cls.methods.push_back({"pageDown", "instance", "(bool)", 1});
        cls.methods.push_back({"pageUp", "instance", "(bool)", 1});
        cls.methods.push_back({"paste", "instance", "()", 1});
        cls.methods.push_back({"pasteFromClipboard", "instance", "()", 1});
        cls.methods.push_back({"redo", "instance", "()", 1});
        cls.methods.push_back({"redo", "instance", "()", 2});
        cls.methods.push_back({"removeListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"scrollDown", "instance", "()", 1});
        cls.methods.push_back({"scrollEditorToPositionCaret", "instance", "(int, int)", 1});
        cls.methods.push_back({"scrollUp", "instance", "()", 1});
        cls.methods.push_back({"selectAll", "instance", "()", 1});
        cls.methods.push_back({"setBorder", "instance", "(BorderSize<int>)", 1});
        cls.methods.push_back({"setCaretPosition", "instance", "(int)", 1});
        cls.methods.push_back({"setCaretVisible", "instance", "(bool)", 1});
        cls.methods.push_back({"setEscapeAndReturnKeysConsumed", "instance", "(bool)", 1});
        cls.methods.push_back({"setFont", "instance", "(Font)", 1});
        cls.methods.push_back({"setHighlightedRegion", "instance", "(Range<int>)", 1});
        cls.methods.push_back({"setIndents", "instance", "(int, int)", 1});
        cls.methods.push_back({"setInputFilter", "instance", "(InputFilter, bool)", 1});
        cls.methods.push_back({"setInputRestrictions", "instance", "(int, String)", 1});
        cls.methods.push_back({"setMultiLine", "instance", "(bool, bool)", 1});
        cls.methods.push_back({"setPasswordCharacter", "instance", "(juce_wchar)", 1});
        cls.methods.push_back({"setPopupMenuEnabled", "instance", "(bool)", 1});
        cls.methods.push_back({"setReadOnly", "instance", "(bool)", 1});
        cls.methods.push_back({"setReturnKeyStartsNewLine", "instance", "(bool)", 1});
        cls.methods.push_back({"setScrollBarThickness", "instance", "(int)", 1});
        cls.methods.push_back({"setScrollToShowCursor", "instance", "(bool)", 1});
        cls.methods.push_back({"setScrollbarsShown", "instance", "(bool)", 1});
        cls.methods.push_back({"setSelectAllWhenFocused", "instance", "(bool)", 1});
        cls.methods.push_back({"setTabKeyUsedAsCharacter", "instance", "(bool)", 1});
        cls.methods.push_back({"setText", "instance", "(String, bool)", 1});
        cls.methods.push_back({"setTextToShowWhenEmpty", "instance", "(String, Colour)", 1});
        cls.methods.push_back({"undo", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Thread ---
    {
        ApiClass cls;
        cls.name = "Thread";
        cls.methods.push_back({"getThreadId", "instance", "()", 1});
        cls.methods.push_back({"getThreadName", "instance", "()", 1});
        cls.methods.push_back({"isThreadRunning", "instance", "()", 1});
        cls.methods.push_back({"notify", "instance", "()", 1});
        cls.methods.push_back({"setAffinityMask", "instance", "(int)", 1});
        cls.methods.push_back({"setPriority", "instance", "(int)", 1});
        cls.methods.push_back({"setThreadFunction", "instance", "(table)", 1});
        cls.methods.push_back({"signalThreadShouldExit", "instance", "()", 1});
        cls.methods.push_back({"startThread", "instance", "(int)", 1});
        cls.methods.push_back({"startThread", "instance", "(int)", 2});
        cls.methods.push_back({"threadShoudExit", "instance", "()", 1});
        cls.methods.push_back({"wait", "instance", "(int)", 1});
        cls.methods.push_back({"waitForThreadToExit", "instance", "(int)", 1});
        classMap[cls.name] = cls;
    }

    // --- Time ---
    {
        ApiClass cls;
        cls.name = "Time";
        cls.methods.push_back({"formatted", "instance", "(String)", 1});
        cls.methods.push_back({"getDayOfMonth", "instance", "()", 1});
        cls.methods.push_back({"getDayOfWeek", "instance", "()", 1});
        cls.methods.push_back({"getDayOfYear", "instance", "()", 1});
        cls.methods.push_back({"getHours", "instance", "()", 1});
        cls.methods.push_back({"getHoursInAmPmFormat", "instance", "()", 1});
        cls.methods.push_back({"getMilliseconds", "instance", "()", 1});
        cls.methods.push_back({"getMinutes", "instance", "()", 1});
        cls.methods.push_back({"getMonth", "instance", "()", 1});
        cls.methods.push_back({"getMonthName", "instance", "(int, bool)", 1});
        cls.methods.push_back({"getSeconds", "instance", "()", 1});
        cls.methods.push_back({"getTimeZone", "instance", "()", 1});
        cls.methods.push_back({"getWeekdayName", "instance", "(int, bool)", 1});
        cls.methods.push_back({"getYear", "instance", "()", 1});
        cls.methods.push_back({"isAfternoon", "instance", "()", 1});
        cls.methods.push_back({"isDaylightSavingTime", "instance", "()", 1});
        cls.methods.push_back({"setSystemTimeToThisTime", "instance", "()", 1});
        cls.methods.push_back({"toMilliseconds", "instance", "()", 1});
        cls.methods.push_back({"toString", "instance", "(bool, bool, bool, bool)", 1});
        cls.staticMethods.push_back({"currentTimeMillis", "static", "()", 1});
        cls.staticMethods.push_back({"getApproximateMillisecondCounter", "static", "()", 1});
        cls.staticMethods.push_back({"getCurrentTime", "static", "()", 1});
        cls.staticMethods.push_back({"getHighResolutionTicks", "static", "()", 1});
        cls.staticMethods.push_back({"getHighResolutionTicksPerSecond", "static", "()", 1});
        cls.staticMethods.push_back({"getMillisecondCounter", "static", "()", 1});
        cls.staticMethods.push_back({"getMillisecondCounterHiRes", "static", "()", 1});
        cls.staticMethods.push_back({"highResolutionTicksToSeconds", "static", "(int64)", 1});
        cls.staticMethods.push_back({"secondsToHighResolutionTicks", "static", "(seconds)", 1});
        cls.staticMethods.push_back({"waitForMillisecondCounter", "static", "(eventTime)", 1});
        classMap[cls.name] = cls;
    }

    // --- Timer ---
    {
        ApiClass cls;
        cls.name = "Timer";
        cls.methods.push_back({"getTimerInterval", "instance", "()", 1});
        cls.methods.push_back({"isTimerRunning", "instance", "()", 1});
        cls.methods.push_back({"setCallback", "instance", "(table)", 1});
        cls.methods.push_back({"startTimer", "instance", "(int)", 1});
        cls.methods.push_back({"stopTimer", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- ToggleButton ---
    {
        ApiClass cls;
        cls.name = "ToggleButton";
        cls.methods.push_back({"changeWidthToFitText", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- Typeface ---
    {
        ApiClass cls;
        cls.name = "Typeface";
        cls.methods.push_back({"applyVerticalHintingTransform", "instance", "(float, Path)", 1});
        cls.methods.push_back({"getAscent", "instance", "()", 1});
        cls.methods.push_back({"getDescent", "instance", "()", 1});
        cls.methods.push_back({"getEdgeTableForGlyph", "instance", "(int, AffineTransform, float)", 1});
        cls.methods.push_back({"getGlyphPositions", "instance", "()", 1});
        cls.methods.push_back({"getHeightToPointsFactor", "instance", "()", 1});
        cls.methods.push_back({"getName", "instance", "()", 1});
        cls.methods.push_back({"getOutlineForGlyph", "instance", "()", 1});
        cls.methods.push_back({"getStringWidth", "instance", "()", 1});
        cls.methods.push_back({"getStyle", "instance", "()", 1});
        cls.methods.push_back({"isHinted", "instance", "()", 1});
        cls.methods.push_back({"isSuitableForFont", "instance", "()", 1});
        cls.staticMethods.push_back({"clearTypefaceCache", "static", "()", 1});
        cls.staticMethods.push_back({"createSystemTypefaceFor", "static", "(fontStream)", 1});
        cls.staticMethods.push_back({"createSystemTypefaceFor", "static", "(fontStream)", 2});
        cls.staticMethods.push_back({"scanFolderForFonts", "static", "(File)", 1});
        cls.staticMethods.push_back({"setTypefaceCacheSize", "static", "(int)", 1});
        classMap[cls.name] = cls;
    }

    // --- URL ---
    {
        ApiClass cls;
        cls.name = "URL";
        cls.methods.push_back({"addEscapeChars", "instance", "(String, bool, bool)", 1});
        cls.methods.push_back({"getDomain", "instance", "()", 1});
        cls.methods.push_back({"getPostData", "instance", "()", 1});
        cls.methods.push_back({"getScheme", "instance", "()", 1});
        cls.methods.push_back({"getSubPath", "instance", "(bool)", 1});
        cls.methods.push_back({"isProbablyAWebsiteURL", "instance", "(String)", 1});
        cls.methods.push_back({"isProbablyAnEmailAddress", "instance", "(String)", 1});
        cls.methods.push_back({"isWellFormed", "instance", "()", 1});
        cls.methods.push_back({"launchInDefaultBrowser", "instance", "()", 1});
        cls.methods.push_back({"readEntireBinaryStream", "instance", "(MemoryBlock, bool)", 1});
        cls.methods.push_back({"readEntireTextStream", "instance", "(bool)", 1});
        cls.methods.push_back({"removeEscapeChars", "instance", "(String)", 1});
        cls.methods.push_back({"toString", "instance", "(bool)", 1});
        cls.methods.push_back({"withFileToUpload", "instance", "(String, File, String)", 1});
        cls.methods.push_back({"withNewSubPath", "instance", "(String)", 1});
        cls.methods.push_back({"withPOSTData", "instance", "(MemoryBlock)", 1});
        cls.methods.push_back({"withPOSTData", "instance", "(MemoryBlock)", 2});
        cls.methods.push_back({"withParameter", "instance", "(String, String)", 1});
        classMap[cls.name] = cls;
    }

    // --- Value ---
    {
        ApiClass cls;
        cls.name = "Value";
        cls.methods.push_back({"addListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"getValue", "instance", "()", 1});
        cls.methods.push_back({"getValueSource", "instance", "()", 1});
        cls.methods.push_back({"referTo", "instance", "(Value)", 1});
        cls.methods.push_back({"refersToSameSourceAs", "instance", "(Value)", 1});
        cls.methods.push_back({"removeListener", "instance", "(Listener)", 1});
        cls.methods.push_back({"setValue", "instance", "(var)", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- ValueTree ---
    {
        ApiClass cls;
        cls.name = "ValueTree";
        cls.methods.push_back({"addChild", "instance", "(ValueTree, int, UndoManager)", 1});
        cls.methods.push_back({"addChild", "instance", "(ValueTree, int, UndoManager)", 2});
        cls.methods.push_back({"addListener", "instance", "()", 1});
        cls.methods.push_back({"createCopy", "instance", "()", 1});
        cls.methods.push_back({"getChild", "instance", "(int)", 1});
        cls.methods.push_back({"getChildWithName", "instance", "(Identifier)", 1});
        cls.methods.push_back({"getChildWithProperty", "instance", "(Identifier, var)", 1});
        cls.methods.push_back({"getNumChildren", "instance", "()", 1});
        cls.methods.push_back({"getNumProperties", "instance", "()", 1});
        cls.methods.push_back({"getOrCreateChildWithName", "instance", "(Identifier, UndoManager)", 1});
        cls.methods.push_back({"getParent", "instance", "()", 1});
        cls.methods.push_back({"getProperty", "instance", "(Identifier, var)", 1});
        cls.methods.push_back({"getProperty", "instance", "(Identifier, var)", 2});
        cls.methods.push_back({"getPropertyName", "instance", "(int)", 1});
        cls.methods.push_back({"getSibling", "instance", "(int)", 1});
        cls.methods.push_back({"getType", "instance", "()", 1});
        cls.methods.push_back({"hasProperty", "instance", "(Identifier)", 1});
        cls.methods.push_back({"hasType", "instance", "(Identifier)", 1});
        cls.methods.push_back({"indexOf", "instance", "(ValueTree)", 1});
        cls.methods.push_back({"isAChildOf", "instance", "(ValueTree)", 1});
        cls.methods.push_back({"isEquivalentTo", "instance", "(ValueTree)", 1});
        cls.methods.push_back({"isValid", "instance", "()", 1});
        cls.methods.push_back({"moveChild", "instance", "(int, int, UndoManager)", 1});
        cls.methods.push_back({"removeAllChildren", "instance", "(UndoManager)", 1});
        cls.methods.push_back({"removeAllProperties", "instance", "(UndoManager)", 1});
        cls.methods.push_back({"removeChild", "instance", "(ValueTree, UndoManager)", 1});
        cls.methods.push_back({"removeChild", "instance", "(ValueTree, UndoManager)", 2});
        cls.methods.push_back({"removeListener", "instance", "()", 1});
        cls.methods.push_back({"removeProperty", "instance", "(Identifier, UndoManager)", 1});
        cls.methods.push_back({"sendPropertyChangeMessage", "instance", "(Identifier)", 1});
        cls.methods.push_back({"setProperty", "instance", "(Identifier, var, UndoManager)", 1});
        cls.methods.push_back({"toXmlString", "instance", "(TextFormat)", 1});
        cls.methods.push_back({"writeToStream", "instance", "(OutputStream)", 1});
        classMap[cls.name] = cls;
    }

    // --- WavAudioFormat ---
    {
        ApiClass cls;
        cls.name = "WavAudioFormat";
        cls.methods.push_back({"canDoMono", "instance", "()", 1});
        cls.methods.push_back({"canDoStereo", "instance", "()", 1});
        cls.methods.push_back({"canHandleFile", "instance", "()", 1});
        cls.methods.push_back({"createBWAVMetadata", "instance", "(\"description\", \"originator\", \"originatorRef\", getCurrentTime()", 1});
        cls.methods.push_back({"createReaderFor", "instance", "(InputStream, bool)", 1});
        cls.methods.push_back({"replaceMetadataInFile", "instance", "(File, StringPairArray)", 1});
        classMap[cls.name] = cls;
    }

    // --- XmlDocument ---
    {
        ApiClass cls;
        cls.name = "XmlDocument";
        cls.methods.push_back({"getDocumentElement", "instance", "(bool)", 1});
        cls.methods.push_back({"getLastParseError", "instance", "()", 1});
        cls.methods.push_back({"setEmptyTextElementsIgnored", "instance", "(bool)", 1});
        cls.methods.push_back({"setInputSource", "instance", "(InputSource)", 1});
        cls.staticMethods.push_back({"parseXML", "static", "()", 1});
        cls.staticMethods.push_back({"parseXML", "static", "()", 2});
        classMap[cls.name] = cls;
    }

    // --- XmlElement ---
    {
        ApiClass cls;
        cls.name = "XmlElement";
        cls.methods.push_back({"addChildElement", "instance", "(XmlElement)", 1});
        cls.methods.push_back({"addTextElement", "instance", "(String)", 1});
        cls.methods.push_back({"compareAttribute", "instance", "(StringRef, StringRef, bool)", 1});
        cls.methods.push_back({"containsChildElement", "instance", "(XmlElement)", 1});
        cls.methods.push_back({"createDocument", "instance", "(StringRef, bool, bool, StringRef, int)", 1});
        cls.methods.push_back({"createNewChildElement", "instance", "(StringRef)", 1});
        cls.methods.push_back({"deleteAllChildElements", "instance", "()", 1});
        cls.methods.push_back({"deleteAllChildElementsWithTagName", "instance", "(StringRef)", 1});
        cls.methods.push_back({"deleteAllTextElements", "instance", "()", 1});
        cls.methods.push_back({"findParentElementOf", "instance", "(XmlElement)", 1});
        cls.methods.push_back({"getAllSubText", "instance", "()", 1});
        cls.methods.push_back({"getAttributeName", "instance", "(int)", 1});
        cls.methods.push_back({"getAttributeValue", "instance", "(int)", 1});
        cls.methods.push_back({"getBoolAttribute", "instance", "(StringRef, bool)", 1});
        cls.methods.push_back({"getChildByAttribute", "instance", "(StringRef, StringRef)", 1});
        cls.methods.push_back({"getChildByName", "instance", "(StringRef)", 1});
        cls.methods.push_back({"getChildElement", "instance", "(int)", 1});
        cls.methods.push_back({"getChildElementAllSubText", "instance", "(StringRef, String)", 1});
        cls.methods.push_back({"getChildElementAllSubText", "instance", "(StringRef, String)", 2});
        cls.methods.push_back({"getDoubleAttribute", "instance", "(StringRef, double)", 1});
        cls.methods.push_back({"getFirstChildElement", "instance", "()", 1});
        cls.methods.push_back({"getIntAttribute", "instance", "(StringRef, int)", 1});
        cls.methods.push_back({"getNamespace", "instance", "()", 1});
        cls.methods.push_back({"getNextElement", "instance", "()", 1});
        cls.methods.push_back({"getNextElementWithTagName", "instance", "(StringRef)", 1});
        cls.methods.push_back({"getNumAttributes", "instance", "()", 1});
        cls.methods.push_back({"getNumChildElements", "instance", "()", 1});
        cls.methods.push_back({"getStringAttribute", "instance", "(StringRef, String)", 1});
        cls.methods.push_back({"getStringAttribute", "instance", "(StringRef, String)", 2});
        cls.methods.push_back({"getTagName", "instance", "()", 1});
        cls.methods.push_back({"getTagNameWithoutNamespace", "instance", "()", 1});
        cls.methods.push_back({"getText", "instance", "()", 1});
        cls.methods.push_back({"hasAttribute", "instance", "(StringRef)", 1});
        cls.methods.push_back({"hasTagName", "instance", "(StringRef)", 1});
        cls.methods.push_back({"hasTagNameIgnoringNamespace", "instance", "(StringRef)", 1});
        cls.methods.push_back({"insertChildElement", "instance", "(XmlElement, int)", 1});
        cls.methods.push_back({"isEquivalentTo", "instance", "(XmlElement, bool)", 1});
        cls.methods.push_back({"isTextElement", "instance", "()", 1});
        cls.methods.push_back({"prependChildElement", "instance", "(XmlElement)", 1});
        cls.methods.push_back({"removeAllAttributes", "instance", "()", 1});
        cls.methods.push_back({"removeAttribute", "instance", "(Identifier)", 1});
        cls.methods.push_back({"removeChildElement", "instance", "(XmlElement, bool)", 1});
        cls.methods.push_back({"replaceChildElement", "instance", "(XmlElement, XmlElement)", 1});
        cls.methods.push_back({"setAttribute", "instance", "(Identifier, double)", 1});
        cls.methods.push_back({"setAttribute", "instance", "(Identifier, double)", 2});
        cls.methods.push_back({"setAttribute", "instance", "(Identifier, double)", 3});
        cls.methods.push_back({"setText", "instance", "(String)", 1});
        cls.methods.push_back({"writeToFile", "instance", "(File, StringRef, StringRef, int)", 1});
        cls.methods.push_back({"writeToStream", "instance", "(OutputStream, StringRef, bool, bool, StringRef, int)", 1});
        cls.staticMethods.push_back({"createTextElement", "static", "(String)", 1});
        classMap[cls.name] = cls;
    }

    // --- ZipFile ---
    {
        ApiClass cls;
        cls.name = "ZipFile";
        cls.methods.push_back({"createStreamForEntry", "instance", "(ZipEntry)", 1});
        cls.methods.push_back({"createStreamForEntry", "instance", "(ZipEntry)", 2});
        cls.methods.push_back({"getEntry", "instance", "(String, bool)", 1});
        cls.methods.push_back({"getEntry", "instance", "(String, bool)", 2});
        cls.methods.push_back({"getIndexOfFileName", "instance", "(String, bool)", 1});
        cls.methods.push_back({"getNumEntries", "instance", "()", 1});
        cls.methods.push_back({"sortEntriesByFilename", "instance", "()", 1});
        cls.methods.push_back({"uncompressEntry", "instance", "(int, File, bool)", 1});
        cls.methods.push_back({"uncompressTo", "instance", "(File, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- juce ---
    {
        ApiClass cls;
        cls.name = "juce";
        cls.staticMethods.push_back({"double_Pi", "static", "()", 1});
        cls.staticMethods.push_back({"float_Pi", "static", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- panel ---
    {
        ApiClass cls;
        cls.name = "panel";
        cls.methods.push_back({"dumpDebugData", "instance", "()", 1});
        cls.methods.push_back({"getBootstrapState", "instance", "()", 1});
        cls.methods.push_back({"getButton", "instance", "()", 1});
        cls.methods.push_back({"getButtonComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getCanvas", "instance", "()", 1});
        cls.methods.push_back({"getCombo", "instance", "()", 1});
        cls.methods.push_back({"getComboComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getFileListBox", "instance", "()", 1});
        cls.methods.push_back({"getFileListBoxComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getFixedImageSlider", "instance", "()", 1});
        cls.methods.push_back({"getFixedImageSliderComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getFixedSlider", "instance", "()", 1});
        cls.methods.push_back({"getFixedSliderComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getGlobalVariable", "instance", "(int)", 1});
        cls.methods.push_back({"getImageButton", "instance", "()", 1});
        cls.methods.push_back({"getImageButtonComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getImageSlider", "instance", "()", 1});
        cls.methods.push_back({"getImageSliderComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getInputComparator", "instance", "()", 1});
        cls.methods.push_back({"getLCDLabel", "instance", "()", 1});
        cls.methods.push_back({"getLCDLabelComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getLabel", "instance", "()", 1});
        cls.methods.push_back({"getLabelComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getListBox", "instance", "()", 1});
        cls.methods.push_back({"getListBoxComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getModulator", "instance", "(String)", 1});
        cls.methods.push_back({"getModulatorByIndex", "instance", "(int)", 1});
        cls.methods.push_back({"getModulatorByName", "instance", "()", 1});
        cls.methods.push_back({"getModulatorValuesAsData", "instance", "(ValueTree, String, CtrlrByteEncoding, int, bool)", 1});
        cls.methods.push_back({"getModulatorValuesAsData", "instance", "(ValueTree, String, CtrlrByteEncoding, int, bool)", 2});
        cls.methods.push_back({"getModulatorValuesAsData", "instance", "(ValueTree, String, CtrlrByteEncoding, int, bool)", 3});
        cls.methods.push_back({"getModulatorWithProperty", "instance", "(String, int)", 2});
        cls.methods.push_back({"getModulatorsWildcard", "instance", "()", 1});
        cls.methods.push_back({"getModulatorsWildcard", "instance", "()", 2});
        cls.methods.push_back({"getModulatorsWithProperty", "instance", "(Identifier, var)", 1});
        cls.methods.push_back({"getNumModulators", "instance", "()", 1});
        cls.methods.push_back({"getPanelEditor", "instance", "()", 1});
        cls.methods.push_back({"getProgramState", "instance", "()", 1});
        cls.methods.push_back({"getRestoreState", "instance", "()", 1});
        cls.methods.push_back({"getSlider", "instance", "()", 1});
        cls.methods.push_back({"getSliderComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getToggleButton", "instance", "()", 1});
        cls.methods.push_back({"getToggleButtonComponent", "instance", "(String)", 1});
        cls.methods.push_back({"getWaveform", "instance", "()", 1});
        cls.methods.push_back({"getWaveformComponent", "instance", "(String)", 1});
        cls.methods.push_back({"isRestoring", "instance", "()", 1});
        cls.methods.push_back({"sendMidi", "instance", "(CtrlrMidiMessage, double)", 1});
        cls.methods.push_back({"sendMidi", "instance", "(CtrlrMidiMessage, double)", 2});
        cls.methods.push_back({"sendMidi", "instance", "(CtrlrMidiMessage, double)", 3});
        cls.methods.push_back({"sendMidiMessageNow", "instance", "()", 1});
        cls.methods.push_back({"setGlobalVariable", "instance", "(int, int)", 1});
        cls.methods.push_back({"setModulatorValuesFromData", "instance", "(MemoryBlock, String, CtrlrByteEncoding, int, int, bool)", 1});
        cls.methods.push_back({"setProgramState", "instance", "(bool)", 1});
        cls.methods.push_back({"setRestoreState", "instance", "(bool)", 1});
        cls.methods.push_back({"getModulatorWithProperty", "instance", "(String propName, String propValue)", 1});
        cls.methods.push_back({"getModulatorWithProperty", "instance", "(String propName, int propValue)", 2});
        classMap[cls.name] = cls;
    }

    // --- mod ---
    {
        ApiClass cls;
        cls.name = "mod";
        cls.methods.push_back({"getComponent", "instance", "()", 1});
        cls.methods.push_back({"getLuaName", "instance", "()", 1});
        cls.methods.push_back({"getMaxMapped", "instance", "()", 1});
        cls.methods.push_back({"getMaxMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getMaxModulatorValue", "instance", "()", 1});
        cls.methods.push_back({"getMaxModulatorValueInt", "instance", "()", 1});
        cls.methods.push_back({"getMaxNonMapped", "instance", "()", 1});
        cls.methods.push_back({"getMaxNonMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getMidiMessage", "instance", "()", 1});
        cls.methods.push_back({"getMidiMessage", "instance", "()", 2});
        cls.methods.push_back({"getMinMapped", "instance", "()", 1});
        cls.methods.push_back({"getMinMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getMinModulatorValue", "instance", "()", 1});
        cls.methods.push_back({"getMinModulatorValueInt", "instance", "()", 1});
        cls.methods.push_back({"getMinNonMapped", "instance", "()", 1});
        cls.methods.push_back({"getMinNonMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getModulatorName", "instance", "()", 1});
        cls.methods.push_back({"getModulatorValue", "instance", "()", 1});
        cls.methods.push_back({"getModulatorValueInt", "instance", "()", 1});
        cls.methods.push_back({"getName", "instance", "()", 1});
        cls.methods.push_back({"getRestoreState", "instance", "()", 1});
        cls.methods.push_back({"getValue", "instance", "()", 1});
        cls.methods.push_back({"getValueInt", "instance", "()", 1});
        cls.methods.push_back({"getValueMapped", "instance", "()", 1});
        cls.methods.push_back({"getValueMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getValueNonMapped", "instance", "()", 1});
        cls.methods.push_back({"getValueNonMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getVstIndex", "instance", "()", 1});
        cls.methods.push_back({"isRestoring", "instance", "()", 1});
        cls.methods.push_back({"setModulatorValue", "instance", "(int, bool, bool, bool)", 1});
        cls.methods.push_back({"setRestoreState", "instance", "(bool)", 1});
        cls.methods.push_back({"setValue", "instance", "(int, bool, bool)", 1});
        cls.methods.push_back({"setValue", "instance", "(int, bool, bool)", 2});
        cls.methods.push_back({"setValueMapped", "instance", "(int, bool, bool)", 1});
        cls.methods.push_back({"setValueMapped", "instance", "(int, bool, bool)", 2});
        cls.methods.push_back({"setValueNonMapped", "instance", "(int, bool, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- modulator ---
    {
        ApiClass cls;
        cls.name = "modulator";
        cls.methods.push_back({"getComponent", "instance", "()", 1});
        cls.methods.push_back({"getLuaName", "instance", "()", 1});
        cls.methods.push_back({"getMaxMapped", "instance", "()", 1});
        cls.methods.push_back({"getMaxMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getMaxModulatorValue", "instance", "()", 1});
        cls.methods.push_back({"getMaxModulatorValueInt", "instance", "()", 1});
        cls.methods.push_back({"getMaxNonMapped", "instance", "()", 1});
        cls.methods.push_back({"getMaxNonMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getMidiMessage", "instance", "()", 1});
        cls.methods.push_back({"getMidiMessage", "instance", "()", 2});
        cls.methods.push_back({"getMinMapped", "instance", "()", 1});
        cls.methods.push_back({"getMinMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getMinModulatorValue", "instance", "()", 1});
        cls.methods.push_back({"getMinModulatorValueInt", "instance", "()", 1});
        cls.methods.push_back({"getMinNonMapped", "instance", "()", 1});
        cls.methods.push_back({"getMinNonMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getModulatorName", "instance", "()", 1});
        cls.methods.push_back({"getModulatorValue", "instance", "()", 1});
        cls.methods.push_back({"getModulatorValueInt", "instance", "()", 1});
        cls.methods.push_back({"getName", "instance", "()", 1});
        cls.methods.push_back({"getRestoreState", "instance", "()", 1});
        cls.methods.push_back({"getValue", "instance", "()", 1});
        cls.methods.push_back({"getValueInt", "instance", "()", 1});
        cls.methods.push_back({"getValueMapped", "instance", "()", 1});
        cls.methods.push_back({"getValueMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getValueNonMapped", "instance", "()", 1});
        cls.methods.push_back({"getValueNonMappedInt", "instance", "()", 1});
        cls.methods.push_back({"getVstIndex", "instance", "()", 1});
        cls.methods.push_back({"isRestoring", "instance", "()", 1});
        cls.methods.push_back({"setModulatorValue", "instance", "(int, bool, bool, bool)", 1});
        cls.methods.push_back({"setRestoreState", "instance", "(bool)", 1});
        cls.methods.push_back({"setValue", "instance", "(int, bool, bool)", 1});
        cls.methods.push_back({"setValue", "instance", "(int, bool, bool)", 2});
        cls.methods.push_back({"setValueMapped", "instance", "(int, bool, bool)", 1});
        cls.methods.push_back({"setValueMapped", "instance", "(int, bool, bool)", 2});
        cls.methods.push_back({"setValueNonMapped", "instance", "(int, bool, bool)", 1});
        classMap[cls.name] = cls;
    }

    // --- utils ---
    {
        ApiClass cls;
        cls.name = "utils";
        cls.staticMethods.push_back({"askForTextInputWindow", "static", "(String, String, String, String, bool, String, String)", 1});
        cls.staticMethods.push_back({"base64_decode", "static", "(String)", 1});
        cls.staticMethods.push_back({"base64_encode", "static", "(String)", 1});
        cls.staticMethods.push_back({"get16bitSigned", "static", "()", 1});
        cls.staticMethods.push_back({"get8bitSigned", "static", "()", 1});
        cls.staticMethods.push_back({"getDirectoryWindow", "static", "(String, File)", 1});
        cls.staticMethods.push_back({"getMidiInputDevices", "static", "()", 1});
        cls.staticMethods.push_back({"getMidiOutputDevices", "static", "()", 1});
        cls.staticMethods.push_back({"getPi", "static", "()", 1});
        cls.staticMethods.push_back({"getVersionMajor", "static", "()", 1});
        cls.staticMethods.push_back({"getVersionMinor", "static", "()", 1});
        cls.staticMethods.push_back({"getVersionRevision", "static", "()", 1});
        cls.staticMethods.push_back({"getVersionString", "static", "()", 1});
        cls.staticMethods.push_back({"infoWindow", "static", "(String, String)", 1});
        cls.staticMethods.push_back({"openFileWindow", "static", "(String, File, String, bool)", 1});
        cls.staticMethods.push_back({"openMultipleFilesWindow", "static", "(String, File, String, bool, table)", 1});
        cls.staticMethods.push_back({"packDsiData", "static", "()", 1});
        cls.staticMethods.push_back({"questionWindow", "static", "(String, String, String, String)", 1});
        cls.staticMethods.push_back({"saveFileWindow", "static", "(String, File, String, bool)", 1});
        cls.staticMethods.push_back({"unpackDsiData", "static", "()", 1});
        cls.staticMethods.push_back({"warnWindow", "static", "(String, String)", 1});
        classMap[cls.name] = cls;
    }

    // --- MemoryBlock ---
    {
        ApiClass cls;
        cls.name = "MemoryBlock";
        cls.methods.push_back({"append", "instance", "(MemoryBlock)", 1});
        cls.methods.push_back({"compressGzip", "instance", "()", 1});
        cls.methods.push_back({"compressZlib", "instance", "()", 1});
        cls.methods.push_back({"createFromTable", "instance", "(table)", 1});
        cls.methods.push_back({"decompressGzip", "instance", "()", 1});
        cls.methods.push_back({"decompressZlib", "instance", "()", 1});
        cls.methods.push_back({"ensureSize", "instance", "()", 1});
        cls.methods.push_back({"fillWith", "instance", "()", 1});
        cls.methods.push_back({"fromBase64Encoding", "instance", "()", 1});
        cls.methods.push_back({"getBitRange", "instance", "(int, int)", 1});
        cls.methods.push_back({"getByte", "instance", "(int)", 1});
        cls.methods.push_back({"getData", "instance", "()", 1});
        cls.methods.push_back({"getRawAddress", "instance", "(MemoryBlock::getData())", 1});
        cls.methods.push_back({"getRange", "instance", "(int, int)", 1});
        cls.methods.push_back({"getSize", "instance", "()", 1});
        cls.methods.push_back({"init", "instance", "(int)", 1});
        cls.methods.push_back({"insert", "instance", "(MemoryBlock, int, int)", 1});
        cls.methods.push_back({"insert", "instance", "(MemoryBlock, int, int)", 2});
        cls.methods.push_back({"insertIntoTable", "instance", "(table)", 1});
        cls.methods.push_back({"loadFromHexString", "instance", "(String hex)", 1});
        cls.methods.push_back({"pushLString", "instance", "()", 1});
        cls.methods.push_back({"removeSection", "instance", "(int, int)", 1});
        cls.methods.push_back({"replaceWith", "instance", "(MemoryBlock)", 1});
        cls.methods.push_back({"setBitRange", "instance", "(int, int, int)", 1});
        cls.methods.push_back({"setByte", "instance", "(int, uint8)", 1});
        cls.methods.push_back({"setSize", "instance", "()", 1});
        cls.methods.push_back({"swapWith", "instance", "()", 1});
        cls.methods.push_back({"toBase64Encoding", "instance", "()", 1});
        cls.methods.push_back({"toBinarySafeString", "instance", "()", 1});
        cls.methods.push_back({"toBinaryString", "instance", "(lua_State)", 1});
        cls.methods.push_back({"toHexString", "instance", "(int groupSize)", 1});
        cls.methods.push_back({"toLuaString", "instance", "()", 1});
        cls.methods.push_back({"toLuaTable", "instance", "(table)", 1});
        cls.methods.push_back({"toRawString", "instance", "()", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        cls.methods.push_back({"toStringNoTerminator", "instance", "()", 1});
        cls.methods.push_back({"toStringWithNulls", "instance", "()", 1});
        cls.staticMethods.push_back({"createWithSize", "static", "(int)", 1});
        cls.staticMethods.push_back({"fromLuaBinaryString", "static", "(table)", 1});
        cls.staticMethods.push_back({"fromLuaString", "static", "(table, String)", 1});
        cls.staticMethods.push_back({"fromLuaString", "static", "(table, String)", 2});
        cls.staticMethods.push_back({"fromLuaStringToAscii", "static", "(String)", 1});
        cls.staticMethods.push_back({"fromLuaStringToAscii", "static", "(String)", 2});
        cls.staticMethods.push_back({"fromLuaTable", "static", "(table)", 1});
        cls.staticMethods.push_back({"copyFrom", "static", "(MemoryBlock source, int start, int size)", 1});
        cls.staticMethods.push_back({"copyTo", "static", "(MemoryBlock dest, int start, int size)", 1});
        classMap[cls.name] = cls;
    }

    // --- AlertWindow ---
    {
        ApiClass cls;
        cls.name = "AlertWindow";
        cls.methods.push_back({"addButton", "instance", "()", 1});
        cls.methods.push_back({"addComboBox", "instance", "()", 1});
        cls.methods.push_back({"addCustomComponent", "instance", "()", 1});
        cls.methods.push_back({"addProgressBarComponent", "instance", "()", 1});
        cls.methods.push_back({"addTextBlock", "instance", "()", 1});
        cls.methods.push_back({"addTextEditor", "instance", "()", 1});
        cls.methods.push_back({"containsAnyExtraComponents", "instance", "()", 1});
        cls.methods.push_back({"exitModalState", "instance", "()", 1});
        cls.methods.push_back({"getAlertType", "instance", "()", 1});
        cls.methods.push_back({"getComboBoxComponent", "instance", "()", 1});
        cls.methods.push_back({"getCustomComponent", "instance", "()", 1});
        cls.methods.push_back({"getNumButtons", "instance", "()", 1});
        cls.methods.push_back({"getNumCustomComponents", "instance", "()", 1});
        cls.methods.push_back({"getTextEditor", "instance", "()", 1});
        cls.methods.push_back({"getTextEditorContents", "instance", "()", 1});
        cls.methods.push_back({"removeCustomComponent", "instance", "()", 1});
        cls.methods.push_back({"runModalLoop", "instance", "()", 1});
        cls.methods.push_back({"setEscapeKeyCancels", "instance", "()", 1});
        cls.methods.push_back({"setMessage", "instance", "()", 1});
        cls.methods.push_back({"setModalHandler", "instance", "(table)", 1});
        cls.methods.push_back({"triggerButtonClick", "instance", "()", 1});
        cls.staticMethods.push_back({"queryText", "static", "(AlertIconType, String, String, String, String, String, String, bool, table)", 1});
        cls.staticMethods.push_back({"showMessageBox", "static", "(AlertIconType, String, String, String)", 1});
        cls.staticMethods.push_back({"showMessageBoxAsync", "static", "(AlertIconType, String, String, String)", 1});
        cls.staticMethods.push_back({"showNativeDialogBox", "static", "(String, String, bool)", 1});
        cls.staticMethods.push_back({"showOkCancelBox", "static", "(AlertIconType, String, String, String, String)", 1});
        cls.staticMethods.push_back({"showYesNoCancelBox", "static", "(AlertIconType, String, String, String, String, String)", 1});
        classMap[cls.name] = cls;
    }

    // --- BubbleMessageComponent ---
    {
        ApiClass cls;
        cls.name = "BubbleMessageComponent";
        cls.methods.push_back({"getContentSize", "instance", "(int, int)", 1});
        cls.methods.push_back({"showAt", "instance", "(Rectangle<int>, String, int, bool, bool)", 1});
        cls.methods.push_back({"showAt", "instance", "(Rectangle<int>, String, int, bool, bool)", 2});
        cls.methods.push_back({"showAt", "instance", "(Rectangle<int>, String, int, bool, bool)", 3});
        cls.methods.push_back({"showAt", "instance", "(Rectangle<int>, String, int, bool, bool)", 4});
        classMap[cls.name] = cls;
    }

    // --- AudioFormatManager ---
    {
        ApiClass cls;
        cls.name = "AudioFormatManager";
        cls.methods.push_back({"createReaderFor", "instance", "()", 1});
        cls.methods.push_back({"findFormatForFileExtension", "instance", "()", 1});
        cls.methods.push_back({"getDefaultFormat", "instance", "()", 1});
        cls.methods.push_back({"getKnownFormat", "instance", "()", 1});
        cls.methods.push_back({"getNumKnownFormats", "instance", "()", 1});
        cls.methods.push_back({"getWildcardForAllFormats", "instance", "()", 1});
        cls.methods.push_back({"registerBasicFormats", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- PopupMenu ---
    {
        ApiClass cls;
        cls.name = "PopupMenu";
        cls.methods.push_back({"addColouredItem", "instance", "()", 1});
        cls.methods.push_back({"addColumnBreak", "instance", "()", 1});
        cls.methods.push_back({"addItem", "instance", "()", 1});
        cls.methods.push_back({"addItem", "instance", "()", 2});
        cls.methods.push_back({"addSectionHeader", "instance", "()", 1});
        cls.methods.push_back({"addSeparator", "instance", "()", 1});
        cls.methods.push_back({"addSubMenu", "instance", "(String, LPopupMenu, bool, Image, bool, int)", 1});
        cls.methods.push_back({"clear", "instance", "()", 1});
        cls.methods.push_back({"getNumItems", "instance", "()", 1});
        cls.methods.push_back({"show", "instance", "(int)", 1});
        cls.methods.push_back({"show", "instance", "(int)", 2});
        cls.methods.push_back({"showAt", "instance", "(Rectangle<int>, int)", 1});
        cls.methods.push_back({"showAt", "instance", "(Rectangle<int>, int)", 2});
        classMap[cls.name] = cls;
    }

    // --- RelativeCoordinate ---
    {
        ApiClass cls;
        cls.name = "RelativeCoordinate";
        cls.methods.push_back({"getExpression", "instance", "()", 1});
        cls.methods.push_back({"isDynamic", "instance", "()", 1});
        cls.methods.push_back({"isRecursive", "instance", "()", 1});
        cls.methods.push_back({"moveToAbsolute", "instance", "()", 1});
        cls.methods.push_back({"resolve", "instance", "()", 1});
        cls.methods.push_back({"toString", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

    // --- ThreadWithProgressWindow ---
    {
        ApiClass cls;
        cls.name = "ThreadWithProgressWindow";
        cls.methods.push_back({"runThread", "instance", "()", 1});
        cls.methods.push_back({"setProgress", "instance", "(double)", 1});
        cls.methods.push_back({"setStatusMessage", "instance", "(String)", 1});
        cls.methods.push_back({"setThreadFunction", "instance", "(table)", 1});
        cls.methods.push_back({"wait", "instance", "()", 1});
        classMap[cls.name] = cls;
    }

}