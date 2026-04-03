// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"

#include <string>

namespace PushLib {
    class Painter;
}

namespace PushUI {

struct StaticLabel : public PushLib::Widget {
    StaticLabel(PushLib::Widget *parent, const std::string_view &text);
    ~StaticLabel();

    void paint(PushLib::Painter &painter) override;

    PushLib::Font font() const { return _font; }
    void font(PushLib::Font font);

    private:
    PushLib::Font _font;
    const std::string_view &_text;

    void recalculateSize();
};

}