// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <cstdint>

namespace slr {

struct Color {
    int r;
    int g; 
    int b;
    int a;

    Color & operator=(const Color &other) {
        r = other.r;
        g = other.g;
        b = other.b;
        a = other.a;
        return *this;
    }

    uint32_t argb32() const {
        return (
            (a & 0xFF) << 24 | 
            (r & 0xFF) << 16 | 
            (g & 0xFF) << 8  |
            (b & 0xFF)
        );
    }

    uint32_t rgba32() const {
        return (
            (r & 0xFF) << 24 | 
            (g & 0xFF) << 16 | 
            (b & 0xFF) << 8  |
            (a & 0xFF)
        );
    }
};

}