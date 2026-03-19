// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"

#include <string>

namespace PushLib {
    class Painter;
}

namespace PushUI {

struct Label : public PushLib::Widget {
    Label(PushLib::Widget *parent, std::string text = "");
    ~Label();

    void paint(PushLib::Painter &painter) override;

    private:
    PushLib::Font _font;
    PushLib::Color _color;
    std::string _text;
};

}