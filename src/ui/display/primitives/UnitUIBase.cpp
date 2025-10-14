// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/RouteManager.h"
#include "ui/display/primitives/UIContext.h"

#include "snapshots/AudioUnitView.h"
#include "core/Events.h"

#include "logger.h"

namespace UI {

UnitUIBase::UnitUIBase(slr::AudioUnitView * view) : _view(view)
{

}

UnitUIBase::~UnitUIBase() {

}

const slr::ID UnitUIBase::id() const {
    return _view->id();
}

const slr::Color & UnitUIBase::color() const {
    return _view->color();
}


UnitControlPopup::UnitControlPopup(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx) {
    setSize(300, 300);
    setPos(LayoutDef::TRACK_CONTROL_PANEL_WIDTH-100, 150);

    _deleteBtn = new Button(this, LV_SYMBOL_TRASH);
    _deleteBtn->setPos(10, 10);
    _deleteBtn->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _deleteBtn->setFont(&DEFAULT_FONT);
    _deleteBtn->setCallback([this]() {
        // std::cout << "Delete track: " << (int)_track->id() << std::endl;
        LOG_INFO("Delete track: %i", _currentUnit->id());
        slr::Events::DeleteTrack e = {
            .targetId = _currentUnit->id()
        };
        slr::EmitEvent(e);

        this->_uictx->setLastSelected(nullptr);
        this->_uictx->_popManager->disableUnitControl();
    });

    _routeManagerBtn = new Button(this, "Routes");
    _routeManagerBtn->setPos(LayoutDef::BUTTON_SIZE+20, 10);
    _routeManagerBtn->setFont(&DEFAULT_FONT);
    _routeManagerBtn->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _routeManagerBtn->setCallback([this]() {
        // LOG_INFO("Call the manager!!! track %i", _currentTrack->id());
        this->_uictx->_popManager->disableUnitControl();
        this->_uictx->_popManager->enableRouteManager(this->_currentUnit->id());
    });
}

UnitControlPopup::~UnitControlPopup() {
    delete _deleteBtn;
    delete _routeManagerBtn;
}


}