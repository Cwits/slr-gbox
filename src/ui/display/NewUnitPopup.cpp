// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/display/NewUnitPopup.h"
#include "ui/display/layoutSizes.h"

#include "ui/display/primitives/Button.h"

#include "core/Actions.h"
#include "logger.h" 

namespace UI {

NewUnitPopup::NewUnitPopup(BaseWidget *parent, UIContext * const uictx) :
    Popup(parent, uictx)
{
    setSize(Layout::ROUTE_MANAGER_WIDTH, Layout::ROUTE_MANAGER_HEIGHT);
    setPos(Layout::ROUTE_MANAGER_X, Layout::ROUTE_MANAGER_Y);
    setColor(lv_color_hex(0x858585));

    _btnTrack = std::make_unique<Button>(this, "Track");
    _btnTrack->setSize(Layout::Button, Layout::Button);
    _btnTrack->setPos(100, 100);
    _btnTrack->setCallback([this]() {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "Track";
        slr::EmitAction(std::move(action));
    });

    _btnMixer = std::make_unique<Button>(this, "Mixer");
    _btnMixer->setSize(Layout::Button, Layout::Button);
    _btnMixer->setPos(300, 100);
    _btnMixer->setCallback([this]() {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "Mixer";
        slr::EmitAction(std::move(action));
    });

    _btnOsc = std::make_unique<Button>(this, "SimpleOSC");
    _btnOsc->setSize(Layout::Button, Layout::Button);
    _btnOsc->setPos(500, 100);
    _btnOsc->setCallback([this]() {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "SimpleOSC";
        slr::EmitAction(std::move(action));
    });

    _btnSampler = std::make_unique<Button>(this, "Sampler");
    _btnSampler->setSize(Layout::Button, Layout::Button);
    _btnSampler->setPos(100, 200);
    _btnSampler->setCallback([this]() {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "Sampler";
        slr::EmitAction(std::move(action));
    });
}

NewUnitPopup::~NewUnitPopup() {
}

void NewUnitPopup::update() {

}


}