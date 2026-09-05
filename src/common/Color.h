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
};

}