// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/ModuleView.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultStyles.h"

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/display/primitives/UIContext.h"

#include "snapshots/AudioUnitView.h"

namespace UI { 

ModuleView::ModuleView(BaseWidget* parent, UIContext * const uictx) : View(parent, uictx) {
    setPos(LayoutDef::WORKSPACE_POSITION_X, LayoutDef::WORKSPACE_POSITION_Y);
    setSize(LayoutDef::WORKSPACE_WIDTH, LayoutDef::WORKSPACE_HEIGHT);
    lv_obj_add_style(_lvhost, &workspace, 0);
    lv_obj_set_style_bg_color(_lvhost, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);

    _lb = lv_label_create(_lvhost);
    lv_label_set_text(_lb, "Module");
    lv_obj_center(_lb);
}

ModuleView::~ModuleView() {
    lv_obj_delete(_lb);
}

void ModuleView::update() {
    hideAllChilds();
    
    UnitUIBase * unit = _uictx->getLastSelected();

    if(!unit) {
        lv_label_set_text(_lb, "Module not selected");
        lv_obj_clear_flag(_lb, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(_lb, LV_OBJ_FLAG_HIDDEN);
        unit->moduleUI()->show();
    }
}


}