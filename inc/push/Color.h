// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>

namespace PushLib {

using Color = uint16_t;
using Pixel = uint16_t;

enum class COLORS {
    Black   = 0x0000,
    Red     = 0x001F, // 
    Green   = 0x07E0, //
    Blue    = 0xFFE0, //...
    White   = 0xFFFF
};

inline Color rgb(int r, int g, int b) {
    uint16_t tmpr, tmpg, tmpb;
    tmpr = tmpg = tmpb = 0;

    if(r>0) tmpr = 0 + (float)((float)(0x001F-0)/(float)(255-0)) * (r - 0);
    if(g>0) tmpg = 0x001F + (float)((float)(0x07E0 - 0x001F)/(float)(255-0)) * (g - 0);
    if(b>0) tmpb = 0x07E0 + (float)((float)(0xF800 - 0x07E0)/(float)(255-0)) * (b - 0);

    return tmpr | tmpg | tmpb;
}

}