// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/GridView.h"

#include "ui/display/layoutSizes.h"
#include "ui/display/defaultStyles.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/DragContext.h"
#include "ui/display/primitives/FileView.h"
#include "ui/display/primitives/UnitUIBase.h"

#include "core/Events.h"
#include "snapshots/FileContainerView.h"

#include "logger.h"

#include <cmath>

namespace UI {

GridControl::GridControl(GridView * parent, UIContext * const uictx) 
    : BaseWidget(parent, true),
    _grid(parent),
    _uictx(uictx)
{
    setColor(lv_palette_main(LV_PALETTE_CYAN));
    setPos(LayoutDef::TRACK_CONTROL_PANEL_X, LayoutDef::TRACK_CONTROL_PANEL_Y);
    setSize(LayoutDef::TRACK_CONTROL_PANEL_WIDTH, LayoutDef::TRACK_CONTROL_PANEL_HEIGHT);
    
    _flags.isTap = true;

    lv_obj_add_style(_lvhost, &workspace, 0);
    lv_obj_set_scrollbar_mode(_lvhost, LV_SCROLLBAR_MODE_OFF);

    _lastSelectedRect = lv_obj_create(_lvhost);
    lv_obj_add_flag(_lastSelectedRect, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(_lastSelectedRect, LayoutDef::TRACK_CONTROL_PANEL_WIDTH, LayoutDef::TRACK_HEIGHT);
    lv_obj_set_style_bg_opa(_lastSelectedRect, LV_OPA_0, 0);
    
    show();
}

GridControl::~GridControl() {
    lv_obj_delete(_lastSelectedRect);
}

bool GridControl::handleTap(GestLib::TapGesture &tap) {
    int notAbsY = tap.y - LayoutDef::TOP_PANEL_HEIGHT;
    UnitUIBase * u = nullptr;
    const std::vector<UnitUIBase*> &list = _uictx->_unitsUI;
    for(std::size_t i=0; i<list.size(); ++i) {
        UnitUIBase * unit = list.at(i);
        int cy = unit->gridY();
        if(notAbsY >= cy && notAbsY <= (cy+LayoutDef::TRACK_HEIGHT)) {
            u = unit;
            break;
        }
    }

    if(u) {
        _uictx->setLastSelected(u);
        lv_obj_set_pos(_lastSelectedRect, 0, u->gridY());
        lv_obj_clear_flag(_lastSelectedRect, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_to_index(_lastSelectedRect, -1);
        return true;
    } else {
        _uictx->setLastSelected(nullptr);
        return true;
    }
}

GridGrid::GridGrid(GridView * parent, UIContext * const uictx) 
    : BaseWidget(parent, true), 
    _grid(parent),
    _uictx(uictx)
{
    setColor(lv_palette_main(LV_PALETTE_AMBER));
    setPos(LayoutDef::GRID_X, LayoutDef::GRID_Y);
    setSize(LayoutDef::GRID_WIDTH, LayoutDef::GRID_HEIGHT);
    lv_obj_set_scrollbar_mode(_lvhost, LV_SCROLLBAR_MODE_OFF);
    
    lv_obj_add_style(_lvhost, &workspace, 0);
    lv_obj_set_scrollbar_mode(_lvhost, LV_SCROLLBAR_MODE_OFF);
    _flags.isDrag = true;

    // lv_obj_set_style_bg_color(lvhost(), )
    // lv_obj_set_style_bg_opa(lvhost(), LV_OPA_100, 0);

    show();
}

GridGrid::~GridGrid() {

}

bool GridGrid::handleDrag(GestLib::DragGesture & drag) {
    if(drag.state == GestLib::GestureState::Start) {

    } else if(drag.state == GestLib::GestureState::Move) {
        
    } else if(drag.state == GestLib::GestureState::End) {
        LOG_INFO("Drag End x: %d, y: %d", drag.x, drag.y);
        DragContext & ctx = _uictx->_dragContext;
        
        if(ctx.dragOnGoing && ctx.origin != nullptr) {
            ctx.dragOnGoing = false;
            if(ctx.payload.type != DragPayload::DataType::FilePath) {
                return true;
            }
            
            //try find appropriate track
            const std::vector<UnitUIBase*> &list = _uictx->_unitsUI;
            int notAbsY = drag.y - (LayoutDef::TOP_PANEL_HEIGHT + LayoutDef::TIMELINE_HEIGHT);
            for(auto & unit : list) {
                if(notAbsY >= unit->gridY() && notAbsY <= (unit->gridY()+LayoutDef::TRACK_HEIGHT) && unit->canLoadFiles()) {
                    LOG_INFO("Loading file %s to unitId: %d", ctx.payload.filePath.path->c_str(), unit->id());
                    slr::Events::OpenFile e = {
                        .targetId = unit->id(),
                        .path = *ctx.payload.filePath.path
                    };
                    slr::EmitEvent(e);
                }
            }
            ctx.reset();
        }
    }

    return true;
}


//1. надо ли мне что бы все треки добавлялись как children к gridview? - 

GridView::GridView(BaseWidget * parent, UIContext * uictx) : View(parent, uictx) {
    setPos(LayoutDef::WORKSPACE_POSITION_X, LayoutDef::WORKSPACE_POSITION_Y);
    setSize(LayoutDef::WORKSPACE_WIDTH, LayoutDef::WORKSPACE_HEIGHT); 
    lv_obj_set_scrollbar_mode(_lvhost, LV_SCROLLBAR_MODE_OFF);
    
    _control = new GridControl(this, uictx);
    _grid = new GridGrid(this, uictx);
    _timeline = new Timeline(_grid);
    
    _flags.isSwipe = true;
    show();
}

GridView::~GridView() {
    //have to call explicitly before deleting _control and _grid

    delete _timeline;
    delete _control;
    delete _grid;
}

bool GridView::handleSwipe(GestLib::SwipeGesture & swipe) {
    
    if(swipe.state == GestLib::GestureState::Start) {
        // LOG_INFO("Swipe start from grid view x: %d, y: %d, dx: %d, dy: %d", 
            // swipe.x, swipe.y, swipe.dx, swipe.dy);
    } else if(swipe.state == GestLib::GestureState::Move) {
        // LOG_INFO("Swipe move from grid view x: %d, y: %d, dx: %d, dy: %d", 
            // swipe.x, swipe.y, swipe.dx, swipe.dy);

        if(std::abs(swipe.dx) > 4) {
            //horizontal move

            //make grid left-right scroll
            int mul = 160 * swipe.dx;
            slr::frame_t oldNudge = _timeline->nudge();

            if(swipe.dx > 0) {
                //move from left to right - decrease start position
                if(oldNudge > 0) {
                    slr::frame_t newNudge = 0;
                    int64_t diff = oldNudge - mul;
                    if(diff >= 0) {
                        newNudge = oldNudge - mul;
                    } else {
                        newNudge = 0;
                    }

                    _timeline->setNudge(newNudge);

                    const std::vector<UnitUIBase*> &list = _uictx->_unitsUI;
                    for(auto * base : list) {
                        base->setNudge(newNudge, _horizontalZoom);
                    }
                }
            } else if(swipe.dx < 0) {
                //move from right to left - increase start position
                slr::frame_t newNudge = oldNudge + std::abs(mul);
                _timeline->setNudge(newNudge);
                
                const std::vector<UnitUIBase*> &list = _uictx->_unitsUI;
                for(auto * base : list) {
                    base->setNudge(newNudge, _horizontalZoom);
                }
            }

        } else if(std::abs(swipe.dy) > 4) {
            //vertical move
            int mul = swipe.dy;
            //make grid(tracks) up-down scroll
            const std::vector<UnitUIBase*> &list = _uictx->_unitsUI; 
            int tmp = LayoutDef::GRID_HEIGHT-LayoutDef::TRACK_HEIGHT;
            if(list.size()*LayoutDef::TRACK_HEIGHT < tmp) return true;


            if(swipe.dy > 0) {
                //from top to bottom -> scroll down
                int tmpgrid = list.at(0)->gridY();
                int tmpcalc = LayoutDef::calcTrackY(0);
                // LOG_INFO("%d %d", tmpgrid, tmpcalc);
                if(tmpgrid < tmpcalc) {
                    int diff = list.at(0)->gridY() - mul;
                    if(diff <= 0) {
                        // mul += diff;
                    }

                    const std::vector<UnitUIBase*> &list = _uictx->_unitsUI;
                    for(auto * base : list) {
                        base->updatePosition(0, base->gridY()+mul);
                    }
                    int cy = lv_obj_get_y(_control->_lastSelectedRect);
                    lv_obj_set_y(_control->_lastSelectedRect, cy+mul);
                }
            } else if(swipe.dy < 0) {
                //from bottom to top -> scroll up
                const std::vector<UnitUIBase*> &list = _uictx->_unitsUI;
                if(list.back()->gridY() < (LayoutDef::GRID_HEIGHT-LayoutDef::TRACK_HEIGHT)) return true;
                
                for(auto * base : list) {
                    base->updatePosition(0, base->gridY()+mul);
                }
                int cy = lv_obj_get_y(_control->_lastSelectedRect);
                lv_obj_set_y(_control->_lastSelectedRect, cy+mul);
            }
        }
    } else if(swipe.state == GestLib::GestureState::End) {
        // LOG_INFO("Swipe end from grid view  x: %d, y: %d, dx: %d, dy: %d", 
            // swipe.x, swipe.y, swipe.dx, swipe.dy);
    }



    return true;
}

}