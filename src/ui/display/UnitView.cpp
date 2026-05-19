// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/UnitView.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultStyles.h"

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/display/primitives/UIContext.h"

#include "snapshots/AudioUnitView.h"
#include "logger.h"

namespace UI { 

UnitView::UnitView(BaseWidget* parent, UIContext * const uictx) : View(parent, uictx) {
    setPos(LayoutDef::WORKSPACE_POSITION_X, LayoutDef::WORKSPACE_POSITION_Y);
    setSize(LayoutDef::WORKSPACE_WIDTH, LayoutDef::WORKSPACE_HEIGHT);
    lv_obj_add_style(_lvhost, &workspace, 0);
    lv_obj_set_style_bg_color(_lvhost, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);

    _lb = lv_label_create(_lvhost);
    lv_label_set_text(_lb, "Module");
    lv_obj_center(_lb);
    _lastShownModule = nullptr;

    _flags.isDrag = true;
}

UnitView::~UnitView() {
    lv_obj_delete(_lb);
}

void UnitView::update() {
    hideAllChilds();
    
    UnitUIBase * unit = _uictx->getLastSelected();

    if(!unit) {
        lv_label_set_text(_lb, "Module not selected");
        lv_obj_clear_flag(_lb, LV_OBJ_FLAG_HIDDEN);
        _lastShownModule = nullptr;
    } else {
        lv_obj_add_flag(_lb, LV_OBJ_FLAG_HIDDEN);
        unit->unitUI()->show();
        _lastShownModule = unit;
    }
}

void UnitView::pollUIUpdate() {
    if(_lastShownModule)
        _lastShownModule->unitUI()->pollUIUpdate();
}

bool UnitView::handleDrag(GestLib::DragGesture & drag) {
    if(_lastShownModule) {
        if(_lastShownModule->unitUI()->canHandleGesture(GestLib::Gestures::Drag)) {
            GestLib::Gesture g;
            g.type = GestLib::Gestures::Drag;
            g.drag = drag;
            return _lastShownModule->unitUI()->handleGesture(g);
        }
    }

    DragContext & ctx = *_uictx->dragContext();
    if(drag.state == GestLib::GestureState::Start) {

    } else if(drag.state == GestLib::GestureState::Move) {
        if(ctx.dragOnGoing) {
            ctx.updateIconPos(drag.x, drag.y);
        }
    } else if(drag.state == GestLib::GestureState::End) {
        LOG_INFO("Gesture ended in unit view");
        if(1) {
        // if(_lastShownModule->canLoadAsAsset(ctx.))
        /* 
            asset can be various types of things?
            preset
            audio file
            script?
            whatever?

        */
        } else if(0) {
            /* 
            tryload as clip for timeline
            */
        }

        
        ctx.reset();
    }

    return true;
}

}