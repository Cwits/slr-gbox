// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/display/DragViewSelector.h"

#include "ui/display/primitives/DragContext.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/Label.h"

#include "ui/display/PopupManager.h"
#include "ui/display/layoutSizes.h"

#include "lvgl.h"
// #include "lvgl/misc/lv_timer.h"

#include "logger.h"

namespace UI {

struct Box2d {
    int x;
    int y;
    int w;
    int h;

    bool isPointWithin(int xp, int yp) const {
        if(xp >= x && xp <= (x+w) &&
            yp >= y && yp <= (y+h)) return true;

        return false;
    }
};

const Box2d gridZone = { .x = 20, .y = (980/2) - 100, .w = 200, .h = 200 };
const Box2d unitZone = { .x = 250, .y = (980/2) - 100, .w = 200, .h = 200 };
const Box2d modEngineZone = { .x = 500, .y = 20, .w = 200, .h = 200 };

DragViewSelector::DragViewSelector(BaseWidget *parent, UIContext * const uictx) :
    Popup(parent, uictx)
{
    
    setSize(LayoutDef::ROUTE_MANAGER_WIDTH, LayoutDef::ROUTE_MANAGER_HEIGHT);
    setPos(LayoutDef::ROUTE_MANAGER_X, LayoutDef::ROUTE_MANAGER_Y);
    setColor(lv_color_hex(0x858585));

    _flags.isDrag = true;

    _lastDragX = -1;
    _lastDragY = -1;

    // _timDrag = std::unique_ptr(lv_create_timer(&DragViewSelector::timerClb, 200, this), [](lv_obj_t *obj) {
    //     lv_obj_delete(obj);
    // });
    _timDrag = lv_timer_create(&DragViewSelector::timerClb, 2000, this);
    lv_timer_pause(_timDrag);
    lv_timer_set_auto_delete(_timDrag, false);

    _gridZoneRect = lv_obj_create(lvhost());
    lv_obj_set_size(_gridZoneRect, gridZone.w, gridZone.h);
    lv_obj_set_pos(_gridZoneRect, gridZone.x, gridZone.y);
    lv_obj_set_style_bg_color(_gridZoneRect, lv_color_make(255, 0, 0), 0);
    
    _unitZoneRect = lv_obj_create(lvhost());
    lv_obj_set_size(_unitZoneRect, unitZone.w, unitZone.h);
    lv_obj_set_pos(_unitZoneRect, unitZone.x, unitZone.y);
    lv_obj_set_style_bg_color(_unitZoneRect, lv_color_make(0, 255, 0), 0);

    _lblName = std::make_unique<Label>(this, "Select target");
    _lblName->setSize(200, 40);
    _lblName->setPos(LayoutDef::ROUTE_MANAGER_WIDTH, 20);
    _lblName->setFont(&DEFAULT_FONT);
    
    _lblGrid = std::make_unique<Label>(this, "Grid");
    _lblGrid->setSize(gridZone.w, 40);
    _lblGrid->setPos(gridZone.x + (gridZone.w/2) - 40, gridZone.y + (gridZone.h/2) - 10);
    _lblGrid->setFont(&DEFAULT_FONT);
    
    _lblUnit = std::make_unique<Label>(this, "Unit");
    _lblUnit->setSize(unitZone.w, 40);
    _lblUnit->setPos(unitZone.x + (unitZone.w/2) - 40, unitZone.y + (unitZone.h/2) - 10);
    _lblUnit->setFont(&DEFAULT_FONT);

}

DragViewSelector::~DragViewSelector() {
    // lv_obj_delete(_timDrag);
    lv_obj_delete(_gridZoneRect);
    lv_obj_delete(_unitZoneRect);
}

void DragViewSelector::update() {

}

void DragViewSelector::reset() {
    _lastDragX = -1;
    _lastDragY = -1;
    lv_timer_reset(_timDrag);
    lv_timer_pause(_timDrag);

}

bool DragViewSelector::handleDrag(GestLib::DragGesture & drag) {
    DragContext & ctx = *_uictx->dragContext();
    if(drag.state == GestLib::GestureState::Start) {

    } else if(drag.state == GestLib::GestureState::Move) {
        // LOG_INFO("Here");
        if(ctx.dragOnGoing) {
            ctx.updateIconPos(drag.x, drag.y);
            if(_lastDragX == -1 && _lastDragY == -1) {
                //first time
                lv_timer_resume(_timDrag);
            }
            //reset timer
            _lastDragX = drag.x;
            _lastDragY = drag.y;
            lv_timer_reset(_timDrag);
        }
    } else if(drag.state == GestLib::GestureState::End) {
        LOG_INFO("Drag End x: %d, y: %d", drag.x, drag.y);
        
        ctx.reset();
    }

    return true;
}

void DragViewSelector::timerClb(lv_timer_t * timer) {
    DragViewSelector * _this = static_cast<DragViewSelector*>(lv_timer_get_user_data(timer));
    if(_this->_lastDragX == -1 || _this->_lastDragY == -1) {
        //oops
        return;
    }

    int xpos = _this->_lastDragX - _this->getX();
    int ypos = _this->_lastDragY - _this->getY();

    if(gridZone.isPointWithin(xpos, ypos)) {
        //transfer gesture to grid
        _this->_uictx->transferGesture(_this->_uictx->grid(), GestLib::Gestures::Drag);
        _this->_uictx->switchToView(MainView::Grid);
    } else if(unitZone.isPointWithin(xpos, ypos)) {
        //transfer gesture to unit        
        _this->_uictx->transferGesture(_this->_uictx->unitView(), GestLib::Gestures::Drag);
        _this->_uictx->switchToView(MainView::Unit);
    } //...

    _this->reset();
    _this->_uictx->_popManager->disableDragSelector();
}


}