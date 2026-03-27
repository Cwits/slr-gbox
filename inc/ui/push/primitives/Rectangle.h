// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"

namespace PushLib {
    class Painter;
}

namespace PushUI {

struct Rectangle : public PushLib::Widget {
    Rectangle(PushLib::Widget *parent, bool filled = true);
    ~Rectangle();

    void paint(PushLib::Painter &painter) override;

    private:
    bool _filled;
};

}