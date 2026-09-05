// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/utility/DragContext.h"
#include "display/primitives/BaseWidget.h"

namespace UI {

DragContext::DragContext(BaseWidget *parent) {
    _dragIcon = lv_img_create(parent->lvhost());
    lv_image_set_src(_dragIcon, LV_SYMBOL_FILE);
    // lv_image_set_scale(_dragIcon, 240); //idk... this doesn't work
}

DragContext::~DragContext() {
    lv_obj_delete(_dragIcon);
}

void DragContext::reset() {
    hideIcon();
    origin = nullptr;
    startX = 0;
    startY = 0;
    payload.type = DragPayload::DataType::Error;
    dragOnGoing = false;
}

void DragContext::updateIconPos(int x, int y) {
    lv_obj_set_pos(_dragIcon, x, y);
}

void DragContext::showIcon() {
    lv_obj_move_to_index(_dragIcon, -1);
    lv_obj_clear_flag(_dragIcon, LV_OBJ_FLAG_HIDDEN);
}

void DragContext::hideIcon() {
    lv_obj_add_flag(_dragIcon, LV_OBJ_FLAG_HIDDEN);   
}


}