// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Color.h"

#include <memory>

namespace PushLib {

struct Painter {
    Painter();
    ~Painter();

    void clear();
    void drawPixel(int x, int y, Color color);
    
    void fill(Color color);
    
    void Rectangle(int x, int y, int w, int h, Color color);
    void EmptyRectangle(int x, int y, int w, int h, int thick, Color color);

    Pixel * canvas();

    private:
    std::unique_ptr<Pixel> _displayCanvas;
};

}