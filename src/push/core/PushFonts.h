// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>

namespace PushLib {

struct Font {
    int FontWidth;
    int FontHeight;
    const uint16_t * data;

    Font & operator=(const Font &other) {
        FontWidth = other.FontWidth;
        FontHeight = other.FontHeight;
        data = other.data;
        return *this;
    }

    bool operator==(const Font &other) {
        return (FontWidth == other.FontWidth && FontHeight == other.FontHeight);
    }

    bool operator!=(const Font &other) {
        return (FontWidth != other.FontWidth || FontHeight != other.FontHeight);
    }
};


extern Font Font_6x8;
extern Font Font_7x10;
extern Font Font_11x18;
extern Font Font_16x26;

}