// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <atomic>
#include "common/defines.h"

namespace slr {

template<typename T>
struct DoubleBuffer {
    DoubleBuffer() {}
    DoubleBuffer(T buffer1, T buffer2) : _buffer1(std::move(buffer1)), _buffer2(std::move(buffer2)) { }
    
    void init(T buffer1, T buffer2) {
        _buffer1 = std::move(buffer1);
        _buffer2 = std::move(buffer2);
    }

    const T readable() const {
        if(_state) return _buffer1;
        else return _buffer2;
    };

    T writable() const {
        if(_state) return _buffer2;
        else return _buffer1;
    }

    void swap() {
        if(_state) _state = false;
        else _state = true;
    }

    private:
    bool _state;
    T _buffer1;
    T _buffer2;
};

}