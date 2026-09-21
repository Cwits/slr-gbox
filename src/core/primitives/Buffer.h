// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "common/defines.h"
#include <memory>

namespace slr {

/*
    TODO: BufferView<T> class
    Future improvement:
    move the _accesors to bufferview
*/

template<typename T>
struct Buffer {
    Buffer(size_t size, size_t channels, bool noData = false) {
        _size = size;
        _channels = channels;
        _accesors = std::unique_ptr<T*[]>(new T*[channels]);

        if(!noData) {
            _data = std::unique_ptr<T[]>(new T[channels*size]{});
    
            for(size_t i=0; i<channels; ++i) {
                _accesors.get()[i] = (_data.get() + (i*size));
            }
        }
    }

    T* operator[](size_t channel) {
        return _accesors.get()[channel];
    }

    const T* operator[](size_t channel) const {
        return _accesors.get()[channel];
    }

    T* raw() { return _data.get(); }
    const T* raw() const { return _data.get(); }

    T** rawAccesor() { return _accesors.get(); }
    const T* const* rawAccesor() const { return _accesors.get(); }
    
    size_t size() const { return _size; }
    size_t channels() const { return _channels; }

    Buffer(const Buffer& other) = delete;
    Buffer(Buffer&& other) = delete;
    Buffer& operator=(const Buffer& other) = delete;
    Buffer& operator=(Buffer&& other) = delete;

    void setAccesor(T** ptrs) {
        for(size_t i=0; i<_channels; ++i) {
            _accesors[i] = ptrs[i];
        }
    }

    bool hasData() const { return _data; }

    private:
    size_t _size;
    size_t _channels;
    std::unique_ptr<T*[]> _accesors;
    std::unique_ptr<T[]> _data;
};

}