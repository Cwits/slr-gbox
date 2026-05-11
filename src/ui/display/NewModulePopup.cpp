// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/display/NewModulePopup.h"
#include "ui/display/layoutSizes.h"

#include "ui/display/primitives/Button.h"

#include "core/Actions.h"
#include "logger.h"

namespace UI {

NewModulePopup::NewModulePopup(BaseWidget *parent, UIContext * const uictx) :
    Popup(parent, uictx)
{
    setSize(LayoutDef::ROUTE_MANAGER_WIDTH, LayoutDef::ROUTE_MANAGER_HEIGHT);
    setPos(LayoutDef::ROUTE_MANAGER_X, LayoutDef::ROUTE_MANAGER_Y);
    setColor(lv_color_hex(0x858585));

    _btnTrack = new Button(this, "Track");
    _btnTrack->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnTrack->setPos(100, 100);
    _btnTrack->setCallback([this]() {
        LOG_INFO("Create New Track Event");
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "Track";
        slr::EmitAction(std::move(action));
    });

    _btnMixer = new Button(this, "Mixer");
    _btnMixer->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnMixer->setPos(300, 100);
    _btnMixer->setCallback([this]() {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "Mixer";
        slr::EmitAction(std::move(action));
    });

    _btnOsc = new Button(this, "OSC");
    _btnOsc->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnOsc->setPos(500, 100);
    _btnOsc->setCallback([this]() {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "OSC";
        slr::EmitAction(std::move(action));
    });

    _btnSampler = new Button(this, "Sampler");
    _btnSampler->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnSampler->setPos(100, 200);
    _btnSampler->setCallback([this]() {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "Sampler";
        slr::EmitAction(std::move(action));
    });
}

NewModulePopup::~NewModulePopup() {
    delete _btnTrack;
    delete _btnMixer;
    delete _btnOsc;
    delete _btnSampler;
}

void NewModulePopup::update() {

}


}