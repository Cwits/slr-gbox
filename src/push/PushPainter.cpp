// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushPainter.h"
#include "push/PushLib.h"

#include "logger.h"

#include <cmath>

namespace PushLib {

bool outOfBounds(int &val, int min, int max) {
    if(val < min || val > max) return true;
    return false;
}

Painter::Painter() {
    _displayCanvas = std::unique_ptr<Pixel>(new Pixel[DISPLAY_WIDTH*DISPLAY_HEIGHT]);
}

Painter::~Painter() {

}

void Painter::clear() {
    fill(COLORS::Black);
}

void Painter::drawPixel(int x, int y, Color color) {
    if(x < 0 || x > DISPLAY_WIDTH) return;
    if(y < 0 || y > DISPLAY_HEIGHT) return;

    _displayCanvas.get()[(y*DISPLAY_WIDTH)+x] = color;
}

void Painter::fill(Color color) {
    Pixel * ptr = _displayCanvas.get();
    for(int x=0; x<DISPLAY_WIDTH; ++x) {
        for(int y=0; y<DISPLAY_HEIGHT; ++y) {
            ptr[(y*DISPLAY_WIDTH)+x] = color;
        }
    }
}

void Painter::filledRectangle(int x, int y, int w, int h, Color color) {
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

void Painter::Rectangle(int x, int y, int w, int h, int thick, Color color) {
    if(x < 0 || (x+w) > DISPLAY_WIDTH) return;
    if(y < 0 || (y+h) > DISPLAY_HEIGHT) return;
    if(thick == 0) return;

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
}

void Painter::Line(int x0, int x1, int y0, int y1, Color color) {
    if(x0 < 0 || x0 > DISPLAY_WIDTH) return;
    if(x1 < 0 || x1 > DISPLAY_WIDTH) return;
    if(y0 < 0 || y0 > DISPLAY_HEIGHT) return;
    if(y1 < 0 || y1 > DISPLAY_HEIGHT) return;

    int16_t steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) 
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) 
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int16_t dx, dy;
    dx = x1 - x0;
    dy = std::abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep;
    if (y0 < y1) 
    {
        ystep = 1;
    } 
    else 
    {
        ystep = -1;
    }
    for (; x0 <= x1; x0++) 
    {
        if (steep) {
            drawPixel(y0, x0, color);
        } else {
            drawPixel(x0, y0, color);
        }

        err -= dy;
        
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void Painter::writeChar(int x, int y, char ch, FontDef &font, Color color) {
    int i, b, j;
    if(ch < 32 || ch > 126) return;

    for(i=0; i<font.FontHeight; ++i) {
        b = font.data[(ch-32) * font.FontHeight + i];

        for(j=0; j<font.FontWidth; ++j) {
            if((b << j) & 0x8000) 
                drawPixel(j+x, i+y, color);
        }
    }
}

void Painter::writeString(int x, int y, std::string str, FontDef &font, Color color) {
    writeStringImpl(x, y, str, font, color);
}

void Painter::writeString(int x, int y, std::string &str, FontDef &font, Color color) {
    writeStringImpl(x, y, str, font, color);
}

void Painter::writeString(int x, int y, std::string *str, FontDef &font, Color color) {
    if(!str) return;
    writeStringImpl(x, y, *str, font, color);
}

void Painter::writeStringLimited(int x, int y, int lim, std::string str, FontDef &font, Color color) {
    LOG_WARN("Not implemented yet");
}

void Painter::writeStringLimited(int x, int y, int lim, std::string &str, FontDef &font, Color color) {
    LOG_WARN("Not implemented yet");
}

void Painter::writeStringLimited(int x, int y, int lim, std::string *str, FontDef &font, Color color) {
    LOG_WARN("Not implemented yet");
    /* 
        if(str.length() > lim) {
            writeStringImpl(substr);
            writeStringImpl(ellipsis);
        }
    */
}

const Pixel * Painter::canvas() {
    return _displayCanvas.get();
}


void Painter::writeStringImpl(int x, int y, const std::string &str, FontDef &font, Color color) {
    int tmpx = 0;
    std::size_t len = str.length();

    for(std::size_t i=0; i<len; ++i) {
        char c = str[i];
        writeChar(x+tmpx, y, c, font, color);
        tmpx += font.FontWidth;
    }
}



}