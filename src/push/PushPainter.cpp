// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushPainter.h"
#include "push/PushLib.h"

namespace PushLib {

Painter::Painter() {
    _displayCanvas = std::unique_ptr<Pixel>(new Pixel[DISPLAY_WIDTH*DISPLAY_HEIGHT]);
}

Painter::~Painter() {

}

void Painter::Rectangle(int x, int y, int w, int h, Color color) {
    if(x < 0 || (x+w) > DISPLAY_WIDTH) return;
    if(y < 0 || (y+h) > DISPLAY_HEIGHT) return;

    Pixel *ptr = _displayCanvas.get();
    int targetx = x+w;
    int targety = y+h;
    for(int xd=x; xd<targetx; ++xd) {
        for(int yd=y; yd<targety; ++yd) {
            ptr[(yd*DISPLAY_WIDTH)+xd] = color;
        }
    }
}

void Painter::EmptyRectangle(int x, int y, int w, int h, int thick, Color color) {
    if(x < 0 || (x+w) > DISPLAY_WIDTH) return;
    if(y < 0 || (y+h) > DISPLAY_HEIGHT) return;

    Pixel *ptr = _displayCanvas.get();
    int targetx = x+w;
    int targety = y+h;

    int tmp = y*DISPLAY_WIDTH;
    for(int xd=x; xd<targetx; ++xd) {
        ptr[xd+tmp] = color;
    }
    tmp = targety*DISPLAY_WIDTH;
    for(int xd=x; xd<targetx; ++xd) {
        ptr[xd+tmp] = color;
    }

    for(int yd=y; yd<targety; ++yd) {
        ptr[(yd*DISPLAY_WIDTH)+x] = color;
    }
    for(int yd=y; yd<targety; ++yd) {
        ptr[(yd*DISPLAY_WIDTH)+targetx] = color;
    }
    
    // for(int xd=x; xd<targetx; ++xd) {
    //     for(int yd=y; yd<targety; ++yd) {
    //         ptr[(yd*DISPLAY_WIDTH)+xd] = color;
    //     }
    // }
}

void Painter::fill(Color color) {
    Pixel * ptr = _displayCanvas.get();
    for(int x=0; x<DISPLAY_WIDTH; ++x) {
        for(int y=0; y<DISPLAY_HEIGHT; ++y) {
            ptr[(y*DISPLAY_WIDTH)+x] = color;
        }
    }
}

void Painter::clear() {
    //:D
}

void Painter::drawPixel(int x, int y, Color color) {
    if(x < 0 || x > DISPLAY_WIDTH) return;
    if(y < 0 || y > DISPLAY_HEIGHT) return;

    _displayCanvas.get()[(y*DISPLAY_WIDTH)+x] = color;
}

Pixel * Painter::canvas() {
    return _displayCanvas.get();
}



}