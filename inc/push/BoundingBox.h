// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cmath>

namespace PushLib {

struct BoundingBox {
    BoundingBox() : x(0), y(0), w(0), h(0) {}
    BoundingBox(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
    int x,y,w,h;

    bool intersects(const BoundingBox &b) const { 
        return !(x+w <= b.x || b.x+b.w <= x ||
                y+h <= b.y || b.y+b.h <= y);
    }

    BoundingBox intersect(const BoundingBox &b) const { 
        int nx = std::max(x, b.x);
        int ny = std::max(y, b.y);
        int nx2 = std::min(x+w, b.x+b.w);
        int ny2 = std::min(y+h, b.y+b.h);
        if(nx2 <= nx || ny2 <= ny) return BoundingBox(0,0,0,0);
        return BoundingBox(nx,ny,nx2-nx,ny2-ny);
    }

    BoundingBox unionWith(const BoundingBox &b) const { 
        int nx = std::min(x, b.x);
        int ny = std::min(y, b.y);
        int nx2 = std::max(x+w, b.x+b.w);
        int ny2 = std::max(y+h, b.y+b.h);
        return BoundingBox(nx,ny,nx2-nx,ny2-ny);
    }
    
    bool empty() const { return w <= 0 || h <= 0; }

    bool operator!=(const BoundingBox &other) const {
        return (other.x != x ||
                other.y != y ||
                other.w != w ||
                other.h !=  h);  
    }
};

}