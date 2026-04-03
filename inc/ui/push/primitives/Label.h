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

    PushLib::Font font() const { return _font; }
    void font(PushLib::Font font);

    std::string text() const { return _text; }
    void text(std::string text);

    void resizeAllowed(bool allowed) { _prohibitedResize = !allowed; }

    private:
    std::string _text;
    PushLib::Font _font;

    bool _prohibitedResize = false; 
    void recalculateSize();
};

}