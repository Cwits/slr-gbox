// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/Widget.h"

namespace PushLib {

Widget::Widget(Widget * parent, PushContext * const pctx) :
    _pctx(pctx)
{

}

Widget::~Widget() {

}

void Widget::removeChild(Widget *child) {

}


}