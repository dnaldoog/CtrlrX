#ifndef __CTRLR_IDS__
#define __CTRLR_IDS__

#include "CtrlrMacros.h"

namespace Ids
    {
#define DECLARE_ID(name)      static const Identifier name (#name)
    
    DECLARE_ID (ctrlr);
    DECLARE_ID (panel);
    DECLARE_ID (manager);
    DECLARE_ID (modulator);
    DECLARE_ID (component);
    DECLARE_ID (midi);
    DECLARE_ID (controllerMIDI);
    DECLARE_ID (vstIndex);
    DECLARE_ID (name);
    DECLARE_ID (description);
    DECLARE_ID (value);
    DECLARE_ID (data);
    DECLARE_ID (category);
    DECLARE_ID (type);
    DECLARE_ID (uiType);
    DECLARE_ID (uiNone);
    DECLARE_ID (time);
    DECLARE_ID (uuid);
    DECLARE_ID (lsb);
    DECLARE_ID (msb);
    DECLARE_ID (id);
    DECLARE_ID (visibleAs);
    DECLARE_ID (method);
    DECLARE_ID (call);
    DECLARE_ID (internal);
    DECLARE_ID (restricted);
    DECLARE_ID (number);
    DECLARE_ID (null);
    DECLARE_ID (timeout);
    DECLARE_ID (active);
    
    DECLARE_ID (modulatorValue);
    DECLARE_ID (modulatorIsStatic);
    DECLARE_ID (modulatorGlobalVariable);
    DECLARE_ID (modulatorValueExpression);
    DECLARE_ID (modulatorValueExpressionReverse);
    DECLARE_ID (modulatorControllerExpression);
    DECLARE_ID (modulatorMuteOnStart);
    DECLARE_ID (modulatorMute);
    DECLARE_ID (modulatorLinkedToPanelProperty);
    DECLARE_ID (modulatorLinkedToModulatorProperty);
    DECLARE_ID (modulatorLinkedToModulatorSource);
    DECLARE_ID (modulatorLinkedToModulator);
    DECLARE_ID (modulatorLinkedToComponent);
    DECLARE_ID (modulatorMax);
    DECLARE_ID (modulatorMin);
    DECLARE_ID (modulatorExcludeFromSnapshot);
    DECLARE_ID (modulatorBaseValue);
    DECLARE_ID (modulatorCustomIndex);
    DECLARE_ID (modulatorCustomIndexGroup);
    DECLARE_ID (modulatorVstExported);
    DECLARE_ID (modulatorVstNameFormat);
    DECLARE_ID (modulatorCustomNameGroup);
    DECLARE_ID (modulatorCustomName);
    
    DECLARE_ID (uiPanelViewPortBackgroundColour);
    
    DECLARE_ID (uiPanelImageAlpha);
    DECLARE_ID (uiPanelImageLayout);
    DECLARE_ID (uiPanelImageResource);
    DECLARE_ID (uiPanelBackgroundColour);
    DECLARE_ID (uiPanelBackgroundColour1);
    DECLARE_ID (uiPanelBackgroundColour2);
    DECLARE_ID (uiPanelBackgroundGradientType);
    DECLARE_ID (uiPanelSnapSize);
    DECLARE_ID (uiPanelEditorWidth);
    DECLARE_ID (uiPanelEditorHeight);
    DECLARE_ID (uiPanelViewPortSize);
    DECLARE_ID (uiPanelPropertiesSize);
    DECLARE_ID (uiPanelEditMode);
    DECLARE_ID (uiPanelZoom);
    DECLARE_ID (uiPanelLock);
    DECLARE_ID (uiPanelDisabledOnEdit);
    DECLARE_ID (uiPanelWidth);
    DECLARE_ID (uiPanelHeight);
    DECLARE_ID (uiViewPortWidth);
    DECLARE_ID (uiViewPortHeight);
    DECLARE_ID (uiViewPortResizable);
    DECLARE_ID (uiViewPortShowScrollBars);
    DECLARE_ID (uiViewPortEnableResizeLimits);
    DECLARE_ID (uiViewPortMinWidth);
    DECLARE_ID (uiViewPortMinHeight);
    DECLARE_ID (uiViewPortMaxWidth);
    DECLARE_ID (uiViewPortMaxHeight);
    DECLARE_ID (uiViewPortEnableFixedAspectRatio);
    DECLARE_ID (uiViewPortFixedAspectRatio);
    DECLARE_ID (uiPanelSnapActive);
    DECLARE_ID (uiPanelCanvasRectangle);
    DECLARE_ID (uiPanelClipboardTree);
    DECLARE_ID (uiPanelPropertiesOnRight);
    DECLARE_ID (uiPanelInvisibleComponentAlpha);
    DECLARE_ID (uiPanelModulatorListViewTree);
    DECLARE_ID (uiPanelModulatorListTreeState);
    DECLARE_ID (uiPanelDisableCombosOnEdit);
    
    DECLARE_ID (uiPanelLegacyMode);
    DECLARE_ID (uiPanelLookAndFeel);
    DECLARE_ID (uiPanelColorScheme);
    DECLARE_ID (uiPanelColourScheme);
    
    DECLARE_ID (uiPanelUIColourWindowBackground);
    DECLARE_ID (uiPanelUIColourWidgetBackground);
    DECLARE_ID (uiPanelUIColourMenuBackground);
    DECLARE_ID (uiPanelUIColourOutline);
    DECLARE_ID (uiPanelUIColourDefaultText);
    DECLARE_ID (uiPanelUIColourDefaultFill);
    DECLARE_ID (uiPanelUIColourHighlightedText);
    DECLARE_ID (uiPanelUIColourHighlightedFill);
    DECLARE_ID (uiPanelUIColourMenuText);
    DECLARE_ID (uiPanelUIColourNumColours);
    
    DECLARE_ID (uiPanelTooltipBackgroundColour);
    DECLARE_ID (uiPanelTooltipOutlineColour);
    DECLARE_ID (uiPanelTooltipCornerRound);
    DECLARE_ID (uiPanelTooltipPlacement);
    DECLARE_ID (uiPanelTooltipFont);
    DECLARE_ID (uiPanelTooltipColour);
    
    DECLARE_ID (uiWindowManager);
    DECLARE_ID (uiChildWindow);
    DECLARE_ID (uiChildWindowName);
    DECLARE_ID (uiChildWindowState);
    DECLARE_ID (uiChildWindowContentState);
    
    DECLARE_ID (uiPanelCanvasLayerName);
    DECLARE_ID (uiPanelCanvasLayerIndex);
    DECLARE_ID (uiPanelCanvasLayerUid);
    DECLARE_ID (uiPanelCanvasLayerColour);
    DECLARE_ID (uiPanelCanvasLayerMouseHandle);
    DECLARE_ID (uiPanelCanvasLayerVisibility);
    DECLARE_ID (uiPanelCanvasLayer);
    DECLARE_ID (uiPanelMenuBarVisible);
    DECLARE_ID (uiPanelMenuBarHideOnExport);
    DECLARE_ID (uiPanelProgramsMenuHideOnExport);
    DECLARE_ID (uiPanelMidiControllerMenuHideOnExport);
    DECLARE_ID (uiPanelMidiThruMenuHideOnExport);
    DECLARE_ID (uiPanelMidiChannelMenuHideOnExport);
    
    DECLARE_ID (panelMidiProgram);
    DECLARE_ID (panelMidiBankLsb);
    DECLARE_ID (panelMidiBankMsb);
    DECLARE_ID (panelMidiSendProgramChangeOnLoad);
    
    DECLARE_ID (panelMidiSnapshotAfterLoad);
    DECLARE_ID (panelMidiSnapshotAfterProgramChange);
    DECLARE_ID (panelMidiSnapshotDelay);
    DECLARE_ID (panelMidiSnapshotShowProgress);
    DECLARE_ID (panelMidiInputThreadPriority);
    DECLARE_ID (panelMidiPauseOut);
    DECLARE_ID (panelMidiPauseIn);
    
    /* device input */
    DECLARE_ID (panelMidiInputChannelDevice);
    DECLARE_ID (panelMidiInputDevice);
    
    /* device output */
    DECLARE_ID (panelMidiOutputChannelDevice);
    DECLARE_ID (panelMidiOutputDevice);
    
    /* device controller */
    DECLARE_ID (panelMidiControllerDevice);
    DECLARE_ID (panelMidiControllerChannelDevice);
    
    /* host input */
    DECLARE_ID (panelMidiInputFromHost);
    DECLARE_ID (panelMidiInputChannelHost);
    DECLARE_ID (panelMidiInputFromHostCompare);
    
    /* host output */
    DECLARE_ID (panelMidiOutputChannelHost);
    DECLARE_ID (panelMidiOutputToHost);
    
    /* thru options */
    DECLARE_ID (panelMidiThruH2H);
    DECLARE_ID (panelMidiThruH2HChannelize);
    
    DECLARE_ID (panelMidiThruH2D);
    DECLARE_ID (panelMidiThruH2DChannelize);
    
    DECLARE_ID (panelMidiThruD2D);
    DECLARE_ID (panelMidiThruD2DChannelize);
    
    DECLARE_ID (panelMidiThruD2H);
    DECLARE_ID (panelMidiThruD2HChannelize);
    
    DECLARE_ID (panelMidiThruC2D);
    DECLARE_ID (panelMidiThruC2DChannelize);
    
    /* OSC options */
    DECLARE_ID (panelOSCEnabled);
    DECLARE_ID (panelOSCPort);
    DECLARE_ID (panelOSCProtocol);
    
    DECLARE_ID (panelScheme);
    DECLARE_ID (panelShowDialogs);
    DECLARE_ID (panelMidiRealtimeIgnore);
    DECLARE_ID (panelMidiControllerToLua);
    DECLARE_ID (panelMidiProgramChangeMethod);
    DECLARE_ID (panelMidiProgramCalloutOnprogramChange);
    DECLARE_ID (panelMidiMatchCacheSize);
    DECLARE_ID (panelMidiGlobalDelay);
    DECLARE_ID (panelFilePath);
    DECLARE_ID (panelIndex);
    DECLARE_ID (panelUID);
    DECLARE_ID (panelInstanceUID);
    DECLARE_ID (panelInstanceManufacturerID);
    DECLARE_ID (panelIsDirty);
    DECLARE_ID (panelComponentGroupList);
    DECLARE_ID (panelGlobalVariables);
    DECLARE_ID (panelAuthorName);
    DECLARE_ID (panelAuthorDonateUrl);
    DECLARE_ID (panelAuthorEmail);
    DECLARE_ID (panelAuthorUrl);
    DECLARE_ID (panelAuthorDesc);
    DECLARE_ID (panelVersionMajor);
    DECLARE_ID (panelVersionMinor);
    DECLARE_ID (panelVersionName);
    DECLARE_ID (panelCtrlrRevision);
    DECLARE_ID (panelVendor);
    DECLARE_ID (panelDevice);
    DECLARE_ID (panelExportResourceEncryption); // Added v5.6.33
    DECLARE_ID (panelExportDelayBtwSteps); // Added v5.6.33
    DECLARE_ID (panelExportCodesign); // Added v5.6.33
    DECLARE_ID (panelReplaceVst3PluginIds); // Added v5.6.32
    DECLARE_ID (panelPlugType); // Added v5.6.32
    DECLARE_ID (panelCertificateMacSelectId); // Added v5.6.32
    DECLARE_ID (panelCertificateMacId); // Added v5.6.32
    DECLARE_ID (panelCertificateWinSelectPath); // Added v5.6.32
    DECLARE_ID (panelCertificateWinPath); // Added v5.6.32
    DECLARE_ID (panelCertificateWinPassCode); // Added v5.6.32
    
    DECLARE_ID (panelModulatorListColumns);
    DECLARE_ID (panelModulatorListCsvDelimiter);
    DECLARE_ID (panelModulatorListXmlRoot);
    DECLARE_ID (panelModulatorListXmlModulator);
    DECLARE_ID (panelModulatorListSortOption);
    DECLARE_ID (panelState);
    DECLARE_ID (panelResources);
    DECLARE_ID (panelCaps);
    DECLARE_ID (panelPropertyDisplayIDs);
    DECLARE_ID (panelMessageTime);
    DECLARE_ID (panelCustomData);
    DECLARE_ID (panelLastSaveDir);
    
    DECLARE_ID (componentRectangle);
    DECLARE_ID (componentSentBack);
    DECLARE_ID (componentVisibleName);
    DECLARE_ID (componentMouseCursor);
    DECLARE_ID (componentGroupName);
    DECLARE_ID (componentTabName);
    DECLARE_ID (componentTabId);
    DECLARE_ID (componentGroupped);
    DECLARE_ID (componentSnapSize);
    DECLARE_ID (componentIsLocked);
    DECLARE_ID (componentDisabled);
    DECLARE_ID (componentRadioGroupId);
    DECLARE_ID (componentRadioGroupNotifyMidi);
    DECLARE_ID (componentVisibility);
    DECLARE_ID (componentLayerUid);
    DECLARE_ID (componentAlpha);
    DECLARE_ID (componentEffect);
    DECLARE_ID (componentEffectRadius);
    DECLARE_ID (componentEffectColour);
    DECLARE_ID (componentEffectOffsetX);
    DECLARE_ID (componentEffectOffsetY);
    DECLARE_ID (componentLabelPosition);
    DECLARE_ID (componentLabelJustification);
    DECLARE_ID (componentLabelHeight);
    DECLARE_ID (componentLabelWidth);
    DECLARE_ID (componentLabelVisible);
    DECLARE_ID (componentLabelAlwaysOnTop);
    DECLARE_ID (componentLabelColour);
    DECLARE_ID (componentLabelFont);
    DECLARE_ID (componentRotation);
    DECLARE_ID (componentExcludedFromLabelDisplay);
    DECLARE_ID (componentValueDecimalPlaces);
    DECLARE_ID (componentLuaMouseMoved);
    DECLARE_ID (componentLuaMouseDown);
    DECLARE_ID (componentLuaMouseUp);
    DECLARE_ID (componentLuaMouseDrag);
    DECLARE_ID (componentLuaMouseDoubleClick);
    DECLARE_ID (componentLuaMouseEnter);
    DECLARE_ID (componentLuaMouseExit);
    DECLARE_ID (componentInternalFunction);
    DECLARE_ID (componentLookAndFeel);
    DECLARE_ID (componentLookAndFeelIsCustom);
    
    DECLARE_ID (uiSliderLookAndFeel);
    DECLARE_ID (uiSliderLookAndFeelIsCustom);
    DECLARE_ID (uiSliderStyle);
    DECLARE_ID (uiSliderMin);
    DECLARE_ID (uiSliderMax);
    DECLARE_ID (uiSliderInterval);
    DECLARE_ID (uiSliderValueSuffix);
    DECLARE_ID (uiSliderValuePosition);
    DECLARE_ID (uiSliderValueHeight);
    DECLARE_ID (uiSliderValueWidth);
    DECLARE_ID (uiSliderTrackCornerSize);
    DECLARE_ID (uiSliderThumbCornerSize);
    DECLARE_ID (uiSliderThumbWidth);
    DECLARE_ID (uiSliderThumbHeight);
    DECLARE_ID (uiSliderThumbFlatOnLeft);
    DECLARE_ID (uiSliderThumbFlatOnRight);
    DECLARE_ID (uiSliderThumbFlatOnTop);
    DECLARE_ID (uiSliderThumbFlatOnBottom);
    DECLARE_ID (uiSliderValueTextColour);
    DECLARE_ID (uiSliderValueBgColour);
    DECLARE_ID (uiSliderRotaryOutlineColour);
    DECLARE_ID (uiSliderRotaryFillColour);
    DECLARE_ID (uiSliderThumbColour);
    DECLARE_ID (uiSliderValueHighlightColour);
    DECLARE_ID (uiSliderValueOutlineColour);
    DECLARE_ID (uiSliderTrackColour);
    DECLARE_ID (uiSliderIncDecButtonColour);
    DECLARE_ID (uiSliderIncDecTextColour);
    DECLARE_ID (uiSliderValueFont);
    DECLARE_ID (uiSliderValueTextJustification);
    DECLARE_ID (uiSliderVelocityMode);
    DECLARE_ID (uiSliderVelocityModeKeyTrigger);
    DECLARE_ID (uiSliderVelocitySensitivity);
    DECLARE_ID (uiSliderVelocityThreshold);
    DECLARE_ID (uiSliderVelocityOffset);
    DECLARE_ID (uiSliderSpringMode);
    DECLARE_ID (uiSliderSpringValue);
    DECLARE_ID (uiSliderMouseWheelInterval);
    DECLARE_ID (uiSliderPopupBubble);
    DECLARE_ID (uiSliderPopupBubbleFont);
    DECLARE_ID (uiSliderPopupBubbleBgColour);
    DECLARE_ID (uiSliderPopupBubbleFgColour);
    DECLARE_ID (uiSliderPopupBubbleOutlineColour);
    DECLARE_ID (uiSliderPopupBubblePlacement);
    DECLARE_ID (uiSliderDoubleClickEnabled);
    DECLARE_ID (uiSliderDoubleClickValue);
    DECLARE_ID (uiSliderDecimalPlaces);
    DECLARE_ID (uiSliderSetNotificationOnlyOnRelease);
    
    
    DECLARE_ID (uiFixedSliderContent);
    
    DECLARE_ID (uiImageSliderResource);
    DECLARE_ID (uiImageSliderResourceFrameWidth);
    DECLARE_ID (uiImageSliderResourceFrameHeight);
    DECLARE_ID (uiImageResource);
    
    DECLARE_ID (uiComboArrowColour);
    DECLARE_ID (uiComboOutlineColour);
    DECLARE_ID (uiComboTextColour);
    DECLARE_ID (uiComboTextJustification);
    DECLARE_ID (uiComboButtonColour);
    DECLARE_ID (uiComboBgColour);
    DECLARE_ID (uiComboContent);
    DECLARE_ID (uiComboFont);
    DECLARE_ID (uiComboMenuFont);
    DECLARE_ID (uiComboMenuBackgroundColour);
    DECLARE_ID (uiComboMenuFontColour);
    DECLARE_ID (uiComboMenuFontHighlightedColour);
    DECLARE_ID (uiComboMenuHighlightColour);
    DECLARE_ID (uiComboMenuBackgroundRibbed);
    DECLARE_ID (uiComboDynamicContent);
    DECLARE_ID (uiComboButtonGradient);
    DECLARE_ID (uiComboButtonGradientColour1);
    DECLARE_ID (uiComboButtonGradientColour2);
    DECLARE_ID (uiComboButtonWidthOverride);
    DECLARE_ID (uiComboButtonWidth);
    DECLARE_ID (uiComboSelectedId);
    DECLARE_ID (uiComboSelectedIndex);
    
    DECLARE_ID (uiListBoxContent);
    DECLARE_ID (uiListBoxRowHeight);
    DECLARE_ID (uiListBoxBackgroundColour);
    DECLARE_ID (uiListBoxHighlightBgColour);
    DECLARE_ID (uiListBoxHighlightFgColour);
    DECLARE_ID (uiListBoxTextColour);
    DECLARE_ID (uiListBoxFont);
    DECLARE_ID (uiListBoxHighlightFont);
    DECLARE_ID (uiListBoxOutline);
    DECLARE_ID (uiListBoxOutlineColour);
    DECLARE_ID (uiListBoxVScrollBgColour);
    DECLARE_ID (uiListBoxVScrollThumbColour);
    DECLARE_ID (uiListBoxVScrollTrackColour);
    DECLARE_ID (uiListBoxHScrollBgColour);
    DECLARE_ID (uiListBoxHScrollThumbColour);
    DECLARE_ID (uiListBoxHScrollTrackColour);
    DECLARE_ID (uiListBoxJustification);
    DECLARE_ID (uiListBoxMultipleSelection);
    DECLARE_ID (uiListBoxItemClicked);
    DECLARE_ID (uiListBoxItemDoubleClicked);
    DECLARE_ID (uiListBoxItemDeleteKeyPressed);
    DECLARE_ID (uiListBoxItemReturnKeyPressed);
    
    DECLARE_ID (uiFileListBoxBgColour);
    DECLARE_ID (uiFileListLineColour);
    DECLARE_ID (uiFileListIndentSize);
    DECLARE_ID (uiFileListFont);
    DECLARE_ID (uiFileListTextColour);
    DECLARE_ID (uiFileListHighlightTextColour);
    DECLARE_ID (uiFileListHighlightBgColour);
    DECLARE_ID (uiFileListBoxHighlightFont);
    DECLARE_ID (uiFileListBoxOutline);
    DECLARE_ID (uiFileListBoxOutlineColour);
    DECLARE_ID (uiFileListBoxVScrollBgColour);
    DECLARE_ID (uiFileListBoxVScrollThumbColour);
    DECLARE_ID (uiFileListBoxVScrollTrackColour);
    DECLARE_ID (uiFileListBoxHScrollBgColour);
    DECLARE_ID (uiFileListBoxHScrollThumbColour);
    DECLARE_ID (uiFileListBoxHScrollTrackColour);
    DECLARE_ID (uiFileListOpenButtonVisible);
    DECLARE_ID (uiFileListFileClicked);
    DECLARE_ID (uiFileListFileDoubleClicked);
    DECLARE_ID (uiFileListCurrentRoot);
    
    DECLARE_ID (uiLabelBgColour);
    DECLARE_ID (uiLabelTextColour);
    DECLARE_ID (uiLabelOutline);
    DECLARE_ID (uiLabelOutlineColour);
    DECLARE_ID (uiLabelFitFont);
    DECLARE_ID (uiLabelFont);
    DECLARE_ID (uiLabelText);
    DECLARE_ID (uiLabelDisplaysAllValues);
    DECLARE_ID (uiLabelDisplayFormat);
    DECLARE_ID (uiLabelJustification);
    DECLARE_ID (uiLabelEditOnSingleClick);
    DECLARE_ID (uiLabelEditOnDoubleClick);
    DECLARE_ID (uiLabelEditFocusDiscardsChanges);
    DECLARE_ID (uiLabelChangedCbk);
    DECLARE_ID (uiLabelInputAllowedChars);
    DECLARE_ID (uiLabelInputMaxLength);
    DECLARE_ID (uiLabelInputHighlightTextColour);
    DECLARE_ID (uiLabelInputHighlightColour);
    DECLARE_ID (uiLCDLabelFont);
    DECLARE_ID (uiLCDLabelFontHeight);
    
    DECLARE_ID (uiGroupOutlineColour1);
    DECLARE_ID (uiGroupOutlineColour2);
    DECLARE_ID (uiGroupOutlineGradientType);
    DECLARE_ID (uiGroupOutlineThickness);
    DECLARE_ID (uiGroupOutlineRoundAngle);
    DECLARE_ID (uiGroupBackgroundColour1);
    DECLARE_ID (uiGroupBackgroundColour2);
    DECLARE_ID (uiGroupBackgroundGradientType);
    DECLARE_ID (uiGroupTextColour);
    DECLARE_ID (uiGroupTextPlacement);
    DECLARE_ID (uiGroupTextFont);
    DECLARE_ID (uiGroupText);
    DECLARE_ID (uiGroupTextMargin);
    DECLARE_ID (uiGroupBackgroundImage);
    DECLARE_ID (uiGroupBackgroundImageLayout);
    DECLARE_ID (uiGroupBackgroundImageAlpha);
    
    DECLARE_ID (uiToggleButtonText);
    DECLARE_ID (uiToggleButtonFocusOutline);
    DECLARE_ID (uiToggleButtontickColour);
    DECLARE_ID (uiToggletickDisabledColour);
    
    DECLARE_ID (uiButtonLookAndFeel);
    DECLARE_ID (uiButtonLookAndFeelIsCustom);
    DECLARE_ID (uiButtonTextColourOff);
    DECLARE_ID (uiButtonTextColourOn);
    DECLARE_ID (uiButtonTextFont);
    DECLARE_ID (uiButtonTextJustification);
    DECLARE_ID (uiButtonColourOff);
    DECLARE_ID (uiButtonColourOn);
    DECLARE_ID (uiButtonContent);
    DECLARE_ID (uiButtonIsToggle);
    DECLARE_ID (uiButtonIsMomentary);
    DECLARE_ID (uiButtonTrueValue);
    DECLARE_ID (uiButtonFalseValue);
    DECLARE_ID (uiButtonRepeat);
    DECLARE_ID (uiButtonRepeatRate);
    DECLARE_ID (uiButtonTriggerOnMouseDown);
    DECLARE_ID (uiButtonConnectedLeft);
    DECLARE_ID (uiButtonConnectedRight);
    DECLARE_ID (uiButtonConnectedTop);
    DECLARE_ID (uiButtonConnectedBottom);
    
    DECLARE_ID (uiImageButtonResource);
    DECLARE_ID (uiImageButtonTextColour);
    DECLARE_ID (uiImageButtonTextWidth);
    DECLARE_ID (uiImageButtonTextHeight);
    DECLARE_ID (uiImageButtonContent);
    DECLARE_ID (uiImageButtonImageWidth);
    DECLARE_ID (uiImageButtonImageHeight);
    DECLARE_ID (uiImageButtonTextPosition);
    DECLARE_ID (uiImageButtonMode);
    
    DECLARE_ID (uiMidiKeyboardOrientation);
    DECLARE_ID (uiMidiKeyboardWhiteButtonColour);
    DECLARE_ID (uiMidiKeyboardBlackButtonColour);
    DECLARE_ID (uiMidiKeyboardSeparatorLineColour);
    DECLARE_ID (uiMidiKeyboardMouseOverColour);
    DECLARE_ID (uiMidiKeyboardMouseDownColour);
    DECLARE_ID (uiMidiKeyboardTextLabelColour);
    DECLARE_ID (uiMidiKeyboardButtonBackgroundColour);
    DECLARE_ID (uiMidiKeyboardButtonArrowColour);
    DECLARE_ID (uiMidiKeyboardLowestVisibleKey);
    DECLARE_ID (uiMidiKeyboardBaseOctaveKeyPress);
    DECLARE_ID (uiMidiKeyboardOctaveFroMiddleC);
    DECLARE_ID (uiMidiKeyboardMapToNoteNumber);
    
    DECLARE_ID (uiTabsNumberOfTabs);
    DECLARE_ID (uiTabsOrientation);
    DECLARE_ID (uiTabsDepth);
    DECLARE_ID (uiTabsOutlineThickness);
    DECLARE_ID (uiTabsFrontTabOutline);
    DECLARE_ID (uiTabsTabOutline);
    DECLARE_ID (uiTabsIndentThickness);
    DECLARE_ID (uiTabsAddTab);
    DECLARE_ID (uiTabsRemoveTab);
    DECLARE_ID (uiTabsCurrentTab);
    DECLARE_ID (uiTabsOutlineGlobalColour);
    DECLARE_ID (uiTabsOutlineGlobalBackgroundColour);
    DECLARE_ID (uiTabsCurrentTabChanged);
    DECLARE_ID (uiTabsFrontTabFont);
    DECLARE_ID (uiTabsTabFont);
    DECLARE_ID (uiTabsOutlineTabColour);
    DECLARE_ID (uiTabsTextTabColour);
    DECLARE_ID (uiTabsFrontTabOutlineColour);
    DECLARE_ID (uiTabsFrontTabTextColour);
    DECLARE_ID (uiTabsTab);
    DECLARE_ID (uiTabsTabName);
    DECLARE_ID (uiTabsTabIndex);
    DECLARE_ID (uiTabsTabContentBackgroundColour);
    DECLARE_ID (uiTabsTabBackgroundColour);
    DECLARE_ID (uiTabsTabBackgroundImage);
    DECLARE_ID (uiTabsTabBackgroundImageLayout);
    DECLARE_ID (uiTabsTabBackgroundImageAlpha);
    
    DECLARE_ID (uiArrowColour);
    DECLARE_ID (uiArrowLineThickness);
    DECLARE_ID (uiArrowHeadWidth);
    DECLARE_ID (uiArrowHeadHeight);
    DECLARE_ID (uiArrowOrientation);
    DECLARE_ID (uiArrowRotation);
    DECLARE_ID (uiArrowStokeThickness);
    
    DECLARE_ID (uiCustomResizedCallback);
    DECLARE_ID (uiCustomPaintCallback);
    DECLARE_ID (uiCustomPaintOverChildrenCallback);
    DECLARE_ID (uiCustomMouseDownCallback);
    DECLARE_ID (uiCustomMouseUpCallback);
    DECLARE_ID (uiCustomMouseEnterCallback);
    DECLARE_ID (uiCustomMouseExitCallback);
    DECLARE_ID (uiCustomMouseDragCallback);
    DECLARE_ID (uiCustomMouseMoveCallback);
    DECLARE_ID (uiCustomKeyDownCallback);
    DECLARE_ID (uiCustomKeyStateChangedCallback);
    DECLARE_ID (uiCustomMouseDoubleClickCallback);
    DECLARE_ID (uiCustomMouseWheelMoveCallback);
    DECLARE_ID (uiCustomMouseDownGrabsFocus);
    DECLARE_ID (uiCustomWantsKeyboardFocus);
    DECLARE_ID (uiCustomSetText);
    DECLARE_ID (uiCostomGetText);
    DECLARE_ID (uiCustomSetValue);
    DECLARE_ID (uiCustomGetValue);
    DECLARE_ID (uiCustomDragAndDropTarget);
    DECLARE_ID (uiCustomDragAndDropContainer);
    DECLARE_ID (uiCustomDrawDragImageWhenOver);
    DECLARE_ID (uiCustomAllowExternalDrags);
    
    DECLARE_ID (uiCustomStartDraggingCallback);
    DECLARE_ID (uiCustomIsInterestedInDragSourceCallback);
    DECLARE_ID (uiCustomItemDragEnterCallback);
    DECLARE_ID (uiCustomItemDragMoveCallback);
    DECLARE_ID (uiCustomItemDragExitCallback);
    DECLARE_ID (uiCustomItemDroppedCallback);
    
    DECLARE_ID (uiWaveformColour);
    DECLARE_ID (uiWaveformBackgroundColour1);
    DECLARE_ID (uiWaveformBackgroundColour2);
    DECLARE_ID (uiWaveformOutlineColour);
    DECLARE_ID (uiWaveformOutlineThickness);
    DECLARE_ID (uiWaveformSourceSamplesPerThumbnailSample);
    DECLARE_ID (uiWaveFormDrawSecondsStart);
    DECLARE_ID (uiWaveFormDrawSecondsEnd);
    DECLARE_ID (uiWaveFormVeritcalZoomFactor);
    DECLARE_ID (uiWaveFormThumbnailChangedCallback);
    DECLARE_ID (uiWaveFormSourceChangedCallback);
    DECLARE_ID (uiWaveFormFilesDroppedCallback);
    
    DECLARE_ID (uiHyperlinkColour);
    DECLARE_ID (uiHyperlinkFont);
    DECLARE_ID (uiHyperlinkFitTextToSize);
    DECLARE_ID (uiHyperlinkTextJustification);
    DECLARE_ID (uiHyperlinkOpensUrl);
    DECLARE_ID (uiHyperlinkUrl);
    
    DECLARE_ID (uiXYSurfaceBgGradientType);
    DECLARE_ID (uiXYSurfaceBackgroundColour1);
    DECLARE_ID (uiXYSurfaceBackgroundColour2);
    DECLARE_ID (uiXYSurfaceOutlineGradientType);
    DECLARE_ID (uiXYSurfaceOutlineColour1);
    DECLARE_ID (uiXYSurfaceOutlineColour2);
    DECLARE_ID (uiXYSurfaceOutlineThickness);
    DECLARE_ID (uiXYSurfaceCornerSize);
    DECLARE_ID (uiXYSurfaceBgImageResource);
    DECLARE_ID (uiXYSurfaceBgImageLayout);
    DECLARE_ID (uiXYSurfaceBgImageAlpha);
    DECLARE_ID (uiXYSurfaceXTrackEnabled);
    DECLARE_ID (uiXYSurfaceXTrackColour);
    DECLARE_ID (uiXYSurfaceXTrackThickness);
    DECLARE_ID (uiXYSurfaceYTrackColour);
    DECLARE_ID (uiXYSurfaceYTrackThickness);
    DECLARE_ID (uiXYSurfaceModSectionLocation);
    DECLARE_ID (uiXYSurfaceModSectionHeight);
    DECLARE_ID (uiXYSurfaceInfoLabelVisible);
    DECLARE_ID (uiXYSurfaceInfoLabelLocation);
    DECLARE_ID (uiXYSurfaceInfoLabelColour);
    DECLARE_ID (uiXYSurfaceInfoLabelFont);
    DECLARE_ID (uiXYSurfaceModulatorBgGradientType);
    DECLARE_ID (uiXYSurfaceModulatorBgColour1);
    DECLARE_ID (uiXYSurfaceModulatorBgColour2);
    DECLARE_ID (uiXYSurfaceModulatorOutlineGradientType);
    DECLARE_ID (uiXYSurfaceModulatorOutlineColour1);
    DECLARE_ID (uiXYSurfaceModulatorOutlineColour2);
    DECLARE_ID (uiXYSurfaceModulatorOutlineThickness);
    DECLARE_ID (uiXYSurfaceModulatorWidth);
    DECLARE_ID (uiXYSurfaceModulatorHeight);
    DECLARE_ID (uiXYSurfaceMaxX);
    DECLARE_ID (uiXYSurfaceDestinationX);
    DECLARE_ID (uiXYSurfaceMaxY);
    DECLARE_ID (uiXYSurfaceDestinationY);
    DECLARE_ID (uiXYSurfaceGradientColour);
    DECLARE_ID (uiXYSurfaceGradientGrain);
    DECLARE_ID (uiXYSuraceXFlip);
    DECLARE_ID (uiXYSuraceYFlip);
    DECLARE_ID (uiXYSuraceShowRightClickMenu);
    DECLARE_ID (uiXYSurfaceDestinationXGroupFilter);
    DECLARE_ID (uiXYSurfaceDestinationYGroupFilter);
    
    DECLARE_ID (uiEnvelopeAddPoint);
    DECLARE_ID (uiEnvelopeRemovePoint);
    DECLARE_ID (uiEnvelopeBgGradientType);
    DECLARE_ID (uiEnvelopeBgColour1);
    DECLARE_ID (uiEnvelopeBgColour2);
    DECLARE_ID (uiEnvelopeOutlineColour);
    DECLARE_ID (uiEnvelopeOutlineThickness);
    DECLARE_ID (uiEnvelopeLineColour);
    DECLARE_ID (uiEnvelopeLineThickness);
    DECLARE_ID (uiEnvelopeLineFill);
    DECLARE_ID (uiEnvelopeLineFillColour1);
    DECLARE_ID (uiEnvelopeLineFillColour2);
    
    DECLARE_ID (uiEnvelopeLegendWidth);
    DECLARE_ID (uiEnvelopeLegendHeight);
    DECLARE_ID (uiEnvelopeLegendVisible);
    DECLARE_ID (uiEnvelopeLegendBgColour);
    DECLARE_ID (uiEnvelopeLegendColour);
    DECLARE_ID (uiEnvelopeLegendFont);
    DECLARE_ID (uiEnvelopeLegendJustification);
    DECLARE_ID (uiEnvelopeLegendOutlineColour);
    DECLARE_ID (uiEnvelopeLegendFormat);
    
    DECLARE_ID (uiEnvelopePoint);
    DECLARE_ID (uiEnvelopePointMinX);
    DECLARE_ID (uiEnvelopePointMaxX);
    DECLARE_ID (uiEnvelopePointLockY);
    DECLARE_ID (uiEnvelopePointLockX);
    DECLARE_ID (uiEnvelopePointMaxXValue);
    DECLARE_ID (uiEnvelopePointMaxYValue);
    DECLARE_ID (uiEnvelopePointSize);
    DECLARE_ID (uiEnvelopePointIndex);
    DECLARE_ID (uiEnvelopePointColour);
    DECLARE_ID (uiEnvelopePointCorner);
    DECLARE_ID (uiEnvelopePointName);
    DECLARE_ID (uiEnvelopePointPosition);
    DECLARE_ID (uiEnvelopePointLabelFormat);
    DECLARE_ID (uiEnvelopePointLabelColour);
    DECLARE_ID (uiEnvelopePointLabelFont);
    DECLARE_ID (uiEnvelopePointLabelBgColour);
    DECLARE_ID (uiEnvelopePointLabelOutlineColour);
    DECLARE_ID (uiEnvelopePointLabelVisible);
    DECLARE_ID (uiEnvelopePointLinkX);
    DECLARE_ID (uiEnvelopePointLinkY);
    DECLARE_ID (uiEnvelopePointExpressionX);
    DECLARE_ID (uiEnvelopePointExpressionValueX);
    DECLARE_ID (uiEnvelopePointExpressionY);
    DECLARE_ID (uiEnvelopePointExpressionValueY);
    DECLARE_ID (uiEnvelopePointValueCalculationMethod);
    DECLARE_ID (uiProgressBarForegroundColour);
    DECLARE_ID (uiProgressBarBackgroundColour);
    DECLARE_ID (uiProgressBarDisplayPercent);
    
    DECLARE_ID (uiSlider);
    DECLARE_ID (uiFixedSlider);
    DECLARE_ID (uiImageSlider);
    DECLARE_ID (uiFixedImageSlider);
    DECLARE_ID (uiToggleButton);
    DECLARE_ID (uiButton);
    DECLARE_ID (uiImageButton);
    DECLARE_ID (uiCombo);
    DECLARE_ID (uiLabel);
    DECLARE_ID (uiLCDLabel);
    DECLARE_ID (uiGroup);
    DECLARE_ID (uiImage);
    DECLARE_ID (uiCustomComponent);
    DECLARE_ID (uiMidiKeyboard);
    DECLARE_ID (uiEnvelope);
    DECLARE_ID (uiEnvelopeADSR);
    DECLARE_ID (uiTabs);
    DECLARE_ID (uiPanelEditor);
    DECLARE_ID (uiArrow);
    DECLARE_ID (uiWaveform);
    DECLARE_ID (uiHyperlink);
    DECLARE_ID (uiXYSurface);
    DECLARE_ID (uiListBox);
    DECLARE_ID (uiFileListBox);
    DECLARE_ID (uiProgressBar);
    
    DECLARE_ID (modulatorReference);
    DECLARE_ID (midiMessageCtrlrNumber);
    DECLARE_ID (midiMessageChannel);
    DECLARE_ID (midiMessageCtrlrValue);
    DECLARE_ID (midiMessageType);
    DECLARE_ID (midiMessageSysExFormula);
    DECLARE_ID (midiMessageChannelOverride);
    DECLARE_ID (midiMessageMultiList);
    DECLARE_ID (midiInChannel);
    DECLARE_ID (midiOutChannel);
    
    DECLARE_ID (midiDev);
    DECLARE_ID (midiDevId);
    DECLARE_ID (midiDevType);
    DECLARE_ID (midiDevState);
    DECLARE_ID (midiDevIndex);
    DECLARE_ID (midiDevHandler);
    DECLARE_ID (midiDeviceErrorState);
    DECLARE_ID (midiDeviceManager);
    
    DECLARE_ID (invalid);
    DECLARE_ID (top);
    DECLARE_ID (bottom);
    DECLARE_ID (left);
    DECLARE_ID (right);
    DECLARE_ID (bottomRight);
    DECLARE_ID (bottomLeft);
    DECLARE_ID (topRight);
    DECLARE_ID (topLeft);
    DECLARE_ID (over);
    DECLARE_ID (under);
    
    DECLARE_ID (lastBrowsedSkinDir);
    DECLARE_ID (lastBrowsedComponentDir);
    DECLARE_ID (luaTreeEditorState);
    DECLARE_ID (properties);
    DECLARE_ID (ctrlrOverrides);
    DECLARE_ID (uiLuaConsoleSnips);
    DECLARE_ID (uiLuaConsoleFiles);
    DECLARE_ID (uiLuaConsoleLastDir);
    DECLARE_ID (uiLuaConsoleInputRemoveAfterRun);
    DECLARE_ID (resources);
    DECLARE_ID (resource);
    DECLARE_ID (resourceSize);
    DECLARE_ID (resourceLoadedTime);
    DECLARE_ID (resourceImage);
    DECLARE_ID (resourceImageWidth);
    DECLARE_ID (resourceImageHeight);
    DECLARE_ID (resourceImagePaintMode);
    DECLARE_ID (resourceImageOrientation);
    DECLARE_ID (resourceBlob);
    DECLARE_ID (resourceHash);
    DECLARE_ID (resourceLicense);
    DECLARE_ID (resourceName);
    DECLARE_ID (resourceType);
    DECLARE_ID (resourceFile);
    DECLARE_ID (resourceSourceFile);
    DECLARE_ID (resourceData);
    DECLARE_ID (resourceExportList);
    DECLARE_ID (resourcePanelSnapshot);
    DECLARE_ID (resourceFont);
        
    DECLARE_ID (ctrlrEditorBounds);
    DECLARE_ID (ctrlrMaxExportedVstParameters);
    DECLARE_ID (ctrlrShutdownDelay);
    DECLARE_ID (ctrlrUseEditorWrapper);
    DECLARE_ID (ctrlrVersionSeparator);
    DECLARE_ID (ctrlrVersionCompressed);
    DECLARE_ID (ctrlrMidiMonInputBufferSize);
    DECLARE_ID (ctrlrMidiMonOutputBufferSize);
    DECLARE_ID (ctrlrLogMidiInput);
    DECLARE_ID (ctrlrLogMidiOutput);
    DECLARE_ID (ctrlrLogOptions);
    DECLARE_ID (ctrlrLuaDisabled);
    DECLARE_ID (ctrlrLastBrowsedResourceDir);
    DECLARE_ID (ctrlrLogToFile);
    DECLARE_ID (ctrlrWarningInBootstrapState); // Added v5.6.32
    DECLARE_ID (ctrlrLuaDebug);
    DECLARE_ID (ctrlrOverwriteResources);
    DECLARE_ID (ctrlrAutoSave);
    DECLARE_ID (ctrlrAutoSaveInterval);
    DECLARE_ID (ctrlrLastBrowsedFileDirectory);
    DECLARE_ID (ctrlrOpenWindowState);
    DECLARE_ID (ctrlrRecenetOpenedPanelFiles);
    DECLARE_ID (ctrlrPropertiesAreURLs);
    DECLARE_ID (ctrlrKeyboardMapping);
    DECLARE_ID (ctrlrCapabilities);
    DECLARE_ID (ctrlrLastMenuItemId);
    DECLARE_ID (ctrlrNativeAlerts);
    DECLARE_ID (ctrlrNativeFileDialogs);
    DECLARE_ID (ctrlrMenuItemBackgroundColour);
    DECLARE_ID (ctrlrMenuItemTextColour);
    DECLARE_ID (ctrlrMenuItemHighlightedTextColour);
    DECLARE_ID (ctrlrMenuItemHighlightColour);
    DECLARE_ID (ctrlrMenuItemFont);
    DECLARE_ID (ctrlrMenuItemSeparatorColour);
    DECLARE_ID (ctrlrMenuItemHeaderColour);
    
    DECLARE_ID (ctrlrMenuBarBackgroundColour1);
    DECLARE_ID (ctrlrMenuBarBackgroundColour2);
    DECLARE_ID (ctrlrMenuBarTextColour);
    DECLARE_ID (ctrlrMenuBarHeight);
    DECLARE_ID (ctrlrMenuBarHighlightedTextColour);
    DECLARE_ID (ctrlrMenuBarHighlightColour);
    DECLARE_ID (ctrlrMenuBarFont);
    DECLARE_ID (ctrlrPrivateKey);
    DECLARE_ID (ctrlrFontSizeBaseValue);
    DECLARE_ID (ctrlrPropertyLineheightBaseValue); // Added v5.6.33
    DECLARE_ID (ctrlrScrollbarThickness);
    DECLARE_ID (ctrlrLegacyMode);
    DECLARE_ID (ctrlrColourScheme);
    DECLARE_ID (ctrlrLookAndFeel);
    DECLARE_ID (ctrlrTabBarDepth);
    
    DECLARE_ID (luaMethodName);
    DECLARE_ID (luaManagerMethods); // need to keep it for compatibility reasons
    DECLARE_ID (luaMethodEditor);
    DECLARE_ID (luaMethodEditorFont);
    DECLARE_ID (luaMethodEditorBgColour);
    DECLARE_ID (luaMethodEditorLineNumbersBgColour); // Added v5.6.31
    DECLARE_ID (luaMethodEditorLineNumbersColour); // Added v5.6.31
    DECLARE_ID (luaMethodGroup);
    DECLARE_ID (luaMethodCode);
    DECLARE_ID (luaMethodSource);
    DECLARE_ID (luaMethodSourcePath);
    DECLARE_ID (luaMethodLinkedProperty);
    DECLARE_ID (luaMethod);
    DECLARE_ID (luaManager);
    DECLARE_ID (luaMethodValid);
    
    DECLARE_ID (luaModulatorValueChange);
    DECLARE_ID (luaModulatorComponentChange);
    DECLARE_ID (luaModulatorPropertyChanged);
    DECLARE_ID (luaModulatorMidiPatternChanged);
    DECLARE_ID (luaModulatorGetValueForMIDI);
    DECLARE_ID (luaModulatorGetValueFromMIDI);
    DECLARE_ID (luaPanelLoaded);
    DECLARE_ID (luaPanelModulatorValueChanged);
    DECLARE_ID (luaPanelBeforeLoad);
    DECLARE_ID (luaPanelMidiReceived);
    DECLARE_ID (luaPanelMidiMultiReceived);
    DECLARE_ID (luaPanelMidiChannelChanged);
    DECLARE_ID (luaPanelPaintBackground);
    DECLARE_ID (luaPanelSaved);
    DECLARE_ID (luaLayoutResized);
    DECLARE_ID (luaViewPortResized);
    DECLARE_ID (luaPanelResized);
    DECLARE_ID (luaPanelOSCReceived);
    DECLARE_ID (luaPanelProgramChanged);
    DECLARE_ID (luaPanelGlobalChanged);
    DECLARE_ID (luaPanelMenubarCustom);
    DECLARE_ID (luaPanelMessageHandler);
    DECLARE_ID (luaPanelFileDragDropHandler);
    DECLARE_ID (luaPanelFileDragEnterHandler);
    DECLARE_ID (luaPanelFileDragExitHandler);
    DECLARE_ID (luaPanelResourcesLoaded);
    DECLARE_ID (luaPanelSaveState);
    DECLARE_ID (luaPanelRestoreState);
    DECLARE_ID (luaPanelMidiSnapshotPre);
    DECLARE_ID (luaPanelMidiSnapshotPost);
    
    DECLARE_ID (luaMidiLibrarySend);
    DECLARE_ID (luaMidiLibraryRequest);
    DECLARE_ID (luaMidiLibraryProcess);
    DECLARE_ID (luaMidiLibraryConfirm);
    DECLARE_ID (luaMidiLibraryUndefined);
    
    DECLARE_ID (luaTransProcess);
    DECLARE_ID (luaTransDataUnpack);
    DECLARE_ID (luaTransDataPack);
    DECLARE_ID (luaTransRequest);
    DECLARE_ID (luaTransRequestData);
    DECLARE_ID (luaTransResponseData);
    DECLARE_ID (luaTransConfData);
    DECLARE_ID (luaTransInfo);
    DECLARE_ID (luaTransTimeout);
    DECLARE_ID (luaTransNameData);
    DECLARE_ID (luaCtrlrSaveState);
    DECLARE_ID (luaCtrlrRestoreState);
    
    DECLARE_ID (luaAudioProcessBlock);
    
    DECLARE_ID (midiBufferEditor);
    DECLARE_ID (midiBufferEditorLeft);
    DECLARE_ID (midiBufferEditorRight);
    DECLARE_ID (midiBufferEditorSource);
    DECLARE_ID (midiBufferEditorFilePath);
    DECLARE_ID (midiBufferEditorContent);
    DECLARE_ID (midiBufferEditorCodeEditor);
    
    DECLARE_ID(midiFilter);
    }

#endif
