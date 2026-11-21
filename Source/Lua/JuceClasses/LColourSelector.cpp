// LColourSelector.cpp
#include "stdafx.h"
#include "LColourSelector.h"
#include "luabind/luabind.hpp"

namespace
{
    class ColourPickerPopupContent : public Component,
                                     private ChangeListener
    {
    public:
        ColourPickerPopupContent(Component* anchorComp,
                                 Component* targetLabelComp,
                                 Component* repaintTargetComp,
                                 Colour initialColour,
                                 int flags,
                                 bool liveUpdate,
                                 luabind::object cb)
            : targetLabel(targetLabelComp),
              repaintTarget(repaintTargetComp),
              liveUpdateOnDrag(liveUpdate),
              callback(cb)
        {
            selector.setCurrentColour(initialColour, dontSendNotification);
            selector.addChangeListener(this);
            addAndMakeVisible(selector);

            setSize(300, 380);

            Rectangle<int> targetArea = anchorComp != nullptr
                ? anchorComp->getScreenBounds()
                : Rectangle<int>(0, 0, 1, 1);

            Component* parentForBox = anchorComp != nullptr
                ? anchorComp->getTopLevelComponent()
                : nullptr;

            CallOutBox::launchAsynchronously(
                std::unique_ptr<Component>(this),
                targetArea,
                parentForBox);
        }

        ~ColourPickerPopupContent() override
        {
            selector.removeChangeListener(this);
            writeColourToTarget();
        }

        void resized() override
        {
            selector.setBounds(getLocalBounds());
        }

    private:
        void changeListenerCallback(ChangeBroadcaster* source) override
        {
            if (source == &selector && liveUpdateOnDrag)
                writeColourToTarget();
        }

        void writeColourToTarget()
        {
            Colour col = selector.getCurrentColour();
            String hexStr = col.toString();

            if (targetLabel != nullptr)
            {
                Label* lbl = dynamic_cast<Label*>(targetLabel.getComponent());

                if (lbl != nullptr)
                    lbl->setText(hexStr, dontSendNotification);
                else
                    targetLabel->setName(hexStr);
            }

            if (repaintTarget != nullptr)
                repaintTarget->repaint();

            if (callback.is_valid() && luabind::type(callback) == LUA_TFUNCTION)
            {
                try { callback(hexStr.toStdString()); }
                catch (...) {}
            }
        }

        ColourSelector selector;
        Component::SafePointer<Component> targetLabel;
        Component::SafePointer<Component> repaintTarget;
        bool liveUpdateOnDrag = false;
        luabind::object callback;
    };   // <-- class ends here

    // Overload 1: takes a Colour
    void showColourPickerPopup(Component* anchorComp,
                               Component* targetLabelComp,
                               Component* repaintTargetComp,
                               const Colour& initialColour,
                               bool liveUpdate)
    {
        if (MessageManager::getInstance() != nullptr)
        {
            const int flags = ColourSelector::showAlphaChannel
                            | ColourSelector::showColourAtTop
                            | ColourSelector::showSliders
                            | ColourSelector::showColourspace;

            new ColourPickerPopupContent(anchorComp, targetLabelComp, repaintTargetComp,
                                         initialColour, flags, liveUpdate,
                                         luabind::object());
        }
    }

    // Overload 2: takes a JUCE String
    void showColourPickerPopup(Component* anchorComp,
                               Component* targetLabelComp,
                               Component* repaintTargetComp,
                               const String& initialHexColour,
                               bool liveUpdate)
    {
        const String trimmed = initialHexColour.trim();
        Colour parsed = trimmed.isEmpty()
            ? Colour(0xffff0000)
            : Colour::fromString(trimmed);

        showColourPickerPopup(anchorComp, targetLabelComp, repaintTargetComp, parsed, liveUpdate);
    }

    // Callback version
    void showColourPickerPopupCb(Component* anchorComp,
                                 Component* targetLabelComp,
                                 Component* repaintTargetComp,
                                 const Colour& initialColour,
                                 bool liveUpdate,
                                 luabind::object cb)
    {
        if (MessageManager::getInstance() != nullptr)
        {
            new ColourPickerPopupContent(anchorComp, targetLabelComp, repaintTargetComp,
                                         initialColour, 0, liveUpdate, cb);
        }
    }

    using ShowPopupColourFn = void(*)(Component*, Component*, Component*, const Colour&, bool);
    using ShowPopupStringFn = void(*)(Component*, Component*, Component*, const String&, bool);
}   // <-- namespace ends here

void LColourSelector::wrapForLua (lua_State *L)
{
    using namespace luabind;

    module(L)
    [
        class_<ColourSelector, bases<Component> >("ColourSelector")
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
            .enum_("ColourSelectorOptions")
            [
                value("showAlphaChannel", ColourSelector::showAlphaChannel),
                value("showColourAtTop", ColourSelector::showColourAtTop),
                value("showSliders", ColourSelector::showSliders),
                value("showColourspace", ColourSelector::showColourspace),
                value("editableColour", ColourSelector::editableColour)
            ],

        def("showColourPickerPopup", static_cast<ShowPopupColourFn>(&showColourPickerPopup)),
        def("showColourPickerPopup", static_cast<ShowPopupStringFn>(&showColourPickerPopup)),
        def("showColourPickerPopupCb", &showColourPickerPopupCb)
    ];
}