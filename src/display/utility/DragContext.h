// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/utility/Macros.h"
#include <string>

/* 
    1. DragContext must be global object accessible from every possible source that can do drag(e.g. place it in MainWindow and access from 
    browser or modengine, or stepsequencer, or whatever).
    2. use tagged union for data - pretty easy to extend later, but data must be pointers with life management relied on origin, 
    or placement new for complex structures.


    hints:
    For complicated data, that isn't good to copy - have to serialize it into some buffer, pass pointer and deserialize in target
*/

LVGL_OBJ_FWD;

namespace UI {

struct BaseWidget;

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

    DragContext(BaseWidget *parent);
    ~DragContext();

    void reset();
    void updateIconPos(int x, int y);
    void showIcon();
    void hideIcon();

    private:
    lv_obj_t * _dragIcon;
};


}