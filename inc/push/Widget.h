// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Color.h"
#include "push/PushLib.h"

#include <vector>

namespace PushLib {

class Display;
class Painter;

struct Widget {
    Widget(Widget * parent);
    virtual ~Widget();

    void addChild(Widget * child) {
        _childrens.push_back(child);
    }
    void removeChild(Widget *child);

    std::vector<Widget*> & childs() { return _childrens; }
    
    void markDirty() { _dirty = false; }
    bool isDirty() const { return _dirty; }

    virtual void paint(Painter & painter) = 0;

    //in order to mark that redraw is necessary - return true?
    virtual bool handleButton(PushLib::ButtonEvent &ev) = 0;

    public:
    std::vector<Widget*> _childrens;
    bool _dirty;

    int _x;
    int _y;

    int _width; //bounding box size...
    int _height;//bounding box size...

    Color _color;
};

}