// LColourSelector.cpp
#include "stdafx.h"
#include "LColourSelector.h"
#include "luabind/luabind.hpp"

// void showColourPickerPopupCb(Component *anchorComp, Component *targetLabelComp, Component *repaintTargetComp,
// 							 const Colour &initialColour, bool liveUpdate, luabind::object cb) {
// 	if (MessageManager::getInstance() == nullptr)
// 		return;

// 	auto popup = std::make_unique<ColourPickerPopupContent>(targetLabelComp, repaintTargetComp, initialColour, 0,
// 															liveUpdate, cb);

// 	Rectangle<int> targetArea = anchorComp != nullptr ? anchorComp->getScreenBounds() : Rectangle<int>(0, 0, 1, 1);

// 	Component *parentForBox = anchorComp != nullptr ? anchorComp->getTopLevelComponent() : nullptr;

// 	CallOutBox::launchAsynchronously(std::move(popup), targetArea, parentForBox);
// } // declared outside namespace for CtrlrLuaUtils

void LColourSelector::wrapForLua(lua_State *L) {
	using namespace luabind;

	module(L)[class_<ColourSelector, bases<Component>>("ColourSelector")
				  .def(constructor<>())
				  .def(constructor<int>())
				  .def(constructor<int, int>())
				  .def(constructor<int, int, int>())
				  .def("getCurrentColour", &ColourSelector::getCurrentColour)
				  .def("setCurrentColour", &ColourSelector::setCurrentColour)
				  .def("getNumSwatches", &ColourSelector::getNumSwatches)
				  .def("getSwatchColour", &ColourSelector::getSwatchColour)
				  .def("setSwatchColour", &ColourSelector::setSwatchColour)
				  .def("addChangeListener", &ChangeBroadcaster::addChangeListener)
				  .def("removeChangeListener", &ChangeBroadcaster::removeChangeListener)
				  .enum_("ColourSelectorOptions")[value("showAlphaChannel", ColourSelector::showAlphaChannel),
												  value("showColourAtTop", ColourSelector::showColourAtTop),
												  value("showSliders", ColourSelector::showSliders),
												  value("showColourspace", ColourSelector::showColourspace),
												  value("editableColour", ColourSelector::editableColour)]];
}