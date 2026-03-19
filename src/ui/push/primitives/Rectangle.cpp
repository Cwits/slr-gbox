// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/primitives/Rectangle.h"

#include "push/PushPainter.h"

namespace PushUI {

Rectangle::Rectangle(PushLib::Widget *parent, bool filled) :
    PushLib::Widget(parent), _filled(filled)
{
}

Rectangle::~Rectangle() {

}

void Rectangle::paint(PushLib::Painter &painter) {
    // painter.filledRectangle(_x, _y, _width, _height, PushLib::COLORS::Blue);
    if(_filled) painter.filledRectangle(_x, _y, _width, _height, _color);
    else painter.rectangle(_x, _y, _width, _height, 1, _color);
}


}