// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/Widget.h"
#include <algorithm>

namespace PushLib {

Widget::Widget(Widget * parent, PushContext * const pctx) :
    _parent(parent),
    _pctx(pctx)
{

}

Widget::~Widget() {

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

}