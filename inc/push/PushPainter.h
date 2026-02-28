// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Color.h"
#include "push/PushFonts.h"

#include <memory>

namespace PushLib {

struct Painter {
    Painter();
    ~Painter();

    void clear();
    void drawPixel(int x, int y, Color color);
    
    void fill(Color color);
    
    void filledRectangle(int x, int y, int w, int h, Color color);
    void Rectangle(int x, int y, int w, int h, int thick, Color color);

    void Line(int x0, int x1, int y0, int y1, Color color);

    void writeChar(int x, int y, char ch, FontDef &font, Color color);
    void writeString(int x, int y, std::string str, FontDef &font, Color color);
    void writeString(int x, int y, std::string &str, FontDef &font, Color color);
    void writeString(int x, int y, std::string *str, FontDef &font, Color color);
    
    //limit in symbols???
    void writeStringLimited(int x, int y, int lim, std::string str, FontDef &font, Color color);
    void writeStringLimited(int x, int y, int lim, std::string &str, FontDef &font, Color color);
    void writeStringLimited(int x, int y, int lim, std::string *str, FontDef &font, Color color);

    const Pixel * canvas();

    private:
    std::unique_ptr<Pixel> _displayCanvas;

    void writeStringImpl(int x, int y, const std::string &str, FontDef &font, Color color);
};

}