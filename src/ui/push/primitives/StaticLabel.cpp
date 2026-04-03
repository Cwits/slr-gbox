// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/primitives/StaticLabel.h"

#include "push/PushPainter.h"

namespace PushUI {

StaticLabel::StaticLabel(PushLib::Widget *parent, const std::string_view &text) :
    PushLib::Widget(parent),
    _text(text),
    _font(PushLib::Font_7x10)
{
    color(PushLib::Colors::White);
    recalculateSize();
    markDirty();
}

StaticLabel::~StaticLabel() {

}

void StaticLabel::paint(PushLib::Painter &painter) {
    int w = _text.size() * _font.FontWidth;
    if(_width < w) painter.writeStringLimited(_x, _y, _width, _height, _width, _text, _font, _color);
    else painter.writeString(_x, _y, _width, _height, _text, _font, _color);
}

void StaticLabel::font(PushLib::Font font) { 
    _font = font; 
    recalculateSize();
    markDirty();
}

void StaticLabel::recalculateSize() {
    _height = _font.FontHeight;
    _width = _font.FontWidth * _text.size();
}

}