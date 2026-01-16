// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/BufferManager.h"
#include "logger.h"
#include "slr_config.h"

#include <exception>

namespace slr {

bool BufferManager::init(int blockSize, int channels) {
    _blockSize = blockSize;
    _channels = channels;

    bool res = true;

    try {
        //regular audio
        _regularAudioHolder.reserve(REGULAR_BUFFER_COUNT);
        _regularAudioFree.reserve(REGULAR_BUFFER_COUNT);
        for(std::size_t i=0; i<REGULAR_BUFFER_COUNT; ++i) {
            std::unique_ptr<AudioBuffer> b = std::make_unique<AudioBuffer>(_blockSize, channels);
            _regularAudioFree.push_back(b.get());
            _regularAudioHolder.push_back(std::move(b));
        }
        //record audio
        _recordAudioHolder.reserve(RECORD_BUFFER_COUNT);
        for(std::size_t i=0; i<RECORD_BUFFER_COUNT; ++i) {
            std::unique_ptr<AudioBuffer> b = std::make_unique<AudioBuffer>(_blockSize*RECORD_BUFFER_MUL, _channels);
            _recordAudioQueue.push(b.get());
            _recordAudioHolder.push_back(std::move(b));
        }
        //regular midi
        _regularMidiHolder.reserve(REGULAR_BUFFER_COUNT);
        _regularMidiFree.reserve(REGULAR_BUFFER_COUNT);
        for(std::size_t i=0; i<REGULAR_BUFFER_COUNT; ++i) {
            std::unique_ptr<MidiBuffer> b = std::make_unique<MidiBuffer>();
            _regularMidiFree.push_back(b.get());
            _regularMidiHolder.push_back(std::move(b));
        }
        //record midi
        _recordMidiHolder.reserve(RECORD_BUFFER_COUNT);
        for(std::size_t i=0; i<RECORD_BUFFER_COUNT; ++i) {
            std::unique_ptr<MidiBufferRecord> b = std::make_unique<MidiBufferRecord>();
            _recordMidiQueue.push(b.get());
            _recordMidiHolder.push_back(std::move(b));
        }
    } catch(...) {
        res = false;
    }

    if(!res) {
        LOG_FATAL("Failed to init Buffer Manager");
    } else {
        LOG_INFO("Buffer manager initialized successfully");
    }

    return res;
}

bool BufferManager::shutdown() {

    return true;
}

AudioBuffer* BufferManager::acquireAudioRegular() {
    if(_regularAudioFree.size() == 0) {
        LOG_FATAL("Run out of free Regular Buffers");
    }

    AudioBuffer *buf = _regularAudioFree.back();
    _regularAudioFree.pop_back();
    // _regularAudioFree.erase(_regularAudioFree.end());

    return buf;
}

bool BufferManager::releaseAudioRegular(AudioBuffer *buf) {
    if(_regularAudioFree.size() >= REGULAR_BUFFER_COUNT) {
        LOG_ERROR("Too much free audio buffers");
        return false;
    }

    _regularAudioFree.push_back(buf);
    return true;
}

AudioBuffer* BufferManager::acquireAudioRecord() {
    AudioBuffer * ptr;
    if(!_recordAudioQueue.pop(ptr)) {
        //ideally there should accure no such things...
        LOG_FATAL("No free Audio Buffers available!");
    }

    return ptr;
}

bool BufferManager::releaseAudioRecord(AudioBuffer *buf) {
    return _recordAudioQueue.push(buf);
}

MidiBuffer* BufferManager::acquireMidiRegular() {
    if(_regularMidiFree.size() == 0) {
        LOG_FATAL("Run out of free midi buffers");
    }
    
    MidiBuffer *buf = _regularMidiFree.back();
    _regularMidiFree.pop_back();
    // _regularMidiFree.erase(_regularMidiFree.end());

    return buf;
}

bool BufferManager::releaseMidiRegular(MidiBuffer *buf) {
    if(_regularMidiFree.size() >= REGULAR_BUFFER_COUNT) {
        LOG_ERROR("Too much free midi buffers");
        return false;
    }

    _regularMidiFree.push_back(buf);
}

MidiBufferRecord* BufferManager::acquireMidiRecord() {
    MidiBufferRecord * ptr = nullptr;
    if(!_recordMidiQueue.pop(ptr)) {
        LOG_FATAL("Unable to obtain Midi Recording Buffer");
    }
    return ptr;
}

bool BufferManager::releaseMidiRecord(MidiBufferRecord *buf) {
    return _recordMidiQueue.push(buf);
}

}//namespace slr