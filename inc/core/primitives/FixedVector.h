// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <cstring>
#include <cassert>

namespace slr {

template<typename T, std::size_t Capacity>
struct FixedVector {
    static_assert(std::is_default_constructible_v<T>);

    FixedVector() : _size(0) {}

    bool push_back(const T& item) {
        if(_size >= Capacity) return false;

        _data[_size] = item;
        _size++;
        return true;
    }

    bool push_back(T&& item) {
        if (_size >= Capacity) return false;
        _data[_size] = std::move(item);
        _size++;
        return true;
    }

    void clear() {
        // std::fill(_data.begin(), _data.begin() + _size, T{});
        _size = 0;
    }

    std::size_t size() const { return _size; }

    T& operator[](std::size_t idx) {
        assert(idx < _size);
        return _data[idx];
    }

    const T& operator[](std::size_t idx) const {
        assert(idx < _size);
        return _data[idx];
    }

    // T& at(std::size_t idx) {
    //     if(idx >= _size) std::assert(false && "Shouldn't be here");

    //     return _data[idx];
    // }

    // const T& at(std::size_t idx) const {
    //     if(idx >= _size) std::assert(false && "Shouldn't be here");

    //     return _data[idx];
    // }

    private:
    std::array<T, Capacity> _data;
    std::size_t _size;
};

}