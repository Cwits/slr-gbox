// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Color.h"
#include "push/PushLib.h"
#include "push/PushPainter.h"
#include "push/PushContext.h"

#include <vector>

namespace PushLib {

struct Widget {
    Widget(Widget * parent, PushContext * const pctx);
    virtual ~Widget();

    void addChild(Widget * child);
    void removeChild(const Widget *child);

    std::vector<Widget*> & childs() { return _childrens; }
    
    void markDirty() { _dirty = false; }
    bool isDirty() const { return _dirty; }

    virtual BoundingBox invalidate() = 0;
    virtual void paint(Painter & painter) = 0;

    //in order to mark that redraw is necessary - return true?
    virtual bool handleButton(PushLib::ButtonEvent &ev) = 0;
    virtual bool handleEncoder(PushLib::EncoderEvent &ev) = 0;

    protected:
    Widget * _parent;
    PushContext * const _pctx;
    std::vector<Widget*> _childrens;
    bool _dirty;

    int _x;
    int _y;

    int _width; //bounding box size...
    int _height;//bounding box size...

    Color _color;
};

}