// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>

namespace PushLib {

struct FontDef {
    const int FontWidth;
    int FontHeight;
    const uint16_t * data;
};


extern FontDef Font_6x8;
extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

}