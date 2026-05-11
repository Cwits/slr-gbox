// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace PushLib {

struct Vec2 {
    Vec2(int x, int y) : _x(x), _y(y) {}
    int _x;
    int _y;

    int x() const { return _x; }
    int y() const { return _y; }

    Vec2 operator+(const Vec2& rhs) { 
        return Vec2(rhs._x + _x, rhs._y + _y);
    }
};

}