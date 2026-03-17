// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Color.h"
#include "push/PushFonts.h"
#include "push/PushLib.h"
#include "push/BoundingBox.h"

#include <string>
#include <memory>

namespace PushLib {

struct Painter {
    Painter();
    ~Painter();

    void pushClip(const BoundingBox &b);
    void popClip();

    BoundingBox& currentClip() { 
        if(_clipStack.empty()) return _empty;
        return _clipStack.back();
    }

    void clear();
    void clearRegion(BoundingBox &b) { clearRegion(b.x, b.y, b.w, b.h); }
    void clearRegion(BoundingBox b) { clearRegion(b.x, b.y, b.w, b.h); }
    void clearRegion(int x, int y, int w, int h);
    void drawPixel(int x, int y, Color color);
    
    void fill(Color color);
    
    void filledRectangle(int x, int y, int w, int h, Color color);
    void rectangle(int x, int y, int w, int h, int thick, Color color);

    void line(int x0, int x1, int y0, int y1, Color color);
    void hLine(int y, int x0, int x1, Color color);
    void vLine(int x, int y0, int y1, Color color);

    void circle(int cx, int cy, int r, int thick, Color color);
    void filledCircle(int cx, int cy, int r, Color color);

    void drawBuffer(BoundingBox size, Pixel *buf);

    void writeChar(int x, int y, char ch, Font &font, Color color);
    void writeString(int x, int y, std::string str, Font &font, Color color);
    void writeString(int x, int y, std::string *str, Font &font, Color color);
    void writeString(int x, int y, std::string &str, Font &font, Color color);
    void writeString(int x, int y, const std::string_view *str, Font &font, Color color);
    void writeString(int x, int y, const std::string_view &str, Font &font, Color color);
    
    //limit in symbols???
    void writeStringLimited(int x, int y, int limPixel, std::string str, Font &font, Color color);
    void writeStringLimited(int x, int y, int limPixel, std::string *str, Font &font, Color color);
    void writeStringLimited(int x, int y, int limPixel, std::string &str, Font &font, Color color);
    void writeStringLimited(int x, int y, int limPixel, const std::string_view *str, Font &font, Color color);
    void writeStringLimited(int x, int y, int limPixel, const std::string_view &str, Font &font, Color color);

    const Pixel * canvas();

    private:
    std::unique_ptr<Pixel> _displayCanvas;

    std::vector<BoundingBox> _clipStack;
    BoundingBox _empty;

    void writeStringImpl(int x, int y, const std::string &str, Font &font, Color &color, bool epsilon);
    void writeStringViewImpl(int x, int y, const std::string_view &str, Font &font, Color &color, bool epsilon);
    
};

}