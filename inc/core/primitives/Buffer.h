// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include <cstring>

namespace slr {

class AudioBufferManagerInst;
class JackDriver;
class DummyDriver;
class AudioFile;
class AudioPeaks;

template<typename T>
class Buffer {
    public:
    Buffer(int blockSize, int channels, bool regular = true) {
        _noDelete = false;
        
        _channels = static_cast<uint8_t>(channels);
        _data = new T*[_channels];
        
        if(regular) {
            _size = blockSize;
        } else {
            _size = blockSize;
        }

        for(int i=0; i<_channels; ++i) {
            _data[i] = new T[_size];
            std::memset(_data[i], 0.f, sizeof(T) * _size);
        }
    }

    Buffer(T ** ptr, uint8_t channels, frame_t size) {
        _noDelete = true;
        _channels = channels;
        _size = size;
        _data = ptr;
    }

    ~Buffer() {
        if(!_noDelete) {
            // delete
            for(int i=0; i<_channels; ++i) {
                delete [] _data[i];
                _data[i] = nullptr;
            }
            delete [] _data;
            _data = nullptr;
            _size = 0;
            _channels = 0;
        } else {
            // delete [] _data;
            // _data = nullptr;
            _size = 0;
            _channels = 0;
        }
    }


    Buffer(const Buffer& other) = delete;
    Buffer(Buffer&& other) = delete;
    Buffer& operator=(const Buffer& other) = delete;
    Buffer& operator=(Buffer&& other) = delete;
    
    T * operator[](int ch) { return _data[ch]; }
    const T * operator[](int ch) const { return _data[ch]; }
    const uint8_t channels() const { return _channels; } 
    const frame_t bufferSize() const { return _size; }

    T ** data() { return _data; }
    private:
    bool _noDelete;
    T ** _data;
    uint8_t _channels;
    frame_t _size;

    
    friend class AudioBufferManagerInst;
    //for drivers to has access to _data and _channels
    friend class JackDriver;
    friend class DummyDriver;
    friend class AudioFile;
    friend class AudioPeaks;
    friend class AudioPeakFile;
};

}