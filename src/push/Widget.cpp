// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/Widget.h"
#include <algorithm>

namespace PushLib {


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


Widget::Widget(Widget * parent) :
    _parent(parent),
    _needZSorting(true),
    _lastBounds(BoundingBox()),
    _dirty(true), 
    _visible(true),
    _x(0), 
    _y(0), 
    _width(0), 
    _height(0), 
    _z(0),
    _color(PushLib::Colors::White)
{
    if(_parent)
        _parent->addChild(this);
}

Widget::~Widget() {
    if(_parent) _parent->removeChild(this);
}

void Widget::addChild(Widget * child) {
    _childrens.push_back(child);
}

void Widget::removeChild(const Widget *child) {
    if(_childrens.size() == 0) return;

    _childrens.erase(
        std::remove_if(_childrens.begin(), _childrens.end(),
            [child](const Widget *w) {
                return w == child;
            }), _childrens.end()
    );
}

void Widget::sortChildsByZ() {
    if(!_needZSorting) return;
    _needZSorting = false;

     std::sort(_childrens.begin(), _childrens.end(),
        [](Widget* a, Widget* b) { return a->z() < b->z(); });
}

void Widget::markAllDirty() {
    markDirty();
    std::for_each(_childrens.begin(), _childrens.end(), [](Widget *ch) {
        ch->markAllDirty();
    });
}

bool Widget::hasAnythingDirty() const {
    bool ret = false;
    ret |= dirty();
    for(auto *c : _childrens) {
        if(ret) break;
        ret |= c->hasAnythingDirty();
    } 
    return ret;
}

// void Widget::paint(Painter &painter) {
//     // if(!isDirty()) return;
    
//     for(auto &ch : _childrens) {
//         if(!ch->isDirty()) continue;

//         ch->paint(painter);
//     }
// }

void Widget::paintChilds(Painter &painter) {
    for(auto &ch : _childrens) {
        if(!ch->dirty()) continue;
        ch->paint(painter);
    }
}

}