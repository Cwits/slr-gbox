// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/primitives/Canvas.h"

#include "push/PushPainter.h"

namespace PushUI {

Canvas::Canvas(PushLib::Widget *parent, int w, int h) :
    PushLib::Widget(parent)
{
    // color(PushLib::COLORS::White);
    _width = w;
    _height = h;
    markDirty();
    _canvas = std::unique_ptr<PushLib::Pixel>(new PushLib::Pixel[w*h]);
    update();
}

Canvas::~Canvas() {

}

void Canvas::paint(PushLib::Painter &painter) {
    PushLib::BoundingBox b = bounds();
    painter.drawBuffer(b, _canvas.get());
}

void Canvas::update() {
    PushLib::Pixel * cnv = _canvas.get();

    PushLib::Color gray = PushLib::rgb2(100, 100, 100); //?? why it is green lol :D
    for(int x=0; x<_width*_height; ++x) {
        cnv[x] = gray;
    }

    gray = PushLib::rgb2(255, 0, 0);
    int half = _height/2;
    for(int i=0; i<_width; ++i) { 
        int amp = half + ((half/2)*std::sin(2*M_PI*20*((float)i/(float)_width)));
        // painter.drawPixel(i, amp, PushLib::COLORS::Red);
        cnv[(amp*_width)+i] = gray;
    }
}

}