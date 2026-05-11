// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/primitives/Pointer.h"
#include "push/PushFonts.h"

#include "push/PushPainter.h"

namespace PushUI {

Pointer::Pointer(PushLib::Widget *parent) :
    PushLib::Widget(parent)
{
    _font = PushLib::Font_7x10;
    _newFont = PushLib::Font_7x10;
    _color = PushLib::Colors::White;
    _x = 0;
    _y = 0;
    markDirty();
}

Pointer::~Pointer() {

}

void Pointer::paint(PushLib::Painter &painter) {
    painter.writeChar(_x, _y, '>', _font, _color);
}

}