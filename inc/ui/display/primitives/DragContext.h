// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <string>

/* 
    1. DragContext must be global object accessible from every possible source that can do drag(e.g. place it in MainWindow and access from 
    browser or modengine, or stepsequencer, or whatever).
    2. use tagged union for data - pretty easy to extend later, but data must be pointers with life management relied on origin, 
    or placement new for complex structures.


    hints:
    Для очень сложных или объемных данных, которые нецелесообразно копировать, 
    можно сериализовать их в буфер, передать указатель на этот буфер, а цель десериализует.
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
    DragContext() = default;
    void reset() {
        origin = nullptr;
        startX = 0;
        startY = 0;
        payload.type = DragPayload::DataType::Error;
        dragOnGoing = false;
    }
};


}