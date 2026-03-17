// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushPainter.h"
#include "push/PushLib.h"

#include "logger.h"

#include <cmath>
#include <cstring>
#include <algorithm>

namespace PushLib {

bool outOfBounds(int &val, int min, int max) {
    if(val < min || val > max) return true;
    return false;
}

inline bool inClip(Painter &p, int x, int y) {
    BoundingBox &c = p.currentClip();
    return (x >= c.x && x < c.x + c.w &&
                y >= c.y && y < c.y + c.h);
}


Painter::Painter() {
    _displayCanvas = std::unique_ptr<Pixel>(new Pixel[DISPLAY_WIDTH*DISPLAY_HEIGHT]);
    _empty = BoundingBox(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
}

Painter::~Painter() {

}

void Painter::pushClip(const BoundingBox &b) {
    if(_clipStack.empty()) {
        _clipStack.push_back(b);
    } else {
        BoundingBox combined = _clipStack.back().intersect(b);
        _clipStack.push_back(combined);
    }
}

void Painter::popClip() {
    if(!_clipStack.empty())
        _clipStack.pop_back();
}

void Painter::clear() {
    fill(COLORS::Black);
}

void Painter::clearRegion(int x, int y, int w, int h) {
    filledRectangle(x, y, w, h, COLORS::Black);
}

void Painter::drawPixel(int x, int y, Color color) {
    if(x < 0 || x > DISPLAY_WIDTH) return;
    if(y < 0 || y > DISPLAY_HEIGHT) return;

    if(!inClip(*this, x,y)) return;
    
    _displayCanvas.get()[(y*DISPLAY_WIDTH)+x] = color;
}

void Painter::fill(Color color) {
    BoundingBox &c = currentClip();
    if(c.empty()) return;

    Pixel* ptr = _displayCanvas.get();
    for(int y = c.y; y < c.y + c.h; ++y) {
        int row = y * DISPLAY_WIDTH;

        for(int x = c.x; x < c.x + c.w; ++x) {
            ptr[row + x] = color;
        }
    }
}

void Painter::filledRectangle(int x, int y, int w, int h, Color color) {
    BoundingBox r(x,y,w,h);
    BoundingBox &c = currentClip();
    BoundingBox vis = r.intersect(c);

    if(vis.empty()) return;

    Pixel *ptr = _displayCanvas.get();

    for(int yd = vis.y; yd < vis.y + vis.h; ++yd) {
        int row = yd * DISPLAY_WIDTH;
        for(int xd = vis.x; xd < vis.x + vis.w; ++xd) {
            ptr[row + xd] = color;
        }
    }
}

void Painter::rectangle(int x, int y, int w, int h, int thick, Color color) {
    if(x < 0 || (x+w) > DISPLAY_WIDTH) return;
    if(y < 0 || (y+h) > DISPLAY_HEIGHT) return;
    if(thick == 0) return;

    filledRectangle(x, y, w, thick, color); //up
    filledRectangle(x, y + h - thick, w, thick, color); //down
    filledRectangle(x, y, thick, h, color); //left
    filledRectangle(x + w - thick, y, thick, h, color); //right
}

void Painter::line(int x0, int x1, int y0, int y1, Color color) {
    if(x0 < 0 || x0 > DISPLAY_WIDTH) return;
    if(x1 < 0 || x1 > DISPLAY_WIDTH) return;
    if(y0 < 0 || y0 > DISPLAY_HEIGHT) return;
    if(y1 < 0 || y1 > DISPLAY_HEIGHT) return;

    int16_t steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int16_t dx, dy;
    dx = x1 - x0;
    dy = std::abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep;
    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    for (; x0 <= x1; x0++) {
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

void Painter::hLine(int y, int x0, int x1, Color color) {
    if(y < 0 || y >= DISPLAY_HEIGHT) return;

    if(x0 > x1) std::swap(x0, x1);

    BoundingBox &c = currentClip();
    x0 = std::max(x0, c.x);
    x1 = std::min(x1, c.x + c.w - 1);

    if(x0 > x1) return;

    Pixel* ptr = _displayCanvas.get() + y * DISPLAY_WIDTH + x0;
    std::fill(ptr, ptr + (x1 - x0 + 1), color);
}

void Painter::vLine(int x, int y0, int y1, Color color) {
    if(x < 0 || x >= DISPLAY_WIDTH) return;

    if(y0 > y1) std::swap(y0, y1);

    BoundingBox &c = currentClip();
    y0 = std::max(y0, c.y);
    y1 = std::min(y1, c.y + c.h - 1);

    if(y0 > y1) return;

    Pixel* ptr = _displayCanvas.get() + x + y0 * DISPLAY_WIDTH;
    for(int y = y0; y <= y1; ++y)
    {
        *ptr = color;
        ptr += DISPLAY_WIDTH;
    }
}

void Painter::circle(int cx, int cy, int r, int thick, Color color) {
    if(r <= 0 || thick <= 0) return;

    BoundingBox &c = currentClip();

    int x = 0;
    int y = r;
    int d = 3 - 2*r;

    auto draw8 = [&](int dx, int dy){
        int px[8] = {cx+dx, cx-dx, cx+dx, cx-dx, cx+dy, cx-dy, cx+dy, cx-dy};
        int py[8] = {cy+dy, cy+dy, cy-dy, cy-dy, cy+dx, cy+dx, cy-dx, cy-dx};
        for(int i=0;i<8;i++){
            if(px[i] >= c.x && px[i] < c.x+c.w && py[i] >= c.y && py[i] < c.y+c.h)
                drawPixel(px[i], py[i], color);
        }
    };

    while(y >= x)
    {
        draw8(x, y);
        x++;
        if(d > 0) { y--; d = d + 4*(x - y) + 10; }
        else d = d + 4*x + 6;
    }

    // TODO: repeat for thickness > 1
}

void Painter::filledCircle(int cx, int cy, int r, Color color) {
    if(r <= 0) return;

    BoundingBox &c = currentClip();

    for(int y = -r; y <= r; ++y)
    {
        int dx = (int)std::sqrt(r*r - y*y);

        int x0 = cx - dx;
        int x1 = cx + dx;
        int yy = cy + y;

        // clip
        if(yy < c.y || yy >= c.y + c.h) continue;
        if(x1 < c.x || x0 >= c.x + c.w) continue;

        x0 = std::max(x0, c.x);
        x1 = std::min(x1, c.x + c.w - 1);

        Pixel* ptr = _displayCanvas.get() + yy * DISPLAY_WIDTH + x0;
        std::fill(ptr, ptr + (x1 - x0 + 1), color);
    }
}

void Painter::drawBuffer(BoundingBox size, Pixel *buf) {
    if(!buf) return;

    BoundingBox screen(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT); //protect from going outside the screen
    BoundingBox &c = currentClip();
    BoundingBox target(size.x, size.y, size.w, size.h);
    BoundingBox vis = target.intersect(c).intersect(screen);

    if(vis.w <= 0 || vis.h <= 0) return;

    Pixel* canvas = _displayCanvas.get();

    int srcOffsetX = vis.x - target.x;
    int srcOffsetY = vis.y - target.y;

    for(int y=0; y<vis.h; ++y) {
        int dstY = vis.y + y;
        int dstIndex = dstY * DISPLAY_WIDTH + vis.x;

        int srcY = srcOffsetY + y;
        int srcIndex = srcY * size.w + srcOffsetX;

        std::memcpy(
            &canvas[dstIndex],
            &buf[srcIndex],
            sizeof(Pixel) * vis.w
        );
    }
}

void Painter::writeChar(int x, int y, char ch, Font &font, Color color) {
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

void Painter::writeString(int x, int y, std::string str, Font &font, Color color) {
    writeStringImpl(x, y, str, font, color, false);
}

void Painter::writeString(int x, int y, std::string &str, Font &font, Color color) {
    writeStringImpl(x, y, str, font, color, false);
}

void Painter::writeString(int x, int y, std::string *str, Font &font, Color color) {
    if(!str) return;
    writeStringImpl(x, y, *str, font, color, false);
}

void Painter::writeString(int x, int y, const std::string_view *str, Font &font, Color color) {
    if(!str) return;
    writeStringViewImpl(x, y, *str, font, color, false);
}

void Painter::writeString(int x, int y, const std::string_view &str, Font &font, Color color) {
    writeStringViewImpl(x, y, str, font, color, false);
}

void Painter::writeStringLimited(int x, int y, int limPixel, std::string str, Font &font, Color color) {
    //some limiting logic
    bool epsilon = false;
    writeStringImpl(x, y, str, font, color, epsilon);
}

void Painter::writeStringLimited(int x, int y, int limPixel, std::string &str, Font &font, Color color) {
    //some limiting logic
    bool epsilon = false;
    writeStringImpl(x, y, str, font, color, epsilon);
}

void Painter::writeStringLimited(int x, int y, int limPixel, std::string *str, Font &font, Color color) {
    if(!str) return;
    //some limiting logic
    bool epsilon = false;
    writeStringImpl(x, y, *str, font, color, epsilon);
}

void Painter::writeStringLimited(int x, int y, int limPixel, const std::string_view *str, Font &font, Color color) {
    if(!str) return;
    //some limiting logic
    bool epsilon = false;
    writeStringViewImpl(x, y, *str, font, color, epsilon);
}

void Painter::writeStringLimited(int x, int y, int limPixel, const std::string_view &str, Font &font, Color color) {
    //some limiting logic
    bool epsilon = false;
    writeStringViewImpl(x, y, str, font, color, epsilon);
}

const Pixel * Painter::canvas() {
    return _displayCanvas.get();
}

void Painter::writeStringImpl(int x, int y, const std::string &str, Font &font, Color &color, bool epsilon) {
    int tmpx = 0;
    std::size_t len = str.length();

    for(std::size_t i=0; i<len; ++i) {
        char c = str[i];
        writeChar(x+tmpx, y, c, font, color);
        tmpx += font.FontWidth;
    }

    if(epsilon) {
        //draw "..."
    }
}

void Painter::writeStringViewImpl(int x, int y, const std::string_view &str, Font &font, Color &color, bool epsilon) {
    int tmpx = 0;
    std::size_t len = str.length();

    for(std::size_t i=0; i<len; ++i) {
        char c = str[i];
        writeChar(x+tmpx, y, c, font, color);
        tmpx += font.FontWidth;
    }

    if(epsilon) {
        //draw "..."
    }
}




}