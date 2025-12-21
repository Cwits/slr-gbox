// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <string>

#include "ui/display/primitives/BaseWidget.h"
#include "lvgl.h"

/* 
    1. DragContext must be global object accessible from every possible source that can do drag(e.g. place it in MainWindow and access from 
    browser or modengine, or stepsequencer, or whatever).
    2. use tagged union for data - pretty easy to extend later, but data must be pointers with life management relied on origin, 
    or placement new for complex structures.


    hints:
    For complicated data, that isn't good to copy - have to serialize it into some buffer, pass pointer and deserialize in target
*/

namespace UI {
class BaseWidget;

struct FilePath {
    std::string * path;
};

struct DragPayload {
    enum class DataType {
        Error,
        FilePath,
        AudioItem,
        MidiItem
    };

    DataType type;
    union {
        FilePath filePath;
    };
};

struct DragContext {
    BaseWidget * origin;
    int startX;
    int startY;
    bool dragOnGoing;

    DragPayload payload;
    void reset() {
        hideIcon();
        origin = nullptr;
        startX = 0;
        startY = 0;
        payload.type = DragPayload::DataType::Error;
        dragOnGoing = false;
    }

    void updateIconPos(int x, int y) {
        lv_obj_set_pos(_dragIcon, x, y);
    }

    void showIcon() {
        lv_obj_move_to_index(_dragIcon, -1);
        lv_obj_clear_flag(_dragIcon, LV_OBJ_FLAG_HIDDEN);
    }

    void hideIcon() {
        lv_obj_add_flag(_dragIcon, LV_OBJ_FLAG_HIDDEN);   
    }

    DragContext(BaseWidget *parent) {
        _dragIcon = lv_img_create(parent->lvhost());
        lv_image_set_src(_dragIcon, LV_SYMBOL_FILE);
        // lv_image_set_scale(_dragIcon, 240); //idk... this doesn't work
    }

    ~DragContext() {
        lv_obj_delete(_dragIcon);
    }

    private:
    lv_obj_t * _dragIcon;
};


}