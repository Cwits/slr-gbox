// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"

#include <memory>

namespace PushLib {
    class Painter;
}

namespace PushUI {

struct Canvas : public PushLib::Widget {
    Canvas(PushLib::Widget *parent, int w, int h);
    ~Canvas();

    void paint(PushLib::Painter &painter) override;

    PushLib::Pixel * canvas() { return _canvas.get(); }

    void update();

    private:
    // PushLib::Color _color;

    std::unique_ptr<PushLib::Pixel> _canvas;    

};

}