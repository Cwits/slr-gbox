// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/primitives/Label.h"
#include "push/PushPainter.h"

namespace PushUI {

Label::Label(PushLib::Widget *parent, std::string text) 
    : PushLib::Widget(parent),
    _text(text),
    _font(PushLib::Font_7x10)
{
    color(PushLib::COLORS::White);
    markDirty();
}

Label::~Label() {

}

void Label::paint(PushLib::Painter &painter) {
    int w = _text.size() * _font.FontWidth;
    if(_width < w) painter.writeStringLimited(_x, _y, _width, static_cast<std::string&>(_text), _font, _color);
    else painter.writeString(_x, _y, static_cast<std::string&>(_text), _font, _color);
}


}