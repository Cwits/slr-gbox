// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Color.h"
#include "push/PushLib.h"
#include "push/PushPainter.h"
#include "push/BoundingBox.h"

#include <vector>
#include <map>

namespace PushLib {

class Painter;

struct Vec2 {
    Vec2(int x, int y) : _x(x), _y(y) {}
    int _x;
    int _y;
};

struct Widget {
    explicit Widget(Widget * parent = nullptr);
    virtual ~Widget();

    void addChild(Widget * child);
    void removeChild(const Widget *child);

    std::vector<Widget*> & childs() { return _childrens; }
    void sortChildsByZ();

    bool dirty() const { return _dirty; }
    void markDirty() { _dirty = true; }
    void clearDirty() { _dirty = false; }
    void markAllDirty();

    bool visible() const { return _visible; }
    void show() { _visible = true; markDirty(); }
    void hide() { _visible = false; markDirty(); }

    virtual void x(int x) { _lastBounds = bounds(); _x = x; markDirty(); }
    virtual int x() const { return _x; }

    virtual void y(int y) { _lastBounds = bounds(); _y = y; markDirty(); }
    virtual int y() const { return _y; }

    virtual void z(int z) { _z = z; markDirty(); _needZSorting = true; }
    virtual int z() const { return _z; }

    virtual void position(int x, int y) { _lastBounds = bounds(); _x = x; _y = y; markDirty(); }
    virtual Vec2 position() const { return Vec2(_x, _y); }

    virtual void width(int width) { _lastBounds = bounds(); _width = width; markDirty(); }
    virtual int width() const { return _width; }

    virtual void height(int height) { _lastBounds = bounds(); _height = height; markDirty(); }
    virtual int height() const { return _height; }

    virtual void size(int width, int height) { _lastBounds = bounds(); _width = width; _height = height; markDirty(); }
    virtual Vec2 size() const { return Vec2(_width, _height); }

    virtual void color(Color color) { _color = color; markDirty(); }
    virtual Color color() const { return _color; }
    
    virtual BoundingBox bounds() { return BoundingBox(_x, _y, _width, _height); }
    BoundingBox lastBounds() const { return _lastBounds; }
    void updateBounds() { _lastBounds = bounds(); }

    virtual void paint(Painter & painter) = 0;
    void paintChilds(Painter &painter);
    //in order to mark that redraw is necessary - return true?
    virtual bool handleButton(PushLib::ButtonEvent &ev) { return false; }
    virtual bool handleEncoder(PushLib::EncoderEvent &ev) { return false; }

    virtual std::vector<PushLib::ButtonColor> buttonsColors() { return std::vector<PushLib::ButtonColor>(); }

    protected:
    Widget * _parent;
    std::vector<Widget*> _childrens;
    bool _needZSorting;

    BoundingBox _lastBounds;

    bool _dirty;
    bool _visible;

    int _x;
    int _y;

    int _width;
    int _height;

    int _z;

    Color _color;
};

}