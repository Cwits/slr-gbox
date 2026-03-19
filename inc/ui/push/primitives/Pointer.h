// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"

#include <string>

namespace PushLib {
    class Painter;
}

namespace PushUI {

struct Pointer : public PushLib::Widget {
    Pointer(PushLib::Widget *parent);
    ~Pointer();

    void paint(PushLib::Painter &painter) override;

    PushLib::Font font() const { return _font; }
    void font(PushLib::Font font) { _newFont = font; markDirty(); }
    
    // void position(int x, int y) override;
    // void x(int x) override;
    // void y(int y) override;

    private:
    PushLib::Font _newFont;
    PushLib::Font _font;

    int _newX;
    int _newY;
};

}