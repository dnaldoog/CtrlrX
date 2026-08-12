#include "stdafx.h"

#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrPanel/CtrlrPanelCanvas.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include "CtrlrPanelLayerList.h"
#include "CtrlrPanelLayerListItem.h"

//==============================================================================
// CtrlrPanelLayerListHeader Implementation
//==============================================================================
CtrlrPanelLayerListHeader::CtrlrPanelLayerListHeader() {
	addAndMakeVisible(indexHeader);
	indexHeader.setText("z-index", juce::dontSendNotification);
	indexHeader.setJustificationType(juce::Justification::centred);
	indexHeader.setFont(juce::Font(12.0f, juce::Font::plain));

	addAndMakeVisible(reorderHeader);
	reorderHeader.setText("Reorder", juce::dontSendNotification);
	reorderHeader.setJustificationType(juce::Justification::centred);
	reorderHeader.setFont(juce::Font(12.0f, juce::Font::plain));

	addAndMakeVisible(visibilityHeader);
	visibilityHeader.setText("Visiblity", juce::dontSendNotification);
	visibilityHeader.setJustificationType(juce::Justification::centred);
	visibilityHeader.setFont(juce::Font(12.0f, juce::Font::plain));

	addAndMakeVisible(nameHeader);
	nameHeader.setText("Name", juce::dontSendNotification);
	nameHeader.setJustificationType(juce::Justification::centred);
	nameHeader.setFont(juce::Font(12.0f, juce::Font::plain));

	addAndMakeVisible(colourHeader);
	colourHeader.setText("Colour", juce::dontSendNotification);
	colourHeader.setJustificationType(juce::Justification::centred);
	colourHeader.setFont(juce::Font(12.0f, juce::Font::plain));

	addAndMakeVisible(editHeader);
	editHeader.setText("Edit View", juce::dontSendNotification);
	editHeader.setJustificationType(juce::Justification::centred);
	editHeader.setFont(juce::Font(12.0f, juce::Font::plain));
}

CtrlrPanelLayerListHeader::~CtrlrPanelLayerListHeader() {}

void CtrlrPanelLayerListHeader::resized() {
	// The proportions must add up to 1.0
	const float layerIndexProportion = 0.07f;
	const float dragIconProportion = 0.07f;
	const float visibilityProportion = 0.07f;
	const float layerNameProportion = 0.33f;
	const float layerColourProportion = 0.23f;
	const float buttonProportion = 0.23f;

	float x = 0.0f;
	const float totalWidth = getWidth();

	// Position elements in the new order
	// 1. Layer Index
	float layerIndexWidth = totalWidth * layerIndexProportion;
	indexHeader.setBounds(x, 0, layerIndexWidth, getHeight());
	x += layerIndexWidth;

	// 2. Drag Icon (Reorder)
	float dragIconWidth = totalWidth * dragIconProportion;
	reorderHeader.setBounds(x, 0, dragIconWidth, getHeight());
	x += dragIconWidth;

	// 3. Visibility
	float visibilityWidth = totalWidth * visibilityProportion;
	visibilityHeader.setBounds(x, 0, visibilityWidth, getHeight());
	x += visibilityWidth;

	// 4. Layer Name
	float layerNameWidth = totalWidth * layerNameProportion;
	nameHeader.setBounds(x, 0, layerNameWidth, getHeight());
	x += layerNameWidth;

	// 5. Layer Colour
	float colourChooserWidth = totalWidth * layerColourProportion;
	colourHeader.setBounds(x, 0, colourChooserWidth, getHeight());
	x += colourChooserWidth;

	// 6. Action Buttons
	float buttonWidth = totalWidth * buttonProportion;
	editHeader.setBounds(x, 0, buttonWidth, getHeight());
	x += buttonWidth;

	// Update separator positions
	separatorPositions.clear();
	separatorPositions.add(static_cast<int>(indexHeader.getRight()));
	separatorPositions.add(static_cast<int>(reorderHeader.getRight()));
	separatorPositions.add(static_cast<int>(visibilityHeader.getRight()));
	separatorPositions.add(static_cast<int>(nameHeader.getRight()));
	separatorPositions.add(static_cast<int>(colourHeader.getRight()));
}

void CtrlrPanelLayerListHeader::paint(juce::Graphics &g) {
	g.setColour(juce::Colours::lightgrey);

	for (int pos : separatorPositions) {
		g.drawLine(pos, 0, pos, getHeight(), 1.0f);
	}
}

//==============================================================================
// CtrlrPanelLayerList Implementation
//==============================================================================
CtrlrPanelLayerList::CtrlrPanelLayerList(CtrlrPanel &_owner)
	: owner(_owner),
	  dropInsertionIndex(-1),
	  layerIsolationActive(false),
	  isolatedLayerIndex(-1),
	  layerList(std::make_unique<juce::ListBox>("Layer List", this)),
	  headerComponent(std::make_unique<CtrlrPanelLayerListHeader>()) {
	// Explicitly call the methods from the base class that provides them
	CtrlrChildWindowContent::addAndMakeVisible(*headerComponent);
	CtrlrChildWindowContent::addAndMakeVisible(*layerList);

	// Explicitly call the setSize method from the base class
	CtrlrChildWindowContent::setSize(600, 400);

	layerList->setRowHeight(40);
	layerList->setMultipleSelectionEnabled(false);
}

CtrlrPanelLayerList::~CtrlrPanelLayerList() {
	// The parent component (`CtrlrChildWindowContent`) takes ownership
	// of these when they are added with `addAndMakeVisible`. We must
	// explicitly release them from our unique_ptr to prevent a double-free crash.
	layerList.release();
	headerComponent.release();
}

//==============================================================================
void CtrlrPanelLayerList::paintOverChildren(Graphics &g) {
	if (dropInsertionIndex >= 0) {
		int lineY = headerComponent->getHeight() + dropInsertionIndex * layerList->getRowHeight() -
					layerList->getViewport()->getViewPositionY();
		g.setColour(Colours::white);
		g.fillRect(layerList->getX(), lineY - 1, layerList->getWidth(), 2);
	}
}
void CtrlrPanelLayerList::paint(Graphics &g) {
	// Draw drop insertion indicator
	if (dropInsertionIndex >= 0) {
		g.setColour(Colours::blue);
		int y = dropInsertionIndex * layerList->getRowHeight() + headerComponent->getHeight();

		// Explicitly call the getWidth() method from the CtrlrChildWindowContent base class
		g.fillRect(0, y - 1, CtrlrChildWindowContent::getWidth(), 3);
	}

	// Draw isolation indicator
	if (layerIsolationActive) {
		g.setColour(Colours::orange.withAlpha(0.3f));
		g.fillRect(2, headerComponent->getHeight() + 2, CtrlrChildWindowContent::getWidth() - 4, 20);

		g.setColour(Colours::orange.darker());
		g.setFont(Font(11.0f, Font::bold));
		g.drawText("LAYER ISOLATION ACTIVE", 5, headerComponent->getHeight() + 2,
				   CtrlrChildWindowContent::getWidth() - 10, 20, Justification::centredLeft);
	}
}

void CtrlrPanelLayerList::resized() {
	const int headerHeight = 30;

	// Set bounds for the header component
	if (headerComponent) {
		// Explicitly call getWidth() from the base class
		headerComponent->setBounds(0, 0, CtrlrChildWindowContent::getWidth(), headerHeight);
	}

	// Set bounds for the list below the header
	// Explicitly call getWidth() and getHeight() from the base class
	layerList->setBounds(0, headerHeight, CtrlrChildWindowContent::getWidth(),
						 CtrlrChildWindowContent::getHeight() - headerHeight);
}

int CtrlrPanelLayerList::getNumRows() {
	return (owner.getEditor()->getCanvas()->getNumLayers());
}

void CtrlrPanelLayerList::paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) {
	if (rowIsSelected) {
		g.setColour(juce::Colours::steelblue.withAlpha(0.4f));
		g.fillRect(0, 0, width, height);

		g.setColour(juce::Colours::steelblue);
		g.drawRect(0, 0, width, height, 1);
	}
}

Component *CtrlrPanelLayerList::refreshComponentForRow(int rowNumber, bool isRowSelected,
													   Component *existingComponentToUpdate) {
	CtrlrPanelLayerListItem *itemInfo = (CtrlrPanelLayerListItem *)existingComponentToUpdate;

	if (itemInfo == 0)
		itemInfo = new CtrlrPanelLayerListItem(*this);

	// Calculate the actual layer index (reverse the order)
	int totalLayers = owner.getEditor()->getCanvas()->getNumLayers();
	int actualLayerIndex = totalLayers - 1 - rowNumber; // Reverse the index

	itemInfo->setRow(actualLayerIndex); // Use the actual layer index for the row
	itemInfo->setLayer(owner.getEditor()->getCanvas()->getLayerFromArray(actualLayerIndex));
	itemInfo->setSelected(isRowSelected);

	return itemInfo;
}

void CtrlrPanelLayerList::setSelectedRow(const int rowToSelect) {
	layerList->selectRow(rowToSelect);
}

void CtrlrPanelLayerList::buttonClicked(Button *button) {}

CtrlrPanel &CtrlrPanelLayerList::getOwner() {
	return (owner);
}

void CtrlrPanelLayerList::addLayer() {
	if (owner.getEditor()) {
		owner.getEditor()->getCanvas()->addLayer(ValueTree());
	}
	layerList->updateContent();
}

void CtrlrPanelLayerList::removeLayer() {
	const int selectedRow = layerList->getSelectedRow();

	// Convert visual row to actual layer index
	int totalLayers = getNumRows();
	int actualLayerIndex = totalLayers - 1 - selectedRow;

	CtrlrPanelLayerListItem *item =
		dynamic_cast<CtrlrPanelLayerListItem *>(layerList->getComponentForRowNumber(selectedRow));
	CtrlrPanelCanvasLayer *layer = 0;
	if (item != nullptr) {
		layer = item->getLayer();
	}

	if (owner.getEditor()) {
		owner.getEditor()->getCanvas()->removeLayer(layer);
	}
	layerList->updateContent();
}

void CtrlrPanelLayerList::moveLayerUp() {
	const int selectedRow = layerList->getSelectedRow();

	// Convert visual row to actual layer index
	int totalLayers = getNumRows();
	int actualLayerIndex = totalLayers - 1 - selectedRow;

	CtrlrPanelLayerListItem *item =
		dynamic_cast<CtrlrPanelLayerListItem *>(layerList->getComponentForRowNumber(selectedRow));
	CtrlrPanelCanvasLayer *layer = 0;
	if (item != nullptr) {
		layer = item->getLayer();
	}

	if (selectedRow - 1 < 0) // Can't move top visual row up
		return;

	if (owner.getEditor()) {
		// In reversed view: visual "up" = actual "down" in the array
		owner.getEditor()->getCanvas()->moveLayer(layer, false); // false = down in actual array
	}
	layerList->updateContent();
	layerList->selectRow(selectedRow - 1);
}

void CtrlrPanelLayerList::moveLayerDown() {
	const int selectedRow = layerList->getSelectedRow();

	// Convert visual row to actual layer index
	int totalLayers = getNumRows();
	int actualLayerIndex = totalLayers - 1 - selectedRow;

	CtrlrPanelLayerListItem *item =
		dynamic_cast<CtrlrPanelLayerListItem *>(layerList->getComponentForRowNumber(selectedRow));
	CtrlrPanelCanvasLayer *layer = 0;
	if (item != nullptr) {
		layer = item->getLayer();
	}

	if (selectedRow + 1 >= getNumRows()) // Can't move bottom visual row down
		return;

	if (owner.getEditor()) {
		// In reversed view: visual "down" = actual "up" in the array
		owner.getEditor()->getCanvas()->moveLayer(layer, true); // true = up in actual array
	}
	layerList->updateContent();
	layerList->selectRow(selectedRow + 1);
}

void CtrlrPanelLayerList::refresh() {
	layerList->updateContent();
	updateAllButtonStates();
}

StringArray CtrlrPanelLayerList::getMenuBarNames() {
	const char *const names[] = {"File", "Edit", "View", nullptr};
	return StringArray(names);
}

PopupMenu CtrlrPanelLayerList::getMenuForIndex(int topLevelMenuIndex, const String &menuName) {
	PopupMenu menu;
	if (topLevelMenuIndex == 0) {
		menu.addItem(1, "Close");
	} else if (topLevelMenuIndex == 1) {
		menu.addItem(2, "Add layer");
		menu.addItem(3, "Remove layer");
		menu.addSectionHeader("Reposition");
		menu.addItem(4, "Move up");
		menu.addItem(5, "Move down");
	} else if (topLevelMenuIndex == 2) {
		menu.addItem(6, "Restore view");
	}
	return (menu);
}

void CtrlrPanelLayerList::menuItemSelected(int menuItemID, int topLevelMenuIndex) {
	if (topLevelMenuIndex == 1) {
		if (menuItemID == 2)
			addLayer();
		if (menuItemID == 3)
			removeLayer();
		if (menuItemID == 4)
			moveLayerUp();
		if (menuItemID == 5)
			moveLayerDown();
	}
	if (topLevelMenuIndex == 2) {
		if (menuItemID == 6)
			restoreLayerVisibility();
		updateAllButtonStates();
	}
	if (topLevelMenuIndex == 0 && menuItemID == 1) {
		// close handle
		owner.getWindowManager().toggle(CtrlrPanelWindowManager::LayerEditor, false);
	}
}

bool CtrlrPanelLayerList::isInterestedInDragSource(const SourceDetails &dragSourceDetails) {
	// We're interested if the drag source contains "layer_item" in the description
	return dragSourceDetails.description.toString().contains("layer_item");
}

void CtrlrPanelLayerList::itemDragEnter(const SourceDetails &dragSourceDetails) {
	CtrlrChildWindowContent::repaint();
}

void CtrlrPanelLayerList::itemDragExit(const SourceDetails &dragSourceDetails) {
	dropInsertionIndex = -1;
	CtrlrChildWindowContent::repaint();
}

int CtrlrPanelLayerList::getVisualRowForDrag(const SourceDetails &dragSourceDetails) {
	Point<int> localPos =
		layerList->getLocalPoint(static_cast<CtrlrChildWindowContent *>(this), dragSourceDetails.localPosition);

	int yPosRelativeToListBox = localPos.y - headerComponent->getHeight();

	int row = layerList->getInsertionIndexForPosition(localPos.x, yPosRelativeToListBox);
	return jmax(0, jmin(row, getNumRows() - 1));
}

int CtrlrPanelLayerList::getInsertionGapForDrag(const SourceDetails &dragSourceDetails) {
	Point<int> localPos =
		layerList->getLocalPoint(static_cast<CtrlrChildWindowContent *>(this), dragSourceDetails.localPosition);

	int gap = layerList->getInsertionIndexForPosition(localPos.x, localPos.y); // no header subtraction
	return jmax(0, jmin(gap, getNumRows()));
}

void CtrlrPanelLayerList::itemDragMove(const SourceDetails &dragSourceDetails) {
	Point<int> localPos =
		layerList->getLocalPoint(static_cast<CtrlrChildWindowContent *>(this), dragSourceDetails.localPosition);

	if (auto *vp = layerList->getViewport()) {
		Point<int> vpPos = vp->getLocalPoint(layerList.get(), localPos);
		vp->autoScroll(vpPos.x, vpPos.y, 20, 8);
	}

	dropInsertionIndex = getInsertionGapForDrag(dragSourceDetails);

	CtrlrChildWindowContent::repaint();
}

void CtrlrPanelLayerList::itemDropped(const SourceDetails &dragSourceDetails) {
	if (!isInterestedInDragSource(dragSourceDetails))
		return;

	String desc = dragSourceDetails.description.toString();
	int sourceVisualRow = desc.getTrailingIntValue();

	int gap = dropInsertionIndex >= 0 ? dropInsertionIndex : getInsertionGapForDrag(dragSourceDetails);

	if (sourceVisualRow >= 0 && sourceVisualRow < getNumRows()) {
		// Convert the gap to a final visual row for an item that's already in the list:
		// dropping "before" a gap that's past the source's current row means the
		// source's own removal shifts everything above the gap up by one.
		int targetVisualRow = (gap <= sourceVisualRow) ? gap : gap - 1;
		targetVisualRow = jmax(0, jmin(targetVisualRow, getNumRows() - 1));

		if (targetVisualRow != sourceVisualRow) {
			int totalLayers = getNumRows();
			int sourceActualIndex = totalLayers - 1 - sourceVisualRow;
			int targetActualIndex = totalLayers - 1 - targetVisualRow;

			moveLayerToPosition(sourceActualIndex, targetActualIndex);
		}
	}

	dropInsertionIndex = -1;
	CtrlrChildWindowContent::repaint();
}
void CtrlrPanelLayerList::moveLayerToPosition(int sourceActualIndex, int targetActualIndex) {
	if (owner.getEditor() && owner.getEditor()->getCanvas()) {
		// Get the layer that's being moved
		CtrlrPanelCanvasLayer *sourceLayer = owner.getEditor()->getCanvas()->getLayerFromArray(sourceActualIndex);

		if (sourceLayer != nullptr) {
			if (targetActualIndex < sourceActualIndex) {
				// Moving to lower actual index (higher in visual list) - call moveLayerUp
				for (int i = sourceActualIndex; i > targetActualIndex; --i) {
					owner.getEditor()->getCanvas()->moveLayer(sourceLayer, true); // true = up in actual array
				}
			} else if (targetActualIndex > sourceActualIndex) {
				// Moving to higher actual index (lower in visual list) - call moveLayerDown
				for (int i = sourceActualIndex; i < targetActualIndex; ++i) {
					owner.getEditor()->getCanvas()->moveLayer(sourceLayer, false); // false = down in actual array
				}
			}

			// Update the list display and select the new visual position
			layerList->updateContent();

			// Convert the target actual index back to visual row for selection
			int totalLayers = getNumRows();
			int targetVisualRow = totalLayers - 1 - targetActualIndex;
			layerList->selectRow(targetVisualRow);
		}
	}
}
void CtrlrPanelLayerList::isolateLayer(int targetLayerIndex) {
	if (!owner.getEditor() || !owner.getEditor()->getCanvas())
		return;

	// FIRST: Save the current states BEFORE making any changes
	owner.saveLayerVisibilityStates();

	// Remember which layer was isolated
	isolatedLayerIndex = targetLayerIndex;

	// THEN: Hide all layers except the target layer
	for (int i = 0; i < getNumRows(); ++i) {
		CtrlrPanelCanvasLayer *layer = owner.getEditor()->getCanvas()->getLayerFromArray(i);
		if (layer) {
			if (i == targetLayerIndex) {
				layer->setProperty(Ids::uiPanelCanvasLayerVisibility, true);
			} else {
				layer->setProperty(Ids::uiPanelCanvasLayerVisibility, false);
			}
		}
	}

	layerIsolationActive = true;
	refresh();
	updateAllButtonStates();

	_DBG("Layer " + String(targetLayerIndex) + " isolated - all other layers hidden");
}

// Update the restoreLayerVisibility method:
void CtrlrPanelLayerList::restoreLayerVisibility() {
	// New: Loop through all layers and explicitly set their 'isolated' property to false.
	for (int i = 0; i < getNumRows(); ++i) {
		if (CtrlrPanelCanvasLayer *layer = owner.getEditor()->getCanvas()->getLayerFromArray(i)) {
			layer->setProperty(Ids::uiPanelCanvasLayerIsIsolated, false, 0);
		}
	}

	// Now, call the existing restoration logic.
	owner.restoreLayerVisibilityStates();
	layerIsolationActive = false;
	isolatedLayerIndex = -1;
	refresh();
	updateAllButtonStates();
	_DBG("Layer visibility restored");
}

void CtrlrPanelLayerList::updateAllButtonStates() {
	for (int i = 0; i < getNumRows(); ++i) {
		if (Component *comp = layerList->getComponentForRowNumber(i)) {
			if (CtrlrPanelLayerListItem *item = dynamic_cast<CtrlrPanelLayerListItem *>(comp)) {
				item->updateButtonStates();
			}
		}
	}
}
bool CtrlrPanelLayerList::isLayerIsolated(int layerIndex) const {
	return (isolatedLayerIndex == layerIndex && owner.hasLayerVisibilityStates());
}
