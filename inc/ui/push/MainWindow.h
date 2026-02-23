// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"

#include <memory>

namespace PushLib {
    class Painter;
}

namespace PushUI {

struct MainWindow : public PushLib::Widget {
    MainWindow();
    ~MainWindow();

    void paint(PushLib::Painter &paint);

    bool handleButton(PushLib::ButtonEvent &ev);
    private:

    int RectX;
    int RectY;
    int RectColor;
};

}