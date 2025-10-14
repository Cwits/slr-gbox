// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioBuffer.h"
#include <cstring>
#include "defines.h"

namespace slr {
/*
AudioBuffer::AudioBuffer(int blockSize, int channels, bool regular) { //if regular == false than it is buffer for record
    _noDelete = false;
    
    _channels = static_cast<uint8_t>(channels);
    _data = new sample_t*[_channels];
    
    if(regular) {
        _size = blockSize;
    } else {
        _size = blockSize*8;
    }

    for(int i=0; i<_channels; ++i) {
        _data[i] = new sample_t[_size];
        std::memset(_data[i], 0.f, sizeof(sample_t) * _size);
    }
}

AudioBuffer::AudioBuffer(sample_t ** ptr, uint8_t channels, frame_t size) {
    _noDelete = true;
    // _data = new sample_t*[channels];
    _channels = channels;
    // for(int i=0; i<_channels; ++i) {
    //     _data[i] = nullptr;
    // }
    _size = size;
    _data = ptr;
}

AudioBuffer::~AudioBuffer() {
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
*/

}