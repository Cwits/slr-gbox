// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/display/NewModulePopup.h"
#include "ui/display/layoutSizes.h"

#include "ui/display/primitives/Button.h"

#include "core/Events.h"
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
        // slr::Events::NewTrack e;
        slr::Events::CreateModule e = {
            .name = "Track"
        };
        slr::EmitEvent(e); 
    });

    _btnMixer = new Button(this, "Mixer");
    _btnMixer->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnMixer->setPos(300, 100);
    _btnMixer->setCallback([this]() {
        slr::Events::CreateModule e = {
            .name = "Mixer"
        };
        slr::EmitEvent(e);
    });
}

NewModulePopup::~NewModulePopup() {
    delete _btnTrack;
    delete _btnMixer;
}

void NewModulePopup::update() {

}


}